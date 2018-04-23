
#define DPLL_A               0x06014
#define DPLL_B               0x06018
#define DPLL_A_MD            0x0601c
#define DPLL_B_MD            0x06020
#define FPA0         0x06040
#define FPA1         0x06044
#define FPB0         0x06048
#define FPB1         0x0604c

#define HTOTAL_A     0x60000
#define HBLANK_A     0x60004
#define HSYNC_A      0x60008
#define VTOTAL_A     0x6000c
#define VBLANK_A     0x60010
#define VSYNC_A      0x60014
#define PIPEASRC     0x6001c
#define BCLRPAT_A    0x60020
#define VSYNCSHIFT_A 0x60028

#define PIPEACONF 0x70008

#define DSPACNTR             0x70180
#define DSPABASE             0x70184
#define DSPASTRIDE           0x70188
#define DSPAKEYVAL           0x70194
#define DSPAKEYMASK          0x70198
#define DSPAPOS                      0x7018C /* reserved */
#define DSPASIZE             0x70190
#define DSPASURF             0x7019C
#define DSPATILEOFF          0x701A4

#define VGACNTRL             0x71400

