#include "frontend_bridge.h"
#include "frontend_log.h"
#include "frontend.h"

void frontend_bridge_func(void * data)
{
	frontend * f = (frontend *) data;
	[f bridge];
}
