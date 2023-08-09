#!/usr/bin/env make -f

core = src/berylscript.o src/lexer.o src/libs/core_lib.o
opt_libs = src/libs/io_lib.o src/io.o src/libs/unix_lib.o src/libs/debug_lib.o

external_libs = libs/math

CFLAGS = -std=c99 -Wall -Wextra -Wpedantic

release: CFLAGS += -O2
release: berylscript

debug: CFLAGS += -g -fsanitize=address,undefined,leak -DDEBUG
debug: berylscript

test: debug
	./test.sh
run: debug
	./berylscript

berylscript: src/main.o $(core) $(opt_libs)
	cc src/main.o $(core) $(opt_libs) -rdynamic -oberylscript $(CFLAGS)

lib: $(core) $(opt_libs)
	ar rcs libBerylScript.ar $(core) $(opt_libs) 

minimal-lib: $(core)
	ar rcs libBerylScript.ar $(core)

install: release
	./build_libs.sh
	./install.sh

clean:
	rm $(core) $(opt_libs) src/main.o

$(core) $(opt_libs) src/main.o:
