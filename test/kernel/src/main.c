#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <acpi.h>
#include <acpi_shutdown_hack.h>
#include <limine.h>

LIMINE_BASE_REVISION(1)

static uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile (
        "inb %1, %0\n\t"
        : "=a"(ret)
        : "d"(port)
        : "memory"
    );
    return ret;
}

static uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile (
        "inw %1, %0\n\t"
        : "=a"(ret)
        : "d"(port)
        : "memory"
    );
    return ret;
}

static void outb(uint16_t port, uint8_t value) {
    asm volatile (
        "outb %1, %0\n\t"
        :
        : "d"(port), "a"(value)
        : "memory"
    );
}

static void outw(uint16_t port, uint16_t value) {
    asm volatile (
        "outw %1, %0\n\t"
        :
        : "d"(port), "a"(value)
        : "memory"
    );
}

struct limine_hhdm_request hhdm_req = {
    LIMINE_HHDM_REQUEST, 0, NULL
};

void _start(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        for (;;) {
            asm ("hlt");
        }
    }

    acpi_init();

    for (size_t i = 0; i < 10000000; i++)
        inb(0x80);

    acpi_shutdown_hack(hhdm_req.response->offset,
                       acpi_find_sdt, inb, inw, outb, outw);

    for (;;) {
        asm ("hlt");
    }
}
