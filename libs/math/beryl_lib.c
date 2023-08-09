#include "berylscript.h"

#include <math.h>

#define PI 3.14159265359

static struct i_val pow_callback(const struct i_val *args, i_size n_args) {
	if(BERYL_TYPEOF(args[0]) != TYPE_NUMBER) {
		beryl_blame_arg(args[0]);
		return BERYL_ERR("Expected number as first argument for 'pow'");
	}
	if(BERYL_TYPEOF(args[1]) != TYPE_NUMBER) {
		beryl_blame_arg(args[1]);
		return BERYL_ERR("Expected number as second argument for 'pow'");
	}
	
	i_float res = pow(beryl_as_num(args[0]), beryl_as_num(args[1]));
	return BERYL_NUMBER(res);
}
static struct beryl_external_fn pow_callback_struct = {
	2,
	"math.pow",
	sizeof("math.pow") - 1,
	pow_callback
};

/*
static struct i_val sqrt_callback(const struct i_val *args, i_size n_args) {
	if(BERYL_TYPEOF(args[0]) != TYPE_NUMBER) {
		beryl_blame_arg(args[0]);
		return BERYL_ERR("Expected number as first argument for 'sqrt'");
	}
	
	i_float num = beryl_as_num(args[0]);
	if(num < 0)
		return BERYL_ERR("Cannot take square root of negative number");

	return BERYL_NUMBER(sqrt(num));
}
static struct beryl_external_fn sqrt_callback_struct = {
	1,
	"math.sqrt",
	sizeof("math.sqrt") - 1,
	sqrt_callback
};
*/

#define UNARY_OP(name, fn) \
static struct i_val name##_callback(const struct i_val *args, i_size n_args) { \
	if(BERYL_TYPEOF(args[0]) != TYPE_NUMBER) { \
		beryl_blame_arg(args[0]); \
		return BERYL_ERR("Expected number as first argument for '" #name "'"); \
	} \
	\
	i_float num = beryl_as_num(args[0]); \
	i_float res = fn(num); \
	if(isnan(res)) { \
		beryl_blame_arg(args[0]);\
		return BERYL_ERR("Invalid argument for '" #name "'");\
	} \
	return BERYL_NUMBER(res); \
} \
static struct beryl_external_fn name##_callback_struct = { \
	1, \
	"math." #name, \
	sizeof("math." #name) - 1, \
	name##_callback \
};
UNARY_OP(sqrt, sqrt);

UNARY_OP(sin, sin);
UNARY_OP(cos, cos);
UNARY_OP(tan, tan);

UNARY_OP(asin, asin);
UNARY_OP(acos, acos);
UNARY_OP(atan, atan);

static struct i_val deg_to_rad_callback(const struct i_val *args, i_size n_args) {
	if(BERYL_TYPEOF(args[0]) != TYPE_NUMBER) {
		beryl_blame_arg(args[0]);
		return BERYL_ERR("Expected number as first argument for 'deg->rad'");
	}

	i_float num = beryl_as_num(args[0]);
	return BERYL_NUMBER(num / 180 * PI);
}
static struct beryl_external_fn deg_to_rad_callback_struct = {
	1,
	"math.deg->rad",
	sizeof("math.deg->rad") - 1,
	deg_to_rad_callback
};

static struct i_val rad_to_deg_callback(const struct i_val *args, i_size n_args) {
	if(BERYL_TYPEOF(args[0]) != TYPE_NUMBER) {
		beryl_blame_arg(args[0]);
		return BERYL_ERR("Expected number as first argument for 'rad->deg'");
	}

	i_float num = beryl_as_num(args[0]);
	return BERYL_NUMBER(num / PI * 180);
}
static struct beryl_external_fn rad_to_deg_callback_struct = {
	1,
	"math.rad->deg",
	sizeof("math.rad->deg") - 1,
	rad_to_deg_callback
};


static bool loaded = false;

static struct i_val lib_val;

#define N_ENTRIES 11

static void init_lib() {
	struct i_val table = beryl_new_table(N_ENTRIES, true);
	if(BERYL_TYPEOF(table) == TYPE_NULL) {
		lib_val = BERYL_ERR("Out of memory");
	}
	
	beryl_table_insert(&table, BERYL_CONST_STR("pow"), BERYL_EXT_FN(&pow_callback_struct), false);
	beryl_table_insert(&table, BERYL_CONST_STR("pi"), BERYL_NUMBER(PI), false);
	beryl_table_insert(&table, BERYL_CONST_STR("sqrt"), BERYL_EXT_FN(&sqrt_callback_struct), false);
	
	beryl_table_insert(&table, BERYL_CONST_STR("sin"), BERYL_EXT_FN(&sin_callback_struct), false);
	beryl_table_insert(&table, BERYL_CONST_STR("cos"), BERYL_EXT_FN(&cos_callback_struct), false);
	beryl_table_insert(&table, BERYL_CONST_STR("tan"), BERYL_EXT_FN(&tan_callback_struct), false);
	beryl_table_insert(&table, BERYL_CONST_STR("asin"), BERYL_EXT_FN(&asin_callback_struct), false);
	beryl_table_insert(&table, BERYL_CONST_STR("acos"), BERYL_EXT_FN(&acos_callback_struct), false);
	beryl_table_insert(&table, BERYL_CONST_STR("atan"), BERYL_EXT_FN(&atan_callback_struct), false);
	
	beryl_table_insert(&table, BERYL_CONST_STR("rad->deg"), BERYL_EXT_FN(&rad_to_deg_callback_struct), false);
	beryl_table_insert(&table, BERYL_CONST_STR("deg->rad"), BERYL_EXT_FN(&deg_to_rad_callback_struct), false);
	
	
	lib_val = table;
}

struct i_val beryl_lib_load() {
	if(!loaded) {
		init_lib();
		loaded = true;
	}
	return beryl_retain(lib_val);
}
