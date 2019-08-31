/*
  GnUeFtwPkg - Smbios2Bin
  cecekpawon - 8/31/2019 2:39:33 PM
*/

#include <Uefi.h>

#include <IndustryStandard/SmBios.h>

#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

void *
GetTable (
  IN  EFI_GUID  *Guid
  )
{
  UINTN i;

  for (i = 0; i < gST->NumberOfTableEntries; i++) {
    if (CompareGuid (&gST->ConfigurationTable[i].VendorGuid, Guid)) {
      return gST->ConfigurationTable[i].VendorTable;
    }
  }

  return NULL;
}

static
void
dump2Bin (char *filename, uint8_t *buffer, int length) {
  FILE *file;

  file = fopen(filename, "wb");
  if (file != NULL) {
    fwrite (buffer, 1, length, file);
    fclose (file);
    printf("Success: 'dmidecode -d %s' to parse from OS\n", filename);
  } else {
    printf("Error: saving file '%s'\n", filename);
  }
}

static
void
getSmbios (uint8_t **Buffer, int *BufferLength)
{
  SMBIOS_TABLE_ENTRY_POINT          *SmbiosTable;
  SMBIOS_TABLE_3_0_ENTRY_POINT      *Smbios3Table;
  VOID                              *SmbiosTableEp;
  UINT8                             *BufferPointer;
  UINTN                             TableAddress;
  UINTN                             StructureSize;
  UINTN                             MaxTableLength;
  UINTN                             MaxCurrentTableLength;
  UINTN                             MaxCurrentTableLengthAll;
  UINT8                             MajorVersion;
  UINT8                             MinorVersion;

  //

  StructureSize             = 0;
  MaxTableLength            = 0;
  MaxCurrentTableLength     = 0;
  MaxCurrentTableLengthAll  = 0;
  MajorVersion              = 0;
  MinorVersion              = 0;
  SmbiosTableEp             = NULL;
  BufferPointer             = NULL;

  SmbiosTable = (SMBIOS_TABLE_ENTRY_POINT *)GetTable (&gEfiSmbiosTableGuid);
  if ((SmbiosTable != NULL)
    && (SmbiosTable->TableAddress != 0)
    )
  {
    SmbiosTableEp             = SmbiosTable;
    TableAddress              = SmbiosTable->TableAddress;
    BufferPointer             = (UINT8 *)(UINTN)SmbiosTable->TableAddress;
    StructureSize             = sizeof (SMBIOS_TABLE_ENTRY_POINT);
    MaxTableLength            = SMBIOS_TABLE_MAX_LENGTH;
    MaxCurrentTableLength     = SmbiosTable->TableLength;
    MajorVersion              = SmbiosTable->MajorVersion;
    MinorVersion              = SmbiosTable->MinorVersion;
  }

  else {
    Smbios3Table = (SMBIOS_TABLE_3_0_ENTRY_POINT *)GetTable (&gEfiSmbios3TableGuid);
    if ((Smbios3Table != NULL)
      && (Smbios3Table->TableAddress != 0)
      )
    {
      SmbiosTableEp           = Smbios3Table;
      TableAddress            = Smbios3Table->TableAddress;
      BufferPointer           = (UINT8 *)(UINTN)TableAddress;
      StructureSize           = sizeof (SMBIOS_TABLE_3_0_ENTRY_POINT);
      MaxTableLength          = SMBIOS_3_0_TABLE_MAX_LENGTH;
      MaxCurrentTableLength   = Smbios3Table->TableMaximumSize;
      MajorVersion            = Smbios3Table->MajorVersion;
      MinorVersion            = Smbios3Table->MinorVersion;
    }
  }

  if (BufferPointer != NULL) {
    printf("Found DMI v%d.%d @ %p Addr 0x%08x Len %ld\n",
              MajorVersion,
              MinorVersion,
              SmbiosTableEp,
              (UINT32)TableAddress,
              (UINT32)MaxCurrentTableLength);

    MaxCurrentTableLengthAll = MaxCurrentTableLength + StructureSize;
    if (MaxCurrentTableLengthAll > MaxTableLength) {
      printf("Error: invalid max table length %ld > %ld\n", (UINT32)MaxCurrentTableLengthAll, (UINT32)MaxTableLength);
    }

    else {

      if (Buffer != NULL) {
        *Buffer = (uint8_t *)BufferPointer;
      }

      if (BufferLength != NULL) {
        *BufferLength = (int)MaxCurrentTableLength;
      }
    }
  }

  else {
    printf("Error: no table found\n");
  }
}

int
main(int argc, char *argv[])
{
  uint8_t   *buffer = NULL;
  int       length = 0;

  if (argc != 2) {
    printf("Usage: %s <outfile>\n", argv[0]);
  } else {
    getSmbios (&buffer, &length);
    if ((buffer != NULL) && (length != 0)) {
      dump2Bin (argv[1], buffer, length);
    }
  }

  return 0;
}
