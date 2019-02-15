hactool currently recognizes the following keys (## represents a hexadecimal number between 00 and 1F):

```
master_key_##                   : The ##th Firmware Master Key. Obtainable with TrustZone code execution.

package1_key_##                 : The ##th Package1 key. Obtainable with Package1ldr code execution.

package2_key_##                 : The ##th Package2 key. Derivable from master_key_## and package2_key_source.

titlekek_##                     : The ##th Titlekek. Derivable from master_key_## and titlekek_source.

package2_key_source             : Found in TrustZone .rodata.
titlekek_source                 : Found in TrustZone .rodata.
aes_kek_generation_source       : Found in TrustZone .rodata.
aes_key_generation_source       : Found in TrustZone .rodata.
key_area_key_application_source : Found in FS .rodata.
key_area_key_ocean_source       : Found in FS .rodata.
key_area_key_system_source      : Found in FS .rodata.
header_kek_source               : Found in FS .rodata.
header_key_source               : Found in FS .rodata.

header_key                      : Derivable from master_key_##, aes generation sources, and header sources.
key_area_key_application_##     : Derivable from master_key_##, aes generation sources, and key_area_key_application_source.
key_area_key_ocean_##           : Derivable from master_key_##, aes generation sources, and key_area_key_ocean_source.
key_area_key_system_##          : Derivable from master_key_##, aes generation sources, and key_area_key_system_source.
```
