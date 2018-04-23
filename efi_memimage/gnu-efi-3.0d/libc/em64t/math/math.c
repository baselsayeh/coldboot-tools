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
    return Operand << Count;
}

UINT64
LIBC_RShiftU64 (
    IN UINT64   Operand,
    IN UINTN    Count
    )
// Right shift 64bit by 32bit and get a 64bit result
{
    return Operand >> Count;
}


UINT64
LIBC_MultU64x32 (
    IN UINT64   Multiplicand,
    IN UINTN    Multiplier
    )
// Multiple 64bit by 32bit and get a 64bit result
{
    return Multiplicand * Multiplier;
}

UINT64 
LIBC_MulU64x64 (
  UINT64 Value1, 
  UINT64 Value2, 
  UINT64 *ResultHigh
  )
// Multiple 64bit by 32bit and get a 64bit result
{
    return Value1 * Value2;
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
    if (Remainder) {
        *Remainder = Dividend % Divisor;
    }

    return Dividend / Divisor;
}

UINT64
LIBC_DivU64x64 (
  UINT64 Dividend,
  UINT64 Divisor,
  UINT64 *Remainder OPTIONAL,
  UINT32 *Error
  )
/*++

Routine Description:
  
  Divide two 64-bit unsigned values.

Arguments:

  Dividend  - dividend
  Divisor    - divisor
  Remainder - remainder of dividend/divisor
  Error     - to flag errors (divide-by-0)

Returns:

  dividend / divisor

Note:

  The 64-bit remainder is in *Remainder and the quotient is the return value.
  *Error = 1 if the divisor is 0, and it is 1 otherwise

--*/
{
  UINT64 Result;
  
  *Error = 0;

  if (Divisor == 0x0) {
    *Error = 1;
    Result = 0x8000000000000000;
    if(Remainder){
           *Remainder = 0x8000000000000000;
    	}
  } else {
    Result = Dividend / Divisor;
   if (Remainder){
           *Remainder = Dividend - Result * Divisor;
   	}
  }
  return Result;
}

INT64
LIBC_DivS64x64 (
  IN INT64 Dividend,
  IN INT64 Divisor,
  OUT INT64 *Remainder OPTIONAL,
  OUT UINT32 *Error
  )
/*++

Routine Description:
  
  Divide two 64-bit signed values.

Arguments:

  Dividend - dividend
  Divisor    - divisor
  Remainder - remainder of Dividend/Divisor
  Error     - to flag errors (divide-by-0)

Returns:

  Dividend / Divisor

Note:

  The 64-bit remainder is in *Remainder and the quotient is the return value.
  *Error = 1 if the divisor is 0, and it is 1 otherwise

--*/
{
  INT64 Result;
  
  *Error = 0;

  if (Divisor == 0x0) {
    *Error = 1;
    Result = 0x8000000000000000;
    if(Remainder){
          *Remainder = 0x8000000000000000;
	}
  } else {
    Result = Dividend / Divisor;
    if (Remainder){
        *Remainder = Dividend - Result * Divisor;
    }
  }
  return Result;
}

