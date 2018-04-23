
#define PXE_VP	0x5650
#define PXE_VN	0x564E

extern int pxe_call (void);

extern int pxe_init(IP4_t *, int *);
extern int pxe_open(void);
extern int pxe_write(IP4_t, UDP_PORT_t, void *, int);
extern int pxe_read(IP4_t *, UDP_PORT_t *, void *, int, int);
extern int pxe_close(void);
