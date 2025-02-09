#ifndef FRONTEND_H
#define FRONTEND_H
#include <Cocoa/Cocoa.h>
//#include "callbacks.h"
#include "clownmdemu-frontend-common/core/clowncommon/clowncommon.h"
#include "audio.h"
#include "emulator.h"
//#include "input.h"

#define HZ_NS_NTSC 16683350 // 59.94 hz
#define HZ_NS_PAL 20000000 // 50 hz

#define SECOND_NS 1000000000

#define FRONTEND_OPENGL 1
#define FRONTEND_EMULATOR_LOG 1

int argc_copy;
char **argv_copy;

@interface frontend : NSObject <NSApplicationDelegate>
{
	NSWindow *window;
	NSView *view;
	BOOL shutdown;
	BOOL paused;
	BOOL hasCartridge;
	NSData * romFile;
	int romSize;
	uint8_t romBuffer[MAX_FILE_SIZE];
	uint64_t timeDelta;
@public
	emulator * emu;
	BOOL logEnabled;
	NSView *frontendView;
	int currentWidth;
	int currentHeight;
}

- (void) reset : (NSMenuItem *) sender;
- (void) iterate : (id) sender;
- (void) display : (id) sender;
- (void) togglePause : (NSMenuItem *) sender;
- (void) toggleLogEnabled : (NSMenuItem *) sender;
- (void) openFile : (NSMenuItem *) sender;
- (void) loadCartridge : (NSString *) fileName;
@end
#endif
