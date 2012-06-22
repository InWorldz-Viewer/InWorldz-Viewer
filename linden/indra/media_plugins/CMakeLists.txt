# -*- cmake -*-

add_subdirectory(base)

add_subdirectory(webkit)

if (LINUX)
    add_subdirectory(gstreamer010)
endif (LINUX)

# We use gstreamer for audio, quicktime for media on win -- MC
if (WINDOWS)
    add_subdirectory(gstreamer010)
    add_subdirectory(quicktime)
    add_subdirectory(winmmshim)
endif (WINDOWS)

# Mac uses quicktime for audio and media -- MC
if (DARWIN)
    add_subdirectory(quicktime)
    add_subdirectory(gstreamer010)
endif (DARWIN)

add_subdirectory(example)
