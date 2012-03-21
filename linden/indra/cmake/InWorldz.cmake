# cmake is a pain to maintain. Include all InWorldz-specific defines here

include(Prebuilt)
include(Variables)

# # # # # # # # # # # # # # # # # # # # # 
#
#	iw_imgbase project
#
# # # # # # # # # # # # # # # # # # # # # 

set(IW_IMAGEBASE_INCLUDE_DIRS
    ${LIBS_OPEN_DIR}/iw_imagebase
    )

set(IW_IMAGEBASE_LIBRARIES iw_imagebase)


# # # # # # # # # # # # # # # # # # # # #
# 
#	kdu and iw_kdu_loader project
#
# # # # # # # # # # # # # # # # # # # # # 

# check for the coresys dir, but any would do
set(TEST_KDU_DIR ${LIBS_PREBUILT_DIR}/kdu/source/coresys)

set(STRING_KDU 	"Building with InWorldz Kakadu (KDU) image decoding.")
set(STRING_OJ 	"Building with OpenJPEG image decoding.")

if (EXISTS ${TEST_KDU_DIR}/)
	set(IW_KDU ON CACHE BOOL "KDU decoding" FORCE)
	message(STATUS ${STRING_KDU})
else (EXISTS ${TEST_KDU_DIR}/)
	set(IW_KDU OFF CACHE BOOL "KDU decoding" FORCE)
	message(STATUS ${STRING_OJ})
endif (EXISTS ${TEST_KDU_DIR}/)

if (IW_KDU)
    # Note to McCabe - Hack till Windows vesions safelly compiles without LLKDU
    if (WINDOWS)
	use_prebuilt_binary(kdu)
    endif (WINDOWS)
    # end Hack
	add_definitions(-DIW_KDU_ENABLED=1)
else (IW_KDU)
	add_definitions(-DIW_KDU_ENABLED=FALSE)
endif (IW_KDU)

# Don't need any checking here, our macro above handles that
set(IW_KDU_INCLUDE_DIRS
		${LIBS_PREBUILT_DIR}/kdu/source/managed/all_includes
		${LIBS_PREBUILT_DIR}/kdu/source/apps/image
		)

# Make a guess using the ARCH setting. Not sure if this is the
# right way to do this?
if (WINDOWS)
    if (ARCH EQUAL x86_64)
	set(KDU_LIB_DIR ${LIBS_PREBUILT_DIR}/kdu/lib_x64)
    else (ARCH EQUAL x86_64)	
	set(KDU_LIB_DIR ${LIBS_PREBUILT_DIR}/kdu/lib_x86)
    endif (ARCH EQUAL x86_64)
elseif (LINUX)
    if (ARCH EQUAL x86_64)
	set(KDU_LIB_DIR ${LIBS_PREBUILT_DIR}/kdu/source/lib/Linux-x86-64-gcc)
    else (ARCH EQUAL x86_64)	
	set(KDU_LIB_DIR ${LIBS_PREBUILT_DIR}/kdu/source/lib/Linux-x86-32-gcc)
    endif (ARCH EQUAL x86_64)
elseif (DARWIN)
    # need to add proper DARWIN/Mac stuff here
endif (WINDOWS)

# We want to avoid compiler errors if the lib is linked without KDU
if (EXISTS ${KDU_LIB_DIR}/)
	if (WINDOWS)
		set(IW_KDU_LIBRARIES
			${IW_IMAGEBASE_LIBRARIES}
			debug ${KDU_LIB_DIR}/kdu_v64D.lib
			optimized ${KDU_LIB_DIR}/kdu_v64R.lib
			)
	elseif (LINUX)
	    set(IW_KDU_LIBRARIES
			${IW_IMAGEBASE_LIBRARIES}
			debug ${KDU_LIB_DIR}/libkdu_a64R.so
			optimized ${KDU_LIB_DIR}/libkdu_a64R.so
#			debug ${KDU_LIB_DIR}/libkdu.a
#			optimized ${KDU_LIB_DIR}/libkdu.a
			)
	elseif (DARWIN)
		set(IW_KDU_LIBRARIES
			${IW_IMAGEBASE_LIBRARIES}
			debug ${KDU_LIB_DIR}/kdu_a64D.dylib
			optimized ${KDU_LIB_DIR}/kdu_a64R.dylib
			)
	endif (WINDOWS)
else (EXISTS ${KDU_LIB_DIR}/)
	set(IW_KDU_LIBRARIES "")
	if (IW_KDU)
		message(STATUS "Can't find compiled KDU libraries expected at " ${KDU_LIB_DIR} "!")
	endif (IW_KDU)
endif (EXISTS ${KDU_LIB_DIR}/)
