#include "frontend.h"
#include "frontend_view.h"
#include "frontend_log.h"
#include "emulator.h"
#include <CoreServices/CoreServices.h>
#include <mach/mach_time.h>

@implementation frontend
- (void) applicationWillFinishLaunching : (NSNotification *) notification
{
	emu = emulator_alloc(self);
	if (!emu)
	{
		frontend_err("emulator_alloc() failed\n");
		exit(1);
	}
	frontend_log("frontend init @ %p\n", self);
	shutdown = NO;
	// set up window
	NSRect screenRect = [[NSScreen mainScreen] frame];
	NSRect viewRect = NSMakeRect(0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT);
	NSRect windowRect = NSMakeRect(NSMidX(screenRect) - NSMidX(viewRect), NSMidY(screenRect) - NSMidY(viewRect), viewRect.size.width, viewRect.size.height);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	window = [[NSWindow alloc] initWithContentRect:windowRect styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask) backing:NSBackingStoreBuffered defer:NO];
#pragma clang diagnostic pop
	[window setTitle:@"clownmdemu"];
	[window setBackgroundColor:[NSColor blackColor]];
	view * windowView = [[[view alloc] initWithFrame:windowRect data:self] autorelease];
	frontendView = windowView;
	[window setContentView:windowView];
	[window setDelegate:windowView];
	[window makeKeyAndOrderFront:nil];
	// set up menu
	NSMenu *menubar = [NSMenu new];
	NSMenuItem *menuBarItem = [NSMenuItem new];
	[menubar addItem:menuBarItem];
	[NSApp setMainMenu:menubar];
	NSMenu *mainMenu = [NSMenu new];
	NSMenuItem *openMenuItem = [[NSMenuItem alloc] initWithTitle:@"Open" action:@selector(openFile:) keyEquivalent:@"o"];
	NSMenuItem *logEnableMenuItem = [[NSMenuItem alloc] initWithTitle:@"Enable Emulator Log" action:@selector(toggleLogEnabled:) keyEquivalent:@"l"];
	NSMenuItem *pauseMenuItem = [[NSMenuItem alloc] initWithTitle:@"Pause" action:@selector(togglePause:) keyEquivalent:@"p"];
	NSMenuItem *resetMenuItem = [[NSMenuItem alloc] initWithTitle:@"Reset" action:@selector(reset:) keyEquivalent:@"r"];
	NSMenuItem *quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
	[mainMenu addItem:openMenuItem];
	[mainMenu addItem:logEnableMenuItem];
	[mainMenu addItem:pauseMenuItem];
	[mainMenu addItem:resetMenuItem];
	[mainMenu addItem:quitMenuItem];
	[menuBarItem setSubmenu:mainMenu];
	romFile = [NSData data];
}

- (void) applicationDidFinishLaunching : (NSNotification *) notification
{
	// run loop
	if (argc_copy > 1)
	{
		NSString * fileName = [NSString stringWithUTF8String:argv_copy[1]];
		[self insertCartridge:fileName];
	}
	[NSThread detachNewThreadSelector:@selector(iterate:) toTarget:self withObject:nil];
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed : (NSApplication *) sender
{
	return YES;
}

- (void) applicationWillTerminate : (NSNotification *) notification
{
	shutdown = YES;
	while (shutdown == YES) sleep(0); // wait for iterator thread to stop
	// release objects
	emulator_shutdown(emu);
	frontend_log("frontend stopped\n");
}

- (void) reset : (NSMenuItem *) sender
{
	emulator_soft_reset(emu);
}

// iterator thread
- (void) iterate : (id) sender
{
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	paused = NO;
	uint64_t next_time;
	struct timeval tv = {0, 100};
	for (;;)
	{
		if (shutdown == YES) break;
		if (emu->has_cartridge == cc_true)
		{
			if (paused == NO)
			{
				// timing code shamelessly stolen from reference frontend
				const uint64_t time_temp = mach_absolute_time();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
				const uint64_t current_time = UnsignedWideToUInt64(AbsoluteToNanoseconds(* (AbsoluteTime *) &time_temp));
#pragma clang diagnostic pop
				if (current_time < next_time) continue;
				else
				{
					// if massively delayed, resynchronize to avoid fast forwarding
					if (current_time >= next_time + SECOND_NS / 10) next_time = current_time;
					next_time += timeDelta;
					emulator_update(emu);
					[self performSelectorOnMainThread:@selector(display:) withObject:nil waitUntilDone:NO];
				}
			}
			else select(0, NULL, NULL, NULL, &tv); // idle, avoid wasting cpu
		}
		else select(0, NULL, NULL, NULL, &tv); // idle, avoid wasting cpu
	}
	shutdown = NO;
	[pool drain];
}

- (void) display : (id) sender
{
	[frontendView setNeedsDisplay:YES];
}

- (void) togglePause : (NSMenuItem *) sender
{
	paused = (paused == YES ? NO : YES);
	emu->audio_output->paused = paused == YES ? cc_true : cc_false;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	[sender setState:(paused == YES ? NSOnState : NSOffState)];
#pragma clang diagnostic pop
}

- (void) toggleLogEnabled : (NSMenuItem *) sender
{
	emu->log_enabled = emu->log_enabled == cc_true ? cc_false : cc_true;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	[sender setState:emu->log_enabled == YES ? NSOnState : NSOffState];
#pragma clang diagnostic pop
}

- (void) openFile : (NSMenuItem *) sender
{
	BOOL wasPaused = paused;
	if (wasPaused == NO) [self togglePause:nil];
	NSOpenPanel * openDialog = [NSOpenPanel openPanel];
	[openDialog setCanChooseFiles:YES];
	[openDialog setCanChooseDirectories:YES];
	[openDialog setAllowsMultipleSelection:NO];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	if ([openDialog runModalForDirectory:nil file:nil types:nil] == NSOKButton)
#pragma clang diagnostic pop
	{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
		NSArray * files = [openDialog filenames];
#pragma clang diagnostic pop
		NSString * fileName = [files objectAtIndex:0];
		[self insertCartridge:fileName];
	}
	if (wasPaused == NO) [self togglePause:nil];
}

- (void) insertCartridge : (NSString *) fileName
{
	frontend_log("file name: %s\n", [fileName UTF8String]);
	if ([[NSFileManager defaultManager] fileExistsAtPath:fileName isDirectory:NULL] == NO)
	{
		frontend_err("file %s does not exist\n", [fileName UTF8String]);
		return;
	}
	NSDictionary * attributes = [[NSFileManager defaultManager] attributesOfItemAtPath:fileName error:nil];
	if (attributes == nil)
	{
		frontend_err("unable to obtain attributes of file %s\n", [fileName UTF8String]);
		return;
	}
	unsigned long long fileSize = [attributes fileSize];
	frontend_log("file size: %u bytes\n", fileSize);
	if (fileSize < 260) frontend_err("file size is less than 260 bytes\n");
	else if (fileSize <= MAX_FILE_SIZE)
	{
		romFile = [NSData dataWithContentsOfFile:fileName];
		if (!romFile) frontend_err("unable to open file!\n");
		else
		{
			NSUInteger size = [romFile length];
			[romFile getBytes:romBuffer length:size];
			emulator_cartridge_insert(emu, romBuffer, size);
			timeDelta = emu->pal == cc_true ? CLOWNMDEMU_DIVIDE_BY_PAL_FRAMERATE(SECOND_NS) : CLOWNMDEMU_DIVIDE_BY_NTSC_FRAMERATE(SECOND_NS);
		}
	}
	else frontend_err("file size exceeds 8MB\n");
}

- (void) bridge
{
	frontend_log("hello from bridge\n");
}
@end
