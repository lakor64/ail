# Digital sound driver: Ad Lib Gold
add_driver(a32algdg dmasnd32.asm)
target_compile_definitions(a32algdg PRIVATE -DADLIBG)

# Digital sound driver: Standard Sound Blaster
add_driver(a32sbdg dmasnd32.asm)
target_compile_definitions(a32sbdg PRIVATE -DSBSTD)

# Digital sound driver: Sound Blaster Pro
add_driver(a32sbpdg dmasnd32.asm)
target_compile_definitions(a32sbpdg PRIVATE -DSBPRO)

# Digital sound driver: Pro Audio Spectrum
add_driver(a32pasdg dmasnd32.asm)
target_compile_definitions(a32pasdg PRIVATE -DPAS)
