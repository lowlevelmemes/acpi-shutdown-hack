#include <stddef.h>
#include <stdint.h>
#include <facp_shutdown_hack.h>

struct facp {
    char     signature[4];
    uint32_t length;
    uint8_t  unneeded1[40 - 8];
    uint32_t dsdt;
    uint8_t  unneeded2[48 - 44];
    uint32_t SMI_CMD;
    uint8_t  ACPI_ENABLE;
    uint8_t  ACPI_DISABLE;
    uint8_t  unneeded3[64 - 54];
    uint32_t PM1a_CNT_BLK;
    uint32_t PM1b_CNT_BLK;
    uint8_t  unneeded4[89 - 72];
    uint8_t  PM1_CNT_LEN;
};

static inline uint8_t parse_integer(uint8_t* s5_addr, uint64_t* value) {
    uint8_t op = *s5_addr++;
    if (op == 0x0) { // ZeroOp
        *value = 0;
        return 1; // 1 Op Byte
    } else if (op == 0x1) { // OneOp
        *value = 1;
        return 1; // 1 Op Byte
    } else if (op == 0xFF) { // OnesOp
        *value = ~0;
        return 1; // 1 Op Byte
    } else if (op == 0xA) { // ByteConst
        *value = s5_addr[0];
        return 2; // 1 Type Byte, 1 Data Byte
    } else if (op == 0xB) { // WordConst
        *value = s5_addr[0] | ((uint16_t)s5_addr[1] << 8);
        return 3; // 1 Type Byte, 3 Data Bytes
    } else if (op == 0xC) { // DWordConst
        *value = s5_addr[0] | ((uint32_t)s5_addr[1] << 8) | ((uint32_t)s5_addr[2] << 16) | ((uint32_t)s5_addr[3] << 24);
        return 5; // 1 Type Byte, 4 Data Bytes
    } else if (op == 0xE) { // QWordConst
        *value = s5_addr[0] | ((uint64_t)s5_addr[1] << 8) | ((uint64_t)s5_addr[2] << 16) | ((uint64_t)s5_addr[3] << 24) \
               | ((uint64_t)s5_addr[4] << 32) | ((uint64_t)s5_addr[5] << 40) | ((uint64_t)s5_addr[6] << 48) | ((uint64_t)s5_addr[7] << 56);
        return 9; // 1 Type Byte, 8 Data Bytes
    } else {
        return 0; // No Integer, so something weird
    }
}

int facp_shutdown_hack(
        uintptr_t direct_map_base,
        void     *(*find_sdt)(const char *signature, size_t index),
        uint8_t   (*inb)(uint16_t port),
        uint16_t  (*inw)(uint16_t port),
        void      (*outb)(uint16_t port, uint8_t value),
        void      (*outw)(uint16_t port, uint16_t value)
    ) {
    struct facp *facp = find_sdt("FACP", 0);

    uint8_t *dsdt_ptr = (uint8_t *)(uintptr_t)facp->dsdt + 36 + direct_map_base;
    size_t   dsdt_len = *((uint32_t *)((uintptr_t)facp->dsdt + 4 + direct_map_base)) - 36;

    uint8_t *s5_addr = 0;
    for (size_t i = 0; i < dsdt_len; i++) {
        if ((dsdt_ptr + i)[0] == '_'
         && (dsdt_ptr + i)[1] == 'S'
         && (dsdt_ptr + i)[2] == '5'
         && (dsdt_ptr + i)[3] == '_') {
            s5_addr = dsdt_ptr + i;
            goto s5_found;
        }
    }
    return -1;

s5_found:
    s5_addr += 4; // Skip last part of NameSeg, the earlier segments of the NameString were already tackled by the search loop
    if (*s5_addr++ != 0x12) // Assert that it is a PackageOp, if its a Method or something there's not much we can do with such a basic parser
        return -1;
    s5_addr += ((*s5_addr & 0xc0) >> 6) + 1; // Skip PkgLength
    if (*s5_addr++ < 2) // Make sure there are at least 2 elements, which we need, normally there are 4
        return -1;

    uint64_t value = 0;
    uint8_t size = parse_integer(s5_addr, &value);
    if (size == 0) // Wasn't able to parse it
        return -1;

    uint16_t SLP_TYPa = value << 10;
    s5_addr += size;


    size = parse_integer(s5_addr, &value);
    if (size == 0) // Wasn't able to parse it
        return -1;

    uint16_t SLP_TYPb = value << 10;
    s5_addr += size;

    outb(facp->SMI_CMD, facp->ACPI_ENABLE);
    for (int i = 0; i < 100; i++)
        inb(0x80);
    
    while (!inw(facp->PM1a_CNT_BLK) & (1 << 0))
        ;

    outw(facp->PM1a_CNT_BLK, SLP_TYPa | (1 << 13));
    if (facp->PM1b_CNT_BLK)
        outw(facp->PM1b_CNT_BLK, SLP_TYPb | (1 << 13));

    for (int i = 0; i < 100; i++)
        inb(0x80);

    return -1;
}
