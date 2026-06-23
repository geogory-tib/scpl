.intel_syntax noprefix
.global _start
.text
_start:
    call main
    mov rdi,rax
    mov rax,60
    syscall
ptr_test:
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
    mov  rbx,[rbp - 8]
    mov  BYTE PTR [rbx],al
    leave
    ret
test:
    push rbp
    mov rbp,rsp
    mov rax,rdi
    sub rsp,1
    mov  BYTE PTR [rbp - 1],al
    mov rax,rsi
    sub rsp,1
    mov  BYTE PTR [rbp - 2],al
    movzx rax,BYTE PTR [rbp - 1]
    mov rbx,2
    mov rcx,4
    add rbx,rcx
    imul rax,rbx
    sub rsp,1
    mov  BYTE PTR [rbp - 3],al
    movzx rax,BYTE PTR [rbp - 3]
    movzx rbx,BYTE PTR [rbp - 2]
    add rax,rbx
    mov  BYTE PTR [rbp - 3],al
    movzx rax,BYTE PTR [rbp - 3]
    leave
    ret
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
    mov rax,22
    sub rsp,1
    mov  BYTE PTR [rbp - 2],al
    movzx rax,BYTE PTR [rbp - 1]
    push rax
    movzx rax,BYTE PTR [rbp - 2]
    push rax
    pop rsi
    pop rdi
    sub rsp,14
    call test
    add rsp,14
    mov rax,rax
    lea  rax,[rbp - 1]
    push rax
    lea  rax,[rbp - 2]
    push rax
    pop rsi
    pop rdi
    sub rsp,14
    call ptr_test
    add rsp,14
    mov rax,rax
    mov rax,0
    leave
    ret
