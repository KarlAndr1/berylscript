#ifndef BERYL_SCRIPT_H_INCLUDED
#define BERYL_SCRIPT_H_INCLUDED

#include <stddef.h>
#include <limits.h>
#include <float.h>

typedef double i_float;
#define BERYL_NUM_MAX_INT (1ll << DBL_MANT_DIG)

typedef unsigned i_size;
#define I_SIZE_MAX UINT_MAX

typedef unsigned short i_refc;
#define I_REFC_MAX USHRT_MAX

typedef _Bool bool;
#define true (1)
#define false (0)

typedef unsigned long long beryl_tag;
#define BERYL_MAX_TAGS ULLONG_MAX

typedef unsigned long long large_uint_type;
#define LARGE_UINT_TYPE_MAX ULLONG_MAX

enum {
	TYPE_NULL,
	TYPE_NUMBER,
	TYPE_BOOL,
	TYPE_STR,
	TYPE_TABLE,
	TYPE_ARRAY,
	TYPE_TAG,
	
	TYPE_FN,
	TYPE_EXT_FN,
	
	TYPE_ERR
};

enum beryl_err_action {
	BERYL_PRINT_ERR,
	BERYL_CATCH_ERR,
	BERYL_PROP_ERR
};

struct beryl_table;

struct i_managed_str;

struct beryl_external_fn;
struct beryl_fn;

#define BERYL_INLINE_STR_MAX_LEN (sizeof(void*))

struct i_val {
	union {
		struct i_managed_str *managed_str;
		struct beryl_table *table;
		const char *str;
		char inline_str[BERYL_INLINE_STR_MAX_LEN];
		i_float num_v;
		bool bool_v;
		struct beryl_external_fn *ext_fn;
		const char *fn;
		beryl_tag tag;
		struct i_val *static_array;
		struct i_managed_array *managed_array;
	} val;
	i_size len;
	bool managed;
	unsigned char type;
};

struct beryl_external_fn {
	int arity;
	const char *name;
	size_t name_len;
	struct i_val (*fn)(const struct i_val *, i_size);
};

struct i_val_pair {
	struct i_val key, val;
};

struct beryl_table {
	i_size cap;
	i_refc ref_c;
	struct i_val_pair entries[];
};

#define BERYL_NULL ((struct i_val) { .type = TYPE_NULL } )
#define BERYL_NUMBER(f) ((struct i_val) { .type = TYPE_NUMBER, .val.num_v = f } )
#define BERYL_BOOL(b) ((struct i_val) { .type = TYPE_BOOL, .val.bool_v = b } )
#define BERYL_TRUE BERYL_BOOL(true)
#define BERYL_FALSE BERYL_BOOL(false)
#define BERYL_STATIC_STR(s, l) ((struct i_val) { .type = TYPE_STR, .len = l, .val.str = s, .managed = false })
#define BERYL_CONST_STR(s) BERYL_STATIC_STR((s), sizeof(s) - 1)
#define BERYL_STATIC_ARRAY(a, l) ((struct i_val) { .type = TYPE_ARRAY, .len = l, .val.static_array = a, .managed = false })

#define BERYL_ERR(msg_str) ((struct i_val) { .type = TYPE_ERR, .managed = false, .len = sizeof(msg_str) - 1, .val.str = msg_str } ) 

#define BERYL_EXT_FN(fn_ptr) ((struct i_val) { .type = TYPE_EXT_FN, .val.ext_fn = fn_ptr } )

#define BERYL_TYPEOF(v) ((v).type)
#define BERYL_LENOF(v) ((v).len)

//#define BERYL_AS_NUM(v) (assert(BERYL_TYPEOF(v) == TYPE_NUMBER), (v).val.num_v);

const char *beryl_get_raw_str(const struct i_val *str);
i_float beryl_as_num(struct i_val val);
bool beryl_as_bool(struct i_val val);
beryl_tag beryl_as_tag(struct i_val val);

struct i_val beryl_new_tag();

bool beryl_is_integer(struct i_val val);

int beryl_val_cmp(struct i_val a, struct i_val b); // -1 if a is larger, 0 if they are equal, 1 if b is larger, and 2 if they are not comparable

bool beryl_set_var(const char *name, i_size name_len, struct i_val val, bool as_const);

struct i_val beryl_new_table(i_size cap, bool padding);

struct i_val beryl_new_array(i_size len, const struct i_val *items, i_size fit_for, bool padded);
const struct i_val *beryl_get_raw_array(struct i_val array);

i_size beryl_get_array_capacity(struct i_val array);

#define BERYL_STATIC_TABLE_SIZE(l) ( sizeof(struct beryl_table) + sizeof(struct i_val_pair) * ((l)*3 / 2) )
struct i_val beryl_static_table(i_size cap, unsigned char *bytes, size_t bytes_size);

struct i_val_pair *beryl_iter_table(struct i_val table_v, struct i_val_pair *iter);

int beryl_table_insert(struct i_val *table_v, struct i_val key, struct i_val val, bool replace);
bool beryl_table_should_grow(struct i_val table, i_size extra);

void beryl_set_io(void (*print)(void *, const char *, size_t), void (*print_i_val)(void *, struct i_val), void *err_f);
void beryl_set_mem(void *(*alloc)(size_t), void (*free)(void *), void *(*realloc)(void *, size_t));

struct i_val beryl_new_string(i_size len, const char *from);

struct i_val beryl_str_as_err(struct i_val str);
struct i_val beryl_err_as_str(struct i_val str);

struct i_val beryl_retain(struct i_val val);
void beryl_release(struct i_val val);
i_refc beryl_get_refcount(struct i_val val);

void beryl_blame_arg(struct i_val val);

void beryl_free(void *ptr);
void *beryl_alloc(size_t n);
void *beryl_realloc(void *ptr, size_t n);

void *beryl_talloc(size_t n); //Quick temporary allocations used by libraries
void beryl_tfree(void *ptr);

struct i_val beryl_call(struct i_val fn, const struct i_val *args, size_t n_args);
struct i_val beryl_pcall(struct i_val fn, const struct i_val *args, size_t n_args);

struct i_val beryl_eval(const char *src, size_t src_len, bool new_scope, enum beryl_err_action err);


#endif
