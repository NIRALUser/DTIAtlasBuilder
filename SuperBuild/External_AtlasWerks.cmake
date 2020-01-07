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

# Make sure that the ExtProjName/IntProjName variables are unique globally
# even if other External_${ExtProjName}.cmake files are sourced by
# SlicerMacroCheckExternalProjectDependency
set(extProjName AtlasWerks) #The find_package known name
set(proj        AtlasWerks) #This local name
set(${extProjName}_REQUIRED_VERSION "")  #If a required version is necessary, then set this, else leave blank

#if(${USE_SYSTEM_${extProjName}})
#  unset(${extProjName}_DIR CACHE)
#endif()

# Sanity checks
if(DEFINED ${extProjName}_DIR AND NOT EXISTS ${${extProjName}_DIR})
  message(FATAL_ERROR "${extProjName}_DIR variable is defined but corresponds to non-existing directory (${${extProjName}_DIR})")
endif()

if(NOT ( DEFINED "USE_SYSTEM_${extProjName}" AND "${USE_SYSTEM_${extProjName}}" ) )
  # option(USE_SYSTEM_VTK "Build using an externally defined version of VTK" OFF)
  # Set dependency list
  set(${proj}_DEPENDENCIES ITKv4 FFTW CLAPACK)
  # Include dependent projects if any
  SlicerMacroCheckExternalProjectDependency(${proj})
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
      -DVTK_DIR:PATH=${VTK_DIR}
      -DLAPACK_DIR:PATH=${CLAPACK_DIR}
      -DFFTW_INSTALL_BASE_PATH:PATH=${FFTW_DIR} # will use find_library to find the libs
      -DFFTWF_LIB:PATH=${FFTW_DIR}/lib/libfftw3f.a # FFTW in float
      -DFFTWD_LIB:PATH=${FFTW_DIR}/lib/libfftw3.a # FFTW in double # needed for AtlasWerks to configure, not to compile with
      -DFFTWF_THREADS_LIB:PATH=${FFTW_DIR}/lib/libfftw3f_threads.a
      -DFFTWD_THREADS_LIB:PATH=${FFTW_DIR}/lib/libfftw3_threads.a
      -DFFTW_INCLUDE_PATH:PATH=${FFTW_DIR}/include # will be used to set FFTW_INSTALL_BASE_PATH by finding the path = remove the /include
      -DAtlasWerks_COMPILE_TESTING:BOOL=OFF
      -DatlasWerks_COMPILE_APP_Affine:BOOL=OFF
      -DatlasWerks_COMPILE_APP_AffineAtlas:BOOL=OFF
      -DatlasWerks_COMPILE_APP_ATLAS_WERKS:BOOL=OFF
      -DatlasWerks_COMPILE_APP_VECTOR_ATLAS_WERKS:BOOL=OFF
      -DatlasWerks_COMPILE_APP_FGROWTH:BOOL=OFF
      -DatlasWerks_COMPILE_APP_FWARP:BOOL=OFF
      -DatlasWerks_COMPILE_APP_ImageConvert:BOOL=OFF
      -DatlasWerks_COMPILE_APP_IMMAP:BOOL=OFF
      -DatlasWerks_COMPILE_APP_LDMM:BOOL=OFF
      -DatlasWerks_COMPILE_APP_GREEDY:BOOL=ON  # Compile Only GreedyAtlas
      -DatlasWerks_COMPILE_APP_TX_APPLY:BOOL=OFF
      -DatlasWerks_COMPILE_APP_TX_WERKS:BOOL=OFF
      -DatlasWerks_COMPILE_APP_UTILITIES:BOOL=OFF
      -DCMAKE_CXX_STANDARD=14
    )
  
  ### --- End Project specific additions

  ExternalProject_Add(${proj}
    GIT_REPOSITORY ${git_protocol}://github.com/NIRALUser/AtlasWerks.git
    GIT_TAG master
    SOURCE_DIR ${EXTERNAL_SOURCE_DIRECTORY}/${proj}
    BINARY_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-build
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
      ## We really do want to install to remove uncertainty about where the tools are
      ## (on Windows, tools might be in subfolders, like "Release", "Debug",...)
      #-DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIRECTORY}/${proj}-install
    DEPENDS
      ${${proj}_DEPENDENCIES} 
    #We only care about GreedyAtlas so we only build this target.
    BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} GreedyAtlas GreedyWarp
    INSTALL_COMMAND ""
    #There is no install in AtlasWerks. We only care about GreedyAtlas so we just copy it. We only do that on Linux since AtlasWerks does not work on the other plateform
  )
  set(${extProjName}_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-build)
else()
  if(${USE_SYSTEM_${extProjName}})
    find_package(${extProjName} ${${extProjName}_REQUIRED_VERSION} REQUIRED)
    message("USING the system ${extProjName}, set ${extProjName}_DIR=${${extProjName}_DIR}")
  endif()
  # The project is provided using ${extProjName}_DIR, nevertheless since other
  # project may depend on ${extProjName}, let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
endif()

list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS ${extProjName}_DIR:PATH)

ProjectDependancyPop(CACHED_extProjName extProjName)
ProjectDependancyPop(CACHED_proj proj)
