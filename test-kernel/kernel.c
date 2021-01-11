#include <stdint.h>
#include <stddef.h>
#include <stivale2.h>
#include <acpi.h>
#include <facp_shutdown_hack.h>

static uint8_t stack[4096];

__attribute__((section(".stivale2hdr"), used))
struct stivale2_header stivale_hdr = {
    .entry_point = 0,
    .stack = (uintptr_t)stack + sizeof(stack),
    .flags = 0,
    .tags = 0
};

void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id) {
    struct stivale2_tag *current_tag = (void *)stivale2_struct->tags;
    for (;;) {
        if (current_tag == NULL) {
            return NULL;
        }

        if (current_tag->identifier == id) {
            return current_tag;
        }

        current_tag = (void *)current_tag->next;
    }
}

static uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile (
        "in %0, %1\n\t"
        : "=a"(ret)
        : "d"(port)
        : "memory"
    );
    return ret;
}

static void outw(uint16_t port, uint16_t value) {
    asm volatile (
        "out %0, %1\n\t"
        :
        : "d"(port), "a"(value)
        : "memory"
    );
}

void _start(struct stivale2_struct *stivale2_struct) {
    struct stivale2_struct_tag_rsdp *rsdp_hdr_tag;
    rsdp_hdr_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_RSDP_ID);

    if (rsdp_hdr_tag == NULL) {
        for (;;) {
            asm ("hlt");
        }
    }

    acpi_init((void *)rsdp_hdr_tag->rsdp);

    for (size_t i = 0; i < 10000000; i++)
        inb(0x80);

    facp_shutdown_hack(0, acpi_find_sdt, inb, outw);

    for (;;) {
        asm ("hlt");
    }
}
