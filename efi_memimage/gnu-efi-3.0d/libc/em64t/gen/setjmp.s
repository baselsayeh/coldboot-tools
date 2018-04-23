;
; Copyright (c) 1999, 2000
; Intel Corporation.
; All rights reserved.
; 
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions
; are met:
; 
; 1. Redistributions of source code must retain the above copyright
;    notice, this list of conditions and the following disclaimer.
; 
; 2. Redistributions in binary form must reproduce the above copyright
;   notice, this list of conditions and the following disclaimer in the
;    documentation and/or other materials provided with the distributio
; 
; 3. All advertising materials mentioning features or use of this softw
;    must display the following acknowledgement:
; 
;    This product includes software developed by Intel Corporation and
;    its contributors.
; 
; 4. Neither the name of Intel Corporation or its contributors may be
;    used to endorse or promote products derived from this software
;    without specific prior written permission.
; 
; THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS 
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PU
; ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS 
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSIN
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWIS
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED O
; THE POSSIBILITY OF SUCH DAMAGE.
; 
;

;
; Module Name:
;
;  setjmp.s
;
; Abstract:
;
;  Contains an implementation of setjmp and longjmp for EM64T.



; int setjmp(jmp_buf buf)
;
;	Setup a non-local goto.
;
; Description:
;
;	SetJump stores the current register set in the area pointed to
;	by "save".  It returns zero.  Subsequent calls to "LongJump" will
;	restore the registers and return non-zero to the same location.
;
; 	buf - RCX
;

PUBLIC setjmp
_TEXT	SEGMENT
setjmp	PROC

	; save rbx, rbp, r12 ~ r15
        mov	QWORD PTR [rcx]     , rbx
	mov	QWORD PTR [rcx + 8 ], rbp

	mov	QWORD PTR [rcx + 16], r12
        mov	QWORD PTR [rcx + 24], r13
	mov	QWORD PTR [rcx + 32], r14
	mov	QWORD PTR [rcx + 40], r15

       ; save rsp to volatile register rax
       mov rax, rsp

       ; ajust rax
       add rax, 8
       
	; save rsp
	mov	QWORD PTR [rcx + 48], rax

	; save rip
	mov 	rdx, [rsp]
	mov	QWORD PTR [rcx + 56], rdx

	; save rdi, rsi
	mov 	QWORD PTR [rcx + 64], rdi
	mov	QWORD PTR [rcx + 72], rsi

	; return 0
	xor	rax, rax

	ret

setjmp	ENDP
_TEXT	ENDS


;
; void longjmp(jmp_buf buf, int val)
;
;	Perform a non-local goto.
;
; Description:
;
;	LongJump initializes the register set to the values saved by a
;	previous 'SetJump' and jumps to the return location saved by that
;	'SetJump'.  This has the effect of unwinding the stack and returning
;	for a second time to the 'SetJump'.
;
;	buf - RCX
;	val - RDX
;

PUBLIC longjmp
_TEXT	SEGMENT
longjmp	PROC

	; rbx, rbp and rx
	mov 	rbx, QWORD PTR [rcx]
	mov 	rbp, QWORD PTR [rcx + 8]
	mov 	r12, QWORD PTR [rcx + 16]
	mov 	r13, QWORD PTR [rcx + 24]
	mov 	r14, QWORD PTR [rcx + 32]
	mov 	r15, QWORD PTR [rcx + 40]

	; check 'val':the second-time return value of 'setjmp'
	cmp 	rdx, 0
	jne 	longjmp_cmp_val
	xor	rax, rax
	mov 	rax, 1

longjmp_cmp_val:
	; rsp
	mov 	rsp, QWORD PTR [rcx + 48]

	; rdi and rsi
	mov 	rdi, QWORD PTR [rcx + 64]
	mov 	rsi, QWORD PTR [rcx + 72]

	;return to 'setjmp'
	jmp QWORD PTR [rcx + 56]

longjmp	ENDP
_TEXT	ENDS

END
