# -*- cmake -*-

include(00-Common)

project(cmake)
    
file(GLOB cmake_FILE_GLOB_LIST
     *.cmake)
     
set(cmake_SOURCE_FILES
    CMakeLists.txt
    )

list(APPEND cmake_SOURCE_FILES ${cmake_FILE_GLOB_LIST})
list(SORT cmake_SOURCE_FILES)

source_group("Shared Rules" FILES ${cmake_SOURCE_FILES})

set(master_SOURCE_FILES
    ../CMakeLists.txt
    ../develop.py
    )

if (SERVER)
  list(APPEND master_SOURCE_FILES ../Server.cmake)
endif (SERVER)

source_group("Master Rules" FILES ${master_SOURCE_FILES})

set_source_files_properties(${cmake_SOURCE_FILES} ${master_SOURCE_FILES}
                            PROPERTIES HEADER_FILE_ONLY TRUE)

add_library(cmake
            cmake_dummy.cpp
            ${cmake_SOURCE_FILES}
            ${master_SOURCE_FILES}
            )
