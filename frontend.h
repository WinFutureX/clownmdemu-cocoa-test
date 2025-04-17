#ifndef FRONTEND_H
#define FRONTEND_H
#include <Cocoa/Cocoa.h>
#include "clownmdemu-frontend-common/core/clowncommon/clowncommon.h"
#include "emulator.h"

#define SECOND_NS 1000000000

int argc_copy;
char **argv_copy;

@interface frontend : NSObject <NSApplicationDelegate>
{
	NSWindow *window;
	NSView *view;
	BOOL shutdown;
	BOOL paused;
	NSData * romFile;
	int romSize;
	uint8_t romBuffer[MAX_FILE_SIZE];
	uint64_t timeDelta;
@public
	emulator emu;
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
- (void) insertCartridge : (NSString *) fileName;
- (void) cartridgeWrite : (uint32_t) addr val : (uint8_t) val;
@end
#endif
