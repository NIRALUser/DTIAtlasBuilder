# Find external tools

# External Projects
include(ExternalProject) # "ExternalProject" is the module that will allow to compile tools
if(CMAKE_EXTRA_GENERATOR) # Extra generator ??
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

# Git protocol
option(USE_GIT_PROTOCOL "If behind a firewall turn this off to use https instead." ON)
set(git_protocol "git")
if(NOT USE_GIT_PROTOCOL)
  set(git_protocol "https")
else(NOT USE_GIT_PROTOCOL)
  set(git_protocol "git")
endif()

#===== Macro set paths ===============================================
macro( SetPathsRecompile )
  foreach( tool ${Tools} )
 #   set(InstallPath ${CMAKE_INSTALL_PREFIX}) # Non cache variable so its value can change and be updated
    set(TOOL${tool} ${EXECUTABLE_OUTPUT_PATH}/${tool} CACHE STRING "Path to the ${tool} executable")
    set(${tool}Path ${TOOL${tool}}) # ${proj}Path =  variable changed in the DTIAB config file (non cache)
    mark_as_advanced(CLEAR TOOL${tool}) # Show the option in the gui
    if(DEFINED TOOL${tool}Sys)
      mark_as_advanced(FORCE TOOL${tool}Sys) # Hide the unuseful option in the gui
    endif()
  endforeach()
endmacro( SetPathsRecompile )

macro( SetPathsSystem )
  foreach( tool ${Tools} )
    set(${tool}Path ${TOOL${tool}Sys})
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
    find_program( TOOL${tool}Sys ${tool} )
    if(${TOOL${tool}Sys} STREQUAL "TOOL${tool}Sys-NOTFOUND") # If program not found, give a warning message and set AllToolsFound variable to OFF
      message( WARNING "${tool} not found. CMake external will download and compile the whole ${Proj} package" )
      set( AllToolsFound OFF )
    endif()
  endforeach()
endmacro()

#===== Macro add tool ===============================================
 # if SourceCodeArgs or CMAKE_ExtraARGS passed to the macro as arguments, only the first word is used (each element of the list is taken as ONE argument) => use as "global variables"
macro( AddToolMacro Proj ) # ex: Proj = dtiprocessTK , tools = dtiprocess, dtiaverage |  Proj = AtlasWerks , tools = GreedyAtlas

  # Update and test tools
  if(COMPILE_EXTERNAL_${Proj}) # If need to recompile, just set the paths here
    SetPathsRecompile() # Uses the list "Tools"
  else(COMPILE_EXTERNAL_${Proj}) # If no need to recompile, search the tools on the system and need to recompile if some tool not found

    # search the tools on the system and warning if not found
    FindToolsMacro( ${Proj} )

    # If some program not found, reset all tools to the recompiled path and recompile the whole package
    if(NOT AllToolsFound) # AllToolsFound set or reset in FindToolsMacro()
      set( COMPILE_EXTERNAL_${Proj} ON CACHE BOOL "" FORCE)
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
      SOURCE_DIR ${Proj} # creates the folder if it doesn't exist
      BINARY_DIR ${Proj}-build
      UPDATE_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${LOCAL_CMAKE_BUILD_OPTIONS}
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX:PATH=${Proj}-install # ${CMAKE_INSTALL_PREFIX}
#        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${Proj}-build/bin # not ${EXECUTABLE_OUTPUT_PATH} because a lot of executables will be generated in the output folder
#        -DEXECUTABLE_OUTPUT_PATH:PATH=${Proj}-build/bin # not ${EXECUTABLE_OUTPUT_PATH} because a lot of executables will be generated in the output folder
        ${CMAKE_ExtraARGS}
      INSTALL_COMMAND ""
# DEPENDS ITK VTK FFTWF FFTWD CLAPACK ${FLTK_PREREQ}
# DEPENDS  ${ITK_DEPEND} ${SlicerExecutionModel_DEPEND} ${VTK_DEPEND}
    )
    # Install step : copy all needed executables to ${EXECUTABLE_OUTPUT_PATH}
    foreach( tool ${Tools} )
      install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${Proj}-build/bin/${tool} DESTINATION ${EXECUTABLE_OUTPUT_PATH}) # bin/${tool} ${CMAKE_INSTALL_PREFIX}
    endforeach()

  endif(COMPILE_EXTERNAL_${Proj})
endmacro( AddToolMacro )

#====================================================================
#====================================================================
## Libraries for tools =============================================

find_package(VTK REQUIRED)
if (VTK_FOUND)
  set(VTK_USE_QVTK TRUE)
  set(VTK_USE_GUISUPPORT TRUE)
  include(${VTK_USE_FILE}) # creates VTK_DIR
else(VTK_FOUND)
  message(FATAL_ERROR, "VTK not found. Please set VTK_DIR.")
endif (VTK_FOUND)

find_package(SlicerExecutionModel REQUIRED)
if(SlicerExecutionModel_FOUND)
  include(${SlicerExecutionModel_USE_FILE}) # creates SlicerExecutionModel_DIR (DTI-Reg)
  include(${SlicerExecutionModel_CMAKE_DIR}/SEMMacroBuildCLI.cmake)
else(SlicerExecutionModel_FOUND)
  message(FATAL_ERROR "SlicerExecutionModel not found. Please set SlicerExecutionModel_DIR")
endif(SlicerExecutionModel_FOUND)

if(COMPILE_EXTERNAL_AtlasWerks) # FFTW and FLTK only needed for AtlasWerks
  set(FFTW_DIR CACHE PATH "Path to the fftw install folder (./include, ./lib/libfftw3f.a)")
  if(NOT FFTW_DIR)
    message(FATAL_ERROR "FFTW not set. Please set FFTW_DIR manually")
  endif()

 # find_package(FLTK REQUIRED)
  if(FLTK_FOUND)
    include_directories(${FLTK_INCLUDE_DIR}) # creates FLTK_DIR
  else(FLTK_FOUND)
 #   message(FATAL_ERROR "FLTK not found. Please set FLTK_DIR")
  endif(FLTK_FOUND)
endif(COMPILE_EXTERNAL_AtlasWerks)

#====================================================================
#===== TOOLS ========================================================
#       ||
#       VV

# ===== dtiprocessTK ==============================================================
set( SourceCodeArgs
  SVN_REPOSITORY "https://www.nitrc.org/svn/dtiprocess/trunk"
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  SVN_REVISION -r 82
  )
set( CMAKE_ExtraARGS
  -DBUILD_TESTING:BOOL=OFF
  -DITK_DIR:PATH=${ITK_DIR}
  -DVTK_DIR:PATH=${VTK_DIR}
  -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
  -DModuleDescriptionParser_DIR:PATH=${ModuleDescriptionParser_DIR}
  -DTCLAP_DIR:PATH=${TCLAP_DIR}
  )
set( Tools
  dtiprocess
  dtiaverage
  )
AddToolMacro( dtiprocessTK ) # AddToolMacro( proj )

# ===== AtlasWerks ================================================================
# code for external tools from https://github.com/Chaircrusher/AtlasWerksBuilder/blob/master/CMakeLists.txt
set( SourceCodeArgs
  URL "http://www.sci.utah.edu/releases/atlaswerks_v0.1.4/AtlasWerks_0.1.4_Linux.tgz"
  URL_MD5 05fc867564e3340d0d448dd0daab578a
  )
set( CMAKE_ExtraARGS
  -DITK_DIR:PATH=${ITK_DIR}
  -DVTK_DIR:PATH=${VTK_DIR}
  -DFLTK_DIR:PATH=${FLTK_DIR}
  -DatlasWerks_COMPILE_APP_IMMAP:BOOL=OFF # Because needs FTLK
  -DFFTWF_LIB:PATH=${FFTW_DIR}/lib/libfftw3f.a # FFTW in float
  -DFFTWD_LIB:PATH=${FFTW_DIR}/lib/libfftw3.a # FFTW in double
  -DFFTW_INCLUDE:PATH=${FFTW_DIR}/include
#  -DFFTWD_THREADS_LIB:PATH=${Prereqs}/lib/libfftw3_threads.a
#  -DFFTWF_THREADS_LIB:PATH=${Prereqs}/lib/libfftw3f_threads.a
#  -DatlasWerks_COMPILE_APP_ImageConvert:BOOL=OFF
#  ${BuildGUIFlag}
#  "-DLAPACK_LIBS:STRING=lapack blas f2c"
#  "-DLAPACK_LIBS_SEARCH_DIRS:STRING=${Prereqs}/lib"
  )
set( Tools
  GreedyAtlas
  )
AddToolMacro( AtlasWerks )

# ===== BRAINSFit =============================================================
set( SourceCodeArgs
  GIT_REPOSITORY ${git_protocol}://github.com/BRAINSia/BRAINSStandAlone.git
  GIT_TAG "12b6d41a74ec30465a07df9c361237f2b77c2955"
  )
set( CMAKE_ExtraARGS
  -DBUILD_TESTING:BOOL=OFF
  -DBUILD_SHARED_LIBS:BOOL=OFF # ${BUILD_SHARED_LIBS}
  -DSuperBuild_BRAINSTools_USE_GIT:BOOL=${USE_GIT_PROTOCOL}
  -DITK_VERSION_MAJOR:STRING=3
  -DITK_DIR:PATH=${ITK_DIR}
  -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
  -DModuleDescriptionParser_DIR:PATH=${ModuleDescriptionParser_DIR}
  -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
  -DBUILD_TESTING:BOOL=OFF
  -DUSE_AutoWorkup:BOOL=OFF
  -DUSE_BRAINSContinuousClass:BOOL=OFF
  -DUSE_BRAINSDemonWarp:BOOL=ON
  -DUSE_BRAINSFit:BOOL=ON
  -DUSE_BRAINSFitEZ:BOOL=OFF
  -DUSE_BRAINSROIAuto:BOOL=OFF
  -DUSE_BRAINSResample:BOOL=OFF
  -DUSE_BRAINSSurfaceTools:BOOL=OFF
  -DUSE_DebugImageViewer:BOOL=OFF
  -DUSE_GTRACT:BOOL=OFF
  -DUSE_SYSTEM_ITK=ON
  -DUSE_SYSTEM_SlicerExecutionModel=ON
  -DUSE_SYSTEM_VTK=ON
  )
set( Tools
  BRAINSFit
  BRAINSDemonWarp
  )
AddToolMacro( BRAINS )

# ===== ANTS/WarpMultiTransform =====================================================
set( SourceCodeArgs
  SVN_REPOSITORY "https://advants.svn.sourceforge.net/svnroot/advants/trunk"
  SVN_REVISION -r 1395
  )
set( CMAKE_ExtraARGS
  -DBUILD_TESTING:BOOL=OFF
  -DBUILD_SHARED_LIBS:BOOL=OFF # ${BUILD_SHARED_LIBS}
  -DSuperBuild_ANTS_USE_GIT_PROTOC:BOOL=${USE_GIT_PROTOCOL}
  )
set( Tools
  ANTS
  WarpImageMultiTransform
  WarpTensorImageMultiTransform
  )
AddToolMacro( ANTS  )

# ===== ResampleDTIlogEuclidean =====================================================
set( SourceCodeArgs
  SVN_REPOSITORY "http://svn.slicer.org/Slicer4/trunk/Modules/CLI/ResampleDTIVolume"
  SVN_REVISION -r 20422
  )
set( CMAKE_ExtraARGS
  ""
  )
set( Tools
  ResampleDTIlogEuclidean
  )
AddToolMacro( ResampleDTI )

# ===== DTI-Reg =====================================================================
set( SourceCodeArgs
  SVN_REPOSITORY "https://www.nitrc.org/svn/dtireg/trunk"
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  SVN_REVISION -r 23
  )
set( CMAKE_ExtraARGS
  -DANTSTOOL:PATH=${ANTSPath}
  -DBRAINSDemonWarpTOOL:PATH=${BRAINSDemonWarpPath}
  -DBRAINSFitTOOL:PATH=${BRAINSFitPath}
  -DCOMPILE_EXTERNAL_dtiprocess:BOOL=OFF
  -DOPT_USE_SYSTEM_BatchMake:BOOL=ON
  -DOPT_USE_SYSTEM_ITK:BOOL=ON
  -DOPT_USE_SYSTEM_SlicerExecution:BOOL=ON
  -DResampleDTITOOL:PATH=${ResampleDTIlogEuclideanPath}
  -DWARPIMAGEMULTITRANSFORMTOOL:PATH=${WarpImageMultiTransformPath}
  -DWARPTENSORIMAGEMULTITRANSFORMTOOL:PATH=${WarpTensorImageMultiTransformPath}
  -DdtiprocessTOOL:PATH=${dtiprocessPath}
  )
set( Tools
  DTI-Reg
  DTI-Reg_Scalar_ANTS.bms
  DTI-Reg_Scalar_BRAINS.bms
  )
AddToolMacro( DTIReg )

# ===== teem (unu) =====================================================================
set( SourceCodeArgs
  SVN_REPOSITORY "https://teem.svn.sourceforge.net/svnroot/teem/teem/trunk"
  SVN_REVISION -r 5888
  )
set( CMAKE_ExtraARGS
  )
set( Tools
  unu
  )
AddToolMacro( teem )

# ===== MriWatcher =====================================================================
set( SourceCodeArgs
  SVN_REPOSITORY "https://www.nitrc.org/svn/mriwatcher/branches/mriwatcher_qt4"
  SVN_REVISION -r 16
  )
set( CMAKE_ExtraARGS
  )
set( Tools
  MriWatcher
  )
AddToolMacro( MriWatcher )

# ===== NIRALUtilities ===================================================================
set( SourceCodeArgs
  SVN_REPOSITORY "https://www.nitrc.org/svn/niral_utilities/trunk"
  SVN_REVISION -r 38
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
)
set( Tools
  ImageMath
  CropDTI
  )
AddToolMacro( NIRALUtilities )

