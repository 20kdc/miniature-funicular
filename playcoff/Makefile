LOADER_SOURCES := loader.c playcoff_sys.c
LOADER_HEADS := playcoff_fmt.h playcoff_sys.h
LOADER_ALL := $(LOADER_HEADS) $(LOADER_SOURCES)

all: loader loader.exe test.obj

loader.exe: $(LOADER_ALL)
	i686-w64-mingw32-gcc -DWINDOWS $(LOADER_SOURCES) -o loader.exe

loader: $(LOADER_ALL)
	gcc -m32 $(LOADER_SOURCES) -ldl -o loader

test.obj: test2.obj scr.ld
	ld -m i386pe -T scr.ld -r test2.obj -o test.obj

test2.obj: test.c
	clang -target i686-windows-gnu -c test.c -o test2.obj
