/* RMUDAnsiTextView */

#import <Cocoa/Cocoa.h>
#import <AGRegex/AGRegex.h>

@interface RMUDAnsiTextView : NSTextView
{
  NSDictionary *ansiColorTable;
  AGRegex *ansiSequenceRegex;
  int currentForegroundColorIndex;
  int currentBackgroundColorIndex;
}
- (NSColor *)currentForegroundColor;
- (NSColor *)currentBackgroundColor;
- (void)setAnsiString:(NSString *)ansiString;
- (void)appendAnsiString:(NSString *)ansiString;
- (void)handleAnsiSequence:(NSString *)ansiSequence;
@end
