
/*      CWPack/goodies/ObjC - CWPackContext.h   */

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


#import <Foundation/Foundation.h>
#include "cwpack.h"

NS_ASSUME_NONNULL_BEGIN





@interface CWPackContext : NSObject

@property (readonly)    cw_pack_context         *context;
@property (readwrite)   BOOL                    useLabels;          // default NO

+ (instancetype) newWithContext:(cw_pack_context*)context;
- (void) packObject:(nullable NSObject*)object;

@end





@interface CWUnpackContext : NSObject

@property (readonly)    cw_unpack_context     *context;

+ (instancetype) newWithContext:(cw_unpack_context*)context;
- (id) unpackNextObject;

@end




@protocol CWPackable <NSObject>

@required
@property (readonly)    int     persistentAttributeCount;
- (void) cwPackSub:(CWPackContext*)ctx;
- (void) cwUnpackSub:(CWUnpackContext*)ctx remainingAttributes:(int)remainingAttributes;

@optional
- (instancetype) cwUnpackSubInit:(CWUnpackContext*)ctx remainingAttributes:(int)remainingAttributes;

@end




@interface CWPackExternalItem : NSObject

@property (readonly)            int         type;
@property (readwrite, strong)   NSData*     data;

+ (instancetype) itemWithType:(int)type data:(NSData*)data;

@end




@interface CWPackGenericClass : NSObject <CWPackable>

@property (readwrite,strong)   NSString         *packerClassName;
@property (readwrite,strong)   NSMutableArray   *attributes;

+ (instancetype) newWithClassName:(NSString*)className;
@end


NS_ASSUME_NONNULL_END
