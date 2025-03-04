#include "frontend_bridge.h"
#include "frontend_log.h"
#include "frontend.h"

void frontend_bridge_cartridge_write(void * data, uint32_t addr, uint8_t val)
{
	frontend * f = (frontend *) data;
	[f cartridgeWrite:addr val:val];
}
