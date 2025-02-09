#include <Cocoa/Cocoa.h>
#include "emulator.h"
#include "frontend.h"
#include "frontend_log.h"
//#include "timer.h"

int main(int argc, char **argv)
{
	// TODO: make the frontend handle file opening and region selection!
	if (argc > 2)
	{
		frontend_err("usage: %s [rom_file]\n", argv[0]);
		return 2;
	}
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	argc_copy = argc;
	argv_copy = argv;
	NSApplication *app = [NSApplication sharedApplication];
	frontend *f = [[frontend new] autorelease];
	[app setDelegate:f];
	[app setActivationPolicy:NSApplicationActivationPolicyRegular];
	[app run];
	[pool drain];
	return 0;
}
