add_subdirectory(null)

if (DOS OR OS2)
    add_subdirectory(xmidi)
    add_subdirectory(dmasnd)
endif()

