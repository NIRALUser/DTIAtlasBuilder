if( NOT EXTERNAL_SOURCE_DIRECTORY )
  set( EXTERNAL_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ExternalSources )
endif()
if( NOT EXTERNAL_BINARY_DIRECTORY )
  set( EXTERNAL_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
endif()

# Make sure this file is included only once
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

# Include dependent projects if any
set(extProjName SlicerExecutionModel) #The find_package known name
set(proj ${extProjName})              #This local name

#if(${USE_SYSTEM_${extProjName}})
#  unset(${extProjName}_DIR CACHE)
#endif()

# Sanity checks
# if(DEFINED ${extProjName}_DIR AND NOT EXISTS ${${extProjName}_DIR})
#   message(FATAL_ERROR "${extProjName}_DIR variable is defined but corresponds to non-existing directory (${${extProjName}_DIR})")
# endif()

# Set dependency list
set(${proj}_DEPENDENCIES ITKv4)

SlicerMacroCheckExternalProjectDependency(${proj})

if(NOT DEFINED ${extProjName}_DIR AND NOT ${USE_SYSTEM_${extProjName}})
  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  ### --- Project specific additions here
  set(${proj}_CMAKE_OPTIONS
      -DITK_DIR:PATH=${ITK_DIR}
    )
  ### --- End Project specific additions
  set(${proj}_REPOSITORY "${git_protocol}://github.com/Slicer/SlicerExecutionModel.git")
  set(${proj}_GIT_TAG 61bb14d57ff45c8de0f506e23b6ec982fcdf0da2)
  ExternalProject_Add(${proj}
    GIT_REPOSITORY ${${proj}_REPOSITORY}
    GIT_TAG ${${proj}_GIT_TAG}
    SOURCE_DIR ${EXTERNAL_SOURCE_DIRECTORY}/${proj}
    BINARY_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-build
    LOG_CONFIGURE 0  # Wrap configure in script to ignore log output from dashboards
    LOG_BUILD     0  # Wrap build in script to to ignore log output from dashboards
    LOG_TEST      0  # Wrap test in script to to ignore log output from dashboards
    LOG_INSTALL   0  # Wrap install in script to to ignore log output from dashboards
    ${cmakeversion_external_update} "${cmakeversion_external_update_value}"
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -Wno-dev
      --no-warn-unused-cli
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      ${COMMON_EXTERNAL_PROJECT_ARGS}
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      ${${proj}_CMAKE_OPTIONS}
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(${extProjName}_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-build)
  set(GenerateCLP_DIR ${${extProjName}_DIR}/GenerateCLP)
else()
  if(${USE_SYSTEM_${extProjName}})
    find_package(${extProjName} REQUIRED)
    if(NOT ${extProjName}_DIR)
      message(FATAL_ERROR "To use the system ${extProjName}, set ${extProjName}_DIR")
    endif()
    message("USING the system ${extProjName}, set ${extProjName}_DIR=${${extProjName}_DIR}")
  endif()
  # The project is provided using ${extProjName}_DIR, nevertheless since other
  # project may depend on ${extProjName}v4, let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
endif()

list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS ${extProjName}_DIR:PATH
GenerateCLP_DIR:PATH
)
_expand_external_project_vars()
set(COMMON_EXTERNAL_PROJECT_ARGS ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})
ProjectDependancyPop(CACHED_extProjName extProjName)
ProjectDependancyPop(CACHED_proj proj)

