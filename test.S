.intel_syntax noprefix
.global _start
.text
_start:
    call main
    mov rdi,rax
    mov rax,60
    syscall
fib:
    push rbp
    mov rbp,rsp
    mov rax,rdi
    sub rsp,8
    mov  QWORD PTR [rbp - 8],rax
    mov rax,rsi
    sub rsp,8
    mov  QWORD PTR [rbp - 16],rax
    mov rax,rdx
    sub rsp,8
    mov  QWORD PTR [rbp - 24],rax
    mov rax,rcx
    sub rsp,8
    mov  QWORD PTR [rbp - 32],rax
    mov  rax,[rbp - 8]
    sub rsp,8
    mov  QWORD PTR [rbp - 40],rax
    mov  rax,[rbp - 16]
    sub rsp,8
    mov  QWORD PTR [rbp - 48],rax
    mov  rax,[rbp - 40]
    mov  rbx,[rbp - 48]
    add rax,rbx
    sub rsp,8
    mov  QWORD PTR [rbp - 56],rax
    mov  rax,[rbp - 24]
    push rax
    mov  rax,[rbp - 32]
    push rax
    pop rbx
    pop rax
    cmp rax,rbx
 sete al
.Llable0:
    jne .Llable1
    mov  rax,[rbp - 56]
    leave
    ret
.Llable1:
    mov  rax,[rbp - 48]
    push rax
    mov  rax,[rbp - 56]
    push rax
    mov  rax,[rbp - 24]
    mov rbx,1
    add rax,rbx
    push rax
    mov  rax,[rbp - 32]
    push rax
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    sub rsp,8
    call fib
    add rsp,8
    mov rax,rax
    leave
    ret
main:
    push rbp
    mov rbp,rsp
    mov rax,0
    push rax
    mov rax,1
    push rax
    mov rax,0
    push rax
    mov rax,3
    push rax
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    call fib
    add rsp,0
    mov rax,rax
    mov rax,0
    leave
    ret
