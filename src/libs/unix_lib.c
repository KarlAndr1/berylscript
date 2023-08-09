#include "../berylscript.h"

#include "../utils.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(__unix__)
	#include <dlfcn.h>
	#include <unistd.h>
	#include <sys/wait.h>
	typedef void *dyn_lib;
	#define DL_IS_NULL(dl) ((dl) == NULL)
	#define DL_OPEN(path) dlopen(path, RTLD_NOW)
	#define DL_PROC(dl, name) dlsym(dl, name)
	
	#define PLATFORM "unix"
#elif defined(__windows__)
	#include <windows.h>
	#include <process.h>
	typedef HINSTANCE dyn_lib;
	#define DL_IS_NULL(dl) ((dl) == NULL)
	#define DL_OPEN(path) LoadLibrary(TEXT(path))
	#define DL_PROC(dl, name) GetProcAddress(dl, name)
	
	#define PLATFORM "windows"
#else
	typedef void *dyn_lib;
	#define DL_IS_NULL(dl) ((dl) == NULL)
	#define DL_OPEN(path) (NULL)
	#define DL_PROC(dl, name) (NULL)
	
	#define PLATFORM "other"
#endif



typedef struct i_val i_val;

static i_val load_dl_callback(const i_val *args, i_size n_args) {
	(void) n_args;

	if(BERYL_TYPEOF(args[0]) != TYPE_STR) {
		beryl_blame_arg(args[0]);
		return BERYL_ERR("Expected string path as first argument");
	}
	i_size path_len = BERYL_LENOF(args[0]);
	char *c_path = beryl_talloc(path_len + 1);
	if(c_path == NULL)
		return BERYL_ERR("Out of memory");
	c_path[path_len] = '\0';
	memcpy(c_path, beryl_get_raw_str((i_val *) &args[0]), path_len);
	
	dyn_lib dl = DL_OPEN(c_path);
	beryl_tfree(c_path);
	
	if(DL_IS_NULL(dl)) {
		beryl_blame_arg(args[0]);
		return BERYL_ERR("Unable to open dynamic library");
	}
	
	void *lib_load = DL_PROC(dl, "beryl_lib_load");
	if(lib_load == NULL) {
		beryl_blame_arg(args[0]);
		return BERYL_ERR("Not a BerylScript library");
	}
	i_val (*lib_load_fn_ptr)() = lib_load;
	
	return lib_load_fn_ptr();
}

static i_val getenv_callback(const i_val *args, i_size n_args) {
	(void) n_args;

	if(BERYL_TYPEOF(args[0]) != TYPE_STR) {
		beryl_blame_arg(args[0]);
		return BERYL_ERR("Expected string as argument for 'getenv'");
	}
	
	i_size len = BERYL_LENOF(args[0]);
	char *c_name = beryl_talloc(len + 1);
	if(c_name == NULL)
		return BERYL_ERR("Out of memory");
	
	memcpy(c_name, beryl_get_raw_str(&args[0]), len);
	c_name[len] = '\0';
	
	const char *env_var = getenv(c_name);
	beryl_tfree(c_name);
	
	if(env_var == NULL)
		return BERYL_NULL;
	
	i_val res = beryl_new_string(strlen(env_var), env_var);
	if(BERYL_TYPEOF(res) == TYPE_NULL)
		return BERYL_ERR("Out of memory");
	return res;
}

static int p_spawn(const char *cmd, char **argv, int *return_code) { // return of 0 means sucess, -1 some system error (check errno), -2 unsupported platform
	#if defined(__unix__)
	
	pid_t pid = fork();
	if(pid == -1)
		return -1;

	if(pid == 0) { //In child process
		execvp(cmd, argv);
		exit(-1); //If exec failed
	} else { //In the parent process
		int wstatus;
		wait(&wstatus);
		if(WIFEXITED(wstatus))
			*return_code = WEXITSTATUS(wstatus);
		else
			*return_code = -1;
		return 0;
	}
	
	
	#elif defined(__windows__)
	
	int res = _spawn(_P_WAIT, cmd, argv);
	if(res == -1)
		return -1;
	
	*return_code = res;
	return 0;
	#else
	return -2;
	#endif
}

static int run_cmd(/* char *cmd, */ char **args, /* size_t argc,*/ int *return_code) { //0 Ok, -1 system error (check errno), -2 not supported, -3 out of memory
	/*
	char **argv = beryl_talloc(sizeof(char*) * (argc + 2));
	if(argv == NULL)
		return -3;
	
	argv[0] = cmd;
	memcpy(argv + 1, args, argc);
	argv[argc + 1] = NULL;
	
	int res = p_spawn(cmd, argv, return_code);
	beryl_tfree(argv);
	return res; */
	
	assert(args[0] != NULL);
	char *cmd = args[0];
	return p_spawn(cmd, args, return_code);
}

static i_val run_callback(const i_val *args, i_size n_args) {
	i_val err;
	char **cmd = beryl_talloc(n_args + 1 * sizeof(char *));
	size_t cmds = 0;
	if(cmd == NULL)
		return BERYL_ERR("Out of memory");
	
	cmd[n_args] = NULL;
	for(i_size i = 0; i < n_args; i++) {
		if(BERYL_TYPEOF(args[i]) != TYPE_STR) {
			beryl_blame_arg(args[i]);
			err = BERYL_ERR("Expected string as argument for 'run'");
			goto ERR;
		}
		
		i_size str_len = BERYL_LENOF(args[i]);
		char *c_str = beryl_alloc(str_len + 1);
		if(c_str == NULL) {
			err = BERYL_ERR("Out of memory");
			goto ERR;
		}
		c_str[str_len] = '\0';
		memcpy(c_str, beryl_get_raw_str(&args[i]), str_len);
		cmd[i] = c_str;
		cmds++;
	}
	
	int return_code;
	int res = run_cmd(cmd, &return_code);
	switch(res) {
		case -1: {
			const char *cerr = strerror(errno);
			i_val err_msg = beryl_new_string(strlen(cerr), cerr);
			if(BERYL_TYPEOF(err_msg) == TYPE_NULL)
				err = BERYL_ERR("Cannot display system error; Out of memory");
			else
				err = beryl_str_as_err(err_msg);
			//err = BERYL_ERR("System error"); //TODO: Make the error message contain the error in errno
			goto ERR;
		}
		
		case -2:
			err = BERYL_ERR("Run is not supported on this platform");
			goto ERR;
		
		case -3:
			err = BERYL_ERR("Out of memory");
			goto ERR;
	}
	
	for(size_t i = 0; i < cmds; i++)
		beryl_free(cmd[i]);
	beryl_tfree(cmd);
	
	return BERYL_NUMBER(return_code);
	
	ERR:
	for(size_t i = 0; i < cmds; i++)
		beryl_free(cmd[i]);
	beryl_tfree(cmd);
	return err;
}

#define FN(arity, name, fn) { arity, name, sizeof(name) - 1, fn }

bool load_unix_lib() {
	static struct beryl_external_fn fns[] = {
		FN(1, "load-dl", load_dl_callback),
		FN(1, "getenv", getenv_callback),
		FN(-2, "run", run_callback)
	};

	for(size_t i = 0; i < LENOF(fns); i++) {
		bool ok = beryl_set_var(fns[i].name, fns[i].name_len, BERYL_EXT_FN(&fns[i]), true);
		if(!ok)
			return false;
	}
	
	beryl_set_var("platform", sizeof("platform") - 1, BERYL_CONST_STR(PLATFORM), true);
	
	return true;
}

