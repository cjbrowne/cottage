.section .text

.global _start
_start:
	# set up end of the stack frame linked list
	movq $0, %rbp
	pushq %rbp # rip=0
	pushq %rbp # rbp=0
	movq %rsp, %rbp

	# we need those in a moment when we call main
	pushq %rsi
	pushq %rdi

	# prepare signals, memory allocation, stdio and such
	call libc_init

	# run the global constructors
	call _init

	# restore argc and argv
	popq %rdi
	popq %rsi

	# run main
	call main

	# terminate the process with the exit code
	movl %eax, %edi
	call exit
.size _start, . - _start

