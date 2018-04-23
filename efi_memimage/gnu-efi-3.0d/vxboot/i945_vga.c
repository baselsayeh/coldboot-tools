#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "i945_reg.h"
#include "video_subr.h"
#include "io.h"
#include "vga.h"

#define CSR_WRITE_4(x, y, z)	\
	*(UINT32 *)(x + y) = z

#define MODE(x, y)		\
	(((x) << 16) | (y & 0xFFFF))
void
enable_vga(base)
	UINT32		base;
{
	/* Disable the pipe. */

	CSR_WRITE_4(base, PIPEACONF, 0);

	/* Reset clocks. */

	CSR_WRITE_4(base, DPLL_A, 0x84800003);
	CSR_WRITE_4(base, DPLL_B, 0x04800003);
	CSR_WRITE_4(base, DPLL_A_MD, 0);
	CSR_WRITE_4(base, FPA0, 0x31108);
	CSR_WRITE_4(base, FPA1, 0x31108);
	CSR_WRITE_4(base, FPB0, 0x31108);
	CSR_WRITE_4(base, FPB1, 0x31108);

	CSR_WRITE_4(base, HTOTAL_A, MODE(799, 639));
	CSR_WRITE_4(base, HBLANK_A, MODE(791, 647));
	CSR_WRITE_4(base, HSYNC_A, MODE(751, 655));
	CSR_WRITE_4(base, VTOTAL_A, MODE(524, 479));
	CSR_WRITE_4(base, VBLANK_A, MODE(516, 487));
	CSR_WRITE_4(base, VSYNC_A, MODE(491, 489));
	CSR_WRITE_4(base, PIPEASRC, 0);
	CSR_WRITE_4(base, BCLRPAT_A, MODE(639, 479));

	/* Reset display size. */

	CSR_WRITE_4(base, DSPACNTR, 0);
	CSR_WRITE_4(base, DSPABASE, 0);
	CSR_WRITE_4(base, DSPASTRIDE, 0);
	CSR_WRITE_4(base, DSPASIZE, 0);
	CSR_WRITE_4(base, DSPACNTR, 0);
	CSR_WRITE_4(base, DSPAKEYVAL, 0);
	CSR_WRITE_4(base, DSPASURF, 0);

	/* Allow a little time for clocks to stabilize. */

	sleep (1);

	/* Re-enable VGA display planes. */

	CSR_WRITE_4(base, VGACNTRL, 0x0020004e);

	/* Re-enable pipe. */

	CSR_WRITE_4(base, PIPEACONF, 0x80000000);

	/* Now perform normal VGA initialization */

        vga_init();
        vga_font_load((unsigned char *)0xa0000, fontdata_8x16, 16, 255);

	return;
}
