if( NOT EXTERNAL_SOURCE_DIRECTORY )
  set( EXTERNAL_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ExternalSources )
endif()
if( NOT EXTERNAL_BINARY_DIRECTORY )
  set( EXTERNAL_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
endif()

# Make sure this file is included only once by creating globally unique varibles
# based on the name of this included file.
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

## External_${extProjName}.cmake files can be recurisvely included,
## and cmake variables are global, so when including sub projects it
## is important make the extProjName and proj variables
## appear to stay constant in one of these files.
## Store global variables before overwriting (then restore at end of this file.)
ProjectDependancyPush(CACHED_extProjName ${extProjName})
ProjectDependancyPush(CACHED_proj ${proj})

set(extProjName CTKAPPLAUNCHER)
set(proj CTKAPPLAUNCHER)

# Sanity checks
if(DEFINED CTKAPPLAUNCHER_DIR AND NOT EXISTS ${CTKAPPLAUNCHER_DIR})
  message(FATAL_ERROR "CTKAPPLAUNCHER_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(${proj})

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

if(NOT DEFINED CTKAPPLAUNCHER_DIR)
  SlicerMacroGetOperatingSystemArchitectureBitness(VAR_PREFIX CTKAPPLAUNCHER)
  set(launcher_version "0.1.11")
  set(item_id "")
  # On windows, use i386 launcher unconditionally
  if("${CTKAPPLAUNCHER_OS}" STREQUAL "win")
    set(CTKAPPLAUNCHER_ARCHITECTURE "i386")
    set(md5 "bd39716da9c2d5455ba95db561d3d883")
    set(item_id "4165")
  elseif("${CTKAPPLAUNCHER_OS}" STREQUAL "linux")
    set(md5 "fcbd0ca4eb01cc66d4691c08d8fa194c")
    set(item_id "4166")
  elseif("${CTKAPPLAUNCHER_OS}" STREQUAL "macosx")
    set(md5 "0af14c017ff7953b6fcca76f4f88c952")
    set(item_id "4164")
  endif()

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL http://packages.kitware.com/api/rest?method=midas.item.download&id=${item_id}&dummy=CTKAppLauncher-${launcher_version}-${CTKAPPLAUNCHER_OS}-${CTKAPPLAUNCHER_ARCHITECTURE}.tar.gz
    URL_MD5 ${md5}
    SOURCE_DIR ${EXTERNAL_SOURCE_DIRECTORY}/${proj}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
  )
  set(CTKAPPLAUNCHER_DIR ${CMAKE_BINARY_DIR}/${proj})
else()
  SlicerMacroEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
endif()
list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS ${extProjName}_DIR:PATH)
_expand_external_project_vars()
set(COMMON_EXTERNAL_PROJECT_ARGS ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})


ProjectDependancyPop(CACHED_extProjName extProjName)
ProjectDependancyPop(CACHED_proj proj)

