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
    mov rbx,2
    mov rcx,2
    imul rbx,rcx
    add rax,rbx
    mov rbx,4
    imul rax,rbx
    sub rsp,1
    mov  BYTE PTR [rbp - 1],al
    mov rax,0
    leave
    ret
