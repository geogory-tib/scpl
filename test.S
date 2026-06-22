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
    mov rax,rdi
    sub rsp,8
    mov  QWORD PTR [rbp - 8],rax
    mov rax,rsi
    sub rsp,8
    mov  QWORD PTR [rbp - 16],rax
    mov  rax,[rbp - 16]
    xor rbx,rbx
    movzx rbx,BYTE PTR [rax]
    mov rax,rbx
    mov  rbx,[rbp - 16]
    xor rcx,rcx
    movzx rcx,BYTE PTR [rbx]
    mov rbx,rcx
    add rax,rbx
    mov  rbx,[rbp - 8]
    mov  BYTE PTR [rbx],al
    leave
    ret
main:
    push rbp
    mov rbp,rsp
    mov rax,22
    sub rsp,1
    mov  BYTE PTR [rbp - 1],al
    lea  rax,[rbp - 1]
    sub rsp,8
    mov  QWORD PTR [rbp - 9],rax
    mov  rax,[rbp - 9]
    push rax
    mov  rax,[rbp - 9]
    push rax
    pop rsi
    pop rdi
    sub rsp,7
    call test
    add rsp,7
    mov rax,rax
    mov rax,0
    leave
    ret
