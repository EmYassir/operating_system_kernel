	.text
	.globl ctx_sw
	// Structure de la pile en entree :
	//   %esp + 4 : adresse de l'ancien contexte
	//   %esp + 8 : adresse du nouveau contexte
ctx_sw:
	//==========================================
	//    0 |   1  |   2  |   3  |   4  |   5  |
	//==========================================
	//  %esp|  %ebx| %esi |  %edi|  %ebp| %flag|

	
	// Sauvegarde de l'ancien contexte
	mov 4(%esp), %ecx
	mov %esp,(%ecx)
	mov %ebx,4(%ecx)
	mov %esi,8(%ecx)
	mov %edi,12(%ecx)
	mov %ebp,16(%ecx)
	pushfl
	popl 20(%ecx)

	// Restauration du nouveau contexte
	mov 8(%esp), %ecx
	mov (%ecx), %esp
	mov 4(%ecx), %ebx
	mov 8(%ecx), %esi
	mov 12(%ecx), %edi
	mov 16(%ecx), %ebp
	pushl 20(%ecx)
	popfl
	    // Activation du nouveau processus :
	ret
