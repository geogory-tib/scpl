.intel_syntax noprefix
.global _start
.text
_start:
    call main
    mov rdi,rax
    mov rax,60
    syscall
test:
    push rbp
    mov rbp,rsp
    mov rax,1
    push rax
    mov rax,[rbp - 8]
    leave
    ret
main:
    push rbp
    mov rbp,rsp
    mov rbx,1
    mov rcx,2
    mov rdx,4
    imul rcx,rdx
    add rbx,rcx
    push rax
    mov rax,22
    mov rbx,4
    imul rax,rbx
    push rax
    mov rax,1
    push rax
    call test
    mov rbx,rax
    pop rax
    mov rcx,2
    imul rbx,rcx
    sub rax,rbx
    push rax
    mov rax,[rbp - 24]
    leave
    ret
