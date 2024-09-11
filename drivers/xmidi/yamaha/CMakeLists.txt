# XMIDI driver: Standard Ad Lib or compatible
add_driver(
    a32adlib
    ../XMIDI32.ASM
    YAMAHA32.INC
)
target_compile_definitions(a32sbfm PRIVATE -ADLIBSTD)

# XMIDI driver: Ad Lib Gold
add_driver(
    a32algfm
    ../XMIDI32.ASM
    YAMAHA32.INC
)
target_compile_definitions(a32sbfm PRIVATE -DADLIBG)

# XMIDI driver: Standard Sound Blaster (OPL2)
add_driver(
    a32sbfm
    ../XMIDI32.ASM
    YAMAHA32.INC
)
target_compile_definitions(a32sbfm PRIVATE -DSBSTD)

# XMIDI driver: Sound Blaster Pro I (dual-3812 version)
add_driver(
    a32sp1fm
    ../XMIDI32.ASM
    YAMAHA32.INC
)
target_compile_definitions(a32sp1fm PRIVATE -DSBPRO1)

# XMIDI driver: Sound Blaster Pro II (OPL3 version) XMIDI driver
add_driver(
    a32sp2fm
    ../XMIDI32.ASM
    YAMAHA32.INC
)
target_compile_definitions(a32sp2fm PRIVATE -DSBPRO2)

# XMIDI driver: Pro Audio Spectrum (dual-3812 version)
add_driver(
    a32pasfm
    ../XMIDI32.ASM
    YAMAHA32.INC
)
target_compile_definitions(a32sp2fm PRIVATE -DPAS)

# XMIDI driver: Pro Audio Spectrum Plus/16 (with OPL3)
add_driver(
    a32pasop
    ../XMIDI32.ASM
    YAMAHA32.INC
)
target_compile_definitions(a32sp2fm PRIVATE -DPAS)
