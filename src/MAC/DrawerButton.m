// This file is hereby placed in the Public Domain -- Neil Stevens

#import "AGRegex.h"
#import "DrawerButton.h"

@implementation DrawerButton

- (void)drawerDidClose:(NSNotification *)notification
{
	[self updateTitle];
}

- (void)drawerDidOpen:(NSNotification *)notification
{
	[self updateTitle];
}

- (void)updateTitle
{
	int state = [drawer state];
	bool opening = state == NSDrawerOpenState | state == NSDrawerOpeningState;
	NSString *newText = opening ? @"Hide" : @"Show";
	AGRegex *re = [AGRegex regexWithPattern:@"^(Show|Hide)"];
	[self setTitle:[re replaceWithString:newText inString:[self title]]];
}

@end
