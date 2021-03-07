#ifndef _FACP_SHUTDOWN_HACK_H_
#define _FACP_SHUTDOWN_HACK_H_

#include <stddef.h>
#include <stdint.h>

int facp_shutdown_hack(
        uintptr_t direct_map_base,
        void   *(*find_sdt)(const char *signature, size_t index),
        uint8_t (*inb)(uint16_t port),
        uint16_t (*inw)(uint16_t port),
        void    (*outb)(uint16_t port, uint8_t value),
        void    (*outw)(uint16_t port, uint16_t value)
    );

#endif
