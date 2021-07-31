PRODUCTS := loader loader.exe test.obj test_cpp_gnu.obj test_cpp_msvc.obj

LOADER_SOURCES := loader.c playcoff_fmt.c playcoff_sys.c
LOADER_HEADS := playcoff_fmt.h playcoff_sys.h
LOADER_ALL := $(LOADER_HEADS) $(LOADER_SOURCES)

all: $(PRODUCTS)

clean:
	rm -f $(PRODUCTS)

loader.exe: $(LOADER_ALL)
	i686-w64-mingw32-gcc -DWINDOWS $(LOADER_SOURCES) -o loader.exe

loader: $(LOADER_ALL)
	gcc -m32 $(LOADER_SOURCES) -ldl -o loader

test.obj: test.c
	clang -target i686-windows-gnu -c test.c -o test.obj

test_cpp_gnu.obj: test_cpp.cpp
	clang++ -target i686-windows-gnu -fno-exceptions -c test_cpp.cpp -o test_cpp_gnu.obj

test_cpp_msvc.obj: test_cpp.cpp
	clang++ -target i686-windows-msvc -fno-exceptions -c test_cpp.cpp -o test_cpp_msvc.obj

