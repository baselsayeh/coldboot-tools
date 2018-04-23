#include "vga.h"
#include "io.h"

extern const u8 VgaLookupTable[];

static u8 read_seq_b(u16 addr);
static u8 read_gra_b(u16 addr);

static void
msr_init (void)
{

	outb(0x67, MIS_W);
	return;
}


static void
cr_init (void)
{
	u8 v;
	u8 cvals[] = { 0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
		       0x00, 0x4F, 0x0E, 0x0F, 0x00, 0x00, 0x07, 0x00,
		       0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
		       0xFF };
	int i;

	/* deprotect CRT registers 0-7 */
	outb(0x11, CRT_IC);
	v = inb(CRT_DC);
	v &= 0x7F;
	outb(v, CRT_DC);

	for (i = 0; i < 0x19; i++) {
		outb (i, CRT_IC);
               	inb(0x80);
		outb (cvals[i], CRT_DC);
               	inb(0x80);
	}

	return;
}

static void
sr_init (void)
{
	u8 svals[] = { 0x03, 0x00, 0x03, 0x00, 0x02 };
	int i;

	for (i = 0; i < 0x5; i++) {
		outb(i, SEQ_I);
		inb(0x80);
		outb(svals[i], SEQ_D);
		inb(0x80);
	}

	return;
}

static void
gr_init (void)
{
	u8 gvals[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x0F,
		       0xFF };
	int i;

	for (i = 0; i < 9; i++) {
		outb(i, GRA_I);
		inb(0x80);
		outb(gvals[i], GRA_D);
		inb(0x80);
	}

	return;
}

static void
ar_init (void)
{
	int i;
	u8 avals[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		       0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
		       0x0C, 0x00, 0x0F, 0x08, 0x00 };

        /* write attribute controller registers */
        for (i = 0; i < 0x15; i++) {
                inb(IS1_RC);          /* reset flip-flop */
                inb(0x80);
                outb(i, ATT_IW);
                inb(0x80);
                outb(avals[i], ATT_IW);
                inb(0x80);
        }


	return;
}

static u8
read_seq_b(u16 addr) {
        outb(addr,SEQ_I);
        return inb(SEQ_D);
}

static u8
read_gra_b(u16 addr) {
        outb(addr,GRA_I);
        return inb(GRA_D);
}
/*
static u8
read_crtc_b(u16 addr) {
        outb(addr,CRT_IC);
        return inb(CRT_DC);
}

static u8
read_att_b(u16 addr) {
        inb(IS1_RC);
        inb(0x80); 
        outb(addr,ATT_IW);
        return inb(ATT_R);
}
*/
void
vga_font_load(vidmem, font, height, num_chars)
	unsigned char *		vidmem;
	const unsigned char *	font;
	int			height;
	int			num_chars;
{

/* Note: the font table is 'height' long but the font storage area
 * is 32 bytes long.
 */

        int i,j;
        u8 byte;

        /* set sequencer map 2, odd/even off */

        byte = read_seq_b(SEQ_PLANE_WRITE) & ~0xf;
        write_seq(byte|4,SEQ_PLANE_WRITE);
        byte = read_seq_b(SEQ_MEMORY_MODE);
        write_seq(byte|4,SEQ_MEMORY_MODE);

        /* select graphics map 2, odd/even off, map starts at 0xa0000 */

        write_gra(2,GDC_PLANE_READ);
        byte = read_gra_b(GDC_MODE) & ~0x10;
        write_gra(byte,GDC_MODE);
        write_gra(0,GDC_MISC);

        for (i = 0 ; i < num_chars ; i++) {
                for (j = 0 ; j < height ; j++) {
                        vidmem[i*32+j] = font[i*16+j];
                }
        }

        /* set sequencer back to maps 0,1, odd/even on */

        byte = read_seq_b(SEQ_PLANE_WRITE) & ~0xf;
        write_seq(byte|3,SEQ_PLANE_WRITE);
        byte = read_seq_b(SEQ_MEMORY_MODE) & ~0x4;
        write_seq(byte,SEQ_MEMORY_MODE);

        /* select graphics back to map 0,1, odd/even on */

        write_gra(0,GDC_PLANE_READ);
        byte = read_gra_b(GDC_MODE);
        write_gra(byte|0x10,GDC_MODE);
        write_gra(0xe,GDC_MISC);

	return;
}

void
vga_init (void)
{
	int i;

	msr_init();
	cr_init(); 
	sr_init(); 
	ar_init(); 
	gr_init();

        /* initialize the color table */
        outb(0, PEL_IW);
        i = 0;
        /* length is a magic number right now */
        while ( i < (0x3f*3 + 3) ) {
                outb(VgaLookupTable[i++], PEL_D);
                outb(VgaLookupTable[i++], PEL_D);
                outb(VgaLookupTable[i++], PEL_D);
        }
 
        outb(0x0ff, PEL_MSK); // palette mask

	/* 
         * very important
         * turn on video, disable palette access
	 */
        inb(IS1_RC);          /* reset flip-flop */
        inb(0x80); //delay
        outb(0x20, ATT_IW);
 
        outb(0x01, SEQ_I); /* unblank display */
        outb(0x00, SEQ_D);

	/* Turn on display */

        inb(IS1_RC);
        outb(0x20, ATT_IW);

	return;
}
