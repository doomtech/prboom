// This file is hereby placed in the Public Domain -- Neil Stevens

#import <Cocoa/Cocoa.h>
#import "RMUDAnsiTextView.h"

@interface ConsoleController : NSWindowController
{
	IBOutlet RMUDAnsiTextView *textView;

	NSTask *task;
	id launchDelegate;

	NSPipe *standardOutput;
	NSPipe *standardError;
	bool timerLaunched;
}

- (id)initWithWindow:(id)window;
- (void)dealloc;

- (void)launch:(NSString *)launchPath args:(NSArray *)args delegate:(id)delegate;

- (void)readTimer:(NSTimer *)timer;

@end
