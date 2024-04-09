set(SOURCES_DLL
	dllmain.cpp
	pch.cpp
)

set(HEADERS_DLL
	framework.h
	DuFunctions.h
	pch.h
)



add_library(DuFunctions SHARED ${SOURCES_DLL} ${HEADERS_DLL})

set_target_properties(DuFunctions PROPERTIES PREFIX "")

install(TARGETS DuFunctions
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include
)