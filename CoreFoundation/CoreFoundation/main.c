//
//  main.c
//  CoreFoundation
//
//  Created by Sang Chul Choi on 8/2/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>

int main(int argc, const char * argv[])
{

    // insert code here...
    CFStringRef hello = CFSTR("Hello, world.");
    CFShow(hello);
    CFShow(hello);
    
    UInt8 hours = 8;
    float wage = 10.5;
    CFStringRef hello2 = CFStringCreateWithFormat(NULL, NULL, CFSTR("%2$.2f %1$@"), hello, hours * wage);
    CFShow(hello2);
    
    SInt8 a = -3;
    CFNumberRef an = CFNumberCreate(NULL, kCFNumberSInt8Type, &a);
    CFStringRef numberPrint = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@"), an);
    CFShow(numberPrint);
    
    SInt8 b;
    CFNumberGetValue(an, kCFNumberSInt8Type, &b);
    printf("%d\n", b);
    
    CFRelease(an);
    CFRelease(hello2);
    return 0;
}

