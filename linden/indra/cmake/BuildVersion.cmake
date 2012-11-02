# -*- cmake -*-

function (build_version _target)
  # Read version components from the header file.
  file(STRINGS ${LIBS_OPEN_DIR}/llcommon/llversion${_target}.h lines
       REGEX " LL_VERSION_")
  foreach(line ${lines})
    string(REGEX REPLACE ".*LL_VERSION_([A-Z]+).*" "\\1" comp "${line}")
    string(REGEX REPLACE ".* = ([0-9]+);.*" "\\1" value "${line}")
    set(v${comp} "${value}")
  endforeach(line)

  # Compose the version.
  set(temp_vers "${vMAJOR}.${vMINOR}.${vPATCH}.${vBUILD}")
  if (${temp_vers} MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$")
    set(${_target}_VERSION "${vMAJOR}.${vMINOR}.${vPATCH}.${vBUILD}")
    set(${_target}_VERSION_FULL ${${_target}_VERSION})
    if (IW_REPO_SHA1)
		# if we get the info from git
		set(${_target}_VERSION_FULL "${${_target}_VERSION_FULL}.${IW_REPO_SHA1}")
		if (IW_BUILD_DESC_SHORT)
			# if we have a description
			set(${_target}_VERSION_FULL "${${_target}_VERSION_FULL}-${IW_BUILD_DESC_SHORT}")
		endif (IW_BUILD_DESC_SHORT)
	endif (IW_REPO_SHA1)
  else (${temp_vers} MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "Could not determine ${_target} version (${${_target}_VERSION})")
  endif (${temp_vers} MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$")

  # Report version to caller.
  set(${_target}_VERSION "${${_target}_VERSION}" PARENT_SCOPE)
  set(${_target}_VERSION_FULL "${${_target}_VERSION_FULL}" PARENT_SCOPE)
message("Building version: ${${_target}_VERSION_FULL}")
endfunction (build_version)

# Avian - get misc git repo info
function (version_build_info)
    # get repo info
    set(temp_path ${CMAKE_CURRENT_SOURCE_DIR}/../..)
    set(temp_cmd "git")
    execute_process(COMMAND ${temp_cmd} config --get user.name
        WORKING_DIRECTORY ${temp_path}
        RESULT_VARIABLE temp_result
        OUTPUT_VARIABLE temp_output
        ERROR_VARIABLE temp_error)
    if (temp_output) 
		# We have something
		string(STRIP ${temp_output} temp_output)
		string(REGEX REPLACE "[^a-z^A-Z^0-9^_^-^(^)^.]" "_" temp_output ${temp_output})
		string(LENGTH ${temp_output} temp_len)
		if( temp_len )
			set(IW_REPO_USER "${temp_output}" CACHE STRING "The GIT User Name")
		else ( temp_len )
			set(IW_REPO_USER "Unknown" CACHE STRING "The GIT User Name")
		endif  ( temp_len )

		execute_process(COMMAND ${temp_cmd} show -s --abbrev-commit --oneline --no-notes --format=%h 
			WORKING_DIRECTORY ${temp_path} 
			RESULT_VARIABLE temp_result 
			OUTPUT_VARIABLE temp_output 
			ERROR_VARIABLE temp_error)
		string(STRIP ${temp_output} temp_output)
		string(LENGTH ${temp_output} temp_len)
		if( temp_len )
			set(IW_REPO_SHA1 "${temp_output}" CACHE STRING "The GIT Commit SHA1")
		else ( temp_len )
			set(IW_REPO_SHA1 "0000000" CACHE STRING "The GIT Commit SHA1")
		endif ( temp_len )
	else (temp_output)
		# We don't have anything, bail out
		message(WARNING "No output from git! To use commit hashes, 'git' must be in your PATH!")
	endif (temp_output)    
endfunction(version_build_info)

