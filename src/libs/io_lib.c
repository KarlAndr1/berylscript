#include "../berylscript.h"

#include "../utils.h"

#include <stdio.h>
#include <string.h>

#include "../io.h"

typedef struct i_val i_val;

#define FN(arity, name, fn) { arity, name, sizeof(name) - 1, fn }

#ifdef __windows__
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

static char *as_path(const char *str, size_t len) {
	char *res = beryl_talloc(len + 1);
	if(res == NULL)
		return NULL;
	
	memcpy(res, str, len);
	res[len] = '\0';
	return res;
}

static void free_path(char *path) {
	beryl_tfree(path);
}



static i_val read_callback(const i_val *args, i_size n_args) {
	(void) n_args;
	if(BERYL_TYPEOF(args[0]) != TYPE_STR) {
		beryl_blame_arg(args[0]);
		return BERYL_ERR("Expected string path as argument for 'read'");
	}
	
	i_size path_len = BERYL_LENOF(args[0]);
	char *c_path = as_path(beryl_get_raw_str(&args[0]), path_len);
	if(c_path == NULL)
		return BERYL_ERR("Out of memory");
	
	size_t file_len;
	char *file_contents = load_file(c_path, &file_len);
	free_path(c_path);
	if(file_contents == NULL) {
		beryl_blame_arg(args[0]);
		return BERYL_ERR("Unable to read from file");
	}
	
	if(file_len > I_SIZE_MAX) {
		beryl_free(file_contents);
		return BERYL_ERR("File too large");
	}
	
	i_val str_res = beryl_new_string(file_len, file_contents);
	beryl_free(file_contents);
	
	if(BERYL_TYPEOF(str_res) == TYPE_NULL)
		return BERYL_ERR("Out of memory");
	
	return str_res;
}

static struct i_val print_callback(const struct i_val *args, i_size n_args) {
	for(i_size i = 0; i < n_args; i++) {
		print_i_val(stdout, args[i]);
		if(i != n_args - 1)
			putchar(' ');
	}
	putchar('\n');
	
	return BERYL_NULL;
}

static i_val write_callback(const i_val *args, i_size n_args) {
	(void) n_args;
	if(BERYL_TYPEOF(args[0]) != TYPE_STR) {
		beryl_blame_arg(args[0]);
		return BERYL_ERR("Expected file path as first argument for 'write'");
	}
	if(BERYL_TYPEOF(args[1]) != TYPE_STR) {
		beryl_blame_arg(args[1]);
		return BERYL_ERR("Expected string as second argument for 'write'");
	}
	
	char *c_path = as_path(beryl_get_raw_str(&args[0]), BERYL_LENOF(args[0]));
	if(c_path == NULL)
		return BERYL_ERR("Out of memory");
	
	FILE *f = fopen(c_path, "w");
	free_path(c_path);
	if(f == NULL) {
		free_path(c_path);
		return BERYL_ERR("Unable to open file");
	}
	
	size_t res = fwrite(beryl_get_raw_str(&args[1]), sizeof(char), BERYL_LENOF(args[1]), f);
	fclose(f);

	if(res != BERYL_LENOF(args[1])) {
		beryl_blame_arg(args[0]);
		return BERYL_ERR("Error when writing to file");
	}
	
	return BERYL_NULL;
}

bool load_io_lib() {
	static struct beryl_external_fn fns[] = {
		FN(1, "read", read_callback),
		FN(-2, "print", print_callback),
		FN(2, "write", write_callback)
	};

	for(size_t i = 0; i < LENOF(fns); i++) {
		bool ok = beryl_set_var(fns[i].name, fns[i].name_len, BERYL_EXT_FN(&fns[i]), true);
		if(!ok)
			return false;
	}
	
	beryl_set_var("path-separator", sizeof("path-separator") - 1, BERYL_CONST_STR(PATH_SEPARATOR), true);
	
	return true;
}
