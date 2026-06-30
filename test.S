.intel_syntax noprefix
.global _start
.text
_start:
    call main
    mov rdi,rax
    mov rax,60
    syscall
main:
    push rbp
    mov rbp,rsp
    mov rax,1
    mov rbx,22
    add rax,rbx
    sub rsp,1
    mov  BYTE PTR [rbp - 1],al
    movzx rax,BYTE PTR [rbp - 1]
    push rax
    mov rax,22
    push rax
    xor rax,rax
    pop rbx
    pop rcx
    cmp rbx,rcx
   sete al
    xor al,0x01
.Llable0:
    test rax,rax
    je .Llable1
    mov rax,0
    mov  BYTE PTR [rbp - 1],al
.Llable1:
    mov rax,0
    leave
    ret
