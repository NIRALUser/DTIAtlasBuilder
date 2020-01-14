if( NOT EXTERNAL_SOURCE_DIRECTORY )
  set( EXTERNAL_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ExternalSources )
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

# Make sure that the ExtProjName/IntProjName variables are unique globally
# even if other External_${ExtProjName}.cmake files are sourced by
# SlicerMacroCheckExternalProjectDependency
set(extProjName ANTs) #The find_package known name
set(proj        ANTs) #This local name
set(${extProjName}_REQUIRED_VERSION "")  #If a required version is necessary, then set this, else leave blank

#if(${USE_SYSTEM_${extProjName}})
#  unset(${extProjName}_DIR CACHE)
#endif()

# Sanity checks
if(DEFINED ${extProjName}_SOURCE_DIR AND NOT EXISTS ${${extProjName}_SOURCE_DIR})
  message(FATAL_ERROR "${extProjName}_SOURCE_DIR variable is defined but corresponds to non-existing directory (${${extProjName}_SOURCE_DIR})")
endif()

if(DEFINED ${extProjName}_LIBRARY_DIR AND NOT EXISTS ${${extProjName}_LIBRARY_DIR})
  message(FATAL_ERROR "${extProjName}_LIBRARY_DIR variable is defined but corresponds to non-existing directory (${${extProjName}_LIBRARY_DIR})")
endif()

# Set dependency list
set(${proj}_DEPENDENCIES ITKv4 SlicerExecutionModel)
if(${PROJECT_NAME}_BUILD_DICOM_SUPPORT)
  list(APPEND ${proj}_DEPENDENCIES DCMTK)
endif()

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(${proj})

if(NOT ( DEFINED "USE_SYSTEM_${extProjName}" AND "${USE_SYSTEM_${extProjName}}" ) )
  #message(STATUS "${__indent}Adding project ${proj}")

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
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/${proj}-install
      -DANTS_SUPERBUILD:BOOL=OFF
      -DUSE_SYSTEM_ITK:BOOL=ON
      -DUSE_SYSTEM_SlicerExecutionModel:BOOL=ON
      -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
      -DITK_DIR:PATH=${ITK_DIR}
      -DITK_VERSION_MAJOR=${ITK_REQUIRED_VERSION}
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DBUILD_ALL_ANTS_APPS:BOOL=ON
      -DBUILD_SHARED_LIBS:BOOL=OFF
      -DUSE_VTK:BOOL=OFF
   )
 if(${PRIMARY_PROJECT_NAME}_USE_QT)
   list(APPEND ${proj}_CMAKE_OPTIONS -DANTS_USE_QT:BOOL=ON)
 endif()
 if(CMAKE_CXX_STANDARD)
  list(APPEND ${proj}_CMAKE_OPTIONS -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD})
 endif()
  ### --- End Project specific additions
  set(${proj}_REPOSITORY "https://github.com/ANTsX/ANTs.git")
  set(${proj}_GIT_TAG v2.2.0)
  ExternalProject_Add(${proj}
    GIT_REPOSITORY ${${proj}_REPOSITORY}
    GIT_TAG ${${proj}_GIT_TAG}
    SOURCE_DIR ${EXTERNAL_SOURCE_DIRECTORY}/${proj}
    BINARY_DIR ${proj}-build
    LOG_CONFIGURE 0  # Wrap configure in script to ignore log output from dashboards
    LOG_BUILD     0  # Wrap build in script to to ignore log output from dashboards
    LOG_TEST      0  # Wrap test in script to to ignore log output from dashboards
    LOG_INSTALL   0  # Wrap install in script to to ignore log output from dashboards
    ${cmakeversion_external_update} "${cmakeversion_external_update_value}"
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      ${COMMON_EXTERNAL_PROJECT_ARGS}
      ${${proj}_CMAKE_OPTIONS}
    DEPENDS
    ${${proj}_DEPENDENCIES}
  )
  set(${extProjName}_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/ExternalSources/${proj})
  set(${extProjName}_LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib)
  set(${extProjName}_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install)
  set(${extProjName}_BINARY_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install/bin)
  
  set(USE_ANTS ON)
else()
  if(${USE_SYSTEM_${extProjName}})
    set(${extProjName}_SOURCE_DIR ""
      CACHE PATH "Set ANTs source dir")
    set(${extProjName}_LIBRARY_DIR "" 
      CACHE PATH "Set ANTs Library")
    if(NOT DEFINED ${extProjName}_SOURCE_DIR OR NOT DEFINED ${extProjName}_LIBRARY_DIR)
      message(FATAL_ERROR "To use system ${extProjName}, set ${extProjName}_SOURCE_DIR and ${extProjName}_LIBRARY_DIR")
    elseif(NOT EXISTS "${${extProjName}_SOURCE_DIR}")
      message(FATAL_ERROR "${extProjName}_SOURCE_DIR (${${extProjName}_SOURCE_DIR}) does not exist")
    elseif(NOT EXISTS "${${extProjName}_LIBRARY_DIR}")
      message(FATAL_ERROR "${extProjName}_LIBRARY_DIR (${${extProjName}_LIBRARY_DIR}) does not exist")
    else()
      message("using ${extProjName}_SOURCE_DIR=${${extProjName}_SOURCE_DIR} and
${extProjName}_LIBRARY_DIR=${${extProjName}_LIBRARY_DIR}")
    endif()
    message("USING the system ${extProjName}, set ${extProjName}_SOURCE_DIR=${${extProjName}_SOURCE_DIR}")
  endif()
  if( NOT TARGET ${proj} )
    # The project is provided using ${extProjName}_DIR, nevertheless since other
    # project may depend on ${extProjName}, let's add an 'empty' one
    SlicerMacroEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
  endif()
endif()

list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS ${extProjName}_DIR:PATH)
list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS ${extProjName}_BINARY_DIR:PATH)

_expand_external_project_vars()
set(COMMON_EXTERNAL_PROJECT_ARGS ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})

ProjectDependancyPop(CACHED_extProjName extProjName)
ProjectDependancyPop(CACHED_proj proj)
