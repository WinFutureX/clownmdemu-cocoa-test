#ifndef FRONTEND_BRIDGE_H
#define FRONTEND_BRIDGE_H

#include <stdint.h>

void frontend_bridge_cartridge_write(void * data, uint32_t addr, uint8_t val);

#endif
