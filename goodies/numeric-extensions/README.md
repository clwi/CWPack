# CWPack / Goodies / Numeric Extensions


Numeric Extensions solves the byte order problem when the value of an extension item is a numeric entity. It uses the same byte order as `cw_pack_signed` and `cw_pack_double`.

## Pack

```
void cw_pack_ext_integer (cw_pack_context* pack_context, int8_t type, int64_t i);
void cw_pack_ext_float (cw_pack_context* pack_context, int8_t type, float f);
void cw_pack_ext_double (cw_pack_context* pack_context, int8_t type, double d);
```

## Unpack

```
int64_t get_ext_integer (cw_unpack_context* unpack_context);
float get_ext_float (cw_unpack_context* unpack_context);
double get_ext_double (cw_unpack_context* unpack_context);
```
The `get_ext_...` functions assume that the user first has done a successful `cw_unpack_next` call. They return error if the unpacked item isn't an ext item or if the item has wrong length.
