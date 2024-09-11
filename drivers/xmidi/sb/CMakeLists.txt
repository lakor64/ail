# XMIDI driver: MT-32 family with Sound Blaster MIDI-compatible interface
add_driver(a32mt32s SHARED
    ../XMIDI32.ASM
    SBMIDI32.INC
)
target_compile_definitions(a32mt32 PRIVATE -DMT32 -DSBMIDI)
