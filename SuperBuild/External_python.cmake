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

set(extProjName python) #The find_package known name
set(proj        python) #This local name
set(${extProjName}_REQUIRED_VERSION "")  #If a required version is necessary, then set this, else leave blank

#if(${USE_SYSTEM_${extProjName}})
#  unset(${extProjName}_DIR CACHE)
#endif()

# Sanity checks
if(DEFINED ${extProjName}_DIR AND NOT EXISTS ${${extProjName}_DIR})
  message(FATAL_ERROR "${extProjName}_DIR variable is defined but corresponds to non-existing directory (${${extProjName}_DIR})")
endif()

# Set dependency list
set(${proj}_DEPENDENCIES CTKAPPLAUNCHER zlib)
if(${PROJECT_NAME}_USE_PYTHONQT_WITH_TCL)
  if(WIN32)
    list(APPEND ${proj}_DEPENDENCIES tcl)
  else()
    list(APPEND ${proj}_DEPENDENCIES tcl tk)
  endif()
endif()

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(${proj})

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  unset(PYTHON_INCLUDE_DIR CACHE)
  unset(PYTHON_LIBRARY CACHE)
  unset(PYTHON_EXECUTABLE CACHE)
  find_package(PythonLibs REQUIRED)
  find_package(PythonInterp REQUIRED)
  set(PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_DIRS})
  set(PYTHON_LIBRARY ${PYTHON_LIBRARIES})
  set(PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE})
endif()

if((NOT DEFINED PYTHON_INCLUDE_DIR
   OR NOT DEFINED PYTHON_LIBRARY
   OR NOT DEFINED PYTHON_EXECUTABLE) AND NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  set(python_SOURCE_DIR "${EXTERNAL_SOURCE_DIRECTORY}/Python-2.7.3")

  set(EXTERNAL_PROJECT_OPTIONAL_ARGS)
  if(MSVC)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      PATCH_COMMAND ${CMAKE_COMMAND}
        -DPYTHON_SRC_DIR:PATH=${python_SOURCE_DIR}
        -P ${CMAKE_CURRENT_LIST_DIR}/External_${proj}_patch.cmake
      )
  endif()

  ExternalProject_Add(python-source
    URL "http://www.python.org/ftp/python/2.7.3/Python-2.7.3.tgz"
    URL_MD5 "2cf641732ac23b18d139be077bd906cd"
    DOWNLOAD_DIR ${EXTERNAL_BINARY_DIRECTORY}
    SOURCE_DIR ${python_SOURCE_DIR}
    ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS CMAKE_ARGS)

  if(Slicer_USE_PYTHONQT_WITH_TCL)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS
      -DTCL_LIBRARY:FILEPATH=${TCL_LIBRARY}
      -DTCL_INCLUDE_PATH:PATH=${EXTERNAL_BINARY_DIRECTORY}/tcl-build/include
      -DTK_LIBRARY:FILEPATH=${TK_LIBRARY}
      -DTK_INCLUDE_PATH:PATH=${EXTERNAL_BINARY_DIRECTORY}/tcl-build/include
      )
  endif()

  if(PYTHON_ENABLE_SSL)
    list_to_string(${EP_LIST_SEPARATOR} "${OPENSSL_LIBRARIES}" EP_OPENSSL_LIBRARIES)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS
      -DOPENSSL_INCLUDE_DIR:PATH=${OPENSSL_INCLUDE_DIR}
      -DOPENSSL_LIBRARIES:STRING=${EP_OPENSSL_LIBRARIES}
      )
  endif()

  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)

  # Force Python build to "Release".
  if(CMAKE_CONFIGURATION_TYPES)
    set(SAVED_CMAKE_CFG_INTDIR ${CMAKE_CFG_INTDIR})
    set(CMAKE_CFG_INTDIR "Release")
  else()
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DCMAKE_BUILD_TYPE:STRING=Release)
  endif()
  if( BUILD_PYTHON_SHARED )
    set( BUILD_PYTHON_STATIC_SHARED_ARGS
      -DBUILD_SHARED:BOOL=ON
      -DBUILD_STATIC:BOOL=OFF
       )
  else()
    set( BUILD_PYTHON_STATIC_SHARED_ARGS
      -DBUILD_SHARED:BOOL=OFF
      -DBUILD_STATIC:BOOL=ON
       )
  endif()
  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${git_protocol}://github.com/davidsansome/python-cmake-buildsystem.git"
    GIT_TAG "82e14946e3640be603601623d9bd282b992f2d7c"
    SOURCE_DIR ${EXTERNAL_SOURCE_DIRECTORY}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_ARGS -Wno-dev --no-warn-unused-cli
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIRECTORY}/${proj}-install
      ${BUILD_PYTHON_STATIC_SHARED_ARGS}
      -DUSE_SYSTEM_LIBRARIES:BOOL=OFF
      -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}
      -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARY}
      -DENABLE_TKINTER:BOOL=${Slicer_USE_PYTHONQT_WITH_TCL}
      -DENABLE_SSL:BOOL=${PYTHON_ENABLE_SSL}
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
    ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS}
    DEPENDS
      python-source ${${proj}_DEPENDENCIES}
    )
  set(python_DIR ${EXTERNAL_BINARY_DIRECTORY}/${proj}-install)

  if(UNIX)
    if( BUILD_PYTHON_SHARED )
      set(python_IMPORT_SUFFIX so)
      if(APPLE)
        set(python_IMPORT_SUFFIX dylib)
      endif()
    else()
      set(python_IMPORT_SUFFIX a)
    endif()
    set(slicer_PYTHON_SHARED_LIBRARY_DIR ${python_DIR}/lib)
    set(PYTHON_INCLUDE_DIR ${python_DIR}/include/python2.7)
    set(PYTHON_LIBRARY ${python_DIR}/lib/libpython2.7.${python_IMPORT_SUFFIX})
    set(PYTHON_EXECUTABLE ${python_DIR}/bin/customPython)
    set(slicer_PYTHON_REAL_EXECUTABLE ${python_DIR}/bin/python)
  elseif(WIN32)
    set(slicer_PYTHON_SHARED_LIBRARY_DIR ${python_DIR}/bin)
    set(PYTHON_INCLUDE_DIR ${python_DIR}/include)
    set(PYTHON_LIBRARY ${python_DIR}/libs/python27.lib)
    set(PYTHON_EXECUTABLE ${python_DIR}/bin/customPython.exe)
    set(slicer_PYTHON_REAL_EXECUTABLE ${python_DIR}/bin/python.exe)
  else()
    message(FATAL_ERROR "Unknown system !")
  endif()
  set(PYTHON_INCLUDE_DIRS ${PYTHON_INCLUDE_DIR})
  if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_python)

    # Configure python launcher
    configure_file(
      SuperBuild/External_python_customPython_configure.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/External_python_customPython_configure.cmake
      @ONLY)
    set(python_customPython_configure_args)

    if(PYTHON_ENABLE_SSL)
      set(python_customPython_configure_args -DOPENSSL_EXPORT_LIBRARY_DIR:PATH=${OPENSSL_EXPORT_LIBRARY_DIR})
    endif()

    ExternalProject_Add_Step(${proj} python_customPython_configure
      COMMAND ${CMAKE_COMMAND} ${python_customPython_configure_args}
        -P ${CMAKE_CURRENT_BINARY_DIR}/External_python_customPython_configure.cmake
      DEPENDEES install
      )

  endif()

  if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CFG_INTDIR ${SAVED_CMAKE_CFG_INTDIR}) # Restore CMAKE_CFG_INTDIR
  endif()

else()
  #
  # there's a problem with CMake not detecting the right PYTHON_INCLUDE_DIR, so fix
  # if need be
  if( NOT EXISTS "${PYTHON_INCLUDE_DIR}/Python.h")
    find_file(_python_h "Python.h" PATHS ${PYTHON_INCLUDE_DIR}
      NO_DEFAULT_PATH)
    if(NOT _python_h)
      message(FATAL_ERROR "Can't find python.h in ${PYTHON_INCLUDE_DIR}")
    else()
      get_filename_component(_PYTHON_INCLUDE_DIR "${_python_h}" DIRECTORY)
      set(PYTHON_INCLUDE_DIR "${_PYTHON_INCLUDE_DIR}"
        CACHE PATH "include directory for Python" FORCE)
    endif()
  endif()
  SlicerMacroEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
endif()


list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS
    PYTHON_EXECUTABLE:FILEPATH
    PYTHON_INCLUDE_DIR:PATH
    PYTHON_LIBRARY:FILEPATH
    )
_expand_external_project_vars()
set(COMMON_EXTERNAL_PROJECT_ARGS ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})


if(WIN32)
  set(PYTHON_DEBUG_LIBRARY ${PYTHON_LIBRARY})
  list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS PYTHON_DEBUG_LIBRARY:STRING )
  _expand_external_project_vars()
  set(COMMON_EXTERNAL_PROJECT_ARGS ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})
endif()


ProjectDependancyPop(CACHED_extProjName extProjName)
ProjectDependancyPop(CACHED_proj proj)
