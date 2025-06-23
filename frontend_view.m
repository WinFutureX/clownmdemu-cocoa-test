#include "frontend_view.h"
#include "frontend_log.h"

@implementation view
- (id) initWithFrame : (NSRect) frame data : (void *) data;
{
	parent = (frontend *) data;
#ifdef FRONTEND_NO_OPENGL
	self = [super initWithFrame:frame];
#else
	NSOpenGLPixelFormatAttribute attr[] =
	{
		NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFANoRecovery,
		0
	};
	NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attr];
	self = [super initWithFrame:frame pixelFormat:format];
	[[self openGLContext] makeCurrentContext];
#endif
	return self;
}

- (void) drawRect : (NSRect) dirtyRect
{
#ifdef FRONTEND_NO_OPENGL
	// slower quartz 2d implementation
	if (parent->emu.width == 0 || parent->emu.height == 0) return;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	CGContextRef context = [[NSGraphicsContext currentContext] graphicsPort];
#pragma clang diagnostic pop
	CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
	CFDataRef rgbData = CFDataCreate(NULL, (const UInt8 *) parent->emu.display, (VDP_MAX_SCANLINE_WIDTH * VDP_MAX_SCANLINES * 4));
	CGDataProviderRef provider = CGDataProviderCreateWithCFData(rgbData);
	CGImageRef rgbImageRef = CGImageCreate(parent->emu.width, parent->emu.height, 8, 32, (parent->emu.width * 4), colorspace, (kCGImageAlphaFirst | kCGBitmapByteOrderDefault), provider, NULL, false, kCGRenderingIntentDefault);
	CGContextDrawImage(context, dirtyRect, rgbImageRef);
	// release objects, otherwise memory leaks
	CFRelease(rgbData);
	CGColorSpaceRelease(colorspace);
	CGDataProviderRelease(provider);
	CGImageRelease(rgbImageRef);
#else
	[super drawRect:dirtyRect];
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glRasterPos2f(-1, 1);
	if (parent->emu.width != 0 || parent->emu.height != 0)
	{
		glPixelZoom(((float) VDP_MAX_SCANLINE_WIDTH * 2) / (float) parent->emu.width, -((float) VDP_MAX_SCANLINES / (float) parent->emu.height));
		glDrawPixels(parent->emu.width, parent->emu.height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, parent->emu.display);
	}
	[[self openGLContext] flushBuffer];
#endif
}

- (BOOL) acceptsFirstResponder
{
	return YES;
}

- (BOOL) becomeFirstResponder
{
	return YES;
}

- (BOOL) resignFirstResponder
{
	return YES;
}

- (BOOL) canBecomeKeyView
{
	return YES;
}

- (void) keyDown : (NSEvent *) event
{
	if ([event isARepeat] == NO)
	{
		[self handleKeys:event pressed:cc_true];
	}
}

- (void) keyUp : (NSEvent *) event
{
	[self handleKeys:event pressed:cc_false];
}

- (void) handleKeys : (NSEvent *) event pressed : (cc_bool) pressed
{
	switch ([event keyCode])
	{
		case 0: // a
			parent->emu.buttons[0][CLOWNMDEMU_BUTTON_A] = pressed;
			break;
		case 1: // s
			parent->emu.buttons[0][CLOWNMDEMU_BUTTON_B] = pressed;
			break;
		case 2: // d
			parent->emu.buttons[0][CLOWNMDEMU_BUTTON_C] = pressed;
			break;
		case 3: // f
			parent->emu.buttons[0][CLOWNMDEMU_BUTTON_MODE] = pressed;
			break;
		case 12: // q
			parent->emu.buttons[0][CLOWNMDEMU_BUTTON_X] = pressed;
			break;
		case 13: // w
			parent->emu.buttons[0][CLOWNMDEMU_BUTTON_Y] = pressed;
			break;
		case 14: // e
			parent->emu.buttons[0][CLOWNMDEMU_BUTTON_Z]= pressed;
			break;
		case 36: // return
			parent->emu.buttons[0][CLOWNMDEMU_BUTTON_START] = pressed;
			break;
		case 123: // left
			parent->emu.buttons[0][CLOWNMDEMU_BUTTON_LEFT] = pressed;
			break;
		case 124: // right
			parent->emu.buttons[0][CLOWNMDEMU_BUTTON_RIGHT] = pressed;
			break;
		case 125: // down
			parent->emu.buttons[0][CLOWNMDEMU_BUTTON_DOWN] = pressed;
			break;
		case 126: // up
			parent->emu.buttons[0][CLOWNMDEMU_BUTTON_UP] = pressed;
			break;
		default:
			frontend_err("view %s unknown %d\n", pressed == cc_true ? "keyDown" : "keyUp", [event keyCode]);
			if (pressed == cc_true) [super keyDown:event];
			else [super keyUp:event];
			break;
	}
}
@end
