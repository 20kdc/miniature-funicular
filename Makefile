# TODO: autodetect for actual Windows users
COMPILER=i686-w64-mingw32-gcc
HEADERS=

all: bin/test.dll bin/test.exe

test_testdll: bin/test.dll
	wine rundll32 bin/test.dll

test_testexe: bin/test.exe
	wine bin/test.exe

clean:
	rm bin/test.dll bin/test.exe

bin/%.dll: src/%_dll.c $(HEADERS)
	$(COMPILER) -static-libgcc -shared $^ -o $@

bin/%.exe: src/%_exe.c $(HEADERS)
	$(COMPILER) -static-libgcc $^ -o $@

