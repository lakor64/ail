set(SOURCES
    AIL32.C
    AIL32.H
    AIL32DRIVER.H
    AILDLL.H
    AILBASE.H
    AIL32INTERNAL.H
)

if (DOS OR OS2)
    list(APPEND SOURCES 
        AIL32DOS.ASM
        AIL386.MAC
        AIL32.INC
    )
endif()

if (OS2)
    list(APPEND SOURCES DLLLOADLX.C)
elseif (WIN32)
    list(APPEND SOURCES DLLLOADW32.C)
endif()

add_library(ail32 STATIC ${SOURCES})

target_include_directories(ail32 PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_compile_definitions(ail32 PUBLIC -DDPMI)

# TODO: Replace ail32.inc with ch2inc -> ail32driver.h, ch2inc -> ail32internal.h

