
/*      CWPack/goodies/ObjC - CWPackContext.m   */

/*
 The MIT License (MIT)

 Copyright (c) 2021 Claes Wihlborg

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


#include "cwpack_internals.h"
#include "numeric_extensions.h"
#include "cwpack_utils.h"

#import "CWPackContext.h"


#define CWP_ITEM_OBJECT_MARKER              CWP_ITEM_USER_EXT_127




@protocol CWPackPackerClasses <NSObject>
@required
- (void) cwPackPackerTo:(CWPackContext*)ctx;
@end


@interface CWUnpackContext ()
- (id) cwUnpackNextObjectSub:(long)label;
@end


@interface NSObject (CWPackable) <CWPackable>
- (void) cwPackTo:(CWPackContext*)ctx;
@end




#pragma mark -
#pragma mark ************************************** CWPackContext


@implementation CWPackContext
{
    CFMutableDictionaryRef  _labelMap;
    long                    userLabelGenerator;
    long                    packerLabelGenerator;
}

- (instancetype)initWithContext:(cw_pack_context*)context
{
    self = [super init];
    if (self) {
        _context = context;
        _labelMap = CFDictionaryCreateMutable(nil, 0, nil, nil);
    }
    return self;
}


+ (instancetype) newWithContext:(cw_pack_context*)context {
    return [self.alloc initWithContext:context];
}


- (long) labelForObject:(id)object {
    return (long)CFDictionaryGetValue (_labelMap, (const void *)object);
}


- (long) assignLabelForObject:(id)object isMsgpackObject:(BOOL)isMsgpackObject
{
    long label = isMsgpackObject ? --packerLabelGenerator : ++userLabelGenerator;
    CFDictionarySetValue (_labelMap, (const void *)object,(const void *)label);
    return label;
}


- (void) packUserObject:(id)object
{
    [object cwPackTo:self];
}


- (void) packObject:(nullable NSObject*)object
{
    if (!object)
    {
        cw_pack_nil(_context);
        return;
    }

    BOOL isMsgpackObject = [object conformsToProtocol:@protocol(CWPackPackerClasses)];

    if (_useLabels)
    {
        long label = [self labelForObject:object];
        if (label) // object already packed, just pack a reference.
        {
            cw_pack_array_size(_context,1);
            cw_pack_ext_integer(_context, CWP_ITEM_OBJECT_MARKER, label);
            return;
        }

        cw_pack_array_size(_context, 2 + object.persistentAttributeCount);
        label = [self assignLabelForObject:object isMsgpackObject:isMsgpackObject];
        cw_pack_ext_integer (_context, CWP_ITEM_OBJECT_MARKER, label);
        if (isMsgpackObject) {
            [(NSObject<CWPackPackerClasses>*)object cwPackPackerTo:self];
        }
        else {
            [self packUserObject:object];
        }
    }
    else if (isMsgpackObject)
    {
        [(NSObject<CWPackPackerClasses>*)object cwPackPackerTo:self];
    }
    else
    {
        cw_pack_array_size(_context, 2 + object.persistentAttributeCount);
        cw_pack_ext_integer (_context, CWP_ITEM_OBJECT_MARKER, 0);
        [self packUserObject:object];
    }
}

@end



#pragma mark -
#pragma mark ************************************** CWUnpackContext



@implementation CWUnpackContext
{
    NSMutableArray  *_userReferences;
    NSMutableArray  *_packerReferences;
}

- (instancetype)initWithContext:(cw_unpack_context*)context
{
    self = [super init];
    if (self) {
        _context = context;
        _userReferences = [NSMutableArray arrayWithObject:NSNull.null]; //no reference has value 0
        _packerReferences = [NSMutableArray arrayWithObject:NSNull.null]; //no reference has value 0
    }
    return self;
}


+ (instancetype) newWithContext:(cw_unpack_context*)context
{
    return [self.alloc initWithContext:context];
}


- (id) cwUnpackObject:(long)label
{
    id      object;

    switch (_context->item.type)
    {
        case CWP_ITEM_NIL:
            object = [NSNull null];
            break;

        case CWP_ITEM_BOOLEAN:
            object = [NSNumber numberWithBool:_context->item.as.boolean];
            break;

        case CWP_ITEM_POSITIVE_INTEGER:
            object = [NSNumber numberWithUnsignedLongLong:_context->item.as.u64];
            break;

        case CWP_ITEM_NEGATIVE_INTEGER:
            object = [NSNumber numberWithLongLong:_context->item.as.i64];
            break;

        case CWP_ITEM_FLOAT:
            object = [[NSNumber alloc] initWithFloat:_context->item.as.real];
            break;

        case CWP_ITEM_DOUBLE:
            object = [NSNumber numberWithDouble:_context->item.as.long_real];
            break;

        case CWP_ITEM_STR:
        {
            object = [NSString.alloc initWithBytes:_context->item.as.str.start length:_context->item.as.str.length encoding:NSUTF8StringEncoding];
            if (!object) {
                _context->return_code = CWP_RC_VALUE_ERROR;
                return nil;
            }
            break;
        }

        case CWP_ITEM_BIN:
        {
            object = [NSData dataWithBytes:_context->item.as.bin.start length:_context->item.as.bin.length];
            break;
        }

        case CWP_ITEM_TIMESTAMP:
            object = [NSDate dateWithTimeIntervalSince1970:_context->item.as.time.tv_sec + _context->item.as.time.tv_nsec / 1000000000.0];
            break;

        case CWP_ITEM_MAP:
        {
            int i, dim = _context->item.as.map.size;
            NSMutableDictionary *dict = NSMutableDictionary.new;
            if (label) [_packerReferences addObject:dict];
            for(i = 0; i < dim; i++)
            {
                id key = [self cwUnpackNextObjectSub:0];
                id val = [self cwUnpackNextObjectSub:0];
                if (_context->return_code == CWP_RC_OK)
                    dict[key] = val;
            }
            return dict;
        }

        case CWP_ITEM_ARRAY:
        {
            int dim = _context->item.as.array.size;
            if (dim == 0)
            {
                object = NSMutableArray.new;
                break;
            }
            else
            {
                cw_unpack_next(_context);
                if (_context->item.type == CWP_ITEM_OBJECT_MARKER)     //object
                {
                    long idx = get_ext_integer(_context);
                    if (dim == 1)   // object reference
                    {
                        if ((idx<0)? (-idx >= _packerReferences.count) : (idx >= _userReferences.count))
                        {
                            _context->return_code = CWP_RC_VALUE_ERROR;
                            return nil;
                        }
                        return idx>0 ? _userReferences[idx] : _packerReferences[-idx];
                    }
                    if (idx < 0)  // we had a labeled MessagePack object
                    {
                        if (dim != 2)
                        {
                            _context->return_code = CWP_RC_VALUE_ERROR;
                            return nil;
                        }
                        return [self cwUnpackNextObjectSub:idx];
                    }

                    NSString *objClassName = [self cwUnpackNextObjectSub:0];
                    Class objClass = NSClassFromString(objClassName);

                    if (!objClass)
                        object = [CWPackGenericClass newWithClassName:objClassName];
                    else
                        object = [objClass alloc];
                    object = [object cwUnpackSubInit:self remainingAttributes:dim - 2];
                    if (idx == _userReferences.count)
                    {
                        [_userReferences addObject:object];
                    }
                    else
                    {
                        if (idx != 0)
                        {
                            _context->return_code = CWP_RC_VALUE_ERROR;
                            return nil;
                        }
                    }
                    [object cwUnpackSub:self remainingAttributes:dim - 2];
                    return object;
                }
                else  // standard MsgPack array
                {
                    int i;
                    NSMutableArray *arr = NSMutableArray.new;
                    if (label) [_packerReferences addObject:arr];
                    id item = [self cwUnpackObject:0];
                    if (_context->return_code == CWP_RC_OK)
                        [arr addObject:item];
                    for(i = 1; i < dim; i++)
                    {
                        id item = [self cwUnpackNextObjectSub:0];
                        if (_context->return_code == CWP_RC_OK)
                            [arr addObject:item];
                    }
                    return arr;
                }
            }
        }

        default:
            object = [CWPackExternalItem itemWithType:_context->item.type
                                                 data:[NSData dataWithBytes:_context->item.as.ext.start length:_context->item.as.ext.length]];
    }
    if (label)
    {
        if (label != -_packerReferences.count)
            _context->return_code = CWP_RC_MALFORMED_INPUT;
        else
            [_packerReferences addObject:object];
    }
    return object;
}


- (id) cwUnpackNextObjectSub:(long)label
{
    cw_unpack_next(_context);
    if (_context->return_code)
        return nil;
    return [self cwUnpackObject:label];
}


- (id) unpackNextObject
{
    id result = [self cwUnpackNextObjectSub:0];
    if (result == [NSNull null])
        return nil;
    return result;
}

@end



#pragma mark ************************************** NSObject <CWPackable>



@implementation NSObject (CWPackable)

- (int) persistentAttributeCount {return 0;}

- (NSString*) packerClassName {return self.className;}

- (void) cwPackTo:(CWPackContext*)ctx
{
    cw_pack_cstr(ctx.context, self.packerClassName.UTF8String);
    [self cwPackSub:ctx];
}

- (void) cwPackSub:(CWPackContext*)ctx;
{
    [NSException raise:@"Object Not Packable" format:@"Class \"%s\" does not conform to protocol <CWPackable>\n", object_getClassName(self)];
}

- (void) cwUnpackSub:(CWUnpackContext*)ctx remainingAttributes:(int)remainingAttributes
{
    [NSException raise:@"Object Not Unpackable" format:@"Class \"%s\" does not conform to protocol <CWPackable>\n", object_getClassName(self)];
}

- (instancetype) cwUnpackSubInit:(CWUnpackContext*)ctx remainingAttributes:(int)remainingAttributes
{
    return self.init;
}

@end


#pragma mark ************************************** NSNull <CWPackPackerClasses>

@interface NSNull (CWPackable) <CWPackPackerClasses>
@end


@implementation NSNull (CWPackable)

- (void) cwPackPackerTo:(CWPackContext*)ctx
{
    cw_pack_nil (ctx.context);
}
@end



#pragma mark ************************************** NSString, NSMutableString <CWPackPackerClasses>


@interface NSString (CWPackable) <CWPackPackerClasses>
@end


@implementation NSString (CWPackable)

- (void) cwPackPackerTo:(CWPackContext*)ctx
{
    cw_pack_cstr (ctx.context, self.UTF8String);
}
@end


#pragma mark ************************************** NSData <CWPackPackerClasses>

@interface NSData (CWPackable) <CWPackPackerClasses>
@end

@implementation NSData (CWPackable)

- (void) cwPackPackerTo:(CWPackContext*)ctx
{
    cw_pack_bin(ctx.context, self.bytes, (uint32_t)self.length);
}
@end



#pragma mark ************************************** NSDate <CWPackPackerClasses>

@interface NSDate (CWPackable) <CWPackPackerClasses>
@end

@implementation NSDate (CWPackable)

- (void) cwPackPackerTo:(CWPackContext*)ctx
{
    NSTimeInterval ti = self.timeIntervalSince1970;
    cw_pack_time_interval (ctx.context, ti);
}
@end


#pragma mark ************************************** NSNumber <CWPackPackerClasses>

@interface NSNumber (CWPackable) <CWPackPackerClasses>
@end

@implementation NSNumber (CWPackable)

- (void) cwPackPackerTo:(CWPackContext*)ctx
{
    CFNumberType numberType = CFNumberGetType((CFNumberRef)self);
    switch (numberType)
    {
        case kCFNumberSInt8Type:
        case kCFNumberSInt16Type:
        case kCFNumberSInt32Type:
        case kCFNumberSInt64Type:
        case kCFNumberShortType:
        case kCFNumberIntType:
        case kCFNumberLongType:
        case kCFNumberCFIndexType:
        case kCFNumberNSIntegerType:
        case kCFNumberLongLongType:
            cw_pack_signed(ctx.context, self.longLongValue);
            return;

        case kCFNumberFloat32Type:
        case kCFNumberFloatType:
        case kCFNumberCGFloatType:
            cw_pack_float(ctx.context, self.floatValue);
            return;

        case kCFNumberFloat64Type:
        case kCFNumberDoubleType:
            cw_pack_double(ctx.context, self.doubleValue);
            return;

        case kCFNumberCharType:
        {
            int theValue = self.intValue;
            if (theValue == 0)
            {
                cw_pack_boolean(ctx.context, NO);
            }
            if (theValue == 1)
            {
                cw_pack_boolean(ctx.context, YES);
            }
            else
            {
                cw_pack_signed(ctx.context, theValue);
            }
            return;
        }
        default:
            ctx.context->return_code = CWP_RC_ILLEGAL_CALL;  // No pack defined for this object type
            [NSException raise:@"[NSNumber packIn:]" format:@"Cannot recognise type (%ld) of: %@", (long)numberType, self];
    }
}
@end


#pragma mark ************************************** NSArray <CWPackPackerClasses>

@interface NSArray (CWPackable) <CWPackPackerClasses>
@end

@implementation NSArray (CWPackable)

- (void) cwPackPackerTo:(CWPackContext*)ctx
{
    uint32_t i, cnt = (uint32_t)self.count;
    cw_pack_array_size(ctx.context, cnt);
    for (i = 0; i < cnt; i++)
        [ctx packObject:self[i]];
}
@end


#pragma mark ************************************** NSDictionary <CWPackPackerClasses>


@interface NSDictionary (CWPackable) <CWPackPackerClasses>
@end


@implementation NSDictionary (CWPackable)

- (void) cwPackPackerTo:(CWPackContext*)ctx
{
    cw_pack_map_size(ctx.context, (uint32_t)self.count);
    for (id obj in self)
    {
        [ctx packObject:obj];
        [ctx packObject:self[obj]];
    }
}
@end


#pragma mark ************************************** CWPackExternalItem <CWPackPackerClasses>


@interface CWPackExternalItem (CWPackable) <CWPackPackerClasses>
@end


@implementation CWPackExternalItem

- (instancetype) initWithType:(int)type data:(NSData*)data
{
    self = [super init];
    if (self) {
        _type = type;
        _data = data;
    }
    return self;
}


+ (instancetype) itemWithType:(int)type data:(NSData*)data;
{
    return [[self alloc] initWithType:type data:data];
}


- (void) cwPackPackerTo:(CWPackContext*)ctx
{
    cw_pack_ext(ctx.context, _type, _data.bytes, (int)_data.length);
}

@end


#pragma mark -
#pragma mark ************************************** CWPackGenericClass


@implementation CWPackGenericClass

- (instancetype)initWithClassName:(NSString*)className
{
    self = [self init];
    if (self) {
        _packerClassName = className;
    }
    return self;
}

+ (instancetype) newWithClassName:(NSString*)className    {return [self.alloc initWithClassName:className];}

- (int) persistentAttributeCount    {return (int)_attributes.count;}

- (void) cwPackSub:(CWPackContext*)ctx
{
    for (id<CWPackable> o in _attributes)   {[ctx packObject:o];}
}

- (void) cwUnpackSub:(CWUnpackContext*)ctx remainingAttributes:(int)remainingAttributes
{
    _attributes = NSMutableArray.new;
    int i;
    for (i = 0; i < remainingAttributes; i++) {
        id item = [ctx cwUnpackNextObjectSub:0];
        if (ctx.context->return_code == CWP_RC_OK)
            [_attributes addObject:item];
    }
}

@end


#pragma mark ************************************** NSSet <CWPackable>

@interface NSSet (CWPackable)
@end

@implementation NSSet (CWPackable)
- (int) persistentAttributeCount {return (int)self.count + super.persistentAttributeCount;}

- (NSString*) packerClassName {return @"NSSet";}

- (void) cwPackSub:(CWPackContext*)ctx
{
    for (id object in self)
        [ctx packObject:object];
}

- (instancetype) cwUnpackSubInit:(CWUnpackContext*)ctx remainingAttributes:(int)remainingAttributes
{
    return NSMutableSet.new;
}

@end


@interface NSMutableSet (CWPackable)
@end

@implementation NSMutableSet (CWPackable)
- (instancetype) cwUnpackSub:(CWUnpackContext*)ctx remainingAttributes:(int)remainingAttributes
{
    int i;
    for (i=0; i<remainingAttributes; i++)
        [self addObject:[ctx unpackNextObject]];
    return self;
}
@end


#pragma mark ************************************** NSCountableSet <CWPackPackerClasses>

@interface NSCountedSet (CWPackable)
@end

@implementation NSCountedSet (CWPackable)
- (int) persistentAttributeCount {return 2*(int)self.count + super.persistentAttributeCount;}

- (void) cwPackSub:(CWPackContext*)ctx
{
    for (id object in self)
    {
        [ctx packObject:object];
        cw_pack_signed(ctx.context, [self countForObject:object]);
    }
}

- (instancetype) cwUnpackSubInit:(CWUnpackContext*)ctx remainingAttributes:(int)remainingAttributes
{
    return [self initWithCapacity:remainingAttributes/2];
}

- (instancetype) cwUnpackSub:(CWUnpackContext*)ctx remainingAttributes:(int)remainingAttributes
{
    long count = remainingAttributes/2;
    int i;
    for (i=0; i<count; i++)
    {
        id object = [ctx unpackNextObject];
        long r,repetitions = cw_unpack_next_unsigned64(ctx.context);
        for (r=0; r<repetitions; r++)
            [self addObject:object];
    }
    return self;
}

@end
