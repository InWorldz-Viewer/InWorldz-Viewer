# -*- cmake -*-
include(Linking)
include(Prebuilt)

set(OPENAL ON CACHE BOOL "Enable OpenAL")

if (OPENAL)
  if (STANDALONE)
    include(FindPkgConfig)
    include(FindOpenAL)
    pkg_check_modules(OPENAL_LIB REQUIRED openal)
    pkg_check_modules(FREEALUT_LIB REQUIRED freealut)
  else (STANDALONE)
    use_prebuilt_binary(openal-soft)
  endif (STANDALONE)
  
  if (WINDOWS)
    set(OPENAL_LIBRARIES 
	  optimized ${ARCH_PREBUILT_DIRS_RELEASE}/libOpenAL32.dll.a.lib
	  debug ${ARCH_PREBUILT_DIRS_DEBUG}/libOpenAL32.dll.a.lib
      )
  elseif (DARWIN)
    set(OPENAL_LIBRARIES ${ARCH_PREBUILT_DIRS_RELEASE}/libopenal.1.dylib)
  else (WINDOWS)
	set(OPENAL_LIBRARIES 
	  openal
	  alut
    )
  endif (WINDOWS)
endif (OPENAL)

# ALUT_LIB

if (WINDOWS)
set(ALUT_LIB
 optimized ${ARCH_PREBUILT_DIRS_RELEASE}/alut.lib
 debug ${ARCH_PREBUILT_DIRS_DEBUG}/alut.lib
 )
elseif (DARWIN)
find_library( ALUT_LIB
  NAMES alut.0
  PATHS ${ARCH_PREBUILT_DIRS_RELEASE}
  NO_DEFAULT_PATH
  )
else (WINDOWS)
set(ALUT_LIB alut)
endif (WINDOWS)

if (NOT ALUT_LIB)
message(FATAL_ERROR "ALUT not found!")
else (NOT ALUT_LIB)
# message(STATUS "ALUT found: ${ALUT_LIB}")
endif (NOT ALUT_LIB)

if (OPENAL)
  message(STATUS "Building with OpenAL audio support")
endif (OPENAL)
