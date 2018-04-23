#ifndef VIDEO_SUBR_H
#define VIDEO_SUBR_H

void video_tx_byte(unsigned char byte);
void vga_init(void);
void vga_load_regs(void);
void vga_set_amode (void);
void vga_set_gmode (void);
void vga_font_load(unsigned char *vidmem, const unsigned char *font, int height, int num_chars);
extern const unsigned char fontdata_8x16[];

void enable_vga(UINT32);

#endif /* VIDEO_SUBR_H */
