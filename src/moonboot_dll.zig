const std = @import("std");

extern "kernel32" fn LoadLibraryA(name: [*c]const u8) callconv(.Stdcall) ?*c_void;
extern "kernel32" fn GetProcAddress(mod: ?*c_void, nax: [*c]const u8) callconv(.Stdcall) ?*c_void;
extern "kernel32" fn GetModuleFileNameA(mod: ?*c_void, name: [*c]u8, len: c_int) callconv(.Stdcall) c_int;
extern "kernel32" fn ExitProcess(code: c_int) callconv(.Stdcall) noreturn;
extern "user32" fn MessageBoxA(hw: ?*c_void, a: [*c]const u8, b: [*c]const u8, c: c_uint) callconv(.Stdcall) c_int;
extern "msvcrt" fn malloc(s: usize) ?*c_void;
extern "msvcrt" fn realloc(p: ?*c_void, s: usize) ?*c_void;
extern "msvcrt" fn free(p: ?*c_void) ?*c_void;
extern "msvcrt" fn strcpy(p: [*c]u8, px: [*c]const u8) ?*c_void;

pub const newstate_t = ?fn (...) callconv(.C) ?*c_void;
pub const openlibs_t = ?fn (?*c_void) callconv(.C) void;
pub const loadfile_t = ?fn (?*c_void, [*c]const u8) callconv(.C) c_int;
pub const pcall_t = ?fn (?*c_void, c_int, c_int, c_int) callconv(.C) c_int;
pub export fn findDir() [*c]u8 {
    var pathBufferLen: c_int = 512;
    while (true) {
        var buffer: [*c]u8 = @ptrCast([*c]u8, @alignCast(@alignOf(u8), malloc(@bitCast(c_uint, pathBufferLen))));
        var res: c_int = GetModuleFileNameA(null, buffer, pathBufferLen);
        if (res == pathBufferLen) {
            _ = free(buffer);
            pathBufferLen *= @as(c_int, 2);
        } else {
            buffer = @ptrCast([*c]u8, @alignCast(@alignOf(u8), realloc(@ptrCast(?*c_void, buffer), @bitCast(c_uint, (res + @as(c_int, 1))))));
            {
                var i: c_int = res;
                while (i >= @as(c_int, 0)) : (i -= 1) {
                    var ch: u8 = buffer[@intCast(c_uint, i)];
                    if ((@bitCast(c_int, @as(c_uint, ch)) == @as(c_int, '\\')) or (@bitCast(c_int, @as(c_uint, ch)) == @as(c_int, '/'))) {
                        buffer[@intCast(c_uint, (i + @as(c_int, 1)))] = @bitCast(u8, @truncate(i8, @as(c_int, 0)));
                        buffer = @ptrCast([*c]u8, @alignCast(@alignOf(u8), realloc(@ptrCast(?*c_void, buffer), @bitCast(c_uint, (i + @as(c_int, 2))))));
                        return buffer;
                    }
                }
            }
            buffer = @ptrCast([*c]u8, @alignCast(@alignOf(u8), realloc(@ptrCast(?*c_void, buffer), @bitCast(c_uint, @as(c_int, 1)))));
            buffer[@intCast(c_uint, @as(c_int, 0))] = @bitCast(u8, @truncate(i8, @as(c_int, 0)));
            return buffer;
        }
    }
    return null;
}
pub export fn concat(arg_a: [*c]const u8, arg_b: [*c]const u8) [*c]u8 {
    var a = arg_a;
    var b = arg_b;
    var lenA: usize = std.mem.len(a);
    var fin: [*c]u8 = @ptrCast([*c]u8, @alignCast(@alignOf(u8), malloc(((lenA +% std.mem.len(b)) +% @bitCast(c_uint, @as(c_int, 1))))));
    _ = strcpy(fin, a);
    _ = strcpy((fin + lenA), b);
    return fin;
}
pub export fn luaboot() void {
    var dll: ?*c_void = LoadLibraryA("lua51");
    if (!(dll != null)) {
        _ = MessageBoxA(null, "moonboot failed to find LuaJIT (lua51.dll)", "moonboot diagnostic", @as(c_int, 0));
        ExitProcess(@as(c_int, 1));
    }
    var luaL_newstate: newstate_t = @ptrCast(newstate_t, GetProcAddress(dll, "luaL_newstate"));
    var luaL_openlibs: openlibs_t = @ptrCast(openlibs_t, GetProcAddress(dll, "luaL_openlibs"));
    var luaL_loadfile: loadfile_t = @ptrCast(loadfile_t, GetProcAddress(dll, "luaL_loadfile"));
    var lua_pcall: pcall_t = @ptrCast(pcall_t, GetProcAddress(dll, "lua_pcall"));
    var L: ?*c_void = luaL_newstate.?();
    luaL_openlibs.?(L);
    var dir: [*c]u8 = findDir();
    var boot: [*c]u8 = concat(dir, "moonboot/bootloader.lua");
    _ = free(dir);
    if (luaL_loadfile.?(L, boot) != 0) {
        _ = MessageBoxA(null, "moonboot failed to load \'moonboot/bootloader.lua\'", "moonboot diagnostic", @as(c_int, 0));
        ExitProcess(@as(c_int, 1));
    }
    _ = free(boot);
    if (lua_pcall.?(L, @as(c_int, 0), -@as(c_int, 1), @as(c_int, 0)) != 0) {
        _ = MessageBoxA(null, "moonboot failed to run bootloader", "moonboot diagnostic", @as(c_int, 0));
        ExitProcess(@as(c_int, 1));
    }
}
pub export fn DllMain(arg_x: ?*c_void, arg_y: u32, arg_z: ?*c_void) callconv(.Stdcall) c_int {
    var x = arg_x;
    var y = arg_y;
    var z = arg_z;
    @"switch": {
        case: {
            switch (y) {
                @as(c_int, 1) => break :case,
                else => break :@"switch",
            }
        }
        luaboot();
        break :@"switch";
    }
    return 1;
}
