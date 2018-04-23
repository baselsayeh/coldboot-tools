/*++

Copyright (c) 2006  Intel Corporation

Module Name:

    hw.c

Abstract:

    Debug library functions for Hardware IO access



Revision History

--*/
#include "efi.h"
#include "efistdarg.h"

#define EFI_ARG_NUM_MAX 10
#define EFI_REG_ARG_NUM 4

/* Convert SysV calling convention to EFI x86_64 calling convention */

EFI_STATUS uefi_call_wrapper(void *fp, unsigned long va_num, ...)
{
	va_list ap;
	int i;
	unsigned long args[EFI_ARG_NUM_MAX];
	unsigned int arg_size,stack_adjust_size;
	EFI_STATUS status;

	if (va_num > EFI_ARG_NUM_MAX || va_num<0)	{
		return EFI_LOAD_ERROR;
	}
	if (va_num==0)
		/* There is no need to convert arguments for void argument. */
		__asm__ __volatile__("call *%0;ret;"::"r"(fp));

	/* The EFI arguments is stored in an array. Then later on it will be 
	 * pushed into stack or passed to registers according to MS ABI.
	 */
	va_start(ap, va_num);
	for (i = 0; i < va_num; i++) {
		args[i] = va_arg(ap, unsigned long);
	}
	va_end(ap);
	arg_size = va_num*8;
	stack_adjust_size = (va_num > EFI_REG_ARG_NUM? EFI_REG_ARG_NUM : va_num)*8;

	/* Starting from here, assembly code makes sure all registers used are
	 * under controlled by our code itself instead of by gcc.
	 */
	/* Start converting SysV calling convention to MS calling convention. */
	__asm__ __volatile__(
		/* 0. Save preserved registers. EFI call may clobbered them. */
		"		pushq %%rbp;pushq %%rbx;pushq %%r12;"
		"		pushq %%r13;pushq %%r14;pushq %%r15;"
		/* 1. Push arguments passed by stack into stack. */
		"		mov %1, %%r12;"
		"		mov %3, %%r13;"
		"		mov %1, %%rax;"
		"		dec %%rax;"
		"		mov $8, %%bl;"
		"		mul %%bl;"
		"		add %%rax, %%r13;"
		"lstack:"
		"		cmp $4, %%r12;"
		"		jle lregister;" 
		"		pushq (%%r13);"
		"		sub $8, %%r13;"
		"		dec %%r12;" 
		"		jmp lstack;"
		/* 2. Move arguments passed by registers into registers.
		 *    rdi->rcx, rsi->rdx, rdx->r8, rcx->r9.
		 */
		"lregister:"	
		"		mov %3, %%r14;"
		"		mov $0, %%r12;"
		"lloadregister:"
		"		cmp %1, %%r12;"
		"		jge lcall;"
		"		mov (%%r14), %%rcx;"
		"		inc %%r12;"
		"		cmp %1, %%r12;"
		"		jge lcall;"
		"		mov 8(%%r14), %%rdx;"
		"		inc %%r12;"
		"		cmp %1, %%r12;"
		"		jge lcall;"
		"		mov 0x10(%%r14), %%r8;"
		"		inc %%r12;"
		"		cmp %1, %%r12;"
		"		jge lcall;"
		"		mov 0x18(%%r14), %%r9;"
		/* 3. Save stack space for those register arguments. */
		"lcall:				"
		"		sub %2, %%rsp;"
		/* 4. Save arg_size to r12 which is preserved in EFI call. */
		"		mov %4, %%r12;"
		/* 5. Call EFI function. */
		"		call *%5;"
		/* This code was not there before */
		"		mov %%rax, %0;"
		/* 6. Restore stack space reserved for those register
		 * arguments.
		 */
		"		add %%r12, %%rsp;"
		/* 7. Restore preserved registers. */
		"		popq %%r15;popq %%r14;popq %%r13;"
		"		popq %%r12;popq %%rbx;popq %%rbp;"
		:"=r"(status)
		:"r"((unsigned long)va_num),
		 "r"((unsigned long)stack_adjust_size),
		 "r"(args),
		 "r"((unsigned long)arg_size),
		 "r"(fp)
		:"rsp","rbx","rax","r11","r12","r13","r14","rcx","rdx","r8","r9" 
		);
	return status;
}
