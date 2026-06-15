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
    mov  rax,[rbp - 8]
    leave
    ret
main:
    push rbp
    mov rbp,rsp
    mov rax,1
    mov rbx,2
    mov rcx,4
    imul rbx,rcx
    add rax,rbx
    sub rsp,1
    mov  BYTE PTR [rbp - 8],al
    mov rax,0
    mov  BYTE PTR [rbp - 8],al
    sub rsp,15
    call main
    add rsp,15
    mov rax,rax
    leave
    ret
