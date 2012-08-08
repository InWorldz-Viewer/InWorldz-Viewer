# -*- cmake -*-

include(Variables)
include(Linking)
include(FindPkgConfig)
include(Prebuilt)

if (NOT OPENAL)
  set(OPENAL ON CACHE BOOL "Enable OpenAL")
endif (NOT OPENAL)

# If STANDALONE but NOT PKG_CONFIG_FOUND we should fail,
# but why try to find it as prebuilt?
if (OPENAL AND STANDALONE AND PKG_CONFIG_FOUND)

  # This module defines
  # OPENAL_INCLUDE_DIRS
  # OPENAL_LIBRARIES
  # OPENAL_FOUND
  pkg_check_modules(OPENAL REQUIRED freealut)	# freealut links with openal.

elseif (OPENAL)

  message(STATUS "Building with OpenAL audio support")

  # OPENAL_LIB
  use_prebuilt_binary(openal-soft)
  
  if (WINDOWS)
    set(OPENAL_LIB
	  optimized ${ARCH_PREBUILT_DIRS_RELEASE}/openal32.lib
	  debug ${ARCH_PREBUILT_DIRS_DEBUG}/openal32.lib
      )
  elseif (DARWIN)
    # Look for for system's OpenAL.framework
    # Nemu: This code has never looked for the system's OpenAL.framework
    # Nemu: should it?
    set(OPENAL_LIB ${ARCH_PREBUILT_DIRS_RELEASE}/libopenal.1.dylib)
  else (WINDOWS)
    set(OPENAL_LIB openal)
  endif (WINDOWS)


  # OPENAL_INCLUDE_DIR
  if (DARWIN)
    set(OPENAL_INCLUDE_DIR ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/AL)
  else (DARWIN)
    find_path(OPENAL_INCLUDE_DIR
      NAMES al.h
      PATHS ${LIBS_PREBUILT_DIR}/include/AL
      )
  endif (DARWIN)


  # ALUT_LIB
  if (WINDOWS)
   set(ALUT_LIB
	 optimized ${ARCH_PREBUILT_DIRS_RELEASE}/alut.lib
	 debug ${ARCH_PREBUILT_DIRS_DEBUG}/alut.lib
     )
  elseif (DARWIN)
    set(ALUT_LIB ${ARCH_PREBUILT_DIRS_RELEASE}/libalut.0.dylib)
  else (WINDOWS)
    set(ALUT_LIB alut)
  endif (WINDOWS)

  
  # ALUT_INCLUDE_DIR
  if (WINDOWS)
	  find_path(ALUT_INCLUDE_DIR
	    NAMES alut.h
	    PATHS ${OPENAL_INCLUDE_DIR}
	    )
  elseif (DARWIN)
	  set(ALUT_INCLUDE_DIR ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/AL)
  endif (WINDOWS)

  set(OPENAL_LIBRARIES
    ${OPENAL_LIB}
    ${ALUT_LIB}
    )

  set(OPENAL_INCLUDE_DIRS
    ${OPENAL_INCLUDE_DIR}
    ${ALUT_INCLUDE_DIR}
    )

endif (OPENAL AND STANDALONE AND PKG_CONFIG_FOUND)


# This is BROKEN, but I have on idea why -- MC
# if (OPENAL)
  # if (NOT ALUT_LIB)
    # message(FATAL_ERROR "ALUT not found!")
  # else (NOT ALUT_LIB)
    ##message(STATUS "ALUT found: ${ALUT_LIB}")
  # endif (NOT ALUT_LIB)

  # if (NOT ALUT_INCLUDE_DIR)
    # message(FATAL_ERROR "alut.h not found!")
  # else (NOT ALUT_INCLUDE_DIR)
    ##message(STATUS "alut.h found in: ${ALUT_INCLUDE_DIR}")
  # endif (NOT ALUT_INCLUDE_DIR)
  
  # if (NOT OPENAL_LIB)
    # message(FATAL_ERROR "OpenAL not found!")
  # else (NOT OPENAL_LIB)
    ##message(STATUS "OpenAL found: ${OPENAL_LIB}")
  # endif (NOT OPENAL_LIB)

  # if (NOT OPENAL_INCLUDE_DIR)
    # message(FATAL_ERROR "al.h not found!")
  # else (NOT OPENAL_INCLUDE_DIR)
    ##message(STATUS "al.h found in: ${OPENAL_INCLUDE_DIR}")
  # endif (NOT OPENAL_INCLUDE_DIR)
  
  # set(OPENAL_FOUND TRUE CACHE BOOL
    # "Found OpenAL and ALUT libraries successfully"
    # )
# endif(OPENAL)
