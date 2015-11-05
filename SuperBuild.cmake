cmake_minimum_required(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)

set(BUILD_TESTING ON CACHE BOOL "Build, configure and copy testing files")

project(DTIAtlasBuilder)
set(innerproj ${CMAKE_PROJECT_NAME}-inner)

# So error while configuring and not building if Git missing -> sets GIT_EXECUTABLE
find_package(Git REQUIRED)

option(USE_GIT_PROTOCOL "If behind a firewall turn this off to use http instead." ON)
if(NOT USE_GIT_PROTOCOL)
  set(git_protocol "http")
else(NOT USE_GIT_PROTOCOL)
  set(git_protocol "git")
endif()

# Sets Subversion_SVN_EXECUTABLE
find_package(Subversion REQUIRED)

# External Projects
include(ExternalProject) # "ExternalProject" is the module that will allow to compile tools
include(CMakeDependentOption)

if(CMAKE_EXTRA_GENERATOR) # CMake Generator = make, nmake..
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()
set(COMMON_BUILD_OPTIONS_FOR_EXTERNALPACKAGES
  -DMAKECOMMAND:STRING=${MAKECOMMAND}
  -DCMAKE_SKIP_RPATH:BOOL=${CMAKE_SKIP_RPATH}
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  -DCMAKE_CXX_COMPILER:PATH=${CMAKE_CXX_COMPILER}
  -DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE}
  -DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}
  -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
  -DCMAKE_C_COMPILER:PATH=${CMAKE_C_COMPILER}
  -DCMAKE_C_FLAGS_RELEASE:STRING=${CMAKE_C_FLAGS_RELEASE}
  -DCMAKE_C_FLAGS_DEBUG:STRING=${CMAKE_C_FLAGS_DEBUG}
  -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
  -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
  -DCMAKE_LINKER_FLAGS:STRING=${CMAKE_LINKER_FLAGS}
  -DCMAKE_MODULE_LINKER_FLAGS:STRING=${CMAKE_MODULE_LINKER_FLAGS}
  -DCMAKE_GENERATOR:STRING=${CMAKE_GENERATOR}
  -DCMAKE_EXTRA_GENERATOR:STRING=${CMAKE_EXTRA_GENERATOR}
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  -DCMAKE_BUNDLE_OUTPUT_DIRECTORY:PATH=${CMAKE_BUNDLE_OUTPUT_DIRECTORY}
  -DCTEST_NEW_FORMAT:BOOL=${CTEST_NEW_FORMAT}
  -DMEMORYCHECK_COMMAND_OPTIONS:STRING=${MEMORYCHECK_COMMAND_OPTIONS}
  -DMEMORYCHECK_COMMAND:PATH=${MEMORYCHECK_COMMAND}
  -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
  -DCMAKE_MODULE_LINKER_FLAGS:STRING=${CMAKE_MODULE_LINKER_FLAGS}
  -DGIT_EXECUTABLE:PATH=${GIT_EXECUTABLE} # needed when does include(Slicer) for Slicer ext
  -DSubversion_SVN_EXECUTABLE:FILEPATH=${Subversion_SVN_EXECUTABLE}# needed when does include(Slicer) for Slicer ext
)

#===================================================================================
# Slicer Extension
option( DTIAtlasBuilder_BUILD_SLICER_EXTENSION "Build DTIAtlasBuilder as a Slicer extension" OFF )
if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
  #VTK_VERSION_MAJOR is define but not a CACHE variable
  set( VTK_VERSION_MAJOR ${VTK_VERSION_MAJOR} CACHE STRING "Choose the expected VTK major version to build Slicer (5 or 6).")
  set(EXTENSION_NAME DTIAtlasBuilder)
  set(EXTENSION_HOMEPAGE "http://www.nitrc.org/projects/dtiatlasbuilder")
  set(EXTENSION_CATEGORY "Diffusion")
  set(EXTENSION_CONTRIBUTORS "Adrien Kaiser (UNC), Francois Budin (UNC)")
  set(EXTENSION_DESCRIPTION "A tool to create a DTI Atlas Image from a set of DTI Images")
  set(EXTENSION_ICONURL "http://www.nitrc.org/project/screenshot.php?group_id=636&screenshot_id=607")
  set(EXTENSION_SCREENSHOTURLS "http://www.slicer.org/slicerWiki/images/0/02/DTIAtlasBuilder_Interface.png")
  set(EXTENSION_STATUS Beta)
  set(EXTENSION_BUILD_SUBDIRECTORY . )
  set(EXTENSION_DEPENDS "DTIProcess ResampleDTIlogEuclidean DTI-Reg" ) # Specified as a space separated list or 'NA' if any
  set(MODULE_NAME DTIAtlasBuilder)

  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
  # Import DTIProcess, ResampleDTIlogEuclidean, and DTI-Reg targets for the tests
  # DTIProcess_DIR and DTI-Reg_DIR are set because DTIAtlasBuilder is defined as dependent of the extension DTIProcess, ResampleDTIlogEuclidean, and DTI-Reg
  include( ${DTIProcess_DIR}/ImportDTIProcessExtensionExecutables.cmake )
  get_target_property(TOOLdtiprocess dtiprocess IMPORTED_LOCATION_RELEASE )
  get_target_property(TOOLdtiaverage dtiaverage IMPORTED_LOCATION_RELEASE )
  include( ${DTI-Reg_DIR}/ImportDTI-RegExtensionExecutables.cmake )
  get_target_property(TOOLDTI-Reg DTI-Reg IMPORTED_LOCATION_RELEASE )
  include( ${ResampleDTIlogEuclidean_DIR}/ResampleDTIlogEuclidean-exports.cmake )
  get_target_property(TOOLResampleDTIlogEuclidean ResampleDTIlogEuclidean IMPORTED_LOCATION_RELEASE )
  #ANTS needs a more recent version of ITK (4.6) than the one currently in Slicer (05.21.2014)
  #We still use Slicer ITK_DIR to compile DTIAtlasBuilder to avoid conflicts when including Slicer_USE_FILE
  #set( ITK_DIR_Slicer ${ITK_DIR} )
  #set( GenerateCLP_DIR_Slicer ${GenerateCLP_DIR} )
  #unset( ITK_DIR CACHE )
  #unset( ITK_FOUND )
  #unset( SlicerExecutionModel_DIR CACHE )
  #unset( GenerateCLP_DIR CACHE )
  # SlicerExecutionModel_DEFAULT_CLI_RUNTIME_OUTPUT_DIRECTORY and SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION defined in Slicer_USE_FILE
  # SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION is [sthg]/cli_module : must contain only CLIs
  # If build as Slicer Extension, CMAKE_INSTALL_PREFIX is set to [ExtensionsFolder]/DTIAtlaBuilder
  if( APPLE )
    set( CMAKE_EXE_LINKER_FLAGS -Wl,-rpath,@loader_path/../../../../../ )
  endif()
  set( BUILD_SHARED_LIBS OFF CACHE BOOL "Builds shared libs")
  set(TOOLBRAINSFit ${Slicer_HOME}/${Slicer_CLIMODULES_BIN_DIR}/BRAINSFit) # BRAINSFit built in cli modules dir # Slicer_CLIMODULES_BIN_DIR is a relative path
  set(TOOLunu ${Teem_DIR}/bin/unu) # Teem_DIR set when find_package(Slicer) in SlicerConfig.cmake 
  set( TOOLDTIAtlasBuilderLauncher ${CMAKE_CURRENT_BINARY_DIR}/${innerproj}-install/bin/DTIAtlasBuilderLauncher) # Build by this project if DTIAtlasBuilder is an extension
else( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

  option( FORCE_BUILD_ON_MAC_OR_WIN "Force Building on Known failing platforms" OFF )
  if(NOT FORCE_BUILD_ON_MAC_OR_WIN AND ( APPLE OR WIN32 ) ) # If not Slicer ext, not compile because will fail at run time
    message(FATAL_ERROR "DTIAtlasBuilder has known issues and will not run on Mac or Windows\nSet -DFORCE_BUILD_ON_MAC_OR_WIN:BOOL=ON to override")
  endif()

endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

set( COMPILE_PACKAGE ON CACHE BOOL "Compiles all the external projects and tools" )

list( APPEND COMMON_BUILD_OPTIONS_FOR_EXTERNALPACKAGES
  -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
  -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
)


#===================================================================================
# Search needed libraries and packages for DTIAtlasBuilder : ITK_DIR GenerateCLP_DIR ModuleDescriptionParser_DIR TCLAP_DIR QT_QMAKE_EXECUTABLE
if(NOT COMPILE_PACKAGE OR DTIAtlasBuilder_BUILD_SLICER_EXTENSION ) # ITK and SlicerExecutionModel (GenerateCLP) are recompiled in the package
  find_package(SlicerExecutionModel REQUIRED)
  include(${SlicerExecutionModel_USE_FILE})
  include(${GenerateCLP_USE_FILE})

  find_package(ITK REQUIRED)
  include(${ITK_USE_FILE})
endif()

find_package(Qt4 REQUIRED) # For DTIAtlasBuilder
include(${QT_USE_FILE}) # creates QT_QMAKE_EXECUTABLE


#======================================================================================
# Compile package
set( ExtProjList # External packages to compile
  dtiprocessTK # dtiprocess, dtiaverage
  AtlasWerks # GreedyAtlas
  BRAINS # BRAINSFit, BRAINSDemonWarp
  ANTS # ANTS, WarpImageMultiTransform, WarpTensorImageMultiTransform
  ResampleDTI # ResampleDTIlogEuclidean
  DTI-Reg # DTI-Reg
  teem # unu
  MriWatcher # MriWatcher
  NIRALUtilities # ImageMath, CropDTI
  )
set( ToolsList # Needed tools -> to hide unuseful TOOL* variables
  dtiprocess
  dtiaverage
  GreedyAtlas
  BRAINSFit
  BRAINSDemonWarp
  ANTS
  WarpImageMultiTransform
  ResampleDTIlogEuclidean
  DTI-Reg
  MriWatcher
  ImageMath
  CropDTI
  unu
  ITKTransformTools
  )
if(COMPILE_PACKAGE)

  # Define COMPILE_EXTERNAL_* variables
  if( NOT DTIAtlasBuilder_BUILD_SLICER_EXTENSION ) # no Slicer extension -> recompile all

    foreach( tool ${ExtProjList})
      set( COMPILE_EXTERNAL_${tool} ON CACHE BOOL "Compile external ${tool}" )
      mark_as_advanced(CLEAR COMPILE_EXTERNAL_${tool}) # Show variable if been hidden
    endforeach()

  else() # Slicer extension -> recompile only tools that are not in Slicer + not MriWatcher

   foreach( tool DTI-Reg ANTS ResampleDTI dtiprocessTK BRAINS teem MriWatcher ) # Already in Slicer or independent extension -> not recompiled # MriWatcher needs GLUT so disable if Slicer Extension because glut not necesseraly installed
      set( COMPILE_EXTERNAL_${tool} OFF CACHE BOOL "Compile external ${tool}" )
      mark_as_advanced(CLEAR COMPILE_EXTERNAL_${tool}) # Show variable if been hidden
    endforeach()

    foreach( tool AtlasWerks NIRALUtilities ) # Not in Slicer -> recompile
      set( COMPILE_EXTERNAL_${tool} ON CACHE BOOL "Compile external ${tool}" )
      mark_as_advanced(CLEAR COMPILE_EXTERNAL_${tool}) # Show variable if been hidden
    endforeach()

    if(APPLE) # unu is not recompiled with Slicer on MacOS
#      set( COMPILE_EXTERNAL_teem ON CACHE BOOL "Compile external teem" FORCE)
    endif(APPLE)

    if(WIN32 OR APPLE) # DTIAB not working on Windows/Mac so only compile DTIAtlasBuilder's GUI
      set( COMPILE_EXTERNAL_AtlasWerks OFF CACHE BOOL "Compile external AtlasWerks" FORCE)
      set( COMPILE_EXTERNAL_NIRALUtilities OFF CACHE BOOL "Compile external NIRALUtilities" FORCE)
    endif()

  endif()

  # File containing add_external for all tools
  include( ${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild/FindExternalTools.cmake ) # Go execute the given cmake script, and get back into this script when done

else(COMPILE_PACKAGE) # Hide unuseful variables
  foreach( proj ${ExtProjList})
    set( COMPILE_EXTERNAL_${proj} OFF CACHE BOOL "Compile external ${proj}" FORCE ) # For installation step in DTIAtlasBuilder.cmake
    mark_as_advanced(FORCE COMPILE_EXTERNAL_${proj})
  endforeach()
  foreach( tool ${ToolsList})
    mark_as_advanced(FORCE TOOL${tool})
    mark_as_advanced(FORCE TOOL${tool}Sys)
  endforeach()
  foreach( lib VTK SlicerExecutionModel )
    mark_as_advanced(FORCE ${lib}_DIR)
  endforeach()
endif(COMPILE_PACKAGE)

#======================================================================================

ExternalProject_Add(${innerproj} # DTIAtlasBuilder added as Externalproject in case of SlicerExecutionModel recompiled because needs it
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${innerproj}-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DInnerBuildCMakeLists:BOOL=ON
    ${COMMON_BUILD_OPTIONS_FOR_EXTERNALPACKAGES}
    -DUSE_GIT_PROTOCOL:BOOL=${USE_GIT_PROTOCOL}
    -DITK_DIR:PATH=${ITK_DIR}
    -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
    -DQT_QMAKE_EXECUTABLE:PATH=${QT_QMAKE_EXECUTABLE}
    -DBUILD_TESTING:BOOL=${BUILD_TESTING}
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/${innerproj}-install
    # Installation step
    # Slicer extension
    -DDTIAtlasBuilder_BUILD_SLICER_EXTENSION:BOOL=${DTIAtlasBuilder_BUILD_SLICER_EXTENSION}
    -DSlicer_DIR:PATH=${Slicer_DIR}
    -DEXTENSION_NAME:STRING=${EXTENSION_NAME}
    # For the tests
    -DTOOLImageMath:PATH=${TOOLImageMath}
    -DTOOLResampleDTIlogEuclidean:PATH=${TOOLResampleDTIlogEuclidean}
    -DTOOLCropDTI:PATH=${TOOLCropDTI}
    -DTOOLdtiprocess:PATH=${TOOLdtiprocess}
    -DTOOLBRAINSFit:PATH=${TOOLBRAINSFit}
    -DTOOLGreedyAtlas:PATH=${TOOLGreedyAtlas}
    -DTOOLdtiaverage:PATH=${TOOLdtiaverage}
    -DTOOLDTI-Reg:PATH=${TOOLDTI-Reg}
    -DTOOLunu:PATH=${TOOLunu}
    -DTOOLMriWatcher:PATH=${TOOLMriWatcher}
  DEPENDS ${ITK_DEPEND} ${DTIAtlasBuilderExternalToolsDependencies} # DTIAtlasBuilderExternalToolsDependencies contains the names of all the recompiled softwares so DTIAB is compiled last (for install)
)
set( TOOLDTIAtlasBuilder ${CMAKE_CURRENT_BINARY_DIR}/${innerproj}-install/bin/DTIAtlasBuilder )

if(BUILD_TESTING)
  include(CTest)
  if( WIN32 )
    set( EXT .exe )
  endif()
  set(TestingSRCdirectory ${CMAKE_CURRENT_SOURCE_DIR}/Testing)
  set(TestingBINdirectory ${CMAKE_CURRENT_BINARY_DIR}/Testing)
  set(TestDataFolder ${CMAKE_CURRENT_SOURCE_DIR}/Data/Testing)
  set( PACKAGING_TESTS DTIAtlasBuilderGUITestLabels DTIAtlasBuilderTest DTIAtlasBuilderGUITest DTIAtlasBuilder)
  foreach( VAR ${PACKAGING_TESTS})
    add_executable(${VAR} IMPORTED)
    set_property(TARGET ${VAR} PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${innerproj}-install/bin/${VAR}${EXT})
  endforeach()
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Testing/DTIAtlasBuilderSoftConfig.txt.in ${CMAKE_CURRENT_BINARY_DIR}/Testing/DTIAtlasBuilderSoftConfig.txt)
  add_subdirectory(Testing)
endif()

set( PACKAGING_HIDDEN_CLI DTIAtlasBuilder)
if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
  set( PACKAGING_CLI DTIAtlasBuilderLauncher)
  set( PACKAGING_NO_CLI CropDTI ImageMath GreedyAtlas )
  set( CLI_FOLDER ${SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION})
  set( HIDDEN_CLI_FOLDER ${SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION}/../hidden-cli-modules )
  set( NO_CLI_FOLDER ${SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION}/../ExternalBin )
else()
  set( PACKAGING_NO_CLI CropDTI MriWatcher unu ImageMath GreedyAtlas ANTS WarpImageMultiTransform ITKTransformTools)
  set( PACKAGING_CLI ResampleDTIlogEuclidean dtiprocess BRAINSFit BRAINSDemonWarp dtiaverage DTI-Reg)
  set( CLI_FOLDER bin)
  set( HIDDEN_CLI_FOLDER ${CLI_FOLDER} )
  set( NO_CLI_FOLDER ${CLI_FOLDER} )
  # Update the paths to the program in the configuration file, and copy it to the executable directory
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/DTIAtlasBuilderSoftConfig.txt.in ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilderSoftConfig.txt ) # configure and copy with (tool)Path
endif()

foreach(VAR ${PACKAGING_CLI})
  if( TOOL${VAR} )
    install(PROGRAMS ${TOOL${VAR}} DESTINATION ${CLI_FOLDER})
  endif()
endforeach()
foreach(VAR ${PACKAGING_HIDDEN_CLI})
  if( TOOL${VAR} )
    install(PROGRAMS ${TOOL${VAR}} DESTINATION ${HIDDEN_CLI_FOLDER} )
  endif()
endforeach()
foreach(VAR ${PACKAGING_NO_CLI})
  if( TOOL${VAR} )
    install(PROGRAMS ${TOOL${VAR}} DESTINATION ${NO_CLI_FOLDER} )
  endif()
endforeach()

if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
  include(${Slicer_EXTENSION_CPACK})
endif()

