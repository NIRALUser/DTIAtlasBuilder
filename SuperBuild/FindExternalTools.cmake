# Find external tools

# Git protocol
option(USE_GIT_PROTOCOL "If behind a firewall turn this off to use http instead." ON)
set(git_protocol "git")
if(NOT USE_GIT_PROTOCOL)
  set(git_protocol "http")
else(NOT USE_GIT_PROTOCOL)
  set(git_protocol "git")
endif()

#===== Macro set paths ===============================================
macro( SetPathsRecompile )
  foreach( tool ${Tools} )
 #   set(InstallPath ${CMAKE_INSTALL_PREFIX}) # Non cache variable so its value can change and be updated
    set(TOOL${tool} ${CMAKE_INSTALL_PREFIX}/${tool} CACHE STRING "Path to the ${tool} executable")
    get_filename_component(${tool}Path ${TOOL${tool}} REALPATH ABSOLUTE) # Set the real path in the config file
#    set(${tool}Path ${TOOL${tool}}) # ${proj}Path =  variable changed in the DTIAB config file (non cache)
    mark_as_advanced(CLEAR TOOL${tool}) # Show the option in the gui
    if(DEFINED TOOL${tool}Sys)
      mark_as_advanced(FORCE TOOL${tool}Sys) # Hide the unuseful option in the gui
    endif()
  endforeach()
endmacro( SetPathsRecompile )

macro( SetPathsSystem )
  foreach( tool ${Tools} )
    get_filename_component(${tool}Path ${TOOL${tool}}Sys REALPATH ABSOLUTE) # Set the real path in the config file
#    set(${tool}Path ${TOOL${tool}Sys})
    mark_as_advanced(CLEAR TOOL${tool}Sys) # Show the option in the gui
    if(DEFINED TOOL${tool})
      mark_as_advanced(FORCE TOOL${tool}) # Hide the option in the gui
    endif()
  endforeach()
endmacro( SetPathsSystem )

#===== Macro search tools ===============================================
macro( FindToolsMacro Proj )
  set( AllToolsFound ON )
  foreach( tool ${Tools} )
    find_program( TOOL${tool}Sys ${tool}) # search TOOL${tool}Sys in the PATH
    if(${TOOL${tool}Sys} STREQUAL "TOOL${tool}Sys-NOTFOUND") # If program not found, give a warning message and set AllToolsFound variable to OFF
      message( WARNING "${tool} not found. It will not be recompiled, so either set it to ON, or get ${Proj} manually." )
      set( AllToolsFound OFF )
    endif() # Found on system
  endforeach()
endmacro()

#===== Macro add tool ===============================================
 # if SourceCodeArgs or CMAKE_ExtraARGS passed to the macro as arguments, only the first word is used (each element of the list is taken as ONE argument) => use as "global variables"
macro( AddToolMacro Proj )

  # Update and test tools
  if(COMPILE_EXTERNAL_${Proj}) # If need to recompile, just set the paths here
    SetPathsRecompile() # Uses the list "Tools"
  else(COMPILE_EXTERNAL_${Proj}) # If no need to recompile, search the tools on the system and need to recompile if some tool not found

    # search the tools on the system and warning if not found
    # If SlicerExtension, OFF packages are already in Slicer but can be not found -> don't recompile
    if( NOT DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
      FindToolsMacro( ${Proj} )
    endif( NOT DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

    # If some program not found, reset all tools to the recompiled path and recompile the whole package
    if(NOT AllToolsFound) # AllToolsFound set or reset in FindToolsMacro()
#      set( COMPILE_EXTERNAL_${Proj} ON CACHE BOOL "" FORCE)
      SetPathsRecompile() # Uses the list "Tools"
    else()
      SetPathsSystem() # Uses the list "Tools"
    endif()

  endif(COMPILE_EXTERNAL_${Proj})

  # After the main if() because we could need to recompile after not having found all tools on system
  if(COMPILE_EXTERNAL_${Proj})
    # Add project
    ExternalProject_Add(${Proj}
      ${SourceCodeArgs} # No difference between args passed separated with ';', spaces or return to line
      BINARY_DIR DTIAtlasBuilder-build/${Proj}-build
      SOURCE_DIR DTIAtlasBuilder-build/${Proj} # creates the folder if it doesn't exist
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${COMMON_BUILD_OPTIONS_FOR_EXTERNALPACKAGES}
       -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilder-build/${Proj}-build/bin
       -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilder-build/${Proj}-build/bin
       -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilder-build/${Proj}-build/bin
       -DCMAKE_BUNDLE_OUTPUT_DIRECTORY:PATH=${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilder-build/${Proj}-build/bin
       -DCMAKE_INSTALL_PREFIX:PATH=DTIAtlasBuilder-build/${Proj}-install
       ${CMAKE_ExtraARGS}
      INSTALL_COMMAND "" # So the install step of the external project is not done
    )

    list(APPEND DTIAtlasBuilderExternalToolsDependencies ${Proj})

  endif(COMPILE_EXTERNAL_${Proj})
endmacro( AddToolMacro )

#====================================================================
#====================================================================
## Libraries for tools =============================================

# VTK
set(RecompileVTK OFF)
set(VTK_DEPEND "")
if(COMPILE_EXTERNAL_dtiprocessTK OR COMPILE_EXTERNAL_AtlasWerks OR COMPILE_EXTERNAL_BRAINS OR COMPILE_EXTERNAL_NIRALUtilities)
  find_package(VTK QUIET)
  if (VTK_FOUND)
    set(VTK_USE_QVTK TRUE)
    set(VTK_USE_GUISUPPORT TRUE)
    include(${VTK_USE_FILE}) # creates VTK_DIR
  else(VTK_FOUND)
    message("VTK not found. It will be downloaded and recompiled, unless a path is manually specified in the VTK_DIR variable.") # Not a Warning = just info
    set(RecompileVTK ON) # If not found, recompile it
  endif(VTK_FOUND)
endif()

if(RecompileVTK) # BRAINSStandAlone/SuperBuild/External_VTK.cmake
    ExternalProject_Add(VTK
      GIT_REPOSITORY ${git_protocol}://vtk.org/VTK.git
      GIT_TAG "v5.10.0"
      SOURCE_DIR VTK
      BINARY_DIR VTK-build
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${COMMON_BUILD_OPTIONS_FOR_EXTERNALPACKAGES}
        -DBUILD_EXAMPLES:BOOL=OFF
        -DBUILD_TESTING:BOOL=OFF
        -DBUILD_SHARED_LIBS:BOOL=OFF
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/VTK-install
        -DVTK_USE_PARALLEL:BOOL=ON
        -DVTK_LEGACY_REMOVE:BOOL=OFF
        -DVTK_WRAP_TCL:BOOL=OFF
        #-DVTK_USE_RPATH:BOOL=ON # Unused
        -DVTK_WRAP_PYTHON:BOOL=${VTK_WRAP_PYTHON}
        -DVTK_INSTALL_LIB_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      )
    set(VTK_DIR ${CMAKE_CURRENT_BINARY_DIR}/VTK-install/lib/vtk-5.10)
    mark_as_advanced(CLEAR VTK_DIR)
    set(VTK_DEPEND VTK)
endif(RecompileVTK)

# FFTW and CLAPACK for GreedyAtlas
if(COMPILE_EXTERNAL_AtlasWerks) # FFTW D + F build one on(after) another
  # FFTW
  if(WIN32) # If windows, no recompilation so just download binary
    set(FFTW_DOWNLOAD_ARGS
        URL "ftp://ftp.fftw.org/pub/fftw/fftw-3.3.3-dll64.zip")
  else(WIN32) # Download source code and recompile
    set(FFTW_DOWNLOAD_ARGS
        URL "http://www.fftw.org/fftw-3.3.3.tar.gz"
        URL_MD5 0a05ca9c7b3bfddc8278e7c40791a1c2)
  endif(WIN32)
  ExternalProject_Add(FFTW    # FFTW has no CMakeLists.txt # Example : Slicer/SuperBuild/External_python.cmake
    ${FFTW_DOWNLOAD_ARGS}
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/FFTW-install
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/FFTW
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/FFTW-build
    CONFIGURE_COMMAND ""
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${COMMON_BUILD_OPTIONS_FOR_EXTERNALPACKAGES} # So we can give CC to configure*
    INSTALL_COMMAND ""
    BUILD_COMMAND ${CMAKE_COMMAND} -DTOP_BINARY_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild/InstallFFTW.cmake # -DARGNAME:TYPE=VALUE -P <cmake file> = Process script mode
    )
  set(FFTW_DIR ${CMAKE_CURRENT_BINARY_DIR}/FFTW-install)

  # CLAPACK (from http://www.nitrc.org/projects/spharm-pdm or http://github.com/Slicer/Slicer/blob/master-411/SuperBuild/External_CLAPACK.cmake)
  ExternalProject_Add(CLAPACK
    URL "http://www.netlib.org/clapack/clapack-3.2.1-CMAKE.tgz"
    URL_MD5 4fd18eb33f3ff8c5d65a7d43913d661b
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/CLAPACK
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/CLAPACK-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${COMMON_BUILD_OPTIONS_FOR_EXTERNALPACKAGES}
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
    INSTALL_COMMAND "" # No install step
    )
endif(COMPILE_EXTERNAL_AtlasWerks)

# ITK and SlicerExecutionModel
set(RecompileITK OFF)
set(RecompileSEM OFF)

find_package(ITK QUIET) # Not required because will be recompiled if not found
if(ITK_FOUND)
  include(${ITK_USE_FILE}) # set ITK_DIR and ITK_VERSION_MAJOR
  if(NOT ${ITK_VERSION_MAJOR} EQUAL 4)
    set(RecompileITK ON)
  else() # NO recompile ITK
    # If ITK not recompiled, look for SlicerExecutionModel
    find_package(SlicerExecutionModel) # Not required because will be recompiled if not found
    if(SlicerExecutionModel_FOUND)
      include(${SlicerExecutionModel_USE_FILE}) # creates SlicerExecutionModel_DIR (DTI-Reg & BRAINSFit)
    else(SlicerExecutionModel_FOUND)
      message(WARNING "SlicerExecutionModel not found. It will be downloaded and recompiled.")
      set(RecompileSEM ON)
    endif(SlicerExecutionModel_FOUND)
  endif() # (${ITK_VERSION_MAJOR} NOT EQUAL 4)
else(ITK_FOUND)
  set(RecompileITK ON) # Automatically recompile SlicerExecutionModel
endif(ITK_FOUND)

set(ITK_DEPEND "")
set(RecompileBatchMake OFF)
if(RecompileITK)
  message("ITKv4 not found. It will be downloaded and recompiled, unless a path is manually specified in the ITK_DIR variable.") # Not a Warning = just info
  # Download and compile ITKv4
  ExternalProject_Add(I4 # BRAINSStandAlone/SuperBuild/External_ITKv4.cmake # !! All args needed as they are # Name shorten from ITKv4 because Windows ITKv4 path limited to 50 chars
    GIT_REPOSITORY "${git_protocol}://itk.org/ITK.git"
    GIT_TAG 35b90133a793ffd884820e499175db19366fe627 # 2013-07-12 (from Slicer) # 1866ef42887df677a6197ad11ed0ef6e9b239567 # 2013-04-03 (from Slicer)
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/I4 # Path shorten from ITKv4 because Windows SOURCE_DIR path limited to 50 chars
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/I4-b # Path shorten from ITKv4 because Windows SOURCE_DIR path limited to 50 chars
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS # !! ALL options need to be here for all tools to compile with this version of ITK
      ${COMMON_BUILD_OPTIONS_FOR_EXTERNALPACKAGES}
      -Wno-dev
      --no-warn-unused-cli
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/I4-i # Path shorten from ITKv4 because Windows SOURCE_DIR path limited to 50 chars
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DITK_LEGACY_REMOVE:BOOL=OFF
      -DITKV3_COMPATIBILITY:BOOL=ON
      -DITK_BUILD_ALL_MODULES:BOOL=ON
      -DITK_USE_REVIEW:BOOL=OFF # ON ok with BRAINSFit and ANTS not with dtiprocess and ResampleDTI # OFF ok with BRAINSFit
      -DKWSYS_USE_MD5:BOOL=ON # Required by SlicerExecutionModel
      -DUSE_WRAP_ITK:BOOL=OFF ## HACK:  QUICK CHANGE
      -DITK_USE_SYSTEM_DCMTK:BOOL=OFF
#      -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_CURRENT_BINARY_DIR}/ITKv4-build/lib # Needed for BRAINSTools to compile
#      -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_CURRENT_BINARY_DIR}/ITKv4-build/lib # Needed for BRAINSTools to compile
#      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_CURRENT_BINARY_DIR}/ITKv4-build/bin # Needed for BRAINSTools to compile
    )
  set(ITK_DIR ${CMAKE_CURRENT_BINARY_DIR}/I4-i/lib/cmake/ITK-4.4 FORCE) # Use the downloaded ITK for all tools # Path shorten from ITKv4 because Windows SOURCE_DIR path limited to 50 chars
  set(ITK_DEPEND I4)
  set(RecompileSEM ON) # If recompile ITK, recompile SlicerExecutionModel
  set(RecompileBatchMake ON) # If recompile ITK, recompile BatchMake
endif(RecompileITK)

if(RecompileSEM)
  # Download and compile SlicerExecutionModel with the downloaded ITKv4
  ExternalProject_Add(SlicerExecutionModel # BRAINSStandAlone/SuperBuild/External_SlicerExecutionModel.cmake
    GIT_REPOSITORY "${git_protocol}://github.com/Slicer/SlicerExecutionModel.git"
    GIT_TAG aa1a088fca42e77832d8814737735c9c9b321e9a
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/SlicerExecutionModel
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/SlicerExecutionModel-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${COMMON_BUILD_OPTIONS_FOR_EXTERNALPACKAGES}
      -Wno-dev
      --no-warn-unused-cli
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/SlicerExecutionModel-install
      -DBUILD_SHARED_LIBS:BOOL=OFF
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DITK_DIR:PATH=${ITK_DIR}
    INSTALL_COMMAND ""
    DEPENDS ${ITK_DEPEND} # either ITKv4 if recompiled, or empty
    )
  set(SlicerExecutionModel_DIR ${CMAKE_CURRENT_BINARY_DIR}/SlicerExecutionModel-build) # Use the downloaded SlicerExecutionModel for all tools
  mark_as_advanced(CLEAR SlicerExecutionModel_DIR)
  set(GenerateCLP_DIR ${SlicerExecutionModel_DIR}/GenerateCLP)
  set(ModuleDescriptionParser_DIR ${SlicerExecutionModel_DIR}/ModuleDescriptionParser)
  set(TCLAP_DIR ${SlicerExecutionModel_DIR}/tclap)
  list(APPEND ITK_DEPEND SlicerExecutionModel)

endif(RecompileSEM)

# BatchMake for DTI-Reg (after ITK)
set(BatchMake_DEPEND "")
if(COMPILE_EXTERNAL_DTIReg) # BatchMake only needed for DTIReg
  find_package(BatchMake QUIET) # Not required because will be recompiled if not found # No warning displaid
  if(BatchMake_FOUND)
    include(${BatchMake_USE_FILE})
  else(BatchMake_FOUND)
    message("BatchMake not found. It will be downloaded and recompiled, unless a path is manually specified in the BatchMake_DIR variable.") # Not a Warning = just info
    set(RecompileBatchMake ON) # If not found, recompile it
  endif(BatchMake_FOUND )
else(COMPILE_EXTERNAL_DTIReg)
  set(RecompileBatchMake OFF) # if has been enabled by if(RecompileITK): No DTI-Reg = No BatchMake
endif(COMPILE_EXTERNAL_DTIReg)

if(RecompileBatchMake)
  # If SlicerExtension, Use BatchMake CURL_SPECIAL_LIBZ var to compile bmcurl with zlib from Slicer because tries to link with zlib from Slicer (names mangled for Slicer: slicer_zlib_... see SlicerBuildTree/zlib-install/include/zlib_mangle.h)
  # Otherwise compiles with zlib from ITK (mangled itk_zlib_... see BatchMake/Utilities/Zip/itk_zlib_mangle.h) and try to link with zlib from Slicer => FAIL
  set( BatchMakeCURLCmakeArg "" )
  if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )  
    find_library( PathToSlicerZlib
    NAMES zlib
    PATHS ${Slicer_HOME}/../zlib-install/lib # ${Slicer_HOME} is <topofbuildtree>/Slicer-build: defined in SlicerConfig.cmake
    PATH_SUFFIXES Debug Release RelWithDebInfo MinSizeRel # For Windows, it can be any one of these
    NO_DEFAULT_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    )
    set( BatchMakeCURLCmakeArg -DCURL_SPECIAL_LIBZ:PATH=${PathToSlicerZlib} )
  endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

  ExternalProject_Add(BatchMake
    GIT_REPOSITORY ${git_protocol}://batchmake.org/BatchMake.git
    GIT_TAG "8addbdb62f0135ba01ffe12ddfc32121b6d66ef5" # 01-30-2013 # "0abb2faca1251f808ab3d0b820cc27b570a994f1" # 08-26-2012 updated for ITKv4 # "43d21fcccd09e5a12497bc1fb924bc6d5718f98c" # used in DTI-Reg 12-21-2012
    SOURCE_DIR BatchMake
    BINARY_DIR BatchMake-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${COMMON_BUILD_OPTIONS_FOR_EXTERNALPACKAGES}
      -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_CURRENT_BINARY_DIR}/BatchMake-build/bin
      -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_CURRENT_BINARY_DIR}/BatchMake-build/bin
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_CURRENT_BINARY_DIR}/BatchMake-build/bin
      -DCMAKE_BUNDLE_OUTPUT_DIRECTORY:PATH=${CMAKE_CURRENT_BINARY_DIR}/BatchMake-build/bin
      -DBUILD_SHARED_LIBS:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DUSE_FLTK:BOOL=OFF
      -DDASHBOARD_SUPPORT:BOOL=OFF
      -DGRID_SUPPORT:BOOL=OFF
      -DUSE_SPLASHSCREEN:BOOL=OFF
      -DITK_DIR:PATH=${ITK_DIR}
      ${BatchMakeCURLCmakeArg}
    INSTALL_COMMAND ""
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild/BatchMakePatchedZip.c ${CMAKE_CURRENT_BINARY_DIR}/BatchMake/Utilities/Zip/zip.c # No "" # Patch for windows compilation error (declaration of variable after beginning of block - "uLong year")
    DEPENDS ${ITK_DEPEND}
  )
  set(BatchMake_DIR ${CMAKE_CURRENT_BINARY_DIR}/BatchMake-build)
  mark_as_advanced(CLEAR BatchMake_DIR)
  set(BatchMake_ITK_DIR ${ITK_DIR}) # If batchmake recompiled, no include(${BatchMake_USE_FILE}) has been done so BatchMake_ITK_DIR does not exist, and we used ${ITK_DIR} to compile it.
  set(BatchMake_DEPEND BatchMake)
endif(RecompileBatchMake)


## GLUT for MriWatcher -> disable MriWatcher if Slicer Ext for the moment
#  ExternalProject_Add(GLUT
#    URL http://www.opengl.org/resources/libraries/glut/glut-3.7.tar.gz
#    URL_MD5 dc932666e2a1c8a0b148a4c32d111ef3 # $ md5sum (file)
#    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/FFTW-install
#    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/FFTW
#    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/FFTW-build
#    CONFIGURE_COMMAND ""
#    INSTALL_COMMAND ""
#    BUILD_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/FFTW-install/InstallFFTW.cmake # -DARGNAME:TYPE=VALUE -P <cmake file> = Process script mode
#    )
#set(GLUT_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/GLUT-install/include)

#====================================================================
#===== TOOLS ========================================================
#       ||
#       VV

### For Slicer Extension:
## CLI Modules # CLI= (pgm) --xml exists # So the cli_modules go to Extensions/DTIAtlaBuilder/lib/Slicer4.2/cli_module
# dtiprocessTK
# ResampleDTI
# DTIReg
## No CLI Modules # So the non cli_modules don't go to Extensions/DTIAtlaBuilder/lib/Slicer4.2/cli_module but to Extensions/DTIAtlaBuilder/bin
# AtlasWerks
# BRAINS # BRAINS is a cli_module but needs to be in non cli_module to prevent conflict with actual Slicer's BRAINS
# ANTS
# MriWatcher
# NIRALUtilities
# teem # teem is in Slicer but not a cli_module

# ===== dtiprocessTK ==============================================================
set( SourceCodeArgs
  SVN_REPOSITORY "http://www.nitrc.org/svn/dtiprocess/branches/Slicer4Extension" # /dtiprocess"
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  SVN_REVISION -r 149 # 01/31/2013 # 144 # 01/30/2013 version modified by Adrien for windows compilation # 137 # 01/16/2013 # 113 # 12/20/2012 updated for ITKv4.4.0
  )
set( CMAKE_ExtraARGS
  -DBUILD_TESTING:BOOL=OFF
  -DITK_DIR:PATH=${ITK_DIR}
  -DVTK_DIR:PATH=${VTK_DIR}
  -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
  -DModuleDescriptionParser_DIR:PATH=${ModuleDescriptionParser_DIR}
  -DTCLAP_DIR:PATH=${TCLAP_DIR}
  -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
  -DDTIProcess_BUILD_SLICER_EXTENSION:BOOL=OFF
  DEPENDS ${ITK_DEPEND} ${VTK_DEPEND}
  )
set( Tools
  dtiprocess
  dtiaverage
  )
AddToolMacro( dtiprocessTK ) # AddToolMacro( proj ) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== AtlasWerks ================================================================
# code for external tools from http://github.com/Chaircrusher/AtlasWerksBuilder/blob/master/CMakeLists.txt
set( SourceCodeArgs
#  URL "http://www.sci.utah.edu/releases/atlaswerks_v0.1.4/AtlasWerks_0.1.4_Linux.tgz"
#  URL_MD5 05fc867564e3340d0d448dd0daab578a
  GIT_REPOSITORY "${git_protocol}://github.com/BRAINSia/AtlasWerks.git"
  GIT_TAG "4a3e02b9e6aa9ad527c9b1df4b0ab31c737cbd78" # 02-01-2013 fix bug with clang mac build
  )
set( CMAKE_ExtraARGS
  -DITK_DIR:PATH=${ITK_DIR}
  -DVTK_DIR:PATH=${VTK_DIR}
  -DLAPACK_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}/CLAPACK-build
#  -DFFTW_INSTALL_BASE_PATH:PATH=${FFTW_DIR} # will use find_library to find the libs
#  -DFFTWF_LIB:PATH=${FFTW_DIR}/lib/libfftw3f.a # FFTW in float
#  -DFFTWD_LIB:PATH=${FFTW_DIR}/lib/libfftw3.a # FFTW in double # needed for AtlasWerks to configure, not to compile with
#  -DFFTWF_THREADS_LIB:PATH=${FFTW_DIR}/lib/libfftw3f_threads.a
#  -DFFTWD_THREADS_LIB:PATH=${FFTW_DIR}/lib/libfftw3_threads.a
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
  DEPENDS ${ITK_DEPEND} ${VTK_DEPEND} FFTW CLAPACK # Not CMake Arg -> directly after CMakeArg in ExternalProject_Add()
  )
set( Tools
  GreedyAtlas
  )
AddToolMacro( AtlasWerks ) # AddToolMacro( proj ) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== BRAINSFit =============================================================
set( SourceCodeArgs
  GIT_REPOSITORY "${git_protocol}://github.com/BRAINSia/BRAINSStandAlone.git"
  GIT_TAG "1abcb55fafc1c9b94dabd33f5fcc40e248c326ba" # 01-30-2013 fix bug with ITK4.4 # "dd7ad3926a01fbdd098ea858fb95012ca16fb236" # 12/18/2012
# "ff94032edafbc46a95f51db4bce894f0120b5992" : Slicer4 version # /devel/linux/Slicer4_linux64/Slicer/SuperBuild/External_BRAINSTools.cmake -> compiles but segfault
# "31dcba577ee1bac5c4680fc9d7c830d6074020a9" : 12/13/2012
# "98a46a2b08da882d46f04cbf0d539c2b73348049" # version from http://www.nitrc.org/svn/dtiprep/trunk/SuperBuild/External_BRAINSTools.cmake -> compiles but run error "undefined symbol: ModuleEntryPoint"
  )

set( CMAKE_ExtraARGS
  -DBUILD_TESTING:BOOL=OFF
  -DBUILD_SHARED_LIBS:BOOL=OFF
  -DINTEGRATE_WITH_SLICER:BOOL=OFF
  -DBRAINSTools_SUPERBUILD:BOOL=OFF
  -DSuperBuild_BRAINSTools_USE_GIT:BOOL=${USE_GIT_PROTOCOL}
  -DITK_VERSION_MAJOR:STRING=4
  -DITK_DIR:PATH=${ITK_DIR}
  -DVTK_DIR:PATH=${VTK_DIR}
  -DUSE_SYSTEM_ITK=ON
  -DUSE_SYSTEM_SlicerExecutionModel=ON
  -DUSE_SYSTEM_VTK=ON
  -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
  -DModuleDescriptionParser_DIR:PATH=${ModuleDescriptionParser_DIR}
  -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
  -DBUILD_TESTING:BOOL=OFF
  -DUSE_BRAINSDemonWarp:BOOL=ON
  -DUSE_BRAINSFit:BOOL=ON
  -DUSE_BRAINSResample:BOOL=OFF
  -DUSE_AutoWorkup:BOOL=OFF
  -DUSE_ANTS:BOOL=OFF
  -DUSE_BRAINSContinuousClass:BOOL=OFF
  -DUSE_BRAINSFitEZ:BOOL=OFF
  -DUSE_BRAINSROIAuto:BOOL=OFF
  -DUSE_BRAINSSurfaceTools:BOOL=OFF
  -DUSE_DebugImageViewer:BOOL=OFF
  -DUSE_BRAINSABC:BOOL=OFF
  -DUSE_BRAINSConstellationDetector:BOOL=OFF
  -DUSE_BRAINSCreateLabelMapFromProbabilityMaps:BOOL=OFF
  -DUSE_BRAINSCut:BOOL=OFF
  -DUSE_BRAINSImageConvert:BOOL=OFF
  -DUSE_BRAINSInitializedControlPoints:BOOL=OFF
  -DUSE_BRAINSLandmarkInitializer:BOOL=OFF
  -DUSE_BRAINSMultiModeSegment:BOOL=OFF
  -DUSE_BRAINSMush:BOOL=OFF
  -DUSE_BRAINSSnapShotWriter:BOOL=OFF
  -DUSE_BRAINSTransformConvert:BOOL=OFF
  -DUSE_ConvertBetweenFileFormats:BOOL=OFF
  -DUSE_DWIConvert:BOOL=OFF
  -DUSE_DebugImageViewer:BOOL=OFF
  -DUSE_ICCDEF:BOOL=OFF
  -DUSE_ImageCalculator:BOOL=OFF
  -DUSE_GTRACT:BOOL=OFF
  -DLOCAL_SEM_EXECUTABLE_ONLY:BOOL=ON # Variable used in SlicerExecutionModel/CMake/SEMMacroBuildCLI.cmake:l.120 : if true, will only create executable without shared lib lib(exec)Lib.so
  DEPENDS ${ITK_DEPEND} ${VTK_DEPEND} # So ITK is compiled before
  )
set( Tools
  BRAINSFit
  BRAINSDemonWarp
  )
AddToolMacro( BRAINS ) # AddToolMacro( proj ) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== ANTS/WarpMultiTransform =====================================================
set( SourceCodeArgs
  # SVN_REPOSITORY "http://advants.svn.sourceforge.net/svnroot/advants/trunk"
  # SVN_REVISION -r 1685 # 12/13/2012
  GIT_REPOSITORY "${git_protocol}://github.com/stnava/ANTs.git"
  GIT_TAG 6d082e02310077d17e84b6c3c9126759a96b87bc # 2013-05-02 Update to fix comp error due to new ITK # "49a8e5911cc5cbd180f15c63ee8c545ebd3828f9" # 2013-04-11 Prevent Boost from compiling libs
  )
set( CMAKE_ExtraARGS
  -DBUILD_TESTING:BOOL=OFF
  -DBUILD_SHARED_LIBS:BOOL=OFF
  -DBUILD_EXTERNAL_APPLICATIONS:BOOL=OFF
  -DANTS_SUPERBUILD:BOOL=ON
  -DSuperBuild_ANTS_USE_GIT_PROTOCOL:BOOL=${USE_GIT_PROTOCOL}
  -DUSE_SYSTEM_ITK:BOOL=ON
  -DITK_DIR:PATH=${ITK_DIR}
  -DITK_VERSION_MAJOR:STRING=4
  -DUSE_SYSTEM_SlicerExecutionModel:BOOL=ON
  -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
  -DUSE_VTK:BOOL=OFF
  DEPENDS ${ITK_DEPEND}
  )
set( Tools
  ANTS
  WarpImageMultiTransform
  WarpTensorImageMultiTransform
  )
AddToolMacro( ANTS ) # AddToolMacro( proj ) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== ResampleDTIlogEuclidean =====================================================
set( SourceCodeArgs
  GIT_REPOSITORY "${git_protocol}://github.com/NIRALUser/ResampleDTIlogEuclidean.git"
  GIT_TAG a2afcd57c931d224b91e6ce026cb499648e4f017 # 04-22-2013 addition of MatrixOffsetTransformBase support (ANTS affine tfm for DTIReg)
# "84e691a0600128dbe1e9d41a80336e7083e73fa7" # 02/04/2013 fix comp with ITK4.4 + stat lib windows linkage error
# "e78a9ea00d73a11cc52b3c457e32f1302a3403d4" # 12/20/2012
# URL "http://www.insight-journal.org/download/sourcecode/742/11/SourceCode11_ResampleDTIInsightJournal2.tar.gz"
# http://github.com/midas-journal/midas-journal-742/tree/master/ResampleDTIInsightJournal
  )
set( CMAKE_ExtraARGS
  -DBUILD_TESTING:BOOL=OFF
  -DBUILD_GENERATECLP:BOOL=OFF
  -DITK_DIR:PATH=${ITK_DIR}
  -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
  DEPENDS ${ITK_DEPEND}
  )
set( Tools
  ResampleDTIlogEuclidean
  )
AddToolMacro( ResampleDTI ) # AddToolMacro( proj ) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== DTI-Reg =====================================================================
set( SourceCodeArgs
  SVN_REPOSITORY "http://www.nitrc.org/svn/dtireg/trunk"
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  SVN_REVISION -r 47 # 10/28/2013
  )
set( CMAKE_ExtraARGS
  -DANTSTOOL:PATH=${ANTSPath}
  -DBRAINSDemonWarpTOOL:PATH=${BRAINSDemonWarpPath}
  -DBRAINSFitTOOL:PATH=${BRAINSFitPath}
  -DBatchMake_DIR:PATH=${BatchMake_DIR}
  -DITK_DIR:PATH=${BatchMake_ITK_DIR} # Compile DTI-Reg with the same ITK than Batchmake (BatchMake_ITK_DIR is in BatchMake_USE_FILE) -> BatchMakeLib and ITK_LIBRARIES contain the same ITK libs
  -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
  -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
  -DModuleDescriptionParser_DIR:PATH=${ModuleDescriptionParser_DIR}
  -DTCLAP_DIR:PATH=${TCLAP_DIR}
  -DCOMPILE_EXTERNAL_dtiprocess:BOOL=OFF
  -DOPT_USE_SYSTEM_BatchMake:BOOL=ON
  -DOPT_USE_SYSTEM_ITK:BOOL=ON
  -DOPT_USE_SYSTEM_SlicerExecutionModel:BOOL=ON
  -DResampleDTITOOL:PATH=${ResampleDTIlogEuclideanPath}
  -DWARPIMAGEMULTITRANSFORMTOOL:PATH=${WarpImageMultiTransformPath}
  -DWARPTENSORIMAGEMULTITRANSFORMTOOL:PATH=${WarpTensorImageMultiTransformPath}
  -DdtiprocessTOOL:PATH=${dtiprocessPath}
  DEPENDS ${ITK_DEPEND} ${BatchMake_DEPEND}
  )
set( Tools
  DTI-Reg
  )
AddToolMacro( DTIReg ) # AddToolMacro( proj ) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== teem (unu) =====================================================================
set( SourceCodeArgs
  SVN_REPOSITORY "http://svn.code.sf.net/p/teem/code/teem/trunk"  
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  SVN_REVISION -r 5888 # 12/13/2012
  )
set( CMAKE_ExtraARGS
  -DITK_DIR:PATH=${ITK_DIR}
  DEPENDS ${ITK_DEPEND}
  )
set( Tools
  unu
  )
AddToolMacro( teem ) # AddToolMacro( proj ) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== MriWatcher =====================================================================
set( SourceCodeArgs
  SVN_REPOSITORY "http://www.nitrc.org/svn/mriwatcher/branches/mriwatcher_qt4"
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  SVN_REVISION -r 16 # 12/13/2012
  )
set( CMAKE_ExtraARGS
  -DQT_QMAKE_EXECUTABLE:PATH=${QT_QMAKE_EXECUTABLE}
  -DITK_DIR:PATH=${ITK_DIR}
  DEPENDS ${ITK_DEPEND}
  )
set( Tools
  MriWatcher
  )
AddToolMacro( MriWatcher ) # AddToolMacro( proj ) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== NIRALUtilities ===================================================================
set( SourceCodeArgs
  SVN_REPOSITORY "http://www.nitrc.org/svn/niral_utilities/trunk"
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  SVN_REVISION -r 50 # 08/15/2013
  )
set( CMAKE_ExtraARGS
  -DCOMPILE_CONVERTITKFORMATS:BOOL=OFF
  -DCOMPILE_CROPTOOLS:BOOL=ON
  -DCOMPILE_CURVECOMPARE:BOOL=OFF
  -DCOMPILE_DTIAtlasBuilder:BOOL=OFF
  -DCOMPILE_DWI_NIFTINRRDCONVERSION:BOOL=OFF
  -DCOMPILE_IMAGEMATH:BOOL=ON
  -DCOMPILE_IMAGESTAT:BOOL=OFF
  -DCOMPILE_POLYDATAMERGE:BOOL=OFF
  -DCOMPILE_POLYDATATRANSFORM:BOOL=OFF
  -DCOMPILE_TRANSFORMDEFORMATIONFIELD:BOOL=OFF
  -DITK_DIR:PATH=${ITK_DIR}
  -DVTK_DIR:PATH=${VTK_DIR}
  -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
  -DModuleDescriptionParser_DIR:PATH=${ModuleDescriptionParser_DIR}
  -DTCLAP_DIR:PATH=${TCLAP_DIR}
  DEPENDS ${ITK_DEPEND} ${VTK_DEPEND}
  )
set( Tools
  ImageMath
  CropDTI
  )
AddToolMacro( NIRALUtilities ) # AddToolMacro( proj ) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

