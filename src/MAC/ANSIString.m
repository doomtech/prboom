// Copyright (C) 2006 Neil Stevens <neil@hakubi.us>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Except as contained in this notice, the name(s) of the author(s) shall not be
// used in advertising or otherwise to promote the sale, use or other dealings
// in this Software without prior written authorization from the author(s).

#include "ANSIString.h"

@implementation ANSIString

static int findNext(NSString *string, NSString *needle, int start)
{
	if(start >= [string length])
	{
		return NSNotFound;
	}
	else
	{
		int i = [string rangeOfString:needle options:nil
		               range:NSMakeRange(start, [string length] - start)].location;
		return i;
	}
}

+ (NSAttributedString *)parseColorCodes:(NSString *)ansiString
{
	NSMutableAttributedString *retval = [[[NSMutableAttributedString alloc]
	                                    initWithString:@""] autorelease];
	int length = [ansiString length];
	int last = 0;
	int current = -1;
	while((current = findNext(ansiString, @"\e[", current + 1)) != NSNotFound)
	{
		int updateLength = current - last + 1;
		NSAttributedString *update = [[NSAttributedString alloc]
		              initWithString:[ansiString
		              substringWithRange:NSMakeRange(last, updateLength)]];
		[retval appendAttributedString:update];
		[update release];
		last = current;

		int end = findNext(ansiString, @"m", current + 1);
		if(end == NSNotFound)
		{
			current = length;
		}
		else
		{
			bool valid = true;
			int i;
			for(i = current + 2; valid && (i < end); ++i)
			{
				unichar c = [ansiString characterAtIndex:i];
				switch(c)
				{
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case ';':
					break;
				default:
					valid = false;
				}
			}
			if(valid)
			{
				int codeLength = end - current + 1;
				current += codeLength - 1;
				last = current + 1;
			}
		}
	}
	if(last < length)
	{
		NSAttributedString *rest = [[NSAttributedString alloc]
		                            initWithString:[ansiString substringFromIndex:last]];
		[retval appendAttributedString:rest];
		[rest release];
	}

	return retval;
}

@end
