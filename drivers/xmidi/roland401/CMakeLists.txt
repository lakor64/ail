# XMIDI driver: MT-32 family with Roland MPU-401-compatible interface
add_driver(a32mt32 SHARED
    ../XMIDI32.ASM
    MT3232.INC
    MPU40132.INC
)
target_compile_definitions(a32mt32 PRIVATE -DMT32 -DMPU401)
