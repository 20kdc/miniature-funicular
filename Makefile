# TODO: autodetect for actual Windows users
COMPILER=i686-w64-mingw32-gcc
INCLUDES=

all: bin/mod.exe bin/test.dll bin/test.exe bin/test.mod.exe

test_testdll: bin/test.dll
	wine rundll32 bin/test.dll

test_testexe: bin/test.exe
	wine bin/test.exe

test_testmodexe: bin/test.mod.exe
	wine bin/test.mod.exe

bin/test.mod.exe: bin/test.exe bin/mod.exe
	cp bin/test.exe bin/test.mod.exe
	wine bin/mod.exe test.dll bin/test.mod.exe

clean:
	rm bin/test.dll bin/test.exe bin/mod.exe bin/test.mod.exe

bin/%.dll: src/%_dll.c $(INCLUDES)
	$(COMPILER) -static-libgcc -Wno-multichar -shared $^ -o $@

bin/%.exe: src/%_exe.c $(INCLUDES)
	$(COMPILER) -static-libgcc -Wno-multichar $^ -o $@

