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
    push 1
    push 2
    push 4
    pop rbx
    pop rax
    imul rax,rbx
    push rax
    pop rbx
    pop rax
    add rax,rbx
    push rax
    pop rax
    push rax
    push 22
    push 4
    pop rbx
    pop rax
    imul rax,rbx
    push rax
    pop rax
    push rax
    mov rax,[rbp - 16]
    leave
    ret
