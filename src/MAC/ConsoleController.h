// This file is hereby placed in the Public Domain -- Neil Stevens

#import <Cocoa/Cocoa.h>

@interface ConsoleController : NSWindowController
{
	IBOutlet NSTextView *textView;

	id launchDelegate;
}

- (id)initWithWindow:(id)window;
- (void)dealloc;

- (void)launch:(NSString *)launchPath args:(NSArray *)args delegate:(id)delegate;

- (void)taskComplete:(NSNotification *)notification;
- (void)dataReady:(NSNotification *)notification;

@end
