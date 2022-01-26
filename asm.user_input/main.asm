section .data
  your_name db "What is your name? ", 0
  your_name_size db 20
  hello db "Hello, ", 0
  hello_size db 8


section .text
  global _start


_start:

  call print_name
  call print_hello

  mov rax, 60
  mov rdi, 0
  syscall


print_name:
  mov rax, 1
  mov rdi, 1
  mov rsi, your_name
  mov rdx, your_name_size
  syscall
  ret

print_hello:
  mov rax, 1
  mov rdi, 1
  mov rsi, hello
  mov rdx, [hello_size]
  syscall
  ret

