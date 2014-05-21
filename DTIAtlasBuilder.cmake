cmake_minimum_required(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)

#======================================================================================
# Generation of moc_GUI.cxx does not need all Slicer libs so do it first to avoid processing long cmd line with all libs

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

QT4_ADD_RESOURCES(RCC_SRCS DTIAtlasBuilder.qrc) # QResource for the icon
QT4_WRAP_CPP(QtProject_HEADERS_MOC GUI.h)
QT4_WRAP_UI(UI_FILES GUIwindow.ui)

#======================================================================================
# For Slicer Extension -> will create target "ExperimentalUpload" inside inner build. !! Needs to be before add testing
if(DTIAtlasBuilder_BUILD_SLICER_EXTENSION)
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
endif(DTIAtlasBuilder_BUILD_SLICER_EXTENSION)

#======================================================================================
# As the external project gives this CMakeLists the paths to the needed libraries (*_DIR), find_package will just use the existing *_DIR

if(NOT ITK_FOUND)
  find_package(ITK REQUIRED)
  include(${ITK_USE_FILE})
endif(NOT ITK_FOUND)

find_package(GenerateCLP REQUIRED)
include(${GenerateCLP_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

#======================================================================================
# Compile step for DTIAtlasBuilder
if(DTIAtlasBuilder_BUILD_SLICER_EXTENSION) # to configure GUI.cxx
  set(SlicerExtCXXVar "true")
else(DTIAtlasBuilder_BUILD_SLICER_EXTENSION)
  set(SlicerExtCXXVar "false")
endif(DTIAtlasBuilder_BUILD_SLICER_EXTENSION)

# Add the compilation date in xml file for it to appear in --help
if(WIN32)
  execute_process(COMMAND "cmd" " /C date /T" OUTPUT_VARIABLE TODAY)
  string(REGEX REPLACE "....(..)/(..)/(....).*" "\\1/\\2/\\3" TODAY ${TODAY}) # to remove the end of line and the name of day at the beginning
else() # Unix
  execute_process(COMMAND "date" "+%m/%d/%Y" OUTPUT_VARIABLE TODAY)
  string(REGEX REPLACE "(..)/(..)/(....).*" "\\1/\\2/\\3" TODAY ${TODAY}) # to remove the end of line
endif()

configure_file(DTIAtlasBuilder.xml.in ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilder.xml)
# xml info in GUI
file(READ ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilder.xml var)

string(REGEX MATCH "<version>.*</version>" ext "${var}")
string(REPLACE "<version>" "" version_number ${ext} )
string(REPLACE "</version>" "" version_number ${version_number})

ADD_DEFINITIONS(-DDTIAtlasBuilder_VERSION="${version_number}")


# Send python path to the program by configuring GUI.cxx: For testing, c++ program needs to know where Slicer's python is
if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
  # Python # Needed to use the python compiled with Slicer # "PYTHON_EXECUTABLE" given in SlicerConfig.cmake when find_package(Slicer REQUIRED)
  set(SlicerPythonExec ${PYTHON_EXECUTABLE})
  install(PROGRAMS ${SlicerPythonExec} DESTINATION ${NOCLI_INSTALL_DIR})
else( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
  set(SlicerPythonExec "")
endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

configure_file( GUI.cxx.in ${CMAKE_CURRENT_BINARY_DIR}/GUI.cxx ) # to set SlicerPythonExec

# DTIAtlasBuilder target
GENERATECLP(DTIABsources ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilder.xml) # include the GCLP file to the project
if( EXTENSION_SUPERBUILD_BINARY_DIR )
  if( WIN32 OR APPLE )
    add_executable( DTIAtlasBuilderLauncher Launcher.cxx ${DTIABsources} )
    install( TARGETS DTIAtlasBuilderLauncher DESTINATION ${INSTALL_DIR} )
    set( BUILD_LAUNCHER TRUE )
  endif()
endif()
list( APPEND DTIABsources DTIAtlasBuilder.cxx GUI.h ${CMAKE_CURRENT_BINARY_DIR}/GUI.cxx ScriptWriter.h ScriptWriter.cxx ${QtProject_HEADERS_MOC} ${UI_FILES} ${RCC_SRCS})
add_executable(DTIAtlasBuilder ${DTIABsources})  # add the files contained by "DTIABsources" to the project
set_target_properties(DTIAtlasBuilder PROPERTIES COMPILE_FLAGS "-DDTIAtlasBuilder_BUILD_SLICER_EXTENSION=${SlicerExtCXXVar}")# Add preprocessor definitions
target_link_libraries(DTIAtlasBuilder ${QT_LIBRARIES} ${ITK_LIBRARIES})
if( BUILD_LAUNCHER )
  install(TARGETS DTIAtlasBuilder DESTINATION ${INSTALL_DIR}/../hidden-cli-modules)
else()
  install(TARGETS DTIAtlasBuilder DESTINATION ${INSTALL_DIR}) # same if Slicer Ext or not
endif()

#===== Macro install tool ===============================================
macro( InstallToolMacro Proj CLI)
  
  if(COMPILE_EXTERNAL_${Proj})

    # Set Install directory
    if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
      if(${CLI})
        set(MacroInstallDir ${INSTALL_DIR})
      else(${CLI})
        set(MacroInstallDir ${NOCLI_INSTALL_DIR})
      endif(${CLI})
    else( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
       set(MacroInstallDir ${INSTALL_DIR})
    endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
    if( WIN32 )
      set( EXT .exe )
    endif()
    # Find the tools and install commands
    foreach( tool ${Tools} )
      install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${Proj}-install/bin/${tool}${EXT} DESTINATION ${MacroInstallDir})
    endforeach()

  endif(COMPILE_EXTERNAL_${Proj})

endmacro( InstallToolMacro )

## Install step for external projects: need to be here if SlicerExtension because make ExperimentalUpload done in inner build directory


#We build dtiprocess to be able to run the test for DTIAtlasBuilder, but we do not package it anymore as part of DTIAtlasBuilder as it can now directly be downloaded as a dependent extension
#set( Tools
#  dtiprocess
#  dtiaverage
#  )
#InstallToolMacro( dtiprocessTK ON) # proj CLI

set( Tools
  GreedyAtlas
  )
InstallToolMacro( AtlasWerks OFF)

set( Tools
  BRAINSFit
  BRAINSDemonWarp
  )
InstallToolMacro( BRAINS OFF)

set( Tools
  ANTS
  WarpImageMultiTransform
  WarpTensorImageMultiTransform
  )
InstallToolMacro( ANTS OFF)

set( Tools
  ResampleDTIlogEuclidean
  )
InstallToolMacro( ResampleDTI ON)

set( Tools
  DTI-Reg
  )
InstallToolMacro( DTIReg ON)

set( Tools
  ITKTransformTools
  )
InstallToolMacro( DTIReg OFF)

set( Tools
  unu
  )
InstallToolMacro( teem OFF)

set( Tools
  MriWatcher
  )
InstallToolMacro( MriWatcher OFF)

set( Tools
  ImageMath
  CropDTI
  )
InstallToolMacro( NIRALUtilities OFF)

#======================================================================================
# Testing for DTIAtlasBuilder
if(BUILD_TESTING)
  # Config file for testing (paths to the built softwares in build directory) = before installing
  if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION ) # if Slicer Ext, BRAINSFit and unu not recompiled so set to Slicer binary dirs in test config file
    set(BRAINSFITpathTestingConfigFile ${Slicer_HOME}/${Slicer_CLIMODULES_BIN_DIR}/BRAINSFit) # BRAINSFit built in cli modules dir # Slicer_CLIMODULES_BIN_DIR is a relative path
    set(unupathTestingConfigFile ${Teem_DIR}/bin/unu) # Teem_DIR set when find_package(Slicer) in SlicerConfig.cmake 
  else( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
    set(BRAINSFITpathTestingConfigFile ${CMAKE_CURRENT_BINARY_DIR}/BRAINS-install/bin/BRAINSFit)
    set(unupathTestingConfigFile ${CMAKE_CURRENT_BINARY_DIR}/teem-install/bin/unu)
  endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION ) 
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Testing/DTIAtlasBuilderSoftConfig.txt.in ${CMAKE_CURRENT_BINARY_DIR}/Testing/DTIAtlasBuilderSoftConfig.txt)

  set(TestingSRCdirectory ${CMAKE_CURRENT_SOURCE_DIR}/Testing)
  set(TestingBINdirectory ${CMAKE_CURRENT_BINARY_DIR}/Testing)
  set(TestDataFolder ${CMAKE_CURRENT_SOURCE_DIR}/Data/Testing)
  add_library(DTIAtlasBuilderLib STATIC ${DTIABsources}) # STATIC is also the default
  set_target_properties(DTIAtlasBuilderLib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint -DDTIAtlasBuilder_BUILD_SLICER_EXTENSION=${SlicerExtCXXVar}") # replace the main in DTIAtlasBuilder.cxx by the itkTest function ModuleEntryPoint
  target_link_libraries(DTIAtlasBuilderLib ${QT_LIBRARIES} ${ITK_LIBRARIES})
  set_target_properties(DTIAtlasBuilderLib PROPERTIES LABELS DTIAtlasBuilder)
  # Create Tests
  include(CTest)
  add_subdirectory( ${TestingSRCdirectory} ) # contains a CMakeLists.txt
#  include_directories( ${TestingSRCdirectory} ) # contains a CMakeLists.txt
endif()

# For Slicer Extension
if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
  if(NOT Slicer_SOURCE_DIR)
    set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
    include(${Slicer_EXTENSION_CPACK}) # So we can make package
  endif()
endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
