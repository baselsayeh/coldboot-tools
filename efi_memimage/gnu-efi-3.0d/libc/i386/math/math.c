/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    math.c

Abstract:

Revision History

--*/

#include "efi_interface.h"

UINT64
LIBC_LShiftU64 (
    IN UINT64   Operand,
    IN UINTN    Count
    )
// Left shift 64bit by 32bit and get a 64bit result
{
    return (LShiftU64(Operand, Count));
#ifdef notdef
    UINT64      Result;

    _asm {
        mov     eax, dword ptr Operand[0]
        mov     edx, dword ptr Operand[4]
        mov     ecx, Count
        and     ecx, 63

        shld    edx, eax, cl
        shl     eax, cl

        cmp     ecx, 32
        jc      short ls10

        mov     edx, eax
        xor     eax, eax

ls10:
        mov     dword ptr Result[0], eax
        mov     dword ptr Result[4], edx
    }

    return Result;
#endif
}

UINT64
LIBC_RShiftU64 (
    IN UINT64   Operand,
    IN UINTN    Count
    )
// Right shift 64bit by 32bit and get a 64bit result
{
    return (RShiftU64(Operand, Count));
#ifdef notdef
    UINT64      Result;

    _asm {
        mov     eax, dword ptr Operand[0]
        mov     edx, dword ptr Operand[4]
        mov     ecx, Count
        and     ecx, 63

        shrd    eax, edx, cl
        shr     edx, cl

        cmp     ecx, 32
        jc      short rs10

        mov     eax, edx
        xor     edx, edx

rs10:
        mov     dword ptr Result[0], eax
        mov     dword ptr Result[4], edx
    }

    return Result;
#endif
}


UINT64
LIBC_MultU64x32 (
    IN UINT64   Multiplicand,
    IN UINTN    Multiplier
    )
// Multiple 64bit by 32bit and get a 64bit result
{
    return (MultU64x32(Multiplicand, Multiplier));
#ifdef notdef
    UINT64      Result;

    _asm {
        mov     eax, dword ptr Multiplicand[0]
        mul     Multiplier
        mov     dword ptr Result[0], eax
        mov     dword ptr Result[4], edx
        mov     eax, dword ptr Multiplicand[4]
        mul     Multiplier
        add     dword ptr Result[4], eax
    }

    return Result;
#endif
}

UINT64
LIBC_DivU64x32 (
    IN UINT64   Dividend,
    IN UINTN    Divisor,
    OUT UINTN   *Remainder OPTIONAL
    )
// divide 64bit by 32bit and get a 64bit result
// N.B. only works for 31bit divisors!!
{
    return (DivU64x32 (Dividend, Divisor, Remainder));
#ifdef notdef
    UINT32      Rem;
    UINT32      bit;        

    //
    // For each bit in the dividend
    //

    Rem = 0;
    for (bit=0; bit < 64; bit++) {
        _asm {
            shl     dword ptr Dividend[0], 1    ; shift rem:dividend left one
            rcl     dword ptr Dividend[4], 1    
            rcl     dword ptr Rem, 1            

            mov     eax, Rem
            cmp     eax, Divisor                ; Is Rem >= Divisor?
            cmc                                 ; No - do nothing
            sbb     eax, eax                    ; Else, 
            sub     dword ptr Dividend[0], eax  ;   set low bit in dividen
            and     eax, Divisor                ; and
            sub     Rem, eax                    ;   subtract divisor 
        }
    }

    if (Remainder) {
        *Remainder = Rem;
    }

    return Dividend;
#endif
}

UINT64 
LIBC_MulU64x64 (
  UINT64 Value1, 
  UINT64 Value2, 
  UINT64 *ResultHigh
  )
/*++

Routine Description:
  
  Multiply two unsigned 64-bit values.

Arguments:

  Value1      - first value to multiply
  Value2      - value to multiply by Value1
  ResultHigh  - result to flag overflows

Returns:

  Value1 * Value2

Note:

  The 128-bit result is the concatenation of *ResultHigh and the return value 
  The product fits in 64 bits if *ResultHigh == 0x0000000000000000

  Method 1. Use four 32-bit multiplications:
      a * b = 2^64 * a_h * b_h + 2^32 * (a_h * b_l + b_h * a_l) + a_l * b_l
  Method 2. An alternative using only three multiplications is:
      a * b = (2^64 + 2^32) * a_h * b_h + 
               2^32 * (a_h - a_l) * (b_h - b_l) + (2^32 + 1) * a_l * b_l
  The first method was implemented, because of the overhead in the second one

--*/
{
return (Value1 * Value2);
#ifdef notdef
  UINT64  Result;
  UINT64  ResHi;
  UINT32  SavedEbx;
  
  _asm {
    mov dword ptr SavedEbx, ebx     // save across function calls
    mov eax, dword ptr Value1[0] // a_l -> eax
    mov ebx, dword ptr Value2[0] // b_l -> ebx
    mul ebx // a_lxb_l -> edx:eax
    mov dword ptr Result[0], eax // (a_lxb_l)_LOW -> LL; done LL
    mov ecx, edx // (a_lxb_l)_HIGH -> ecx
    mov eax, dword ptr Value1[4] // a_h -> eax (b_l is in ebx)
    mul ebx // a_hxb_l -> edx:eax
    add eax, ecx // (a_lxb_l)_HIGH + (a_hxb_l)_LOW -> CF:eax
    adc edx, 0x0 // (a_lxb_l)_HIGH + CF -> edx (no carry over, so CF = 0)
    mov dword ptr ResHi[0], edx // (a_hxb_l)_HIGH + CF -> HL; partial result
    mov ecx, eax // (a_lxb_l)_HIGH + (a_hxb_l)_LOW -> ecx
    mov eax, dword ptr Value1[0] // a_l -> eax
    mov ebx, dword ptr Value2[4] // b_h -> ebx
    mul ebx // b_hxa_l -> edx:eax
    add eax, ecx // (a_lxb_l)_HIGH + (a_hxb_l)_LOW + (b_hxa_l)_LOW -> CF:eax
    mov dword ptr Result[4], eax // eax -> LH; done LH
    adc edx, 0x0 // add CF to (b_hxa_l)_HIGH (no carry over, so CF = 0)
    mov ecx, edx // (b_hxa_l)_HIGH -> ecx
    mov eax, dword ptr Value1[4] // a_h -> eax (b_h is in ebx)
    mul ebx // a_hxb_h -> edx:eax
    add eax, ecx // (a_hxb_h)_LOW + (b_hxa_l)_HIGH -> CF:eax
    adc edx, 0x0 // (a_hxb_h)_HIGH + CF -> edx (no carry over, so CF = 0)
    add dword ptr ResHi[0], eax // HL; done HL
    adc edx, 0x0
    mov dword ptr ResHi[4], edx // HH; done HH
    mov ebx, dword ptr SavedEbx
  }

  *ResultHigh = ResHi;
  return (Result);
#endif
}

UINT64
LIBC_DivU64x64 (
  IN  UINT64   Dividend,
  IN  UINT64   Divisor,
  OUT UINT64   *Remainder OPTIONAL,
  OUT UINT32   *Error
  )
{
#ifdef notdef
  UINT64      Rem;
  UINT64      Bit;        
#endif

  *Error = 0;
  if (Divisor == 0) {
    *Error = 1;
    if (Remainder) {
      *Remainder = 0x8000000000000000ULL;
    }
    return 0x8000000000000000ULL;
  }

  return (DivU64x32 (Dividend, Divisor, Remainder));

#ifdef notdef
  //
  // For each bit in the dividend
  //

  Rem = 0;
  for (Bit=0; Bit < 64; Bit++) {
      _asm {
      shl     dword ptr Dividend[0], 1    ; shift dividend left one
      rcl     dword ptr Dividend[4], 1    
      rcl     dword ptr Rem[0], 1         ; shift rem left one
      rcl     dword ptr Rem[4], 1    
      }
      if (Rem >= Divisor) {
        Dividend |= 1;
        Rem -= Divisor;
      }
  }

  if (Remainder) {
    *Remainder = Rem;
  }

  return Dividend;
#endif


}

INT64
LIBC_DivS64x64 (
  IN  INT64   Dividend,
  IN  INT64   Divisor,
  OUT INT64   *Remainder OPTIONAL,
  OUT UINT32  *Error
  )
{
  UINT64   Quotient;
  BOOLEAN  Negative;
  BOOLEAN  RemainderNegative;

  Negative = FALSE;
  RemainderNegative = FALSE;
  if (Dividend < 0) {
    Dividend = -Dividend;
    Negative = (BOOLEAN)!Negative;
    RemainderNegative = TRUE;
  }
  if (Divisor < 0) {
    Divisor = -Divisor;
    Negative = (BOOLEAN)!Negative;
  }
  Quotient = LIBC_DivU64x64 ((UINT64)Dividend, (UINT64)Divisor, (INT64 *)Remainder, Error);
  if (*Error) {
    return (INT64)Quotient;
  }
  if (Negative) {
    Quotient = -((INT64)Quotient);
  }
  if (RemainderNegative) {
    if (Remainder) {
      *Remainder = -*Remainder;
    }
  }
  return (INT64)Quotient;
}

