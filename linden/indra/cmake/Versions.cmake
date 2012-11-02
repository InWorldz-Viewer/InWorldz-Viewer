include(BuildVersion)

# Avian - Set up description for max of 10 characters and
#         filter out non-filename characters

# Avian - Check for new build description argument
if ( NOT IW_BUILD_DESC )
    set(IW_BUILD_DESC "" CACHE STRING "Null Build Description")
    set(IW_BUILD_DESC_SHORT "" CACHE STRING "Null Build Short Description")
else ( NOT IW_BUILD_DESC )
    set(IW_BUILD_DESC_SHORT ${IW_BUILD_DESC} CACHE STRING "The Full Build Description")
    string(REGEX REPLACE "[^a-z^A-Z^0-9^_^-^(^)^.]" "_" temp_var ${IW_BUILD_DESC_SHORT})
    string(LENGTH ${temp_var} temp_len)
    if ( temp_len GREATER 10 )
        string(SUBSTRING ${temp_var} 0 10 temp_var)
    endif ( temp_len GREATER 10)
    set(IW_BUILD_DESC_SHORT ${temp_var} CACHE STRING "The Usable Build Description" FORCE)
endif ( NOT IW_BUILD_DESC )

if ( NOT IW_BYPASS )
    set(IW_BYPASS 0 CACHE BOOL "Bypass Mandatory Download")
else ( NOT IW_BYPASS )
    set(IW_BYPASS 1 CACHE BOOL "Bypass Mandatory Download" FORCE)
endif ( NOT IW_BYPASS )

version_build_info()

if(VIEWER)
  build_version(viewer)
endif(VIEWER)

if(SERVER)
  build_version(server)
endif(SERVER)

