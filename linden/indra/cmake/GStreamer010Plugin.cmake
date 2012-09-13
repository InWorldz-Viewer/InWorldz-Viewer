# -*- cmake -*-
include(Prebuilt)

if (STANDALONE)
  include(FindPkgConfig)

  pkg_check_modules(GSTREAMER010 REQUIRED gstreamer-0.10)
  pkg_check_modules(GSTREAMER010_PLUGINS_BASE REQUIRED gstreamer-plugins-base-0.10)

else (STANDALONE)

  # Possibly libxml and glib should have their own .cmake file instead...
  use_prebuilt_binary(gstreamer)	# includes glib, libxml, and iconv on Windows
if (NOT LINUX)
  use_prebuilt_binary(gstreamer-plugins)
endif (NOT LINUX)
  set(GSTREAMER010_FOUND ON FORCE BOOL)
  set(GSTREAMER010_PLUGINS_BASE_FOUND ON FORCE BOOL)
  if (WINDOWS)
    set(GSTREAMER010_INCLUDE_DIRS
		${LIBS_PREBUILT_DIR}/include/gstreamer-0.10
		${LIBS_PREBUILT_DIR}/include/glib
		${LIBS_PREBUILT_DIR}/include/libxml2
		)
  else (WINDOWS)
      use_prebuilt_binary(glib)			# gstreamer needs glib
      if (DARWIN)

         use_prebuilt_binary(libxml)
         set(GSTREAMER010_INCLUDE_DIRS
                ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/gstreamer-0.10
                ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/glib-2.0
                ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/libxml2
            )
       else (DARWIN)
         add_definitions(-DGST_DISABLE_XML -DGST_DISABLE_LOADSAVE)
         set(GSTREAMER010_INCLUDE_DIRS
                ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/gstreamer-0.10
                ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/glib-2.0
            )
      endif (DARWIN)
  endif (WINDOWS)

endif (STANDALONE)

if (WINDOWS)
  # We don't need to explicitly link against gstreamer itself, because
  # LLMediaImplGStreamer probes for the system's copy at runtime.
    set(GSTREAMER010_LIBRARIES
         gstaudio-0.10.lib
         gstbase-0.10.lib
         gstreamer-0.10.lib
         gstvideo-0.10.lib #slvideoplugin
	 gstinterfaces-0.10.lib
         gobject-2.0
         gmodule-2.0
         gthread-2.0
         glib-2.0
         )
elseif (DARWIN)
    set(GSTREAMER010_LIBRARIES
         gstvideo-0.10
         gstaudio-0.10
         gstbase-0.10
         gstreamer-0.10
         gobject-2.0
         gmodule-2.0
         dl
         gthread-2.0
         glib-2.0
         )
else (WINDOWS)
  # We don't need to explicitly link against gstreamer itself, because
  # LLMediaImplGStreamer probes for the system's copy at runtime.
    set(GSTREAMER010_LIBRARIES
         gstvideo-0.10
         gstaudio-0.10
         gstbase-0.10
         gstreamer-0.10
         gobject-2.0
         gmodule-2.0
         dl
         gthread-2.0
         rt
         glib-2.0
         )
endif (WINDOWS)

if (GSTREAMER010_FOUND AND GSTREAMER010_PLUGINS_BASE_FOUND)
  set(GSTREAMER010 ON CACHE BOOL "Build with GStreamer-0.10 streaming media support.")
endif (GSTREAMER010_FOUND AND GSTREAMER010_PLUGINS_BASE_FOUND)

if (GSTREAMER010)
  add_definitions(-DLL_GSTREAMER010_ENABLED=1)
endif (GSTREAMER010)

