# CWPack / Goodies / ObjC


ObjC contains a pack and an unpack context to enable Objective-C objects in the MessagePack stream. The implementation also has the capability to keep the references between objects.

## Contexts

ObjC contains 2 contexts `CWPackContext` and `CWPUnpackContext`.

#### Pack

`CWPackContext` is layered on top of a `cw_pack_context`, which it receives at initiation.
The `cw_pack_context` is then accessible through the property `context`.

The other property on `CWPackContext` is the boolean property `useLabels`. It is defaulted to NO, but should be set to YES if you want references between objects to be kept. This makes the MessagePack stream more verbose so don't use it if you don't have to.

To pack an object you just call: `[myPackContext packObject:anObject]`.

#### Unpack

`CWUnpackContext` is layered on top of a `cw_unpack_context`, which it receives at initiation.
The `cw_unpack_context` is then accessible through the property `context`.

To retreive an object you call `[myUnpackContext unpackNextObject]`.


## Packable objects

When choosing if the packing should be automatic by inspection or explicit, we have chosen the latter, as it gives better control over the (un)packing, which can be important when communicating with others. For that reason, to be packable, objects need to fulfill the protocol `CWPackable`. 

##### @property (readonly) int persistentAttributeCount
 
This is the number of MessagePack items that the description of the object take. If you have a long prefix chain you normally defines it as

```C
- (int) persistentAttributeCount {
	return myAttributeCount + super.persistentAttributeCount;}
```
On the next outermost level, where NSObject is the prefix class, you must not call `super`.

##### - (void) cwPackSub:(CWPackContext*)ctx

This is the routine where you pack. Let´s take an example:

```C
@interface MyClass: SomeOtherClass <CWPackable> 
@property    int     level;
@property    MyClass *link;
- (instancetype) initWithLevel:(int)level link:(MyClass*)link;
@end
```
Note first, one attribute is a link to another instance, so here we should set useLabels to YES. Our pack method could be like this:

```C
- (void) cwPackSub:(CWPackContext*)ctx {
	[super cwPackSub:ctx];
	cw_pack_signed(ctx.context, level);
	[ctx packObject:link];
}
```
If the immediate ancestor to MyClass is NSObject, then  super must not be called.

Unpacking can be a little more tangled due to the possibility of circular references, that is a referenced object has a reference back to the root object. Unpacking happens in two steps: first the object is allocated and in the next step the attributes are set.

Step 1: Allocation & Initiating

##### - (instancetype) cwUnpackSubInit:(CWUnpackContext*)ctx remainingAttributes:(int)remainingAttributes


When `cwUnpackSubInit` is called, self is allocated but not inited. The sole purpose of the method is to return a suitable inited object. It is not neccesary to be the same object as self. If it is sufficient to sent init to the preallocated object (self), this method could be skipped, it's optional.

Step 2: Fetching attributes. 

##### - (void) cwUnpackSub:(CWUnpackContext*)ctx remainingAttributes:(int)remainingAttributes

In the pack example an unpack method could be:

```C
- (void) cwUnpackSub:(CWUnpackContext*)ctx remainingAttributes:(int)remainingAttributes {
	[super cwUnpackSub:ctx remainingAttributes:remainingAttributes - 2]
	level = cw_unpack_next_signed32 (ctx.context);
	link = [ctx unpackNextObject];
}
```
As usual, don't call super on NSObject.

### Packable system objects
(Un)packing of the following system classes is predefined in ObjC:
NSArray, NSDictionary, NSData, NSDate, NSNull, NSNumber, NSSet, NSCountableSet and NSString. In most cases they are mapped at their corresponding MessagePack item. !WARNING! MessagePack has an arbitrary type as key in map. Objective-C demands a string key in the corresponding NSDictionary.

Of those who have both mutable and immutable versions, NSData and NSString are created immutable, and NSArray, NSDictionary and NSSet are created mutable at unpack.

### Special
There are two special cases. 

First, when the MessagePack item is of an unknown EXT type, it will be unpacked as an instance of the `CWPackExternalItem` class.

Second, when the object to be unpacked is of unknown user class, it is unpacked as an instance of the class `CWPackGenericClass`.
