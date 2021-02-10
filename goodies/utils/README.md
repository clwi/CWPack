# CWPack / Goodies / Utils


Utils contains some convenience routines:

### C string packing
```C
void cw_pack_cstr (cw_pack_context* pack_context, const char* v);
```

### Optimized packing of real numbers
Pack float as signed if precision isn't destroyed

```C
void cw_pack_float_opt (cw_pack_context* pack_context, float f);
```
Pack double as signed or float if precision isn't destroyed

```C
void cw_pack_double_opt (cw_pack_context* pack_context, double d);
```

### Packing seconds and fractions thereof that have elapsed since epoch
```C
void cw_pack_time_interval (cw_pack_context* pack_context, double ti);
```
### Easy retreival (expect api)

```C
bool cw_unpack_next_boolean (cw_unpack_context* unpack_context);

int64_t cw_unpack_next_signed64 (cw_unpack_context* unpack_context);
int32_t cw_unpack_next_signed32 (cw_unpack_context* unpack_context);
int16_t cw_unpack_next_signed16 (cw_unpack_context* unpack_context);
int8_t cw_unpack_next_signed8 (cw_unpack_context* unpack_context);

uint64_t cw_unpack_next_unsigned64 (cw_unpack_context* unpack_context);
uint32_t cw_unpack_next_unsigned32 (cw_unpack_context* unpack_context);
uint16_t cw_unpack_next_unsigned16 (cw_unpack_context* unpack_context);
uint8_t cw_unpack_next_unsigned8 (cw_unpack_context* unpack_context);

float cw_unpack_next_float (cw_unpack_context* unpack_context);
double cw_unpack_next_double (cw_unpack_context* unpack_context);
double cw_unpack_next_time_interval (cw_unpack_context* unpack_context);

unsigned int cw_unpack_next_str_lengh (cw_unpack_context* unpack_context);

unsigned int cw_unpack_next_array_size(cw_unpack_context* unpack_context);
unsigned int cw_unpack_next_map_size(cw_unpack_context* unpack_context);
```
The functions signals `CWP_RC_TYPE_ERROR` if next item isn't compatible with the expected type. For int and uint types the functions signals `CWP_RC_VALUE_ERROR` if value is compatible  but out of range.

