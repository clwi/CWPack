/*      CWPack/goodies - cwpack_objc.m   */
/*
 The MIT License (MIT)

 Copyright (c) 2017 Claes Wihlborg

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify,
 merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#import "cwpack_objc.h"
#import "cwpack_utils.h"

#define CWP_ITEM_CLASS_NAME     127


id cwObjectFromBuffer (cw_unpack_context* inbuf);


@implementation NSObject (cwPack)

- (void) packIn:(cw_pack_context*) buff
{
    const char *className= object_getClassName(self);
    cw_pack_ext (buff, CWP_ITEM_CLASS_NAME, className, (uint32_t)strlen(className));
}


+ (instancetype) setWithArray:(NSArray*)array {return nil;}


+ (instancetype) unpackFrom:(cw_unpack_context*) buff
{
    id result = cwObjectFromBuffer(buff);

    if (![[result class] isSubclassOfClass:self])
    {
        if ([[result class] isSubclassOfClass:[NSArray class]] && [self isSubclassOfClass:[NSSet class]])
            return [self setWithArray:result];

        if ([[result class] isSubclassOfClass:[NSNull class]])
            return nil;

        else
        {
            buff->return_code = CWP_RC_TYPE_ERROR;  // Unexpected object type
            [NSException raise:@"[NSObject unpackFrom:]" format:@"Class %@ detected when expected class: %@", [result class], self];
        }
    }
    return result;
}


- (id) initFromContext:(cw_unpack_context*) buff
{
    self = [self init]; // satisfy compiler
    buff->return_code = CWP_RC_ILLEGAL_CALL;  // No unpack defined for this object type
    [NSException raise:@"Not defined" format:@"[%@ initFromContext:]", [self class]];
    return nil;
}


@end


@implementation NSNull (cwPack)

- (void) packIn:(cw_pack_context*) buff
{
    cw_pack_nil (buff);
}
@end


@implementation NSString (cwPack)

- (void) packIn:(cw_pack_context*) buff
{
    cw_pack_cstr (buff, self.UTF8String);
}
@end


@implementation NSData (cwPack)

- (void) packIn:(cw_pack_context*) buff
{
    cw_pack_bin(buff, self.bytes, (uint32_t)self.length);
}
@end


@implementation NSNumber (cwPack)

- (void) packIn:(cw_pack_context*) buff
{
    CFNumberType numberType = CFNumberGetType((CFNumberRef)self);
    switch (numberType)
	{
        case kCFNumberSInt8Type:
        case kCFNumberSInt16Type:
        case kCFNumberShortType:
        case kCFNumberSInt32Type:
        case kCFNumberIntType:
        case kCFNumberLongType:
        case kCFNumberCFIndexType:
        case kCFNumberNSIntegerType:
        case kCFNumberSInt64Type:
        case kCFNumberLongLongType:
            cw_pack_signed(buff, self.longLongValue);
            return;

        case kCFNumberFloat32Type:
        case kCFNumberFloatType:
        case kCFNumberCGFloatType:
            cw_pack_float(buff, self.floatValue);
            return;

        case kCFNumberFloat64Type:
        case kCFNumberDoubleType:
            cw_pack_double(buff, self.doubleValue);
            return;

        case kCFNumberCharType:
        {
            int theValue = self.intValue;
            if (theValue == 0)
            {
                cw_pack_boolean(buff, NO);
            }
            if (theValue == 1)
            {
                cw_pack_boolean(buff, YES);
            }
            else
            {
                cw_pack_signed(buff, theValue);
            }
            return;
        }
        default:
            buff->return_code = CWP_RC_ILLEGAL_CALL;  // No pack defined for this object type
            [NSException raise:@"[NSNumber packIn:]" format:@"Cannot recognise type of: %@", self];
    }
}
@end


@implementation NSArray (cwPack)

- (void) packIn:(cw_pack_context*) buff
{
    uint32_t i, cnt = (uint32_t)self.count;
    cw_pack_array_size(buff, cnt);
    for (i = 0; i < cnt; i++)
        [self[i] packIn:buff];
}
@end


@implementation NSSet (cwPack)

- (void) packIn:(cw_pack_context*) buff
{
    uint32_t cnt = (uint32_t)self.count;
    cw_pack_array_size(buff, cnt);           // Hide the set in an array
    for (id object in self)
        [object packIn:buff];
}
@end


@implementation NSDictionary (cwPack)

- (void) packIn:(cw_pack_context*) buff
{
    cw_pack_map_size(buff, (uint32_t)self.count);
    for (id obj in self)
    {
        [obj packIn:buff];
        [self[obj] packIn:buff];
    }
}
@end


@implementation NSDate (cwPack)

- (void) packIn:(cw_pack_context*) buff
{
    NSTimeInterval ti = self.timeIntervalSince1970;
    cw_pack_time_interval (buff, ti);
}


@end




/*******************************   U N P A C K   ******************************/


id cwObjectFromBuffer (cw_unpack_context* inbuf)
{
    cw_unpack_next(inbuf);
    if (inbuf->return_code)
        return nil;

    switch (inbuf->item.type)
    {
        case CWP_ITEM_NIL:
            return [NSNull null];

        case CWP_ITEM_BOOLEAN:
            return [NSNumber numberWithBool:inbuf->item.as.boolean];

        case CWP_ITEM_POSITIVE_INTEGER:
            return [NSNumber numberWithUnsignedLongLong:inbuf->item.as.u64];

        case CWP_ITEM_NEGATIVE_INTEGER:
            return [NSNumber numberWithLongLong:inbuf->item.as.i64];

        case CWP_ITEM_FLOAT:
            return [[NSNumber alloc] initWithFloat:inbuf->item.as.real];

        case CWP_ITEM_DOUBLE:
            return [NSNumber numberWithDouble:inbuf->item.as.long_real];

        case CWP_ITEM_STR:
            return [[NSString alloc] initWithBytes:inbuf->item.as.str.start length:inbuf->item.as.str.length encoding:NSUTF8StringEncoding];

        case CWP_ITEM_BIN:
            return [NSData dataWithBytes:inbuf->item.as.bin.start length:inbuf->item.as.bin.length];

        case CWP_ITEM_ARRAY:
        {
            int i, dim = inbuf->item.as.array.size;
            NSMutableArray *arr = [NSMutableArray arrayWithCapacity:dim];
            for(i = 0; i < dim; i++)
            {
                id item = cwObjectFromBuffer (inbuf);
                if (!inbuf->return_code)
                    [arr addObject:item];
            }
            return arr;
        }

        case CWP_ITEM_MAP:
        {
            int i, dim = inbuf->item.as.map.size;
            NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithCapacity:dim];
            for(i = 0; i < dim; i++)
            {
                id key = cwObjectFromBuffer (inbuf);
                id val = cwObjectFromBuffer (inbuf);
                if (!inbuf->return_code)
                    [dict setValue:val forKey:key];
            }
            return dict;
        }

        case CWP_ITEM_TIMESTAMP:
        {
            return [NSDate dateWithTimeIntervalSince1970:inbuf->item.as.time.tv_sec + inbuf->item.as.time.tv_nsec / 1000000000.0];
        }

        case CWP_ITEM_CLASS_NAME:
        {
            NSString *cName = [[NSString alloc] initWithBytes:inbuf->item.as.ext.start length:inbuf->item.as.ext.length encoding:NSUTF8StringEncoding];
            Class objectClass = NSClassFromString(cName);
            if (objectClass == NULL)
            {
                [NSException raise:@"cwObjectFromBuffer" format:@"Class not defined for class: %@", cName];
            }
            else
                return [[objectClass alloc] initFromContext:inbuf];
        }

        default:
            return nil;
    }
}



