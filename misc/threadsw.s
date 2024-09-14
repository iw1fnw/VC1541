.global switch_from_to__10ThreadInitPPUlPUl
.global _switch_from_to__10ThreadInitPPUlPUl

switch_from_to__10ThreadInitPPUlPUl:
_switch_from_to__10ThreadInitPPUlPUl:

		#pushl	$0xdeadbeef
		pushl	%eax
		pushl	%ebx
		pushl	%ecx
		pushl	%edx
		pushl	%ebp
		pushl	%esi
		pushl	%edi
		pushfl

		#subl $108, %esp
		#fwait
		#fsave (%esp)
		#fwait

		#pushl	$0x12345678

		movl	36(%esp), %ebx
		movl	40(%esp), %ecx

		movl	%esp, (%ebx)
		movl	%ecx, %esp

		#popl	%eax

		#fwait
		#frstor (%esp)
		#fwait
		#addl $108, %esp

		popl	%eax
		andb	$0xfe, %ah
		pushl	%eax

		popfl
		popl	%edi
		popl	%esi
		popl	%ebp
		popl	%edx
		popl	%ecx
		popl	%ebx
		popl	%eax
		#addl	$4, %esp

		ret
