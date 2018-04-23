ENTRY(boot)
SECTIONS
{
  . = 0x7c00;
  .text :
  {
   *(.text)
   . = ALIGN(64);
  }
  .data :
  {
   *(.data)
   *(.data*)
   *(.rdata)
   *(.rodata)
   *(.rodata*)
   *(.idata)
   . = ALIGN(64);
  }
  .bss :
  {
   *(.bss)
   *(.bss*)
  }
  /DISCARD/ :
  {
  *(.note)
  *(.note*)
  *(.comment)
  }
}
