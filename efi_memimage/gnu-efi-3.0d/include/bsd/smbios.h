/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and
 *    its contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/*
Module Name:

    smbios.h
    
Abstract:
	Include file for EFI SMBIOS
--*/

/*
 *  Core EFI header files
 */
#ifndef _EFI_SMBIOS_H_
#define _EFI_SMBIOS_H_

/*
 SMBIOS 2.2 Structure Table Entry Point
*/
#pragma pack(1)
typedef struct SMBIOSTableEntryPoint
{
	UINT8	AnchorString[4];	/* _SM_ */
	UINT8	EPSChecksum;		/* Checksum of Entry Point Structure*/
	UINT8	EPLength;			/* Length of the Entry Point Structure*/
	UINT8	MajorVersion;		/* Major version of the spec*/
	UINT8	MinorVersion;		/* Minor version of the spec*/
	UINT16	MaxStructSize;		/* Size of the largest SMBIOS Structure*/
	UINT8	EPRevision;			/* Entry Point Revision*/
	UINT8	FormattedArea[5];	/* The value present in the EP Revision field */

/* Defines the interpretation to be placed*/
	UINT8	InterAnchorString[5];	/* _DMI_*/
	UINT8	InterChecksum;			/* Checksum of Intermediate EPS*/
	UINT16	StructTableLen;			/* Total length of SMBIOS Structure Table*/
 	UINT32	StructTableAddr;		/* 32-bit physical starting address of the */

/* Read-only Structure Table*/
	UINT16	NumStruct;			/* Total number of structures*/
	UINT8	BCDRevision;		/* Compliance with a revision of this spec*/
} SMBIOSTableEntryPoint;

/*Each SMBIOS structure begins with a 4-byte header, as follows:*/

typedef struct SMBIOSHeader
{
	UINT8	StructType;
	UINT8	StructLength;
	UINT16	StructHandle;
} SMBIOSHeader;

/*
  In addition to standard EFI status codes, this specification
  defines additional return values which are compatible with
  the EFI_ERROR() macro 

Define							Value				Meaning
EFI_SMBIOSERR_FAILURE			EFI_SMBIOSERR(1)	Implementation specific error
EFI_SMBIOSERR_STRUCT_NOT_FOUND	EFI_SMBIOSERR(2)	The specified structure not found
EFI_SMBIOSERR_TYPE_UNKNOWN		EFI_SMBIOSERR(3)	The specified the type is unknown
EFI_SMBIOSERR_UNSUPPORTED		EFI_SMBIOSERR(4)	System does not support SMBIOS
*/

#define EFI_SMBIOSERR(val)	EFIERR_OEM(0x30000 | val)

#define EFI_SMBIOSERR_FAILURE			EFI_SMBIOSERR(1)
#define EFI_SMBIOSERR_STRUCT_NOT_FOUND	EFI_SMBIOSERR(2)
#define EFI_SMBIOSERR_TYPE_UNKNOWN		EFI_SMBIOSERR(3)
#define EFI_SMBIOSERR_UNSUPPORTED		EFI_SMBIOSERR(4)

typedef struct DeviceStruct
{
	UINT8	DeviceType;
	CHAR8	DescriptionString[64];
} DeviceStruct;

enum enumStructureType
{
	eSMBIOSType0 = 0,
	eSMBIOSType1 = 1,
	eSMBIOSType2 = 2,
	eSMBIOSType3 = 3,
	eSMBIOSType4 = 4,
	eSMBIOSType5 = 5,
	eSMBIOSType6 = 6,
	eSMBIOSType7 = 7,
	eSMBIOSType8 = 8,
	eSMBIOSType9 = 9,
	eSMBIOSType10 = 10,
	eSMBIOSType11 = 11,
	eSMBIOSType12 = 12,
	eSMBIOSType13 = 13,
	eSMBIOSType14 = 14,
	eSMBIOSType15 = 15,
	eSMBIOSType16 = 16,
	eSMBIOSType17 = 17,
	eSMBIOSType18 = 18,
	eSMBIOSType19 = 19,
	eSMBIOSType20 = 20,
	eSMBIOSType21 = 21,
	eSMBIOSType22 = 22,
	eSMBIOSType23 = 23,
	eSMBIOSType24 = 24,
	eSMBIOSType25 = 25,
	eSMBIOSType26 = 26,
	eSMBIOSType27 = 27,
	eSMBIOSType28 = 28,
	eSMBIOSType29 = 29,
	eSMBIOSType30 = 30,
	eSMBIOSType32 = 32,
	eSMBIOSType33 = 33,
	eSMBIOSType34 = 34,
	eSMBIOSType35 = 35,
	eSMBIOSType36 = 36,
	eSMBIOSType37 = 37,
	eSMBIOSType38 = 38,
	eSMBIOSType39 = 39,
	eSMBIOSType126 = 126,
	eSMBIOSType127 = 127
};

/*Appendix 3:  SMBIOS Structure Definitions*/

typedef struct SMBIOSType0
{
	SMBIOSHeader	Header;
	CHAR8			Vendor[64];
	CHAR8			BIOSVersion[64];
	UINT16			BIOSStartAddrSeg;
	CHAR8			BIOSReleaseDate[64];
	UINT8			BIOSROMSize;
	UINT64			BIOSCharacteristics;
	CHAR8			CharacteristicsExtSize;
	UINT8			BIOSCharacteristicsExt[1];
} SMBIOSType0;

typedef struct SMBIOSType1
{
	SMBIOSHeader	Header;
	CHAR8			Manufacturer[64];
	CHAR8			ProductName[64];
	CHAR8			Version[64];
	CHAR8			SerialNumber[64];
	UINT8			UUID[16];
	UINT8			WakeUpType;
} SMBIOSType1;

typedef struct SMBIOSType2
{
	SMBIOSHeader	Header;
	CHAR8			Manufacturer[64];
	CHAR8			ProductName[64];
	CHAR8			Version[64];
	CHAR8			SerialNumber[64];
} SMBIOSType2;

typedef struct SMBIOSType3
{
	SMBIOSHeader	Header;
	CHAR8			Manufacturer[64];
	UINT8			ChassisType;
	CHAR8			Version[64];
	CHAR8			SerialNumber[64];
	CHAR8			AssetTagNumber[64];
	UINT8			BootupState;
	UINT8			PowerSupplyState;
	UINT8			ThermalState;
	UINT8			SecurityStatus;
	UINT32			OEMDefined;
} SMBIOSType3;

typedef struct SMBIOSType4
{
	SMBIOSHeader	Header;
	CHAR8			SocketDesignation[64];
	UINT8			ProcessorType;
	UINT8			ProcessorFamily;
	CHAR8			ProcessorManufacturer[64];
	UINT64			ProcessorID;
	CHAR8			ProcessorVersion[64];
	UINT8			Voltage;
	UINT16			ExternalClock;
	UINT16			MaxSpeed;
	UINT16			CurrentSpeed;
	UINT8			Status;
	UINT8			ProcessorUpgrade;
	UINT16			L1CacheHandle;
	UINT16			L2CacheHandle;
	UINT16			L3CacheHandle;
	CHAR8			SerialNumber[64];
	CHAR8			AssetTag[64];
	CHAR8			PartNumber[64];
} SMBIOSType4;

typedef struct SMBIOSType5
{
	SMBIOSHeader	Header;
	UINT8			ErrorDetectingMethod;
	UINT8			ErrorCorrectingCapability;
	UINT8			SupportedInterleave;
	UINT8			CurrentInterleave;
	UINT8			MaximumMemoryModuleSize;
	UINT16			SupportedSpeeds;
	UINT16			SupportedMemoryTypes;
	UINT8			MemoryModuleVoltage;
	UINT8			AssociatedMemorySlots;
	UINT16			MemoryModuleConfigHandle;
	UINT8			EnabledErrorCorrectingCapabilities;
} SMBIOSType5;

typedef struct SMBIOSType6
{
	SMBIOSHeader	Header;
	CHAR8			SocketDesignation[64];
	UINT8			BankConnections;
	UINT8			CurrentSpeed;
	UINT16			CurrentMemoryType;
	UINT8			InstalledSize;
	UINT8			EnabledSize;
	UINT8			ErrorStatus;
} SMBIOSType6;

typedef struct SMBIOSType7
{
	SMBIOSHeader	Header;
	CHAR8			SocketDesignation[64];
	UINT16			CacheConfiguration;
	UINT16			MaximumCacheSize;
	UINT16			InstalledSize;
	UINT16			SupportedSRAMType;
	UINT16			CurrentSRAMType;
	UINT8			CacheSpeed;
	UINT8			ErrorCorrectionType;
	UINT8			SystemCacheType;
	UINT8			Associativity;
} SMBIOSType7;

typedef struct SMBIOSType8
{
	SMBIOSHeader	Header;
	CHAR8			InternalReferenceDesignator[64];
	UINT8			InternalConnectorType;
	CHAR8			ExternalReferenceDesignator[64];
	UINT8			ExternalConnectorType;
	UINT8			PortType;
} SMBIOSType8;

typedef struct SMBIOSType9
{
	SMBIOSHeader	Header;
	CHAR8			SlotDesignation[64];
	UINT8			SlotType;
	UINT8			SlotDataBusWidth;
	UINT8			CurrentUsage;
	UINT8			SlotLength;
	UINT16			SlotID;
	UINT8			SlotCharacteristics;
	UINT8			SlotCharacteristics2;
} SMBIOSType9;

typedef struct SMBIOSType10
{
	SMBIOSHeader	Header;
	DeviceStruct	Device;
} SMBIOSType10;

typedef struct SMBIOSType11
{
	SMBIOSHeader	Header;
	UINT8			Count;
	CHAR8			bufOEMString[1][64];
} SMBIOSType11;

typedef struct SMBIOSType12
{
	SMBIOSHeader	Header;
	UINT8			Count;
	CHAR8			bufSysConfigurations[1][64];
} SMBIOSType12;

typedef struct SMBIOSType13
{
	SMBIOSHeader	Header;
	UINT8			InstallableLanguages;
	UINT8			Flags;
	UINT8			reserved[15];
	UINT8			CurrentLanguageIndex;
	CHAR8			IntalledLanguages[1][64];
} SMBIOSType13;

typedef struct SMBIOSType14
{
	SMBIOSHeader	Header;
	CHAR8			GroupName[64];
	UINT8			ItemType;
	UINT16			ItemHandle;
} SMBIOSType14;

typedef struct EVENTLOGTYPE
{
	UINT8			LogType;
	UINT8			DataFormatType;
} EVENTLOGTYPE;

typedef struct SMBIOSType15
{
	SMBIOSHeader	Header;
	UINT16			LogAreaLength;
	UINT16			LogHeaderStartOffset;
	UINT16			LogDataStartOffset;
	UINT8			AccessMethod;
	UINT8			LogStatus;
	UINT32			LogChangeToken;
	UINT32			AccessMethodAddress;
	UINT8			LogHeaderFormat;
	UINT8			NumberOfSupportedLogTypeDescriptors;
	UINT8			LengthOfLogTypeDescriptor;
	EVENTLOGTYPE	EventLogTypeDescriptors[1];
} SMBIOSType15;

typedef struct SMBIOSType16
{
	SMBIOSHeader	Header;
	UINT8			Location;
	UINT8			Use;
	UINT8			MemoryErrorCorrection;
	UINT32			MaximumCapacity;
	UINT16			MemoryErrorInformationHandle;
	UINT16			NumberOfMemoryDevices;
} SMBIOSType16;

typedef struct SMBIOSType17
{
	SMBIOSHeader	Header;
	UINT16			MemoryArrayHandle;
	UINT16			MemoryErrorInformationHandle;
	UINT16			TotalWidth;
	UINT16			DataWidth;
	UINT16			Size;
	UINT8			FormFactor;
	UINT8			DeviceSet;
	CHAR8			DeviceLocator[64];
	CHAR8			BankLocator[64];
	UINT8			MemoryType;
	UINT16			TypeDetail;
	UINT16			Speed;
	CHAR8			Manufacturer[64];
	CHAR8			SerialNumber[64];
	CHAR8			AssetTag[64];
	CHAR8			PartNumber[64];
} SMBIOSType17;

typedef struct SMBIOSType18
{
	SMBIOSHeader	Header;
	UINT8			ErrorType;
	UINT8			ErrorGranularity;
	UINT8			ErrorOperation;
	UINT32			VendorSyndrome;
	UINT32			MemoryArrayErrorAddress;
	UINT32			DeviceErrorAddress;
	UINT32			ErrorResolution;
} SMBIOSType18;

typedef struct SMBIOSType19
{
	SMBIOSHeader	Header;
	UINT32			StartingAddress;
	UINT32			EndingAddress;
	UINT16			MemoryArrayHandle;
	UINT8			PartitionWidth;
} SMBIOSType19;

typedef struct SMBIOSType20
{
	SMBIOSHeader	Header;
	UINT32			StartingAddress;
	UINT32			EndingAddress;
	UINT16			MemoryDeviceHandle;
	UINT16			MemoryArrayMappedAddressHandle;
	UINT8			PartitionRowPosition;
	UINT8			InterleavePosition;
	UINT8			InterleavedDataDepth;
} SMBIOSType20;

typedef struct SMBIOSType21
{
	SMBIOSHeader	Header;
	UINT8			Type;
	UINT8			Interface;
	UINT8			NumberOfButtons;
} SMBIOSType21;

typedef struct SMBIOSType22
{
	SMBIOSHeader	Header;
	CHAR8			Location[64];
	CHAR8			Manufacturer[64];
	CHAR8			ManufactureDate[64];
	CHAR8			SerialNumber[64];
	CHAR8			DeviceName[64];
	CHAR8			BankLocator[64];
	UINT8			DeviceChemistry;
	UINT16			DeviceCapacity;
	UINT16			DesignVoltage;
	CHAR8			SBDSVersionNumber[64];
	UINT8			MaximumErrorInBatteryData;
	UINT16			SBDSSerialNumber;
	UINT16			SBDSManufactureDate;
	CHAR8			SBDSDeviceChemistry[64];
	UINT8			DesignCapacityMultiplier;
	UINT32			OEMSpecific;
} SMBIOSType22;

typedef struct SMBIOSType23
{
	SMBIOSHeader	Header;
	UINT8			Capabilities;
	UINT16			ResetCount;
	UINT16			ResetLimit;
	UINT16			TimerInterval;
	UINT16			Timeout;
} SMBIOSType23;

typedef struct SMBIOSType24
{
	SMBIOSHeader	Header;
	UINT8			HardwareSecuritySettings;
} SMBIOSType24;

typedef struct SMBIOSType25
{
	SMBIOSHeader	Header;
	UINT8			NextScheduledPowerOnMonth;
	UINT8			NextScheduledPowerOnDayOfMonth;
	UINT8			NextScheduledPowerOnHour;
	UINT8			NextScheduledPowerOnMinute;
	UINT8			NextScheduledPowerOnSecond;
} SMBIOSType25;

typedef struct SMBIOSType26
{
	SMBIOSHeader	Header;
	CHAR8			Description[64];
	UINT8			LocationAndStatus;
	UINT16			MaximumValue;
	UINT16			MinimumValue;
	UINT16			Resolution;
	UINT16			Tolerance;
	UINT16			Accuracy;
	UINT32			OEMDefined;
	UINT16			NominalValue;
} SMBIOSType26;

typedef struct SMBIOSType27
{
	SMBIOSHeader	Header;
	UINT16			TemperatureProbeHandle;
	UINT8			DeviceTypeAndStatus;
	UINT8			CoolingUnitGroup;
	UINT32			OEMDefined;
	UINT16			NominalSpeed;
} SMBIOSType27;

typedef struct SMBIOSType28
{
	SMBIOSHeader	Header;
	CHAR8			Description[64];
	UINT8			LocationAndStatus;
	UINT16			MaximumValue;
	UINT16			MinimumValue;
	UINT16			Resolution;
	UINT16			Tolerance;
	UINT16			Accuracy;
	UINT32			OEMDefined;
	UINT32			NominalValue;
} SMBIOSType28;

typedef struct SMBIOSType29
{
	SMBIOSHeader	Header;
	CHAR8			Description[64];
	UINT8			LocationAndStatus;
	UINT16			MaximumValue;
	UINT16			MinimumValue;
	UINT16			Resolution;
	UINT16			Tolerance;
	UINT16			Accuracy;
	UINT32			OEMDefined;
	UINT16			NominalValue;
} SMBIOSType29;

typedef struct	 SMBIOSType30
{
	SMBIOSHeader	Header;
	CHAR8			ManufacturerName[64];
	UINT8			Connections;
} SMBIOSType30;

typedef struct SMBIOSType32
{
	SMBIOSHeader	Header;
	UINT8			Reserved[6];
	UINT8			BootStatus;
} SMBIOSType32;

typedef struct SMBIOSType33
{
	SMBIOSHeader	Header;
	UINT8			ErrorType;
	UINT8			ErrorGranularity;
	UINT8			ErrorOperation;
	UINT32			VendorSyndrome;
	UINT64			MemoryArrayErrorAddress;
	UINT64			DeviceErrorAddress;
	UINT32			ErrorResolution;
} SMBIOSType33;

typedef struct SMBIOSType34
{
	SMBIOSHeader	Header;
	CHAR8			Description[64];
	UINT8			Type;
	UINT32			Address;
	UINT8			AddressType;
} SMBIOSType34;

typedef struct SMBIOSType35
{
	SMBIOSHeader	Header;
	CHAR8			Description[64];
	UINT16			ManagementDeviceHandle;
	UINT16			ComponentHandle;
	UINT16			ThresholdHandle;
} SMBIOSType35;

typedef struct SMBIOSType36
{
	SMBIOSHeader	Header;
	UINT16			LowerThresholdNonCritical;
	UINT16			UpperThresholdNonCritical;
	UINT16			LowerThresholdCritical;
	UINT16			UpperThreaholdCritical;
	UINT16			LowerThresholdNonRecoverable;
	UINT16			UpperThresholdNonRecoverable;
} SMBIOSType36;

typedef struct MEMORYDEVICE
{
	UINT8			DeviceLoad;
	UINT16			DeviceHandle;
} MEMORYDEVICE;

typedef struct SMBIOSType37
{
	SMBIOSHeader	Header;
	UINT8			ChannelType;
	UINT8			MaximumChannelLoad;
	UINT8			MemoryDeviceCount;
	MEMORYDEVICE	MemoryDevice[1];
} SMBIOSType37;

typedef struct SMBIOSType38
{
	SMBIOSHeader	Header;
	UINT8			InterfaceType;
	UINT8			IPMISpecificationRevision;
	UINT8			I2CSlaveAddress;
	UINT8			NVStorageDeviceAddress;
	UINT64			BaseAddress;
	UINT8			BaseAddressModifier_InterruptInfo;
	UINT8			InterruptNumber;
} SMBIOSType38;

typedef struct SMBIOSType39
{
	SMBIOSHeader	Header;
	UINT8			PowerUnitGroup;
	CHAR8			Location[64];
	CHAR8			DeviceName[64];
	CHAR8			Manufacturer[64];
	CHAR8			SerialNumber[64];
	CHAR8			AssetTagNumber[64];
	CHAR8			ModelPartNumber[64];
	CHAR8			RevisionLevel[64];
	CHAR8			Description[64];
	UINT16			MaxPowerCapacity;
	UINT16			PowerSupplyCharacteristics;
	UINT16			InputVoltageProbeHandle;
	UINT16			CoolingDeviceHandle;
	UINT16			InputCurrentProbeHandle;
} SMBIOSType39;

typedef struct SMBIOSType126
{
	SMBIOSHeader	Header;
} SMBIOSType126;

typedef struct SMBIOSType127
{
	SMBIOSHeader	Header;
} SMBIOSType127;
#pragma pack()

EFI_STATUS
SMBIOS_Initialize (
	IN	EFI_HANDLE          ImageHandle,
	IN	EFI_SYSTEM_TABLE	*SystemTable
	);

EFI_STATUS
SMBIOS_GetTableEntryPoint (
	IN OUT SMBIOSTableEntryPoint **pSMBIOSInfo
	);

EFI_STATUS
SMBIOS_GetStructure (
	IN		UINT16		Type,
	IN		UINT16		Handle,
	IN OUT	VOID		**pStructureBuffer,
	IN OUT	UINT16		*Length,
	IN OUT	UINT16		*Key
	);

EFI_STATUS
SMBIOS_FreeStructure (
	IN		VOID	*pStructureBuffer
);

EFI_STATUS
SMBIOS_GetRawStructure (
	IN		UINT16		Type,
	IN		UINT16		Handle,
	IN OUT	VOID		**pRawBuffer,
	IN OUT	UINT16		*Length,
	IN OUT	UINT16		*Key
	);

EFI_STATUS
NextTableAddress (
	IN	CHAR8	*TableAddress,
	OUT	UINT16	*offset
	);

BOOLEAN
CheckEnumType (
	IN UINT8 Type
	);

EFI_STATUS
GetPendingString(
	IN	UINT8	*StartAddress,
	IN	UINT16	StringNumber,
	OUT CHAR8	StringBuf[64]
	);

UINT16
CopyStruct(
	IN	UINT8	Type,
	OUT	VOID	**StructureBuffer,
	IN	UINT8	*TableAddress
	);

DUMPMEM (void* v, int length);

/*
  General macro for copying packed data buffer to
  an aligned structure.  The macro assumes successive
  elements will be copied so it automatically advances
  the packed data pointer.

*/

#define AlignedCopy( type, src, dst, element ) \
	memcpy((UINT8*)&( ((type*)dst)->element), src, sizeof(((type*)dst)->element));\
    src += sizeof(((type*)dst)->element);

#endif	/* _EFI_SMBIOS_H_ */
