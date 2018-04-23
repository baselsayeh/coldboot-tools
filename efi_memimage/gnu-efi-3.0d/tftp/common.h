
typedef unsigned char uchar;
typedef unsigned long ulong;
/*typedef unsigned short ushort;*/

typedef unsigned long IPaddr_t;
extern char * load_addr;
extern char * BootFile;

extern void udpsend (IPaddr_t, int, int);

#define NETLOOP_CONTINUE        1
#define NETLOOP_RESTART         2
#define NETLOOP_SUCCESS         3
#define NETLOOP_FAIL            4

