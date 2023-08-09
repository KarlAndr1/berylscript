# The BerylScript programming language

BerylScript (wt) is a small programming language with value semantics that is largely based off of (and shares some code with) Beryl.
Like Beryl it is an embeddable scripting language that executes directly from source code, no parsing or compiling done ahead of time.
Most of the core language can run without access to libc or any dynamic allocation functions like malloc.

## Examples

Hello world:
```
print "Hello world!"
```

fib.beryl:
```
let fib = function n do
	if (n == 0) or? (n == 1) do
		n
	end else do
		(fib n - 1) + (fib n - 2)
	end
end
```
Note that 'if' is a function, just like 'print' or 'fib'.

loop.beryl:
```
for 1 11 with i do
	print i
end
```
This prints the numbers 1, 2, ..., 10. for is also a function defined in the standard library.

More code examples can be found in ./test_scripts

## Building/Installing

Run
```
	make
```
to build the project.
Run
```
	make install
``` 
to install the built executable and set up the required directories and environment variables.

Alternatively, one can instead first run build.sh
```
	./build.sh
```
Which will build the project and all the external/optional libraries. Then run install.sh to install everything and
set up the environment.
```
	./install.sh
```
