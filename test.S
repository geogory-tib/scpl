.intel_syntax noprefix
.global _start
.text
_start:
    call main
    mov rdi,rax
    mov rax,60
    syscall
add:
    push rbp
    mov rbp,rsp
    mov rax,rdi
    sub rsp,1
    mov  BYTE PTR [rbp - 8],al
    mov rax,rsi
    sub rsp,1
    mov  BYTE PTR [rbp - 9],al
    movzx rax,BYTE PTR [rbp - 8]
    movzx rbx,BYTE PTR [rbp - 9]
    add rax,rbx
    leave
    ret
main:
    push rbp
    mov rbp,rsp
    mov rax,22
    mov rbx,1
    add rax,rbx
    push rax
    mov rax,4
    push rax
    pop rsi
    pop rdi
    call add
    add rsp,0
    mov rax,rax
    sub rsp,1
    mov  BYTE PTR [rbp - 8],al
    mov rax,0
    leave
    ret
