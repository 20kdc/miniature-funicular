# TODO: autodetect for actual Windows users
COMPILER=i686-w64-mingw32-gcc
STRIP=i686-w64-mingw32-strip
INCLUDES=

all: bin/mod.exe bin/test.dll bin/test.exe bin/test.mod.exe bin/test.mod.dll

test_testdll: bin/test.dll
	wine rundll32 bin/test.dll

test_testexe: bin/test.exe
	wine bin/test.exe

test_testmodexe: bin/test.mod.exe
	wine bin/test.mod.exe

bin/test.mod.exe: bin/test.exe bin/mod.exe
	cp bin/test.exe bin/test.mod.exe
	wine bin/mod.exe test.mod.dll bin/test.mod.exe

bin/test.mod.dll: bin/test.dll bin/mod.exe
	cp bin/test.dll bin/test.mod.dll
	wine bin/mod.exe test.dll bin/test.mod.dll

clean:
	rm bin/test.dll bin/test.exe bin/mod.exe bin/test.mod.exe bin/test.mod.dll

bin/%.dll: src/%_dll.c $(INCLUDES)
	$(COMPILER) -O3 -flto -static-libgcc -Wno-multichar -shared $^ -o $@
	$(STRIP) $@

bin/%.exe: src/%_exe.c $(INCLUDES)
	$(COMPILER) -O3 -flto -static-libgcc -Wno-multichar $^ -o $@
	$(STRIP) $@
