    .text
    .globl syscall

syscall:
	// Disposition des parametres :
	//   1) call (int) : %esp + 24
	//   2) par1 (int) : %esp + 28
	//   3) par2 (int) : %esp + 32
	//   4) par3 (int) : %esp + 36
	//   5) par4 (int) : %esp + 40
	//   6) par5 (int) : %esp + 44

	
	// Sauvegarde des registres
	pushl %esp
	pushl %ebx
	pushl %esi
	pushl %edi
	pushl %ebp

	// Passage des parametres
	movl 24(%esp), %eax
	movl 28(%esp), %ebx
	movl 32(%esp), %ecx
	movl 36(%esp), %edx
	movl 40(%esp), %edi
	movl 44(%esp), %esi
	
	// Appel Traitant
	int $49

	// Restauration des registres
	popl %ebp
	popl %edi
	popl %esi
	popl %ebx
	popl %esp

	// Retour appel système
	ret
