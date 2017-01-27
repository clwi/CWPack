//
//  numeric_extensions.h
//  CWPack
//
//  Created by Claes Wihlborg on 2017-01-16.
//  Copyright © 2017 Claes Wihlborg. All rights reserved.
//

#ifndef numeric_extensions_h
#define numeric_extensions_h

#include "cwpack.h"



#define NUMEXT_ERROR_NOT_EXT        1;
#define NUMEXT_ERROR_WRONG_LENGTH   2;

    
    
    
    void cw_pack_ext_integer (cw_pack_context* pack_context, int8_t type, int64_t i);
    void cw_pack_ext_float (cw_pack_context* pack_context, int8_t type, float f);
    void cw_pack_ext_double (cw_pack_context* pack_context, int8_t type, double d);

    
    int get_ext_integer (cw_unpack_context* unpack_context, int64_t* value);
    int get_ext_float (cw_unpack_context* unpack_context, float* value);
    int get_ext_double (cw_unpack_context* unpack_context, double* value);

    

#endif /* numeric_extensions_h */
