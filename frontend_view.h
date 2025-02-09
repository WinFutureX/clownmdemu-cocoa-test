#ifndef FRONTEND_VIEW_H
#define FRONTEND_VIEW_H

#define GL_SILENCE_DEPRECATION

#include <Cocoa/Cocoa.h>
#include "frontend.h"

#if FRONTEND_OPENGL == 1
#include <OpenGL/gl.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
@interface view : NSOpenGLView <NSWindowDelegate>
#pragma clang diagnostic pop
#else
@interface view : NSView <NSWindowDelegate>
#endif
{
	frontend * parent;
}
- (id) initWithFrame : (NSRect) frame data : (void *) data;
- (void) keyDown : (NSEvent *) event;
- (void) keyUp : (NSEvent *) event;
@end
#endif
