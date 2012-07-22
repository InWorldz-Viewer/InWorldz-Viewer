# InWorldz KDU loader

cmake_minimum_required (VERSION 2.6)

project (iw_kdu_loader)

include(InWorldz)

# stdafx.h is technically our precompiled headers file, but
# cmake doesn't seem to have a good way of handling this.
# todo: find a script that does the job, but we can live
# without a "real" pch in the meantime -- MC

#temp comment out- Avian
set(BUILD_SHARED_LIBS ON)
  
# VS2005-specific, pare these down when we're sure cmake
# does the right thing on, say, 2010
if (WINDOWS)
  add_definitions(
	/O2
	/GL
	/D "WIN32"
	/D "NDEBUG"
	/D "_WINDOWS"
	/D "_USRDLL"
	/D "IW_KDU_LOADER_EXPORTS"
	/D "_WINDLL"
	/D "_UNICODE"
	/D "UNICODE"
	/FD
	/EHsc
	/MD
	#/Yc"stdafx.h"
	/W3
	/nologo
	/c
	/Wp64
	/Zi
	/TP
	/errorReport:prompt
	)
else (WINDOWS)
  #Hack for linux/darwin build of 'iw_kdu_loader/stdadfx.h' - Avian
  include (00-Common)
  #end Hack

  add_definitions(
#	-g
#	-c
#	-shared
	-fPIC
	-D "IW_KDU_LOADER_EXPORTS"
	)
endif (WINDOWS)

include_directories(
	${IW_IMAGEBASE_INCLUDE_DIRS}
	${IW_KDU_INCLUDE_DIRS}
    )
	
set(iw_kdu_loader_SOURCE_FILES
	iw_kdu_loader.cpp
    inworldzJ2C.cpp
	llformat.cpp
	llkdumem.cpp
	stdafx.cpp
    )

set(iw_kdu_loader_HEADER_FILES
    CMakeLists.txt

	is_approx_equal_fraction.h
	iw_kdu_loader.h
    inworldzJ2C.h
	lldefs.h
	llformat.h
	llkdumem.h
	llmath.h
	stdafx.h
	stdtypes.h
    )

set_source_files_properties(${iw_kdu_loader_HEADER_FILES}
                            PROPERTIES HEADER_FILE_ONLY TRUE)

list(APPEND iw_kdu_loader_SOURCE_FILES ${iw_kdu_loader_HEADER_FILES})

if (WINDOWS)
    add_library (iw_kdu_loader ${iw_kdu_loader_SOURCE_FILES})
elseif (LINUX)
    add_library (iw_kdu_loader ${iw_kdu_loader_SOURCE_FILES})
elseif (DARWIN)
    add_library (iw_kdu_loader SHARED ${iw_kdu_loader_SOURCE_FILES})
endif (WINDOWS)


if (WINDOWS)
	# Release -- need to figure out how to set release/debug
	target_link_libraries(iw_kdu_loader
		${IW_KDU_LIBRARIES}
		kernel32 
		user32 
		gdi32 
		winspool 
		comdlg32 
		advapi32 
		shell32 
		ole32 
		oleaut32 
		uuid 
		odbc32 
		odbccp32
		)
		
	set_target_properties(iw_kdu_loader
		PROPERTIES
		LINK_FLAGS "/INCREMENTAL:NO /NOLOGO /DLL /MANIFEST /DEBUG /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /LTCG"
		LINK_FLAGS_DEBUG "/INCREMENTAL:NO /NOLOGO /DLL /MANIFEST /DEBUG /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /LTCG"
		)
		
	# add post-build copying to the right inworldz-bin.exe directory here
elseif (LINUX)
	target_link_libraries(iw_kdu_loader
		${IW_KDU_LIBRARIES}
		)
	set_target_properties(iw_kdu_loader
		PROPERTIES
		LINK_FLAGS "-shared -Wl"
#		LINK_FLAGS "-shared -Wl, -soname "
#		LINK_FLAGS_DEBUG "-shared -Wl, -soname, "
		)
elseif (DARWIN)
	# This is completely the wrong way to do this
	target_link_libraries(iw_kdu_loader
		${IW_IMAGEBASE_LIBRARIES}
		${KDU_LIB_DIR}/libkdu_v64R.a
		)
	set_target_properties(iw_kdu_loader
		PROPERTIES
		LINK_FLAGS "-shared -Wl"
		)
endif (WINDOWS)
