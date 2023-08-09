#include "berylscript.h"

static struct i_val test_fn_callback(const struct i_val *args, i_size n_args) {
	if(BERYL_TYPEOF(args[0]) != TYPE_NUMBER) {
		beryl_blame_arg(args[0]);
		return BERYL_ERR("Expected number");
	}
	return BERYL_NUMBER(beryl_as_num(args[0]) * 2);
}

static struct beryl_external_fn test_fn_struct = {
	1,
	"testlib.double",
	sizeof("testlib.double") - 1,
	test_fn_callback
};

static bool loaded = false;

static struct i_val lib_val;

#define N_ENTRIES 1

static void init_lib() {
	struct i_val table = beryl_new_table(N_ENTRIES, true);
	if(BERYL_TYPEOF(table) == TYPE_NULL) {
		lib_val = BERYL_ERR("Out of memory");
	}
	
	beryl_table_insert(&table, BERYL_CONST_STR("double"), BERYL_EXT_FN(&test_fn_struct));
	lib_val = table;
}

struct i_val beryl_lib_load() {
	if(!loaded) {
		init_lib();
		loaded = true;
	}
	return beryl_retain(lib_val);
}
