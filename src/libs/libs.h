#ifndef LIBS_H_INCLUDED
#define LIBS_H_INCLUDED

#include "../berylscript.h"

bool load_core_lib();
struct i_val i_val_as_string(struct i_val val);

bool load_debug_lib();

bool load_io_lib();

#ifdef __unix__ 
void load_unix_lib();
#endif

#endif
