/**
  Goldfish64/AudioPkg - https://github.com/Goldfish64/AudioPkg

  A lot more easier to use BootChimeLib here. But they embed unnecessary ChimeData.
  Code taken from BootChimeLib + AudioDemo.

  cecekpawon - Thu Aug 29 17:13:41 2019
**/

#ifndef POCKETMOD_PLAYER_H
#define POCKETMOD_PLAYER_H

#define BOOT_CHIME_DEFAULT_VOLUME   EFI_AUDIO_IO_PROTOCOL_MAX_VOLUME

// BootChime vendor variable GUID.
#define BOOT_CHIME_VENDOR_VARIABLE_GUID { \
  0x89D4F995, 0x67E3, 0x4895, { 0x8F, 0x18, 0x45, 0x4B, 0x65, 0x1D, 0x92, 0x15 } \
}

#define BOOT_CHIME_VAR_ATTRIBUTES   (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS)
#define BOOT_CHIME_VAR_DEVICE       (L"Device")
#define BOOT_CHIME_VAR_INDEX        (L"Index")
#define BOOT_CHIME_VAR_VOLUME       (L"Volume")

EFI_STATUS
PocketModPlayer (
  UINT8   *Bytes,
  UINTN   BytesLength
  );

#endif
