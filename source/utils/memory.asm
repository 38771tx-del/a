.code

PUBLIC ntreadvirtualmemory
ntreadvirtualmemory PROC
	mov r10, rcx
	mov eax, 63
	syscall
	ret
ntreadvirtualmemory ENDP

PUBLIC ntwritevirtualmemory
ntwritevirtualmemory PROC
	mov r10, rcx
	mov eax, 58
	syscall
	ret
ntwritevirtualmemory ENDP

PUBLIC ntallocatevirtualmemory
ntallocatevirtualmemory PROC
	mov r10, rcx
	mov eax, 24
	syscall
	ret
ntallocatevirtualmemory ENDP

PUBLIC ntfreevirtualmemory
ntfreevirtualmemory PROC
	mov r10, rcx
	mov eax, 1Bh
	syscall
	ret
ntfreevirtualmemory ENDP

END
