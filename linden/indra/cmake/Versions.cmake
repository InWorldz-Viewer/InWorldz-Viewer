include(BuildVersion)

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
