# XMIDI driver: Tandy 3-voice internal speaker
add_driver(
    a32tandy
    ../XMIDI32.ASM
    SPKR32.INC
)
target_compile_definitions(a32tandy PRIVATE -DTANDY)

# XMIDI driver: IBM-PC internal speaker
add_driver(
    a32spkr
    ../XMIDI32.ASM
    SPKR32.INC
)
target_compile_definitions(a32spkr PRIVATE -DIBMPC)
