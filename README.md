# miniature-funicular

A scratchpad repository for various messing around with x86 32-bit code.

## Contents

### Universal

+ `playcoff` : It's nowhere near done.

### Targets Something Windows-alike

+ `mod_exe` : Modifies an EXE or DLL to make it load another DLL. Cross-platform, but only targets a specific format.

### Needs Something Windows-alike

+ `ceboot` : Starts `C:\CE\cheatengine-i386.exe`. Shows two message boxes using `nootnoot` to confirm it's being started properly. Intended for VMs.
+ `cmdboot_dll` : Starts `cmd.exe` when injected. No guarantee of usefulness.
+ `moonboot_dll` : Implemented in C and translated to Zig (but not well), a DLL that loads Lua/LuaJIT. Can be cross-compiled.
+ `nootnoot` : Routine to ensure an application's windows can appear on the login desktop. Writes debug information to `COM1`. Intended for VMs.
+ `nootnoot_dll` : Inject into any program to give it `nootnoot`'s powers. Intended for VMs.
+ `test_dll` : A test DLL for `mod_exe`. Can be injected into or can do the injection.
+ `test_exe` : A test EXE for `mod_exe` to inject into.

