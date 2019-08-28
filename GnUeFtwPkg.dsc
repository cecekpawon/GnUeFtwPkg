#
# GnUeFtwPkg - https://github.com/cecekpawon/GnUeFtwPkg
# cecekpawon - Wed Aug 14 15:57:45 2019
#

[Defines]
  BUILD_TARGETS           = DEBUG|RELEASE|NOOPT
  PLATFORM_NAME           = GnUeFtwPkg
  PLATFORM_GUID           = EB86B2C2-D7B5-474B-ABCB-A2AEFAC9B3EF
  PLATFORM_VERSION        = 0.1
  SKUID_IDENTIFIER        = DEFAULT
  DSC_SPECIFICATION       = 0x00010006
  SUPPORTED_ARCHITECTURES = X64 #|IA32|ARM|AARCH64

  DEFINE GLOBAL_OPTIONS =

  !if $(TARGET) == RELEASE
    DEFINE GLOBAL_OPTIONS = $(GLOBAL_OPTIONS) -DNDEBUG=TRUE -DMDEPKG_NDEBUG=TRUE
  !elseif $(TARGET) == DEBUG
    #
  !else
    #
  !endif

[LibraryClasses]
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibRepStr/BaseMemoryLibRepStr.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLibDevicePathProtocol/UefiDevicePathLibDevicePathProtocol.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  #
  #
  #
  ZlibLib|GnUeFtwPkg/Library/ZlibLib/ZlibLib.inf

[Components]
  GnUeFtwPkg/Module/edid/edid.inf
  GnUeFtwPkg/Module/hexdump/hexdump.inf
  GnUeFtwPkg/Module/HexDumpW/HexDumpW.inf
  GnUeFtwPkg/Module/tree/tree.inf
  GnUeFtwPkg/Module/Treem/Treem.inf
  GnUeFtwPkg/Module/ZlibZip/minigzip/minigzip.inf
  GnUeFtwPkg/Module/ZlibZip/minizip/miniunz.inf
  GnUeFtwPkg/Module/ZlibZip/minizip/minizip.inf

!include StdLib/StdLib.inc

[BuildOptions]
  #DEFINE  GLOBAL_OPTIONS    = $(GLOBAL_OPTIONS) -DDISABLE_NEW_DEPRECATED_INTERFACES=TRUE

  MSFT:*_*_*_CC_FLAGS   = $(GLOBAL_OPTIONS) /wd4131
