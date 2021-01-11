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

int facp_shutdown_hack(
        uintptr_t direct_map_base,
        void   *(*find_sdt)(const char *signature, size_t index),
        uint8_t (*inb)(uint16_t port),
        void    (*outw)(uint16_t port, uint16_t value)
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
    s5_addr += 5;
    s5_addr += ((*s5_addr & 0xc0) >> 6) + 2;

    if (*s5_addr == 0x0a)
        s5_addr++;
    uint16_t SLP_TYPa = (uint16_t)(*s5_addr) << 10;
    s5_addr++;

    if (*s5_addr == 0x0a)
        s5_addr++;
    uint16_t SLP_TYPb = (uint16_t)(*s5_addr) << 10;
    s5_addr++;

    outw(facp->PM1a_CNT_BLK, SLP_TYPa | (1 << 13));
    if (facp->PM1b_CNT_BLK)
        outw(facp->PM1b_CNT_BLK, SLP_TYPb | (1 << 13));

    for (int i = 0; i < 100; i++)
        inb(0x80);

    return -1;
}
