
#include <efi.h>
#include <efilib.h>

UINT32
find_gfx_base(ImageHandle, SystemTable)
	IN EFI_HANDLE           ImageHandle;
	IN EFI_SYSTEM_TABLE     *SystemTable;
{
	EFI_STATUS                      Status;
	EFI_PCI_IO_PROTOCOL		*IoDev;
	EFI_HANDLE                      *HandleBuffer;
	UINTN                           BufferSize;
	UINTN				seg, bus, dev, func;
	UINT32			bar;
	int			i;

	Status = LibLocateHandle (ByProtocol, &PciIoProtocol, NULL,
	    &BufferSize, &HandleBuffer);

	if (Status != EFI_SUCCESS)
		return(0x0);

	for (i = 0; i < BufferSize; i++) {
		Status = BS->HandleProtocol (HandleBuffer[i],
		    &PciIoProtocol, (void *) &IoDev);
		if (EFI_ERROR (Status))
			break;
		IoDev->GetLocation(IoDev, &seg, &bus, &dev, &func);
		if (seg == 0 && bus == 0 && dev == 2 && func == 0)
			break;
	}

	IoDev->Pci.Read(IoDev, EfiPciIoWidthUint32, 0x10, 1, &bar);

	return (bar);
}
