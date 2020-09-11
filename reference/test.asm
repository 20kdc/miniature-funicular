; nasm test.asm -l test.lst
bits 32
org 0
call aftercall
aftercall:
; simplifies the maths a little: everything is origin-relative
sub dword [esp], aftercall
; push text
mov eax, [esp]
add eax, text
push eax
; resolve LoadLibraryA address VA, read it & call it
; translation from RVA->VA is assisted by relativity
mov eax, [esp + 4]
add eax, 0x80808080
mov eax, [eax]
call eax
; resolve entrypoint RVA and jump to it
pop eax
add eax, 0x80808080
jmp eax
; target library is appended 
text:

