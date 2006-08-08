// This file is hereby placed in the Public Domain -- Neil Stevens

#import "ConsoleController.h"
#import "LauncherApp.h"

@implementation ConsoleController

- (id)initWithWindow:(id)window
{
	[super initWithWindow:window];
	launchDelegate = nil;
}

- (void)dealloc
{
	[super dealloc];
}

- (void)launch:(NSString *)path args:(NSArray *)args delegate:(id)delegate
{
	launchDelegate = delegate;
	[textView clear];

	NSTask *task = [[NSTask alloc] init];
	[task retain];
	[task setLaunchPath:path];
	[task setArguments:args];
	NSPipe *standardOutput = [[NSPipe alloc] init];
	[standardOutput retain];
	[task setStandardOutput:standardOutput];

	[[NSNotificationCenter defaultCenter] addObserver:self
	 selector:@selector(dataReady:)
	 name:NSFileHandleReadCompletionNotification
	 object:[standardOutput fileHandleForReading]];

	[[NSNotificationCenter defaultCenter]
	 addObserver:self selector:@selector(taskComplete:)
	 name:NSTaskDidTerminateNotification object:nil];

	[task launch];
	[[standardOutput fileHandleForReading] readInBackgroundAndNotify];
}

- (void)dataReady:(NSNotification *)notification
{
	NSData *data = [[notification userInfo]
	               objectForKey:NSFileHandleNotificationDataItem];
	NSFileHandle *handle = [notification object];
	if([data length])
	{
		NSString *string = [[NSString alloc] initWithData:data
		                    encoding:NSUTF8StringEncoding];
		[textView appendAnsiString:string];
		[string release];

		[handle readInBackgroundAndNotify];
	}
	else
	{
		[[NSNotificationCenter defaultCenter]
		 removeObserver:self
		 name:NSFileHandleReadCompletionNotification
		 object:[notification object]];
		[[notification object] release];
	}
}

- (void)taskComplete:(NSNotification *)notification
{
	NSTask *task = [notification object];
	if ([task terminationStatus] != 0)
		[self showWindow:nil];
	[[NSNotificationCenter defaultCenter]
	 removeObserver:self
	 name:NSTaskDidTerminateNotification
	 object:[notification object]];
	[task release];
	if(launchDelegate)
		[launchDelegate taskEnded:self];
}

@end
