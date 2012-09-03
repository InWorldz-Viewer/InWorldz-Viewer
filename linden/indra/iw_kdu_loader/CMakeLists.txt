# InWorldz KDU loader

cmake_minimum_required (VERSION 2.6)

project (iw_kdu_loader)

include(InWorldz)
include(Variables)

# stdafx.h is technically our precompiled headers file, but
# cmake doesn't seem to have a good way of handling this.
# todo: find a script that does the job, but we can live
# without a "real" pch in the meantime -- MC

#temp comment out- Avian
set(BUILD_SHARED_LIBS ON)
  
# VS2005-specific, pare these down when we're sure cmake
# does the right thing on, say, 2010
if (WINDOWS)
  set(BUILD_SHARED_LIBS ON)

  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi /MDd"
      CACHE STRING "C++ compiler debug options" FORCE)
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 
      "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Od /Zi /MD"
      CACHE STRING "C++ compiler release-with-debug options" FORCE)
  if (MSVC80)
      set(CMAKE_CXX_FLAGS_RELEASE
		"${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /Oi /Ot /GT /Zi /MD"
		CACHE STRING "C++ compiler release options" FORCE)
	  set(CMAKE_CXX_FLAGS_RELEASESSE2
		"${CMAKE_CXX_FLAGS_RELEASESSE2} /O2 /Ob2 /Oi /Ot /GT /Zi /MD /arch:SSE2"
		CACHE STRING "C++ compiler release (SSE2 optimized) options" FORCE)
  else (MSVC80)
      set(CMAKE_CXX_FLAGS_RELEASE
		"${CMAKE_CXX_FLAGS_RELEASE} ${LL_CXX_FLAGS} /O2 /Zi /MD"
		CACHE STRING "C++ compiler release options" FORCE)
	  set(CMAKE_CXX_FLAGS_RELEASESSE2
		"${CMAKE_CXX_FLAGS_RELEASESSE2} ${LL_CXX_FLAGS} /O2 /Zi /MD /arch:SSE2"
		CACHE STRING "C++ compiler release (SSE2 optimized) options" FORCE)
  endif (MSVC80)
  
  set(CMAKE_CXX_STANDARD_LIBRARIES "")
  set(CMAKE_C_STANDARD_LIBRARIES "")

  add_definitions(
	/GL
	/GF 
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
	/W3
	/Wp64
	/Zi
	/TP
	/Oy-
	/Gy
	/errorReport:prompt
	)
	
  if(MSVC80 OR MSVC90)
    set(CMAKE_CXX_FLAGS_RELEASE
      "${CMAKE_CXX_FLAGS_RELEASE} -D_SECURE_STL=0 -D_HAS_ITERATOR_DEBUGGING=0"
      CACHE STRING "C++ compiler release options" FORCE)
	set(CMAKE_CXX_FLAGS_RELEASESSE2
	  "${CMAKE_CXX_FLAGS_RELEASESSE2} -D_SECURE_STL=0 -D_HAS_ITERATOR_DEBUGGING=0"
	  CACHE STRING "C++ compiler release (SSE2 optimized) options" FORCE)
   
    add_definitions(
      /Zc:wchar_t-
      )
  endif (MSVC80 OR MSVC90)
  
  # Are we using the crummy Visual Studio KDU build workaround?
  if (NOT VS_DISABLE_FATAL_WARNINGS)
    add_definitions(/WX)
  endif (NOT VS_DISABLE_FATAL_WARNINGS)
	
else (WINDOWS)
  #Hack for linux/darwin build of 'iw_kdu_loader/stdadfx.h' - Avian
  include (00-Common)
  #end Hack

  add_definitions(
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
	set(CMAKE_OSX_ARCHITECTURES "i386")
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
