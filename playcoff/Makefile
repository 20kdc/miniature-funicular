all: loader loader.exe test.obj

loader.exe: loader.c
	i686-w64-mingw32-gcc -DWINDOWS loader.c -o loader.exe

loader: loader.c
	gcc -m32 loader.c -o loader

test.obj: test2.obj scr.ld
	ld -m i386pe -T scr.ld -r test2.obj -o test.obj

test2.obj: test.c
	clang -target i686-windows-gnu -c test.c -o test2.obj
