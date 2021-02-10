# CWPack / Goodies / ObjC


ObjC contains a wrapper for objective-C in the form of a category to NSObject.
The category contains two methods:

```C
/* ***********************   P A C K   *****************************/
- (void) packIn:(cw_pack_context*) buff;

/* ***********************   U N P A C K   *************************/
+ (instancetype) unpackFrom:(cw_unpack_context*) buff;
```

The methods are defined for the standard classes NSString, NSData, NSNumber, NSDate, NSNull, NSDictionary, NSArray and NSSet.

For other classes you need to define the methods yourself. E.g.

```C
@interface myClass: NSObject {
	int	            theInt;
	NSMutableSet    theSet;
}
@end

@implementation myClass
- (void) packIn:(cw_pack_context*) buff
{
	cw_pack_signed( buff, theInt );
	[theSet packIn:buff];
}
+ (instancetype) unpackFrom:(cw_unpack_context*) buff
{
	int anInt = cw_unpack_next_signed32( buff );
	NSMutableSet aSet = [NSMutableSet unpackFrom:buff];
	return [[self alloc] initWithInt:anInt set:aSet];
}
@end
```

