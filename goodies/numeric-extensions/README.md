#CWPack - Goodies - Numeric Extensions


Numeric Extensions solves the byte order problem when the value of an extension item is a numeric entity. It uses the same byte order as `cw_pack_signed` and `cw_pack_real`.

## Pack

```C
int cw_pack_ext_integer (cw_pack_context* pack_context, int8_t type, int64_t i);
int cw_pack_ext_float (cw_pack_context* pack_context, int8_t type, float f);
int cw_pack_ext_double (cw_pack_context* pack_context, int8_t type, double d);
```
The packing functions works just as the original packing functions with the same error returns etc..

## Unpack

```C
int get_ext_integer (cw_unpack_context* unpack_context, int64_t* value);
int get_ext_float (cw_unpack_context* unpack_context, float* value);
int get_ext_double (cw_unpack_context* unpack_context, double* value);
```
The `get_ext_...` functions assume that the user first has done a successful `cw_unpack_next` call. They return error if the unpacked item isn't an ext item or if the item has wrong length.
