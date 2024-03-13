global _start
section .text
extern show_registers
_start:
push 60
push 10
pop rcx
pop rax
sub rax, rcx
push rax
push 5
pop rcx
pop rax
sub rax, rcx
push rax
push 4
pop rcx
pop rax
sub rax, rcx
push rax
pop rbx
call show_registers
mov rax, 60
mov rdi, 0
syscall
