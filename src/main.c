#include "berylscript.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "io.h"

static void generic_print_callback(void *f, const char *str, size_t len) {
	fwrite(str, sizeof(char), len, f); 
}

static void print_i_val_callback(void *f, struct i_val val) {
	print_i_val(f, val);
}


struct eval_history_entry {
	struct eval_history_entry *prev;
	size_t len;
	char src[];
};

struct eval_history_entry *eval_history = NULL;

const char *history_add(const char *str, size_t len) {
	struct eval_history_entry *new = malloc(sizeof(struct eval_history_entry) + sizeof(char) * len);
	if(new == NULL)
		return NULL;
	memcpy(&new->src[0], str, len * sizeof(char));
	new->len = len;
	new->prev = eval_history;
	
	eval_history = new;
	return &new->src[0];
}

static bool print_if_err(struct i_val val) {
	if(BERYL_TYPEOF(val) == TYPE_ERR) {
		fputs("\x1B[31m", stdout);
		print_i_val(stdout, val);
		putchar('\n');
		fputs("\x1B[0m", stdout);
		return true;
	}
	return false;
}

static void prompt() {
	beryl_set_var("script-path", sizeof("script_path") - 1, BERYL_CONST_STR(""), false);
	beryl_set_var("script-dir", sizeof("script_dir") - 1, BERYL_CONST_STR("."), false);
	beryl_set_var("argv", sizeof("argv") - 1, BERYL_STATIC_ARRAY(NULL, 0), false);
	
	char line_buff[128];
	while(true) {
		fputs(">", stdout);
		char *res = fgets(line_buff, sizeof(line_buff), stdin);
		if(res == NULL) {
			puts("Unable to read from stdin");
			break;
		}
		if(strcmp(line_buff, "quit\n") == 0)
			break;
		size_t len = strlen(line_buff);
		const char *src = history_add(line_buff, len);
		if(src == NULL) {
			puts("Out of memory");
			break;
		}

		struct i_val eval_res = beryl_eval(src, len, false, BERYL_PRINT_ERR);
		bool is_err = print_if_err(eval_res);
		if(!is_err && BERYL_TYPEOF(eval_res) != TYPE_NULL) {
			print_i_val(stdout, eval_res);
			putchar('\n');
		}
		beryl_release(eval_res);
	}
}

#ifdef __windows__
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

static const char *find_char_from_right(const char *str, size_t len, char c) {
	for(const char *s = str + len - 1; s >= str; s--) {
		if(*s == c)
			return s;
	}
	return NULL;
}

static struct i_val run_script(const char *path, const char **args, int argc) {
	size_t len;
	char *file_contents = load_file(path, &len);
	if(file_contents == NULL) {
		fprintf(stderr, "Unable to read file '%s'\n", path);
		return BERYL_NULL;
	}
	
	const char *src = history_add(file_contents, len);
	free(file_contents);
	if(src == NULL) {
		fputs("Out of memory\n", stderr);
		return BERYL_NULL;
	}
	
	size_t path_len = strlen(path);
	if(path_len > I_SIZE_MAX) {
		fputs("Script path is too long\n", stderr);
		return BERYL_NULL;
	}
	struct i_val script_path = beryl_new_string(path_len, path);
	if(BERYL_TYPEOF(script_path) == TYPE_NULL) {
		fputs("Out of memory\n", stderr);
		return BERYL_NULL;
	}
	beryl_set_var("script-path", sizeof("script_path") - 1, script_path, false);
	beryl_release(script_path);
	
	const char *dir_sep = find_char_from_right(path, path_len, PATH_SEPARATOR);
	
	struct i_val script_dir_path;
	if(dir_sep != NULL) {
		size_t dir_path_len = dir_sep - path;
		script_dir_path = beryl_new_string(dir_path_len, path);
		if(BERYL_TYPEOF(script_dir_path) == TYPE_NULL) {
			fputs("Out of memory\n", stderr);
			return BERYL_NULL;
		}
	} else { //If the path to the script doesn't contain any / then it's a relative path in the current working directory (./)
		script_dir_path = BERYL_CONST_STR(".");
	}
	
	beryl_set_var("script-dir", sizeof("script-dir") - 1, script_dir_path, false);
	beryl_release(script_dir_path);
	
	struct i_val arg_array = beryl_new_array(argc, NULL, argc, false);
	if(BERYL_TYPEOF(arg_array) == TYPE_NULL) {
		fputs("Out of memory\n", stderr);
		return BERYL_NULL;
	}
	
	struct i_val *arg_a = (struct i_val *) beryl_get_raw_array(arg_array);
	for(int i = 0; i < argc; i++) {
		arg_a[i] = beryl_new_string(strlen(args[i]), args[i]);
		
		if(BERYL_TYPEOF(arg_a[i]) == TYPE_NULL) {
			for(int j = i - 1; j >= 0; j--) {
				beryl_release(arg_a[j]);
			}
			beryl_release(arg_array);
			fputs("Out of memory\n", stderr);
			return BERYL_NULL;
		}
	}
	
	beryl_set_var("argv", sizeof("argv") - 1, arg_array, false);
	beryl_release(arg_array);
	
	struct i_val res = beryl_eval(src, len, false, BERYL_PRINT_ERR);
	print_if_err(res);
	return res;
}

#include "libs/libs.h"

int main(int argc, const char **argv) {
	
	beryl_set_mem(malloc, free, realloc);
	beryl_set_io(generic_print_callback, print_i_val_callback, stderr);
	
	//beryl_set_var("print", sizeof("print") - 1, BERYL_EXT_FN(&print_fn), true);
	
	load_core_lib();
	load_debug_lib();
	load_unix_lib();
	load_io_lib();
	
	const char *init_script_path = getenv("BERYL_SCRIPT_INIT");
	if(init_script_path != NULL)
		beryl_release(run_script(init_script_path, NULL, 0));
	
	if(argc == 1) {
		prompt();
	} else if(argc >= 2) {
		struct i_val res = run_script(argv[1], argv + 2, argc - 2);
		beryl_release(res);
	}
	return 0;
}
