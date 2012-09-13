# -*- cmake -*-

project(iw_imagebase)

if (LINUX)
    include(00-Common)
    include(Variables)
endif (LINUX)

include_directories(
	${IW_IMAGEBASE_INCLUDE_DIRS}
    )

set(iw_imagebase_SOURCE_FILES
    imagebaseforkdu.cpp
    )

set(iw_imagebase_HEADER_FILES
    CMakeLists.txt

    imagebaseforkdu.h
    )

if(WORD_SIZE EQUAL 64)
	if(WINDOWS)
		add_definitions(/FIXED:NO)
	else(WINDOWS) # not windows therefore gcc LINUX and DARWIN
		add_definitions(-fPIC)
	endif(WINDOWS)
endif (WORD_SIZE EQUAL 64)

set_source_files_properties(${iw_imagebase_HEADER_FILES}
                            PROPERTIES HEADER_FILE_ONLY TRUE)

list(APPEND iw_imagebase_SOURCE_FILES ${iw_imagebase_HEADER_FILES})

add_library (iw_imagebase ${iw_imagebase_SOURCE_FILES})
