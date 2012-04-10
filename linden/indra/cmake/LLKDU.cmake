# -*- cmake -*-

#Note to McMabe - Hacked this for now until you you can safefully clear out all Windows references to LLKDU 

if (WINDOWS)

include(Prebuilt)

if (NOT STANDALONE)
  use_prebuilt_binary(kdu)
endif (NOT STANDALONE)

endif (WINDOWS)
