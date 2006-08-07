// This file is hereby placed in the Public Domain -- Neil Stevens

#import "ConsoleController.h"
#import "LauncherApp.h"

@implementation ConsoleController

- (id)initWithWindow:(id)window
{
	[super initWithWindow:window];
	task = nil;
	timerLaunched = false;
}

- (void)dealloc
{
	[super dealloc];
}

- (void)launch:(NSString *)path args:(NSArray *)args delegate:(id)delegate
{
	launchDelegate = delegate;

	if(!timerLaunched)
	{
		timerLaunched = true;
		// Check if the task printed any output
		// And also check its status
		[NSTimer scheduledTimerWithTimeInterval:1.0 target:self
		         selector:@selector(readTimer:) userInfo:nil repeats:true];
	}

	// Execute
	standardOutput = [[NSPipe alloc] init];
	standardError = [[NSPipe alloc] init];
	[standardOutput retain];
	[standardError retain];
	[textView clear];

	task = [[NSTask alloc] init];
	[task retain];
	[task setLaunchPath:path];
	[task setArguments:args];
	[task setStandardOutput:standardOutput];
	[task setStandardError:standardError];

	[task launch];
}

static void *buffer = 0;
static NSString *readPipe(NSPipe *pipe)
{
	// NSFileHandle doesn't do nonblocking IO, so we'll do it ourselves
	int fd = [[pipe fileHandleForReading] fileDescriptor];
	int flags = fcntl(fd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);

	if(!buffer)
		buffer = malloc(1000000);
	int size = read(fd, buffer, 1000000);

	if(size > 0)
	{
		NSData *data = [NSData dataWithBytesNoCopy:buffer length:size
		                freeWhenDone:false];

		// Stick the data into the console text view
		NSString *string = [[NSString alloc] initWithData:data
		                    encoding:NSUTF8StringEncoding];
		[string retain];
		return string;
	}
	return @"";
}

- (void)readTimer:(NSTimer *)timer
{
	if(task == nil)
		return;

	NSString *stdoutString = readPipe(standardOutput);
	[textView appendAnsiString:stdoutString];
	[stdoutString release];

	NSString *stderrString = readPipe(standardError);
	// Ignore for now
	[stderrString release];

	if(![task isRunning])
	{
		if ([task terminationStatus] != 0)
			[self showWindow:nil];
		[task release];
		task = nil;
		[standardError release];
		[standardOutput release];
		if(launchDelegate)
			[launchDelegate taskEnded:self];
	}
}

@end
