#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <efi.h>
#include <efilib.h>
#include "efiConsoleControl.h"
#include "data.h"
#include "video_subr.h"
#include "io.h"
#include "vga.h"

/* $Id: vxboot.c,v 1.5 2007/10/22 20:49:05 wpaul Exp $ */

EFI_GUID gEfiConsoleControlProtocolGuid = EFI_CONSOLE_CONTROL_PROTOCOL_GUID;

extern void * prot_to_real(void);
extern void * real_entry(void);
extern void x86_getgdt(struct gdt *);
extern void x86_setgdt(struct gdt *);
extern EFI_STATUS EfiNetEntry(EFI_HANDLE, EFI_SYSTEM_TABLE *);
extern int ifconfig (char *, char *, char *);
extern int tftpget (char *, char *, char *, char *);
extern UINT32 find_gfx_base(EFI_HANDLE, EFI_SYSTEM_TABLE *);

EFI_STATUS
EFIAPI
efi_main (IN EFI_HANDLE           ImageHandle,
	      IN EFI_SYSTEM_TABLE     *SystemTable)
{
	EFI_CONSOLE_CONTROL_PROTOCOL * ConsoleControl;
	EFI_STATUS		efi_sts;
	UINT16 *		p;
	UINTN			mapSize;
	EFI_MEMORY_DESCRIPTOR	*map, *mapentry;
	UINTN			mapKey;
	UINTN			descSize;
	UINT32			descVer;
	UINT32			memtot = 0;
	int 			i;
	struct gdt		gtable;
	struct x86desc *	gt;
	UINT32			vga_base;
	char			myip[64];
	char			serverip[64];
	char			filename[256];
	char			yesno[16];
	UINTN			buflen;
	EFI_INPUT_KEY		Key;
	char			msg[] = "Starting VxWorks bootrom";

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

	Print(L"VxWorks EFI/ia32 bootstrap loader for the Mac Mini\n");
	Print(L"$Revision: 1.5 $ $Date: 2007/10/22 20:49:05 $\n");

	/* Initialize network subsystem and libc. */

	efi_sts = EfiNetEntry (ImageHandle, SystemTable);
	efi_sts = EfiSocketInit (ImageHandle, SystemTable);

	map = LibMemoryMap (&mapSize, &mapKey, &descSize, &descVer);
	if (map != NULL && descVer == EFI_MEMORY_DESCRIPTOR_VERSION) {
		mapentry = map;
		for (i = 0; i < mapSize; i++) {
			if (mapentry->Type == EfiConventionalMemory)
				memtot += mapentry->NumberOfPages * 4096;
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
	if (efi_sts == EFI_SUCCESS && buflen < sizeof(myip)) {
		buflen = sizeof(serverip);
		efi_sts = RT->GetVariable (L"WIND_serverip", &EfiGlobalVariable,
		    NULL, &buflen, serverip);
	}
	if (efi_sts == EFI_SUCCESS && buflen < sizeof(serverip)) {
		buflen = sizeof(filename);
		efi_sts = RT->GetVariable (L"WIND_filename", &EfiGlobalVariable,
		    NULL, &buflen, filename);
	}
	if (efi_sts == EFI_SUCCESS && buflen < sizeof(filename)) {
		printf ("Found saved boot values in NVRAM\n");
		printf ("My IP: [%s]\n", myip);
		printf ("Server IP: [%s]\n", serverip);
		printf ("Filename: [%s]\n", filename);

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
			LibDeleteVariable (L"WIND_serverip",&EfiGlobalVariable);
			LibDeleteVariable (L"WIND_filename",&EfiGlobalVariable);
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

serveripbad:
        printf("Enter server IP address: ");
        fflush(stdout);
        fgets(serverip, sizeof(serverip), stdin);
	if (strchr(serverip, '\n') != NULL)
		*strchr(serverip, '\n') = 0x0;

	if (inet_addr(serverip) == -1) {
		printf("Invalid IP address: [%s]\n");
		printf("Please try again\n");
		goto serveripbad;
	}

	printf ("Enter bootrom filename: ");
        fflush(stdout);
        fgets(filename, sizeof(filename), stdin);
	if (strchr(filename, '\n') != NULL)
		*strchr(filename, '\n') = 0x0;

	printf ("Save settings? [y/n]: ");
        fflush(stdout);
        fgets(yesno, sizeof(yesno), stdin);

	if (strchr(yesno, 'y') || strchr(yesno, 'Y')) {
		RT->SetVariable(L"WIND_myip", &EfiGlobalVariable,
		    EFI_VARIABLE_BOOTSERVICE_ACCESS|
		    EFI_VARIABLE_NON_VOLATILE,
		    strlen(myip), myip);
		RT->SetVariable(L"WIND_serverip", &EfiGlobalVariable,
		    EFI_VARIABLE_BOOTSERVICE_ACCESS|
		    EFI_VARIABLE_NON_VOLATILE,
		    strlen(serverip), serverip);
		RT->SetVariable(L"WIND_filename", &EfiGlobalVariable,
		    EFI_VARIABLE_BOOTSERVICE_ACCESS|
		    EFI_VARIABLE_NON_VOLATILE,
		    strlen(filename), filename);
	}

boot:
	printf ("Initializing network interfaces.\n");

	ifconfig("lo0", "127.0.0.1", "255.0.0.0");
	ifconfig("sni0", myip, "255.255.255.0");

	/* Install primary real mode bootstrap. */

	bcopy (real_entry, (char *)BIOS_ENTRY, 256);

	/* Install bootrom code. */

	tftpget (filename, myip, serverip, (char *)VXWORKS_ENTRY);

	printf ("Switching to VGA mode\n");

	sleep (1);

	/* Find memio BAR of VGA hardware */

	vga_base = find_gfx_base(ImageHandle, SystemTable);

	BS->ExitBootServices(ImageHandle, mapKey);

	/* Force display back to VGA mode. */

	enable_vga(vga_base);

	/* Set up new GDT so we can switch to real mode. */

	x86_getgdt(&gtable);
	gt = gtable.base;
	gt[5].x_lolimit = 0xFFFF;
	gt[5].x_flags = GDT_CFLAGS;
	gt[6].x_lolimit = 0xFFFF;
	gt[6].x_flags = GDT_DFLAGS;
	x86_setgdt(&gtable);

	/* Display start message in VGA mode. */

	p = (UINT16 *)0xb8000;
	for (i = 0; i < strlen(msg); i++)
		p[i] = 0x1F00 | msg[i];

	sleep (3);

	/* Jump to VxWorks. */

	prot_to_real();

	/* Not reached. */

	return (EFI_SUCCESS);
}
