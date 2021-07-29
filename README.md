# miniature-funicular

A scratchpad repository for various messing around with x86 32-bit code.

## Contents

### Universal

+ `playcoff` : It's nowhere near done.

### Targets Something Windows-alike

+ `mod_exe` : Modifies an EXE or DLL to make it load another DLL. Cross-platform, but only targets a specific format.

### Needs Something Windows-alike

+ `test_dll` : A test DLL for `mod_exe`. Can be injected into or can do the injection.
+ `test_exe` : A test EXE for `mod_exe` to inject into.
+ `moonboot_dll` : Implemented in C and translated to Zig (but not well), a DLL that loads Lua/LuaJIT. Can be cross-compiled.

