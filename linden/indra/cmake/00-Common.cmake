# -*- cmake -*-
#
# Compilation options shared by all viewer components.

include(Variables)


# Portable compilation flags.

set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG -DLL_DEBUG=1")
set(CMAKE_CXX_FLAGS_RELEASE
    "-DLL_RELEASE=1 -DLL_RELEASE_FOR_DOWNLOAD=1 -D_SECURE_SCL=0 -DLL_SEND_CRASH_REPORTS=1 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASESSE2
    "-DLL_RELEASE=1 -DLL_RELEASE_FOR_DOWNLOAD=1 -D_SECURE_SCL=0 -DLL_SEND_CRASH_REPORTS=1 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 
    "-DLL_RELEASE=1 -D_SECURE_SCL=0 -DLL_SEND_CRASH_REPORTS=0 -DNDEBUG -DLL_RELEASE_WITH_DEBUG_INFO=1")
# releasefast will use releasesse2 stuff later in the file
set(CMAKE_CXX_FLAGS_RELEASEFAST "")


# Don't bother with a MinSizeRel build.

set(CMAKE_CONFIGURATION_TYPES "ReleaseFast;RelWithDebInfo;Release;ReleaseSSE2;Debug" CACHE STRING
    "Supported build types." FORCE)

# Platform-specific compilation flags.

if (WINDOWS)
  # Don't build DLLs.
  set(BUILD_SHARED_LIBS OFF)

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
      /DLL_WINDOWS=1
      /DUNICODE
      /D_UNICODE 
      /GS
      /TP
      /W3
      /c
      /Zc:forScope
      /nologo
      /Oy-
      /MP
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
  
  # Various libs are compiler specific, generate some variables here we can just use
  # when we require them instead of reimplementing the test each time.
  
  if (MSVC71)
	    set(MSVC_DIR 7.1)
	    set(MSVC_SUFFIX 71)
    elseif (MSVC80)
	    set(MSVC_DIR 8.0)
	    set(MSVC_SUFFIX 80)
    elseif (MSVC90)
	    set(MSVC_DIR 9.0)
	    set(MSVC_SUFFIX 90)
    endif (MSVC71)
    
endif (WINDOWS)


if (LINUX)
  set(CMAKE_SKIP_RPATH TRUE)

  # Here's a giant hack for Fedora 8, where we can't use
  # _FORTIFY_SOURCE if we're using a compiler older than gcc 4.1.

  find_program(GXX g++)
  mark_as_advanced(GXX)

  if (GXX)
    execute_process(
        COMMAND ${GXX} --version
        COMMAND sed "s/^[gc+ ]*//"
        COMMAND head -1
        OUTPUT_VARIABLE GXX_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
  else (GXX)
    set(GXX_VERSION x)
  endif (GXX)

  # The quoting hack here is necessary in case we're using distcc or
  # ccache as our compiler.  CMake doesn't pass the command line
  # through the shell by default, so we end up trying to run "distcc"
  # " g++" - notice the leading space.  Ugh.

  execute_process(
      COMMAND sh -c "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} --version"
      COMMAND sed "s/^[gc+ ]*//"
      COMMAND head -1
      OUTPUT_VARIABLE CXX_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE)

  if (${GXX_VERSION} STREQUAL ${CXX_VERSION})
    add_definitions(-D_FORTIFY_SOURCE=2)
  else (${GXX_VERSION} STREQUAL ${CXX_VERSION})
    if (NOT ${GXX_VERSION} MATCHES " 4.1.*Red Hat")
      add_definitions(-D_FORTIFY_SOURCE=2)
    endif (NOT ${GXX_VERSION} MATCHES " 4.1.*Red Hat")
  endif (${GXX_VERSION} STREQUAL ${CXX_VERSION})
 
  #Lets actualy get a numerical version of gxx's version
  STRING(REGEX REPLACE ".* ([0-9])\\.([0-9])\\.([0-9]).*" "\\1\\2\\3" CXX_VERSION ${CXX_VERSION})
  
  #gcc 4.3 and above don't like the LL boost
  if(${CXX_VERSION} GREATER 429)
    add_definitions(-Wno-parentheses)
  endif (${CXX_VERSION} GREATER 429)

  # End of hacks.

  add_definitions(
      -DLL_LINUX=1
      -D_REENTRANT
      -fexceptions
      -fno-math-errno
      -fno-strict-aliasing
      -fsigned-char
      -pthread
      )

  if (SERVER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth-60")
    if (EXISTS /etc/debian_version)
      FILE(READ /etc/debian_version DEBIAN_VERSION)
    else (EXISTS /etc/debian_version)
      set(DEBIAN_VERSION "")
    endif (EXISTS /etc/debian_version)

    if (NOT DEBIAN_VERSION STREQUAL "3.1")
      add_definitions(-DCTYPE_WORKAROUND)
    endif (NOT DEBIAN_VERSION STREQUAL "3.1")

    if (EXISTS /usr/lib64/mysql4/mysql)
      link_directories(/usr/lib64/mysql4/mysql)
    elseif (EXISTS /usr/lib/mysql4/mysql)
      link_directories(/usr/lib/mysql4/mysql)
    endif (EXISTS /usr/lib/mysql4/mysql)

    add_definitions(
        -msse2
        -mfpmath=sse
        )
  endif (SERVER)

  if (VIEWER)
    add_definitions(-DAPPID=inworldz)
    add_definitions(-fvisibility=hidden)
    # don't catch SIGCHLD in our base application class for the viewer - some of our 3rd party libs may need their *own* SIGCHLD handler to work.  Sigh!  The viewer doesn't need to catch SIGCHLD anyway.
    add_definitions(-DLL_IGNORE_SIGCHLD)
    if (NOT STANDALONE)
      # this stops us requiring a really recent glibc at runtime
      add_definitions(-fno-stack-protector)
    endif (NOT STANDALONE)
    set(CMAKE_CXX_FLAGS_RELEASESSE2 "${CMAKE_CXX_FLAGS_RELEASESSE2} -msse2 -mfpmath=sse")
  endif (VIEWER)

  set(CMAKE_CXX_FLAGS_DEBUG "-fno-inline -g ${CMAKE_CXX_FLAGS_DEBUG}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 ${CMAKE_CXX_FLAGS_RELEASE}")
  set(CMAKE_CXX_FLAGS_RELEASEFAST "-O3 ${CMAKE_CXX_FLAGS_RELEASESSE2}")
  set(CMAKE_CXX_FLAGS_RELEASESSE2 "-O2 ${CMAKE_CXX_FLAGS_RELEASESSE2}")
endif (LINUX)


if (DARWIN)
  add_definitions(-DLL_DARWIN=1)
  set(CMAKE_CXX_LINK_FLAGS "-Wl,-headerpad_max_install_names,-search_paths_first")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_CXX_LINK_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3 -w -m32")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse3 -w -m32")
  # NOTE: it's critical that the optimization flag is put in front.
  # NOTE: it's critical to have both CXX_FLAGS and C_FLAGS covered.
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O0 ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O0 ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
  set(CMAKE_CXX_FLAGS_RELEASESSE2 "-msse2 -mfpmath=sse ${CMAKE_CXX_FLAGS_RELEASESSE2}")	
  set(CMAKE_C_FLAGS_RELEASESSE2 "-msse2 -mfpmath=sse ${CMAKE_C_FLAGS_RELEASESSE2}")
endif (DARWIN)


if (LINUX OR DARWIN)
  set(GCC_WARNINGS "-Wall -Wno-sign-compare -Wno-trigraphs")

  if (NOT GCC_DISABLE_FATAL_WARNINGS)
    set(GCC_WARNINGS "${GCC_WARNINGS} -Werror")
  endif (NOT GCC_DISABLE_FATAL_WARNINGS)

  set(GCC_CXX_WARNINGS "${GCC_WARNINGS} -Wno-reorder -Wno-non-virtual-dtor -Woverloaded-virtual")

  set(CMAKE_C_FLAGS "${GCC_WARNINGS} ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${GCC_CXX_WARNINGS} ${CMAKE_CXX_FLAGS}")

  if (WORD_SIZE EQUAL 32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
  elseif (WORD_SIZE EQUAL 64)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
  endif (WORD_SIZE EQUAL 32)
endif (LINUX OR DARWIN)


if (STANDALONE)
  add_definitions(-DLL_STANDALONE=1)

  if (LINUX AND ${ARCH} STREQUAL "i686")
    add_definitions(-march=pentiumpro)
  endif (LINUX AND ${ARCH} STREQUAL "i686")

else (STANDALONE)
  set(${ARCH}_linux_INCLUDES
      ELFIO
      atk-1.0
      dbus
      GL
      libxml2
      glib-2.0
      gstreamer-0.10
      gtk-2.0
      llfreetype2
      pango-1.0

      dbus
      GL
      libxml2
      apr-1
      SDL
      )
endif (STANDALONE)

if(SERVER)
  include_directories(${LIBS_PREBUILT_DIR}/include/havok)
endif(SERVER)

SET(CMAKE_EXE_LINKER_FLAGS_RELEASESSE2
    "${CMAKE_EXE_LINKER_FLAGS_RELEASE}" CACHE STRING
    "Flags used for linking binaries under SSE2 optimized build."
    FORCE )
SET(CMAKE_SHARED_LINKER_FLAGS_RELEASESSE2
    "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}" CACHE STRING
    "Flags used by the shared libraries linker under SSE2 optimized build."
    FORCE )
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_RELEASESSE2
    CMAKE_C_FLAGS_RELEASESSE2
    CMAKE_EXE_LINKER_FLAGS_RELEASESSE2
    CMAKE_SHARED_LINKER_FLAGS_RELEASESSE2 )
