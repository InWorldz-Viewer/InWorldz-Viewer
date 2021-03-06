# -*- cmake -*-

project(media_plugin_gstreamer010)

include(00-Common)
include(LLCommon)
include(LLImage)
include(LLPlugin)
include(LLMath)
include(LLRender)
include(LLWindow)
include(Linking)
include(PluginAPI)
include(MediaPluginBase)
include(FindOpenGL)

include(GStreamer010Plugin)

include_directories(
    ${LLPLUGIN_INCLUDE_DIRS}
    ${MEDIA_PLUGIN_BASE_INCLUDE_DIRS}
    ${LLCOMMON_INCLUDE_DIRS}
    ${LLMATH_INCLUDE_DIRS}
    ${LLIMAGE_INCLUDE_DIRS}
    ${LLRENDER_INCLUDE_DIRS}
    ${LLWINDOW_INCLUDE_DIRS}
    ${GSTREAMER010_INCLUDE_DIRS}
    ${GSTREAMER010_PLUGINS_BASE_INCLUDE_DIRS}
)

### media_plugin_gstreamer010

set(media_plugin_gstreamer010_SOURCE_FILES
    media_plugin_gstreamer010.cpp
    llmediaimplgstreamervidplug.cpp
    )

set(media_plugin_gstreamer010_HEADER_FILES
    llmediaimplgstreamer.h
    llmediaimplgstreamervidplug.h
    llmediaimplgstreamertriviallogging.h
    )
	
set_source_files_properties(${media_plugin_gstreamer010_HEADER_FILES}
							PROPERTIES HEADER_FILE_ONLY TRUE)

list(APPEND media_plugin_gstreamer010_SOURCE_FILES ${media_plugin_gstreamer010_HEADER_FILES})

add_library(media_plugin_gstreamer010
    SHARED
    ${media_plugin_gstreamer010_SOURCE_FILES}
)

if (DARWIN)
	find_library(COREFOUNDATION_LIBRARY NAMES CoreFoundation) 
endif (DARWIN)

target_link_libraries(media_plugin_gstreamer010
  ${LLPLUGIN_LIBRARIES}
  ${MEDIA_PLUGIN_BASE_LIBRARIES}
  ${LLCOMMON_LIBRARIES}
  ${PLUGIN_API_WINDOWS_LIBRARIES}
  ${GSTREAMER010_LIBRARIES}
  ${COREFOUNDATION_LIBRARY}
)

add_dependencies(media_plugin_gstreamer010
  ${LLPLUGIN_LIBRARIES}
  ${MEDIA_PLUGIN_BASE_LIBRARIES}
  ${LLCOMMON_LIBRARIES}
)


