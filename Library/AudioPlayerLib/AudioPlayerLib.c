/**
  Goldfish64/AudioPkg - https://github.com/Goldfish64/AudioPkg

  A lot more easier to use BootChimeLib here. But they embed unnecessary ChimeData.
  Code taken from BootChimeLib + AudioDemo.

  cecekpawon - Thu Aug 29 17:13:41 2019
**/

#include <Uefi.h>

#include <Protocol/AudioIo.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Library/AudioPlayerLib.h>
#include <Library/WaveLib.h>

BOOLEAN   gPlaybackComplete = FALSE;

STATIC
VOID
EFIAPI
AudioPlayerAsyncCallback (
  IN  EFI_AUDIO_IO_PROTOCOL   *AudioIo,
  IN  VOID                    *Context
  )
{
  gPlaybackComplete = TRUE;
}

EFI_STATUS
EFIAPI
AudioPlayer (
  IN  UINT8   *Bytes,
  IN  UINTN   BytesLength
  )
{
  EFI_STATUS                  Status;

  UINTN                       i;

  // Audio I/O.
  EFI_HANDLE                  *AudioIoHandles;
  UINTN                       AudioIoHandleCount;
  EFI_AUDIO_IO_PROTOCOL       *AudioIo;
  BOOLEAN                     FoundAudioIo;

  EFI_AUDIO_IO_PROTOCOL_BITS  Bits;
  EFI_AUDIO_IO_PROTOCOL_FREQ  Freq;

  // Device Path.
  EFI_DEVICE_PATH_PROTOCOL    *StoredDevicePath;
  UINTN                       StoredDevicePathSize;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
  CHAR16                      *StoredDevicePathStr;

  // Output.
  UINTN                       OutputPortIndex;
  UINTN                       OutputPortIndexSize;

  // Volume.
  UINT8                       OutputVolume;
  UINTN                       OutputVolumeSize;

  // Read WAVE.
  WAVE_FILE_DATA              WaveData;

  // Keystroke.
  EFI_INPUT_KEY               Key;
  UINTN                       EventIndex;

  EFI_GUID                    gBootChimeVendorVariableGuid = BOOT_CHIME_VENDOR_VARIABLE_GUID;

  //

  StoredDevicePath = NULL;

  AudioIoHandles      = NULL;
  AudioIoHandleCount  = 0;
  Status              = gBS->LocateHandleBuffer (
                                ByProtocol,
                                &gEfiAudioIoProtocolGuid,
                                NULL,
                                &AudioIoHandleCount,
                                &AudioIoHandles
                                );

  Print (L"Audio handles %u - %r\n", AudioIoHandleCount, Status);

  if (EFI_ERROR (Status)) {
    goto DONE;
  }

  // Get stored device path size.
  StoredDevicePathSize  = 0;
  Status                = gRT->GetVariable (
                                  BOOT_CHIME_VAR_DEVICE,
                                  &gBootChimeVendorVariableGuid,
                                  NULL,
                                  &StoredDevicePathSize,
                                  NULL
                                  );
  if (EFI_ERROR (Status) && (Status != EFI_BUFFER_TOO_SMALL)) {
    Print (L"Failed to get stored %s size. Run AudioPkg - BootChimeCfg first.\n", BOOT_CHIME_VAR_DEVICE);
    goto DONE;
  }

  // Allocate space for device path.
  StoredDevicePath = AllocateZeroPool (StoredDevicePathSize);
  if (StoredDevicePath == NULL) {
    Print (L"Cannot allocate StoredDevicePath.\n");
    Status = EFI_OUT_OF_RESOURCES;
    goto DONE;
  }

  // Get stored device path string.
  Status = gRT->GetVariable (
                   BOOT_CHIME_VAR_DEVICE,
                   &gBootChimeVendorVariableGuid,
                   NULL,
                   &StoredDevicePathSize,
                   StoredDevicePath
                   );
  if (EFI_ERROR (Status)) {
    Print (L"Failed to get stored %s value. Run AudioPkg - BootChimeCfg first.\n", BOOT_CHIME_VAR_DEVICE);
    goto DONE;
  }

  StoredDevicePathStr = ConvertDevicePathToText (StoredDevicePath, FALSE, FALSE);
  if (StoredDevicePathStr != NULL) {
    Print (L"Got stored %s: %s\n", BOOT_CHIME_VAR_DEVICE, StoredDevicePathStr);
    FreePool (StoredDevicePathStr);
  }

  // Get stored device index.
  OutputPortIndex     = 0;
  OutputPortIndexSize = sizeof (OutputPortIndex);
  Status              = gRT->GetVariable (
                                BOOT_CHIME_VAR_INDEX,
                                &gBootChimeVendorVariableGuid,
                                NULL,
                                &OutputPortIndexSize,
                                &OutputPortIndex
                                );
  if (EFI_ERROR (Status)) {
    Print (L"Failed to get stored %s. Run AudioPkg - BootChimeCfg first.\n", BOOT_CHIME_VAR_INDEX);
    goto DONE;
  }

  Print (L"Got stored %s: %d\n", BOOT_CHIME_VAR_INDEX, OutputPortIndex);

  // Get stored volume.
  OutputVolume      = 0;
  OutputVolumeSize  = sizeof (OutputVolume);
  Status            = gRT->GetVariable (
                              BOOT_CHIME_VAR_VOLUME,
                              &gBootChimeVendorVariableGuid,
                              NULL,
                              &OutputVolumeSize,
                              &OutputVolume
                              );
  if (EFI_ERROR (Status)) {
    Print (L"Failed to get stored %s. Run AudioPkg - BootChimeCfg first.\n", BOOT_CHIME_VAR_VOLUME);
    goto DONE;
  }

  Print (L"Got stored %s: %d\n", BOOT_CHIME_VAR_VOLUME, OutputVolume);

  FoundAudioIo = FALSE;

  // Try to find the matching device exposing an Audio I/O protocol.
  for (i = 0; i < AudioIoHandleCount; i++) {
    // Open Device Path protocol.
    DevicePath  = NULL;
    Status      = gBS->HandleProtocol (AudioIoHandles[i], &gEfiDevicePathProtocolGuid, (VOID**)&DevicePath);
    if (EFI_ERROR (Status)) {
      continue;
    }

    // Compare Device Paths. If they match, we have our Audio I/O device.
    if (!CompareMem (StoredDevicePath, DevicePath, StoredDevicePathSize)) {
      // Open Audio I/O protocol.
      AudioIo = NULL;
      Status  = gBS->HandleProtocol (AudioIoHandles[i], &gEfiAudioIoProtocolGuid, (VOID**)&AudioIo);
      if (!EFI_ERROR (Status)) {
        Print (L"Found AudioIo @ index %d.\n", i);
        FoundAudioIo = TRUE;
      }
      break;
    }
  }

  // If the Audio I/O variable is still null, we couldn't find it.
  if (!FoundAudioIo) {
    Print (L"No matching AudioIo found.\n");
    Status = EFI_NOT_FOUND;
    goto DONE;
  }

  // Read WAVE.
  Status = WaveGetFileData (Bytes, (UINT32)BytesLength, &WaveData);
  if (EFI_ERROR (Status)) {
    Print (L"WaveGetFileData failed.\n");
    goto DONE;
  }

  Print (L"Data length: %u bytes\n", WaveData.DataLength);
  Print (L"Format length: %u bytes\n", WaveData.FormatLength);
  Print (L"  Channels: %u  Sample rate: %u Hz  Bits: %u\n",
            WaveData.Format->Channels,
            WaveData.Format->SamplesPerSec,
            WaveData.Format->BitsPerSample);
  Print (L"Samples length: %u bytes\n", WaveData.SamplesLength);

  switch (WaveData.Format->BitsPerSample) {
    case 16:
      Bits = EfiAudioIoBits16;
      break;

    case 20:
      Bits = EfiAudioIoBits20;
      break;

    case 24:
      Bits = EfiAudioIoBits24;
      break;

    default:
      Bits = 0;
      Status = EFI_UNSUPPORTED;
      break;
  }

  if (EFI_ERROR (Status)) {
    Print (L"Invalid BitsPerSample.\n");
    goto DONE;
  }

  switch (WaveData.Format->SamplesPerSec) {
    case 44100:
      Freq = EfiAudioIoFreq44kHz;
      break;

    case 48000:
      Freq = EfiAudioIoFreq48kHz;
      break;

    default:
      Freq = 0;
      Status = EFI_UNSUPPORTED;
      break;
  }

  if (EFI_ERROR (Status)) {
    Print (L"Invalid SamplesPerSec.\n");
    goto DONE;
  }

  Status = AudioIo->SetupPlayback (AudioIo, (UINT8)i, OutputVolume, Freq, Bits, (UINT8)WaveData.Format->Channels);
  if (EFI_ERROR (Status)) {
    Print (L"SetupPlayback failed.\n");
    goto DONE;
  }

  //Status = AudioIo->StartPlayback (AudioIo, WaveData.Samples, WaveData.SamplesLength, 0);
  Status = AudioIo->StartPlaybackAsync (AudioIo, WaveData.Samples, WaveData.SamplesLength, 0, AudioPlayerAsyncCallback, NULL);
  if (EFI_ERROR (Status)) {
    Print (L"StartPlayback failed.\n");
    goto DONE;
  }

  Print (L"Press ESC to exit.\n");

  do {
    gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &EventIndex);

    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    if (EFI_ERROR (Status)) {
      if (Status == EFI_NOT_READY) {
        continue;
      }
      break;
    }

    if (Key.ScanCode == SCAN_ESC) {
      gPlaybackComplete = TRUE;
      Status = AudioIo->StopPlayback (AudioIo);
      Print (L"StopPlayback - %r.\n", Status);
      break;
    }
  } while (!gPlaybackComplete);

  Status = EFI_SUCCESS;

DONE:

  if (StoredDevicePath != NULL) {
    FreePool (StoredDevicePath);
  }

  if (AudioIoHandles != NULL) {
    FreePool (AudioIoHandles);
  }

  return Status;
}
