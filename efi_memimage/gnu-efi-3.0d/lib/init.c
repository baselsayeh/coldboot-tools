/*++

Copyright (c) 1998  Intel Corporation

Module Name:


Abstract:




Revision History

--*/

#include "lib.h"
#include <efilib.h>

#define EFI_1_02_REVISION ((1<<16) | 02)
#define EFI_1_10_REVISION ((1<<16) | 10)

VOID
EFIDebugVariable (
    VOID
    );

VOID
InitializeLib (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
/*++

Routine Description:

    Initializes EFI library for use
    
Arguments:

    Firmware's EFI system table
    
Returns:

    None

--*/ 
{
    EFI_LOADED_IMAGE        *LoadedImage;
    EFI_STATUS              Status;
    CHAR8                   *LangCode;

#ifndef EFI_APP_MULTIMODAL
    EfiCheckVersion(ImageHandle, SystemTable);
#endif

    if (!LibInitialized) {
        LibInitialized = TRUE;
        LibFwInstance = FALSE;

        //
        // Set up global pointer to the system table, boot services table,
        // and runtime services table
        //

        ST = SystemTable;
        BS = SystemTable->BootServices;
        RT = SystemTable->RuntimeServices;
//        ASSERT (CheckCrc(0, &ST->Hdr));
//        ASSERT (CheckCrc(0, &BS->Hdr));
//        ASSERT (CheckCrc(0, &RT->Hdr));


        //
        // Initialize pool allocation type
        //

        if (ImageHandle) {
            Status = BS->HandleProtocol (
                            ImageHandle, 
                            &LoadedImageProtocol,
                            (VOID*)&LoadedImage
                            );

            if (!EFI_ERROR(Status)) {
                PoolAllocationType = LoadedImage->ImageDataType;
            }
            
            EFIDebugVariable ();
        }

        //
        // Initialize Guid table
        //

        InitializeGuid();

        InitializeLibPlatform(ImageHandle,SystemTable);
    }

    //
    // 
    //

    if (ImageHandle && UnicodeInterface == &LibStubUnicodeInterface) {
        LangCode = LibGetVariable (VarLanguage, &EfiGlobalVariable);
        InitializeUnicodeSupport (LangCode);
        if (LangCode) {
            FreePool (LangCode);
        }
    }
	
}

VOID
InitializeUnicodeSupport (
    CHAR8 *LangCode
    )
{
    EFI_UNICODE_COLLATION_INTERFACE *Ui;
    EFI_STATUS                      Status;
    CHAR8                           *Languages;
    UINTN                           Index, Position, Length;
    UINTN                           NoHandles;
    EFI_HANDLE                      *Handles;

    //
    // If we don't know it, lookup the current language code
    //

    LibLocateHandle (ByProtocol, &UnicodeCollationProtocol, NULL, &NoHandles, &Handles);
    if (!LangCode || !NoHandles) {
        goto Done;
    }

    //
    // Check all driver's for a matching language code
    //

    for (Index=0; Index < NoHandles; Index++) {
        Status = BS->HandleProtocol (Handles[Index], &UnicodeCollationProtocol, (VOID*)&Ui);
        if (EFI_ERROR(Status)) {
            continue;
        }

        //
        // Check for a matching language code
        //

        Languages = Ui->SupportedLanguages;
        Length = strlena(Languages);
        for (Position=0; Position < Length; Position += ISO_639_2_ENTRY_SIZE) {

            //
            // If this code matches, use this driver
            //

            if (CompareMem (Languages+Position, LangCode, ISO_639_2_ENTRY_SIZE) == 0) {
                UnicodeInterface = Ui;
                goto Done;
            }
        }
    }

Done:
    //
    // Cleanup
    //

    if (Handles) {
        FreePool (Handles);
    }
}

VOID
EFIDebugVariable (
    VOID
    )
{
    EFI_STATUS      Status;
    UINT32          Attributes;
    UINTN           DataSize;
    UINTN           NewEFIDebug;

    DataSize = sizeof(EFIDebug);
    Status = RT->GetVariable(L"EFIDebug", &EfiGlobalVariable, &Attributes, &DataSize, &NewEFIDebug);
    if (!EFI_ERROR(Status)) {
        EFIDebug = NewEFIDebug;
    } 
}

void
EfiCheckVersion(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
     )
{
      
      if (SystemTable->Hdr.Revision < EFI_SYSTEM_TABLE_REVISION){
          SystemTable->ConOut->OutputString(
		  	SystemTable->ConOut,
		  	L"Please start this image in EFI higher version \r\n"
		  	);
          SystemTable->BootServices->Exit(ImageHandle,EFI_UNSUPPORTED,0,NULL);
      	}
	return;     	  
}

void
EfiGetVersion(
    IN UINTN               *MajorVersion,
    IN UINTN               *MinorVersion
)
{
	UINT32 		Revision;
	
	Revision    = ST->Hdr.Revision;	
	
	*MinorVersion = Revision & 0xFFFF;
	*MajorVersion = Revision >> 16;		
}

BOOLEAN
IsEfi102(
)
{
    if (ST->Hdr.Revision == EFI_1_02_REVISION) {
		return TRUE;
   	}
	return FALSE;
}

BOOLEAN
IsEfi110(
)
{
    if (ST->Hdr.Revision == EFI_1_10_REVISION) {
		return TRUE;
   	}
	return FALSE;
}

STATIC
BOOLEAN
IsFfi110OrHigher(
)
{
    if (ST->Hdr.Revision >= EFI_1_10_REVISION) {
		return TRUE;
   	}
	return FALSE;

}

#if defined(EFI_APP_MULTIMODAL)

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(UINTN) - 1) & ~(sizeof(UINTN) - 1) )
typedef CHAR8 * va_list;
#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)  ( ap = (va_list)0 )

EFI_STATUS 
BOOTSERVICE
MultimodalConnectController (
  IN  EFI_HANDLE                ControllerHandle,
  IN  EFI_HANDLE                *DriverImageHandle    OPTIONAL,
  IN  EFI_DEVICE_PATH           *RemainingDevicePath  OPTIONAL,
  IN  BOOLEAN                   Recursive
  )

{
    if (IsFfi110OrHigher())
	{
    	return BS->ConnectController(ControllerHandle,
    				DriverImageHandle,
    				RemainingDevicePath,
    				Recursive);
    }	    
    return EFI_UNSUPPORTED;   
}

EFI_STATUS
BOOTSERVICE 
MultimodalDisconnectController (
  IN  EFI_HANDLE  ControllerHandle,
  IN  EFI_HANDLE  DriverImageHandle, OPTIONAL
  IN  EFI_HANDLE  ChildHandle        OPTIONAL
  )

{
    if (IsFfi110OrHigher())
    {
    	return BS->DisconnectController(ControllerHandle,
    				DriverImageHandle,
    				ChildHandle);
    }    
    return EFI_UNSUPPORTED;   
}

EFI_STATUS
BOOTSERVICE
MultimodalOpenProtocol (
  IN  EFI_HANDLE                UserHandle,
  IN  EFI_GUID                  *Protocol,
  OUT VOID                      **Interface,
  IN  EFI_HANDLE                ImageHandle,
  IN  EFI_HANDLE                ControllerHandle,
  IN  UINT32                    Attributes
  )
{
    if (IsFfi110OrHigher())
    {
    	return BS->OpenProtocol(UserHandle,
    				Protocol,
    				Interface,
    				ImageHandle,
    				ControllerHandle,
    				Attributes);
	}    
	return EFI_UNSUPPORTED;   
}

EFI_STATUS
BOOTSERVICE
MultimodalCloseProtocol (
  IN  EFI_HANDLE                UserHandle,
  IN  EFI_GUID                  *Protocol,
  IN  EFI_HANDLE                ImageHandle,
  IN  EFI_HANDLE                ControllerHandle  OPTIONAL
  )
{
    if (IsFfi110OrHigher())
    {
    	return BS->CloseProtocol(UserHandle,
    				Protocol,
    				ImageHandle,
    				ControllerHandle);
    }
    
    return EFI_UNSUPPORTED;   
}


EFI_STATUS
BOOTSERVICE
MultimodalOpenProtocolInformation (
  IN  EFI_HANDLE                          UserHandle,
  IN  EFI_GUID                            *Protocol,
  IN  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY **EntryBuffer,
  OUT UINTN                               *EntryCount
  )
{
    if (IsFfi110OrHigher())
    {
    	return BS->OpenProtocolInformation(UserHandle,
    				Protocol,
    				EntryBuffer,
    				EntryCount);
    }
	
    return EFI_UNSUPPORTED;   
}

EFI_STATUS
BOOTSERVICE
MultimodalProtocolsPerHandle (
  IN EFI_HANDLE       UserHandle,
  OUT EFI_GUID        ***ProtocolBuffer,
  OUT UINTN           *ProtocolBufferCount
  )
{
    if (IsFfi110OrHigher())
    {
    	return BS->ProtocolsPerHandle(UserHandle,
    				ProtocolBuffer,
    				ProtocolBufferCount);
    }
    
    return EFI_UNSUPPORTED;   
}


EFI_STATUS
BOOTSERVICE
MultimodalLocateHandleBuffer (
  IN EFI_LOCATE_SEARCH_TYPE       SearchType,
  IN EFI_GUID                     *Protocol OPTIONAL,
  IN VOID                         *SearchKey OPTIONAL,
  IN OUT UINTN                    *NumberHandles,
  OUT EFI_HANDLE                  **Buffer
  )
{
    if (IsFfi110OrHigher())
    {
    	return BS->LocateHandleBuffer(SearchType,
    				Protocol,
    				SearchKey,
    				NumberHandles,
    				Buffer);
	}

	return EFI_UNSUPPORTED;   

}


EFI_STATUS
BOOTSERVICE
MultimodalLocateProtocol (
  EFI_GUID  *Protocol,
  VOID      *Registration, OPTIONAL
  VOID      **Interface
  )
{
    if (IsFfi110OrHigher())
    {
    	return BS->LocateProtocol(Protocol,
    				Registration,
    				Interface);
    }
    
    return EFI_UNSUPPORTED;   
}

EFI_STATUS
BOOTSERVICE
MultimodalInstallMultipleProtocolInterfaces (
  IN OUT EFI_HANDLE           *Handle,
  ...
  )
{
  va_list     args;
  va_list	  Ptr;
  CHAR8*	  BufEnd;
  struct {
	  CHAR8 argbuf[512];
  }Buf;
  
  if (!IsFfi110OrHigher())
  {
	return EFI_UNSUPPORTED;
  }
  
  //
  // Fill all args into Buf
  // Pass Buf to BS->MultimodalInstallMultipleProtocolInterfaces
  //

  BufEnd = (CHAR8*)&Buf + sizeof(Buf) - sizeof(EFI_GUID*) - sizeof(VOID*);

  Ptr=(va_list)&Buf;

  va_start (args, Handle);
  while (Ptr<BufEnd){
  	
	*(EFI_GUID**)Ptr = va_arg(args, EFI_GUID*);
    if (*(VOID**)Ptr == NULL) break;
	va_arg(Ptr, EFI_GUID*);
	
   *(VOID**)Ptr = va_arg(args, VOID*);
	if (*(VOID**)Ptr == NULL) break;
	va_arg(Ptr, VOID*);
  }
  
  if (Ptr>=BufEnd)  {
	  return EFI_OUT_OF_RESOURCES;
  }
  *(VOID**)Ptr = NULL;
  
  return BS->InstallMultipleProtocolInterfaces(Handle, Buf);  
}


EFI_STATUS
BOOTSERVICE
MultimodalUninstallMultipleProtocolInterfaces (
  IN EFI_HANDLE           Handle,
  ...
  )
{
  va_list     args;
  va_list	  Ptr;
  CHAR8*	  BufEnd;
  struct {
	  CHAR8 argbuf[512];
  }Buf;
  
  if (!IsFfi110OrHigher())
  {
	return EFI_UNSUPPORTED;
  }
  
  //
  // Fill all args into Buf
  // Pass Buf to BS->MultimodalUninstallMultipleProtocolInterfaces
  //

  BufEnd = (CHAR8*)&Buf + sizeof(Buf) - sizeof(EFI_GUID*) - sizeof(VOID*);

  Ptr=(va_list)&Buf;

  va_start (args, Handle);
  while (Ptr<BufEnd){
  	
	*(EFI_GUID**)Ptr = va_arg(args, EFI_GUID*);
    if (*(VOID**)Ptr == NULL) break;
	va_arg(Ptr, EFI_GUID*);
	
   *(VOID**)Ptr = va_arg(args, VOID*);
	if (*(VOID**)Ptr == NULL) break;
	va_arg(Ptr, VOID*);
  }
  
  if (Ptr>=BufEnd)  {
	  return EFI_OUT_OF_RESOURCES;
  }
  *(VOID**)Ptr = NULL;
  
  return BS->UninstallMultipleProtocolInterfaces(Handle, Buf);
}


EFI_STATUS
RUNTIMEFUNCTION
MultimodalCalculateCrc32 (
    VOID   *pt,
    UINTN  Size,
    UINT32 *Crc
    )
{
    if (IsFfi110OrHigher())
    {
    	return BS->CalculateCrc32(pt, 
    				Size,
    				Crc);
    }
	
	return EFI_UNSUPPORTED;  
}

VOID 
BOOTSERVICE 
MultimodalSetMem (
  IN VOID   *Buffer,
  IN UINTN  Size,
  IN UINT8  Value    
  )
{
    if (IsFfi110OrHigher())
    {
    	BS->SetMem(Buffer,Size, Value);
	}    
	else {
		SetMem(Buffer, Size, Value);
	}	
	return ;
}

VOID 
BOOTSERVICE
MultimodalCopyMem (
  VOID *Dest,
  VOID *Src,
  UINTN Length
  )
{
    if (IsFfi110OrHigher())
    {
	   BS->CopyMem(Dest,Src, Length);
	   
    }    
	else {
		CopyMem(Dest, Src, Length);
	}
    return;   
}

EFI_TPL 
BOOTSERVICE 
MultimodalRaiseTPL (
    IN EFI_TPL      NewTpl
    )
{
    return BS->RaiseTPL(NewTpl);    
}

VOID
BOOTSERVICE 
MultimodalRestoreTPL(
    IN EFI_TPL      OldTpl
    )
{
     BS->RestoreTPL(OldTpl);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalAllocatePages(
    IN EFI_ALLOCATE_TYPE            Type,
    IN EFI_MEMORY_TYPE              MemoryType,
    IN UINTN                        NoPages,
    OUT EFI_PHYSICAL_ADDRESS        *Memory
    )
{
       return BS->AllocatePages(Type,
       			MemoryType,
       			NoPages,
       			Memory);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalFreePages(
    IN EFI_PHYSICAL_ADDRESS         Memory,
    IN UINTN                        NoPages
    )
{
       return BS->FreePages(Memory,
       			NoPages);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalGetMemoryMap(
    IN OUT UINTN                    *MemoryMapSize,
    IN OUT EFI_MEMORY_DESCRIPTOR    *MemoryMap,
    OUT UINTN                       *MapKey,
    OUT UINTN                       *DescriptorSize,
    OUT UINT32                      *DescriptorVersion
    )
{
       return BS->GetMemoryMap(MemoryMapSize,
       			MemoryMap,
       			MapKey,
       			DescriptorSize,
       			DescriptorVersion);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalAllocatePool(
    IN EFI_MEMORY_TYPE              PoolType,
    IN UINTN                        Size,
    OUT VOID                        **Buffer
    )
{
       return BS->AllocatePool(PoolType,
       			Size,
       			Buffer);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalFreePool(
    IN VOID                         *Buffer
    )
{
       return BS->FreePool(Buffer);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalCreateEvent(
    IN UINT32                       Type,
    IN EFI_TPL                      NotifyTpl,
    IN EFI_EVENT_NOTIFY             NotifyFunction,
    IN VOID                         *NotifyContext,
    OUT EFI_EVENT                   *Event
    )
{
       return BS->CreateEvent(Type,
       			NotifyTpl,
       			NotifyFunction,
       			NotifyContext,
       			Event);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalSetTimer(
    IN EFI_EVENT                Event,
    IN EFI_TIMER_DELAY          Type,
    IN UINT64                   TriggerTime
    )
{
       return BS->SetTimer(Event,
       			Type,
       			TriggerTime);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalWaitForEvent(
    IN UINTN                    NumberOfEvents,
    IN EFI_EVENT                *Event,
    OUT UINTN                   *Index
    )
{
       return BS->WaitForEvent(NumberOfEvents,
       			Event,
       			Index);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalSignalEvent(
    IN EFI_EVENT                Event
    )
{
       return BS->SignalEvent(Event);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalCloseEvent(
    IN EFI_EVENT                Event
    )
{
       return BS->CloseEvent(Event);
}


EFI_STATUS 
BOOTSERVICE 
MultimodalCheckEvent(
    IN EFI_EVENT                Event
    )
{
       return BS->CheckEvent(Event);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalInstallProtocolInterface(
    IN OUT EFI_HANDLE           *Handle,
    IN EFI_GUID                 *Protocol,
    IN EFI_INTERFACE_TYPE       InterfaceType,
    IN VOID                     *Interface
    )
{
       return BS->InstallProtocolInterface(Handle,
       			Protocol,
       			InterfaceType,
       			Interface);
}


EFI_STATUS 
BOOTSERVICE 
MultimodalReinstallProtocolInterface(
    IN EFI_HANDLE               Handle,
    IN EFI_GUID                 *Protocol,
    IN VOID                     *OldInterface,
    IN VOID                     *NewInterface
    )
{
       return BS->ReinstallProtocolInterface(Handle,
       			Protocol,
       			OldInterface,
       			NewInterface);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalUninstallProtocolInterface(
    IN EFI_HANDLE               Handle,
    IN EFI_GUID                 *Protocol,
    IN VOID                     *Interface
    )
{
       return BS->UninstallProtocolInterface(Handle,
       			Protocol,
       			Interface);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalHandleProtocol(
    IN EFI_HANDLE               Handle,
    IN EFI_GUID                 *Protocol,
    OUT VOID                    **Interface
    )
{
       return BS->HandleProtocol(Handle,
       			Protocol,
       			Interface);
}


EFI_STATUS  
BOOTSERVICE 
MultimodalRegisterProtocolNotify(
    IN EFI_GUID                 *Protocol,
    IN EFI_EVENT                Event,
    OUT VOID                    **Registration
    )
{
       return BS->RegisterProtocolNotify(Protocol,
       			Event,
       			Registration);
}

EFI_STATUS 
BOOTSERVICE 
MultimodalLocateHandle(
    IN EFI_LOCATE_SEARCH_TYPE   SearchType,
    IN EFI_GUID                 *Protocol OPTIONAL,
    IN VOID                     *SearchKey OPTIONAL,
    IN OUT UINTN                *BufferSize,
    OUT EFI_HANDLE              *Buffer
    )
{
       return BS->LocateHandle(SearchType,
       			Protocol,
       			SearchKey,
       			BufferSize,
       			Buffer);
}




EFI_STATUS 
BOOTSERVICE 
MultimodalLocateDevicePath(
    IN EFI_GUID                 *Protocol,
    IN OUT EFI_DEVICE_PATH      **DevicePath,
    OUT EFI_HANDLE              *Device
    )
{
       return BS->LocateDevicePath(Protocol,
       			DevicePath,
       			Device);
}




EFI_STATUS 
BOOTSERVICE 
MultimodalInstallConfigurationTable(
    IN EFI_GUID                 *Guid,
    IN VOID                     *Table
    )
{
       return BS->InstallConfigurationTable(Guid,
       			Table);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalPCHandleProtocol(
    IN EFI_HANDLE               Handle,
    IN EFI_GUID                 *Protocol,
    OUT VOID                    **Interface
    )
{
    if (IsEfi102()){
     	return ((EFI_HANDLE_PROTOCOL)BS->Reserved)(
       			Handle,
       			Protocol,
       			Interface
       			);
    }
	return EFI_UNSUPPORTED;     
}



EFI_STATUS 
BOOTSERVICE 
MultimodalLoadImage(
    IN BOOLEAN                      BootPolicy,
    IN EFI_HANDLE                   ParentImageHandle,
    IN EFI_DEVICE_PATH              *FilePath,
    IN VOID                         *SourceBuffer   OPTIONAL,
    IN UINTN                        SourceSize,
    OUT EFI_HANDLE                  *ImageHandle
    )
{
       return BS->LoadImage(BootPolicy,
       			ParentImageHandle,
       			FilePath,
       			SourceBuffer,
       			SourceSize,
       			ImageHandle);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalStartImage(
    IN EFI_HANDLE                   ImageHandle,
    OUT UINTN                       *ExitDataSize,
    OUT CHAR16                      **ExitData  OPTIONAL
    )
{
       return BS->StartImage(ImageHandle,
       			ExitDataSize,
       			ExitData);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalExit(
    IN EFI_HANDLE                   ImageHandle,
    IN EFI_STATUS                   ExitStatus,
    IN UINTN                        ExitDataSize,
    IN CHAR16                       *ExitData OPTIONAL
    )
{
       return BS->Exit(ImageHandle,
       			ExitStatus,
       			ExitDataSize,
       			ExitData);

}


EFI_STATUS 
BOOTSERVICE 
MultimodalUnloadImage(
    IN EFI_HANDLE                   ImageHandle
    )
{
       return BS->UnloadImage(ImageHandle);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalExitBootServices(
    IN EFI_HANDLE                   ImageHandle,
    IN UINTN                        MapKey
    )
{
       return BS->ExitBootServices(ImageHandle,
       			MapKey);

}


EFI_STATUS 
BOOTSERVICE 
MultimodalGetNextMonotonicCount(
    OUT UINT64                  *Count
    )
{
       return BS->GetNextMonotonicCount(Count);
}

EFI_STATUS 
BOOTSERVICE 
MultimodalStall(
    IN UINTN                    Microseconds
    )
{
       return BS->Stall(Microseconds);
}



EFI_STATUS 
BOOTSERVICE 
MultimodalSetWatchdogTimer(
    IN UINTN                    Timeout,
    IN UINT64                   WatchdogCode,
    IN UINTN                    DataSize,
    IN CHAR16                   *WatchdogData OPTIONAL
    )
{

       return BS->SetWatchdogTimer(Timeout,
       			WatchdogCode,
       			DataSize,
       			WatchdogData);
}

#endif
