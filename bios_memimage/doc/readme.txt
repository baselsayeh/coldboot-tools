This distribution contains proof of concept utilities for capturing
memory dumps from Intel x86-based and AMD/Intel x86-64 based PC
systems. It turns out that many systems manufactured today exhibit RAM
persistence; that is, the contents of RAM survive for brief periods
even after it's been powered off. This phenomenon can be exploited for
various purposes, both good and evil.

RAM persistence can be exploited using both hardware and software mechanisms.
One major drawback to hardware exploits is that they require a certain
amount of specialized expertise and a willingness and/or opportunity to
disassemble or possibly damage the system being exploited. With many
systems, there is no alternative to this, particularly with computers
that perform destructive memory tests or ECC scrubbing at startup. However,
there are a surprising number of machines where the contents of RAM survive
undamaged well after the system BIOS or boot code has finished running,
and these can be exploited much more easily using only software.

Most PC systems available today support booting over the network using
PXE (the Preboot eXecution Environment) or from USB mass storage
devices. Systems with PXE support use DHCP to request an IP address
and can then use TFTP to download an initial bootstrap program. This
program in turn can make use of PXE services to download additional
files from the network. This is possible because the PXE ROM also
includes a very simple UDP/IP implementation and an UNDI driver for
the underlying hardware: the presence of the UNDI driver in ROM
eliminates the need for native driver support in the bootstrap
program, which allows a single bootstrap loader image to run on any
PXE-enabled system.

When booting from a USB device, the BIOS simply treats the attached
mass storage device (which can be a standard disk drive or a flash
memory stick) as a boot disk drive, allowing the system to be booted from
the USB disk instead of the internal disk.

The PXE MEMORY SCRAPER is a standalone program which is downloaded
into a target computer via PXE and uses PXE's networking capabilities
to allow the target to be controlled by a remote utility. The utility
sends commands to the PXE scraper requesting it to transmit blocks of
the target's memory over the network and then writes the blocks out to
disk. This allows the remote utility to obtain a complete dump of the
target's RAM, after which it sends another command telling the PXE
scraper to reboot or shut down the target.

The USB SCRAPER is a similar utility, but it dumps the contents of RAM
to an attached disk device instead of to the network. The same USB
storage device used to hold the scraper itself is also used to contain
the memory dump. The contents of the dump can be retrieved later using
a separate utility.

Both scrapers are written mostly in C, with a few pieces written in i386
or x86-64 assembly. The client side utilities are written entirely in C
and use either simple socket calls for communicating with the target, or
stdio operations to read from the disk holding a captured memory dump.
One of the challenges in implementing the scraper itself is that it needs
to run in both protected mode and real mode at various times. The CPU must
run in protected mode in order to access all 4GB of the address space.
But the scraper needs to make calls to the BIOS and to PXE, both of which
can only be done in real mode. (The PXE specification allows for a
protected mode API as well, however most PXE implementations don't seem
to support one.) A similar problem exists with x86-64, but there one must
transition between real mode and 64-bit long mode instead.

There are two ways to solve this problem: one is to use vm86 mode and the
other is to briefly 'thunk' from protected mode or long mode to real mode
and then back again. Using vm86 is required in order to safely execute
real mode code in a protected mode OS environment, since it provides the
ability to prevent buggy or malicious real mode code from crashing the
entire system. But the scrapers are implemented as a standalone program
rather than an OS, so we can get away with using the thunking method instead,
which is much simpler to use than vm86.

The thunking scheme used here was originally inspired by the Mach OS
bootloader, which was also briefly used on FreeBSD. (There came a point
during FreeBSD development when the kernel would no longer fit in the
lower 640K of RAM. The Mach loader used the protected/real mode thunking
technique to allow the kernel image to be loaded into upper memory while
still providing access to the BIOS disk services.) While the process
of switching an x86 CPU into protected mode is widely known, most software
that enters protected mode never leaves it again. (With the 80286
processor, it was not actually possible to return to real mode without
completely resetting the CPU).

While switching back to real mode is not commonly done, it's not that
difficult. In order to do it, the global descriptor table must be
loaded with both 32-bit code and data descriptors and 16-bit code
and data descriptors. Returning to real mode is actually a two stage
process. Software must first perform an intersegment jump in order to
switch to the 16-bit code segment, so that it enters 16 bit addressing
mode. At that point, the segment selector registers can be set to select
the 16-bit data segment and the PE bit in the CR0 register can be cleared.
Finally, software must perform one more intersegment jump to GDT segment 0,
which completes the transition back to real mode. From this point, software
can continue running in real mode, or switch the CPU back into protected
mode again if desired.

For x86-64 systems, once we get into protected mode, we then also have
to switch to long mode. This also requires the use of paging, so page
tables are needed as well.

One complication here is that the tools used to build the PXE scraper
must be capable of producing both real mode and protected mode code.
The GNU C compiler always generates 32-bit protected mode assembly code,
so any code written in C must execute in protected mode. Recent versions
of the GNU assembler can be switched between 16 bit real mode and
32-bit protected mode code generation through the use of the .code16
and .code32 assembler directives. While it's also possible to use these
directives from the C compiler (in conjunction with inline assembler),
this mechanism is somewhat error prone, as well as unwieldy. For this
for this reason, the PXE scraper is written such that the routines that
must run in real mode are coded separately in assembler. This is limited
mainly to the routines that make calls to the BIOS and the pxe_call()
function used to access the PXE UDP and UNDI APIs.

For x86-64, a 64-bit version of the GNU assembler supports a .code64
directive as well as .code32 and .code16, allowing long mode, protected
mode and real mode code to be combined. A 64-bit version of GCC will
only generate 64-bit code, however.

Note that there are other tools available which could be used instead
of GCC/GAS, such as the Borland C compiler suite (which is now open
source). However, the GNU tools have one significant advantage over other
tools, which is that they are readily available: if you already have a
FreeBSD or Linux host, GCC and GAS are already installed. (For those who
insist on using Windows, the Cygwin or MinGW GCC tools can be used to
build the 32-bit versions of the scrapers too.)
