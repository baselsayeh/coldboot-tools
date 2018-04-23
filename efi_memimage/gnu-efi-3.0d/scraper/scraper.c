#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include <efi.h>
#include <efilib.h>
#include "efiConsoleControl.h"

#include "protocol.h"
#include "data.h"

EFI_GUID gEfiConsoleControlProtocolGuid = EFI_CONSOLE_CONTROL_PROTOCOL_GUID;

extern EFI_STATUS EfiNetEntry(EFI_HANDLE, EFI_SYSTEM_TABLE *);
extern int ifconfig (char *, char *, char *);
extern void if_force_poll(void);
extern int hz;

char buf[65536];


struct ll {
	UINT32	lo;
	UINT32	hi;
};

#define SENDSIZE 1024

static void
send_block(sock, daddr, len, size)
	int		sock;
	uint32_t	daddr;
        int             len;
        int             size;
{
        int             i;
        SCRAPE_MSG *    m;
        uint8_t *       p;
        char *          s;
	int		n;

        m = (SCRAPE_MSG *)buf;
        p = buf;
        p += sizeof(SCRAPE_MSG);
        s = (char *)daddr;

        for (i = 0; i < len / size; i++) {
                bcopy ((uint32_t *)s, (uint32_t *)p, size);
                m->sm_cmd = htonl(SM_CMD_RESP);
                m->sm_len = htonl(size);
                m->sm_seq = htonl((uint32_t)s);
                m->sm_off = 0;
		n = write (sock, (void *)buf, size + sizeof(SCRAPE_MSG));
		if (n < 0)
			printf("write failed\n");
                s += size;
        }

        return;
}

EFI_STATUS
EFIAPI
efi_main (IN EFI_HANDLE           ImageHandle,
          IN EFI_SYSTEM_TABLE     *SystemTable)
{
	EFI_CONSOLE_CONTROL_PROTOCOL * ConsoleControl;
	EFI_STATUS		efi_sts;
	UINTN			mapSize;
	EFI_MEMORY_DESCRIPTOR	*map, *mapentry;
	UINTN			mapKey;
	UINTN			descSize;
	UINT32			descVer;
	UINT32			memtot = 0;
	int 			i;
	char			myip[64];
	char			serverip[64];
	char			filename[256];
	char			yesno[16];
	UINTN			buflen;
	EFI_INPUT_KEY		Key;
	int			s;
	struct sockaddr_in	sin;
	struct sockaddr_in	rsin;
	int			rlen;
	fd_set			fdset;
	struct bios_smap	memory_map[32];
        struct bios_smap	*bmap;
        struct scrape_msg       *m;
        int                     n;

	InitializeLib(ImageHandle, SystemTable);

	/* Reset input buffer to flush any stale keystrokes. */

	ST->ConIn->Reset(ST->ConIn, TRUE);

	if (LibLocateProtocol(&gEfiConsoleControlProtocolGuid,
            (void **)&ConsoleControl) == EFI_SUCCESS) {
		EFI_CONSOLE_CONTROL_SCREEN_MODE currentMode;

		ConsoleControl->GetMode(ConsoleControl,
                    &currentMode, NULL, NULL);
		if (currentMode == EfiConsoleControlScreenGraphics)
      			ConsoleControl->SetMode(ConsoleControl,
			    EfiConsoleControlScreenText);
	}

	Print(L"EFI memory scraper, written by Bill Paul\n");

	/* Initialize network subsystem and libc. */

	efi_sts = EfiNetEntry (ImageHandle, SystemTable);
	efi_sts = EfiSocketInit (ImageHandle, SystemTable);

	n = 0;
	bzero (memory_map, sizeof (memory_map));

	map = LibMemoryMap (&mapSize, &mapKey, &descSize, &descVer);
	if (map != NULL && descVer == EFI_MEMORY_DESCRIPTOR_VERSION) {
		mapentry = map;
		for (i = 0; i < mapSize; i++) {
			if (mapentry->Type == EfiConventionalMemory) {
				struct ll *addr;
				addr = (struct ll *)&mapentry->PhysicalStart;
				printf("segment %d: %x%x (%d)\n", n, addr->hi,
				    addr->lo, mapentry->NumberOfPages * 4096);
				memory_map[n].baselo = htonl(addr->lo);
				memory_map[n].lenlo =
				    htonl(mapentry->NumberOfPages * 4096);

				n++;

				memtot += mapentry->NumberOfPages * 4096;
			}
			mapentry = NextMemoryDescriptor(mapentry, descSize);
		}
	}

	printf ("Total memory: %lu bytes (%x)\n", memtot, memtot);

	bzero(myip, sizeof(myip));
	bzero(serverip, sizeof(serverip));
	bzero(filename, sizeof(filename));

	buflen = sizeof(myip);
	efi_sts = RT->GetVariable (L"WIND_myip", &EfiGlobalVariable, NULL,
	    &buflen, myip);
	if (efi_sts == EFI_SUCCESS && buflen < sizeof(filename)) {
		printf ("Found saved boot values in NVRAM\n");
		printf ("My IP: [%s]\n", myip);

		printf ("Press a key to abort booting with defaults:  ");
		fflush (stdout);

		/*
		 * Ideally, we should use the standard 'termios' way of
		 * detecting single keystrokes, by using the tcgetattr()
		 * and tcsetattr() routines to switch from 'cooked' to
		 * 'raw' mode. However, our special version of libc has
		 * no termios support, so we use the EFI library's
		 * console support instead.
	 	 */

		bzero ((char *)&Key, sizeof(Key));
		for (i = 5; i > -1; i--) {
			printf("\b%d", i);
			fflush (stdout);
			ST->ConIn->ReadKeyStroke(ST->ConIn, &Key);
			if (Key.UnicodeChar)
				break;
			sleep (1);
		}
		printf ("\n");

		/* No key pressed, boot. */

		if (Key.UnicodeChar == 0)
			goto boot;

		printf ("Boot with saved values? [y/n]: ");
        	fgets(yesno, sizeof(yesno), stdin);
		if (strchr(yesno, 'y') || strchr(yesno, 'Y'))
			goto boot;

		printf ("Delete these saved values? [y/n]: ");
        	fgets(yesno, sizeof(yesno), stdin);
		if (strchr(yesno, 'y') || strchr(yesno, 'Y')) {
			LibDeleteVariable (L"WIND_myip", &EfiGlobalVariable);
		}
	}

myipbad:
        printf("Enter my IP address: ");
        fflush(stdout);
        fgets(myip, sizeof(myip), stdin);
	if (strchr(myip, '\n') != NULL)
		*strchr(myip, '\n') = 0x0;

	if (inet_addr(myip) == -1) {
		printf("Invalid IP address: [%s]\n");
		printf("Please try again\n");
		goto myipbad;
	}

	printf ("Save settings? [y/n]: ");
        fflush(stdout);
        fgets(yesno, sizeof(yesno), stdin);

	if (strchr(yesno, 'y') || strchr(yesno, 'Y')) {
		RT->SetVariable(L"WIND_myip", &EfiGlobalVariable,
		    EFI_VARIABLE_BOOTSERVICE_ACCESS|
		    EFI_VARIABLE_NON_VOLATILE,
		    strlen(myip), myip);
	}

boot:
	printf ("Initializing network interfaces.\n");

	ifconfig("lo0", "127.0.0.1", "255.0.0.0");
	ifconfig("sni0", myip, "255.255.255.0");

	printf ("Network is up.\n");

        m = (SCRAPE_MSG *)buf;

	/* memory scraper code goes here. */

	bzero ((char *)&sin, sizeof(sin));

	s = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(31337);

	bind (s, (struct sockaddr *)&sin, sizeof(sin));

	/* Wait for a connection. */

	printf ("Waiting for handshake...\n");

        FD_ZERO(&fdset);
        FD_SET(s, &fdset);
        select (FD_SETSIZE, &fdset, NULL, NULL, NULL);

        n = recvfrom (s, m, sizeof (bmap) + sizeof(SCRAPE_MSG),
            0, (struct sockaddr *)&rsin, (size_t *)&rlen);

	printf ("Got handshake from: %s\n", inet_ntoa(rsin.sin_addr));

        if (ntohl(m->sm_cmd) != SM_CMD_PROBE) {
                printf("malformed probe\n");
                goto done;
        }

	connect (s, (struct sockaddr *)&rsin, rlen);

        m->sm_cmd = htonl(SM_CMD_RESP);
        m->sm_len = 0;
        m->sm_off = htonl(SENDSIZE);
        m->sm_seq = 0;
        bmap = (struct bios_smap *)(buf + sizeof (SCRAPE_MSG));

	bcopy (memory_map, buf + sizeof (SCRAPE_MSG), sizeof(memory_map)); 

	printf ("Sending reply\n");

        n = write (s, buf, sizeof(SCRAPE_MSG) + sizeof(memory_map));

	printf ("Ready to handle transfers.\n");

        while (1) {
                n = read(s, (void *)buf, sizeof(SCRAPE_MSG));

		if (n < 0)
			printf("read failed\n");

                /* If asked to quit, then quit. */
                if (ntohl(m->sm_cmd) == SM_CMD_QUIT)
                        break;

                if (ntohl(m->sm_cmd) != SM_CMD_REQ) {
                        printf("Malformed request\n");
                        break;
                }

                send_block (s, ntohl(m->sm_seq), ntohl(m->sm_len), SENDSIZE);
        }

done:
	printf ("Resetting...\n");

	sleep (5);

	RT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);

	BS->ExitBootServices(ImageHandle, mapKey);

	return (EFI_SUCCESS);
}
