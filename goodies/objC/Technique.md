# CWPack / Goodies / ObjC / Technique

We have choosen to represent objects as arrays. To be able to differentiate objects from normal arrays we have put an object marker as the first slot of the array. The object marker is an EXT item with type 127. The second array slot is kept by the class name. The rest of the array slots are kept by the objects attributes.

### User objects

We have an example:

```C
@interface MyClass: NSObject <CWPackable>
@property    int     level;
@property    MyClass *link;
- (instancetype) initWithLevel:(int)level link:(MyClass*)link;
@end
```
When packed into a MessagePack stream it may look like:

```C
[(127,<00>) "MyClass" 37 NULL]
```
This was the simple example where the link was nil. But look at this code fragment:

```C
MyClass *a = [MyClass.alloc initWithLevel:10 link:nil];
MyClass *b = [MyClass.alloc initWithLevel:20 link:a];
a.link = b; 
[myContext packObject:a];
```
To not fall into eternal recursion we need to break the circular reference in some way. We do it by giving the objects labels (set useLabel = YES on pack context). The labels are just consecutive numbers starting at 1. We store the label in the objects markers payload. Let´s start packing:

```C
[(127,<01>)			// object a got the label 1, we note that in a table
[(127,<01>) "MyClass" 10 [(127,<02>) "MyClass" 20 
```
Now it is time to insert object *a* again. But this time we have the object in our table and instead of packing the object we just pack a reference to it. A reference is an array with a single slot that is the object marker. The final result becomes:

```C
[(127,<01>) "MyClass" 10 [(127,<02>) "MyClass" 20 [(127,<01>)]]]
```
The cwpack dump utility "knows" object markers and prints the above message as:

1->MyClass(10 2->MyClass(20 ->1))

### MessagePack objects

It is not just user objects that can contain circular references. Take this example:

```C
NSMutableArray *a = NSMutableArray.new;
[a addObject:a];
[myContext packObject:a];
```
Obviously we need a way to label MessagePack objects also. We do as for user objects, but instead for a class-name in the second slot we put the MessagePack object. In this case, there are just 2 slots.

There is one problem here. If the MessagePack object is a string, it looks exactly like an user object with no attributes. We solve this ambiguity by letting MessagePack objects have negative labels starting with -1. So the packing above becomes:

```C
[(127,<ff>) [[(127,<ff>)]]]
```
Or prettyprinted: -1->[-> -1]
