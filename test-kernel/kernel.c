#include <stdint.h>
#include <stddef.h>
#include <acpi.h>
#include <acpi_shutdown_hack.h>
#include <limine.h>

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

static uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile (
        "in %0, %1\n\t"
        : "=a"(ret)
        : "d"(port)
        : "memory"
    );
    return ret;
}

static void outb(uint16_t port, uint8_t value) {
    asm volatile (
        "out %0, %1\n\t"
        :
        : "d"(port), "a"(value)
        : "memory"
    );
}

static void outw(uint16_t port, uint16_t value) {
    asm volatile (
        "out %0, %1\n\t"
        :
        : "d"(port), "a"(value)
        : "memory"
    );
}

static volatile struct limine_hhdm_request hhdm_req = {
    LIMINE_HHDM_REQUEST, 0, NULL
};

void _start(void) {
    acpi_init();

    for (size_t i = 0; i < 10000000; i++)
        inb(0x80);

    acpi_shutdown_hack(hhdm_req.response->offset,
                       acpi_find_sdt, inb, inw, outb, outw);

    for (;;) {
        asm ("hlt");
    }
}
