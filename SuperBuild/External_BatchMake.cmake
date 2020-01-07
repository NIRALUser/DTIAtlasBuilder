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
set(extProjName BatchMake) #The find_package known name
set(proj ${extProjName})              #This local name

#if(${USE_SYSTEM_${extProjName}})
#  unset(${extProjName}_DIR CACHE)
#endif()

# Sanity checks
if(DEFINED ${extProjName}_DIR AND NOT EXISTS ${${extProjName}_DIR})
  message(FATAL_ERROR "${extProjName}_DIR variable is defined but corresponds to non-existing directory (${${extProjName}_DIR})")
endif()

# Set dependency list
set(${proj}_DEPENDENCIES ITKv4)

SlicerMacroCheckExternalProjectDependency(${proj})

if(UNIX AND (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
  set(CMAKE_CXX_FLAGS_BatchMake -std=c++11)
endif()

set(${proj}_INSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}/${proj}-install")

if(NOT DEFINED ${extProjName}_DIR AND NOT ${USE_SYSTEM_${extProjName}})
  #message(STATUS "${__indent}Adding project ${proj}")
  ExternalProject_Add(${proj}
      GIT_REPOSITORY ${git_protocol}://github.com/NIRALUser/BatchMake.git
      GIT_TAG release
      SOURCE_DIR ${EXTERNAL_SOURCE_DIRECTORY}/${proj}
      BINARY_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-build
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${COMMON_EXTERNAL_PROJECT_ARGS}
        -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${EXTERNAL_BINARY_DIRECTORY}/${proj}-build/bin
        -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${EXTERNAL_BINARY_DIRECTORY}/${proj}-build/bin
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${EXTERNAL_BINARY_DIRECTORY}/${proj}-build/bin
        -DCMAKE_BUNDLE_OUTPUT_DIRECTORY:PATH=${EXTERNAL_BINARY_DIRECTORY}/${proj}-build/bin
        -DBUILD_SHARED_LIBS:BOOL=OFF
        -DBUILD_TESTING:BOOL=OFF
        -DUSE_FLTK:BOOL=OFF
        -DDASHBOARD_SUPPORT:BOOL=OFF
        -DGRID_SUPPORT:BOOL=OFF
        -DUSE_SPLASHSCREEN:BOOL=OFF
        -DITK_DIR:PATH=${ITK_DIR}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS_BatchMake}
        ${BatchMakeCURLCmakeArg}
      INSTALL_COMMAND ""
      DEPENDS  ${${proj}_DEPENDENCIES}
    )
    set(BatchMake_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-build)
    set(BatchMake_SOURCE_DIR ${EXTERNAL_SOURCE_DIRECTORY}/${proj})
    mark_as_advanced(CLEAR BatchMake_DIR)
    set(BatchMake_ITK_DIR ${ITK_DIR}) # If batchmake recompiled, no include(${BatchMake_USE_FILE}) has been done so BatchMake_ITK_DIR does not exist, and we used ${ITK_DIR} to compile it.
    set(BatchMake_DEPEND BatchMake)

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

list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS ${extProjName}_DIR:PATH)
_expand_external_project_vars()
set(COMMON_EXTERNAL_PROJECT_ARGS ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})

ProjectDependancyPop(CACHED_extProjName extProjName)
ProjectDependancyPop(CACHED_proj proj)

