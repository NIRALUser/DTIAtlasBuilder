cmake_minimum_required(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)
#======================================================================================
# Generation of moc_GUI.cxx does not need all Slicer libs so do it first to avoid processing long cmd line with all libs

set(INSTALL_RUNTIME_DESTINATION bin)
set(INSTALL_LIBRARY_DESTINATION bin)
set(INSTALL_ARCHIVE_DESTINATION lib/static)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
  
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
  
  SET(INSTALL_RUNTIME_DESTINATION ${Slicer_INSTALL_CLIMODULES_BIN_DIR})
  SET(INSTALL_LIBRARY_DESTINATION ${Slicer_INSTALL_CLIMODULES_LIB_DIR})
  SET(INSTALL_ARCHIVE_DESTINATION ${Slicer_INSTALL_CLIMODULES_LIB_DIR})

  add_definitions(-DSlicer_CLIMODULES_BIN_DIR="${Slicer_CLIMODULES_BIN_DIR}")
else()
  add_definitions(-DSlicer_CLIMODULES_BIN_DIR="")
endif()

find_package(Qt5 COMPONENTS Core Widgets Gui)

if(Qt5_FOUND)
  find_package(Qt5 COMPONENTS Widgets REQUIRED)

  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  add_definitions(${Qt5Widgets_DEFINITIONS})

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
  set(QT_LIBRARIES ${Qt5Widgets_LIBRARIES})

  qt5_add_resources(RCC_SRCS DTIAtlasBuilder.qrc)
  qt5_wrap_cpp(QtProject_HEADERS_MOC GUI.h)
  qt5_wrap_ui(UI_FILES GUIwindow.ui)
else()
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})

  QT4_ADD_RESOURCES(RCC_SRCS DTIAtlasBuilder.qrc) # QResource for the icon
  QT4_WRAP_CPP(QtProject_HEADERS_MOC GUI.h)
  QT4_WRAP_UI(UI_FILES GUIwindow.ui)
endif()

#======================================================================================

#find_package(GenerateCLP REQUIRED)
#include(${GenerateCLP_USE_FILE})

#======================================================================================
# As the external project gives this CMakeLists the paths to the needed libraries (*_DIR), find_package will just use the existing *_DIR
set(ITK_IO_MODULES_USED 
ITKIOImageBase
ITKIONRRD
ITKIOBMP
ITKIOGIPL
ITKIOHDF5
ITKIOIPL
ITKIOJPEG
ITKIOLSM
ITKIOMRC
ITKIOMesh
ITKIOMeta
ITKIONIFTI
ITKIOPNG
ITKIORAW
ITKIOTIFF
ITKIOVTK
ITKIOGDCM
)

find_package(ITK COMPONENTS
  ITKCommon
  ITKIOImageBase
  ITKImageIntensity
  ITKTestKernel
  ${ITK_IO_MODULES_USED}
)
include(${ITK_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

#======================================================================================
# Compile step for DTIAtlasBuilder
if(DTIAtlasBuilder_BUILD_SLICER_EXTENSION) # to configure GUI.cxx
  set(SlicerExtCXXVar "true")
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
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

#configure_file(DTIAtlasBuilder.xml.in ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilder.xml)
# xml info in GUI
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/DTIAtlasBuilder.xml var)

string(REGEX MATCH "<version>.*</version>" ext "${var}")
string(REPLACE "<version>" "" version_number ${ext} )
string(REPLACE "</version>" "" version_number ${version_number})

ADD_DEFINITIONS(-DDTIAtlasBuilder_VERSION="${version_number}")

# DTIAtlasBuilder target
#GENERATECLP(DTIABsources ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilder.xml) # include the GCLP file to the project
#if( EXTENSION_SUPERBUILD_BINARY_DIR )
  #add_executable( DTIAtlasBuilderLauncher Launcher.cxx ${DTIABsources} )
  #install( TARGETS DTIAtlasBuilderLauncher DESTINATION bin )
#endif()
#list( APPEND DTIABsources DTIAtlasBuilder.cxx GUI.h GUI.cxx ScriptWriter.h ScriptWriter.cxx ${QtProject_HEADERS_MOC} ${UI_FILES} ${RCC_SRCS})
#add_executable(DTIAtlasBuilder ${DTIABsources})  # add the files contained by "DTIABsources" to the project
#target_link_libraries(DTIAtlasBuilder ${QT_LIBRARIES} ${ITK_LIBRARIES})
#install(TARGETS DTIAtlasBuilder DESTINATION bin)

set(DTIABsources GUI.h GUI.cxx ScriptWriter.h ScriptWriter.cxx ${QtProject_HEADERS_MOC} ${UI_FILES} ${RCC_SRCS})

#Find SlicerExecutionModel
FIND_PACKAGE(SlicerExecutionModel REQUIRED)
INCLUDE(${SlicerExecutionModel_USE_FILE})


SEMMacroBuildCLI(
    NAME DTIAtlasBuilder
    EXECUTABLE_ONLY
    ADDITIONAL_SRCS ${DTIABsources}
    TARGET_LIBRARIES ${QT_LIBRARIES} ${ITK_LIBRARIES}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    INSTALL_RUNTIME_DESTINATION ${INSTALL_RUNTIME_DESTINATION}
    INSTALL_LIBRARY_DESTINATION ${INSTALL_LIBRARY_DESTINATION}
    INSTALL_ARCHIVE_DESTINATION ${INSTALL_ARCHIVE_DESTINATION}
    )
  export(TARGETS ${LOCAL_NAME} APPEND FILE ${CMAKE_BINARY_DIR}/${PRIMARY_PROJECT_NAME}-exports.cmake)

set_target_properties(DTIAtlasBuilder PROPERTIES COMPILE_FLAGS "-DDTIAtlasBuilder_BUILD_SLICER_EXTENSION=${SlicerExtCXXVar}")# Add preprocessor definitions




#======================================================================================
# Testing for DTIAtlasBuilder
if(BUILD_TESTING)
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Testing/DTIAtlasBuilderSoftConfig.txt.in ${CMAKE_CURRENT_BINARY_DIR}/Testing/DTIAtlasBuilderSoftConfig.txt)
  set(TestingSRCdirectory ${CMAKE_CURRENT_SOURCE_DIR}/Testing)
  set(TestingBINdirectory ${CMAKE_CURRENT_BINARY_DIR}/Testing)
  set(TestDataFolder ${CMAKE_CURRENT_SOURCE_DIR}/Data/Testing)
  #add_library(DTIAtlasBuilderLib STATIC ${DTIABsources}) # STATIC is also the default
  set_target_properties(DTIAtlasBuilderLib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint -DDTIAtlasBuilder_BUILD_SLICER_EXTENSION=${SlicerExtCXXVar}") # replace the main in DTIAtlasBuilder.cxx by the itkTest function ModuleEntryPoint
  target_link_libraries(DTIAtlasBuilderLib ${QT_LIBRARIES} ${ITK_LIBRARIES})
  set_target_properties(DTIAtlasBuilderLib PROPERTIES LABELS DTIAtlasBuilder)
  # Create Tests
  include(CTest)
  add_subdirectory( ${TestingSRCdirectory} ) # contains a CMakeLists.txt
#  include_directories( ${TestingSRCdirectory} ) # contains a CMakeLists.txt
endif()

### ~Install default configuration file

find_program(DTI-RegPath DTI-Reg PATHS ${DTI-Reg_BINARY_DIR})
find_program(dtiaveragePath dtiaverage PATHS ${DTIProcess_BINARY_DIR})
find_program(dtiprocessPath dtiprocess PATHS ${DTIProcess_BINARY_DIR})
find_program(GreedyAtlasPath GreedyAtlas PATHS ${AtlasWerks_BINARY_DIR})
find_program(GreedyWarp GreedyWarp PATHS ${AtlasWerks_BINARY_DIR})
find_program(unuPath unu PATHS ${teem_BINARY_DIR})
find_program(CropDTIPath CropDTI PATHS ${niral_utilities_BINARY_DIR})
find_program(ImageMathPath ImageMath PATHS ${niral_utilities_BINARY_DIR})
find_program(BRAINSFitPath BRAINSFit PATHS ${BRAINSTools_BINARY_DIR})
find_program(ResampleDTIlogEuclideanPath ResampleDTIlogEuclidean PATHS ${ResampleDTIlogEuclidean_BINARY_DIR})
find_program(MriWatcherPath MriWatcher PATHS ${MriWatcher_BINARY_DIR})

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/DTIAtlasBuilderSoftConfig.txt.in ${CMAKE_INSTALL_PREFIX}/bin/DTIAtlasBuilderSoftConfig.txt)

## INSTALL TOOLS

# set(TOOL_LIST
#     ${DTI-Reg_DIR}/bin/DTI-Reg
#     ${DTIProcess_DIR}/bin/dtiprocess
#     ${DTIProcess_DIR}/bin/dtiaverage
#     ${niral_utilities_DIR}/../../../bin/CropDTI
#     ${niral_utilities_DIR}/../../../bin/ImageMath
#     ${teem_DIR}/../bin/unu
#     ${AtlasWerks_DIR}/bin/GreedyAtlas
#     ${AtlasWerks_DIR}/bin/GreedyWarp
#     ${MriWatcher_DIR}/MriWatcher 
#     ${BRAINSTools_DIR}/bin/BRAINSFit
#     ${ResampleDTIlogEuclidean_DIR}/../ResampleDTIlogEuclidean-install/bin/ResampleDTIlogEuclidean
#   )

# foreach(file ${TOOL_LIST})
#     install(PROGRAMS ${file}
#       DESTINATION ${INSTALL_RUNTIME_DESTINATION}
#       COMPONENT RUNTIME)
# endforeach()

if(AtlasWerks_DIR)
  if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
    install(PROGRAMS ${AtlasWerks_DIR}/bin/GreedyAtlas
      DESTINATION ${INSTALL_RUNTIME_DESTINATION}/../ExternalBin
      COMPONENT RUNTIME)

    install(PROGRAMS ${AtlasWerks_DIR}/bin/GreedyWarp
      DESTINATION ${INSTALL_RUNTIME_DESTINATION}/../ExternalBin
      COMPONENT RUNTIME)
  else()
    install(PROGRAMS ${AtlasWerks_DIR}/bin/GreedyAtlas
    DESTINATION ${INSTALL_RUNTIME_DESTINATION}
    COMPONENT RUNTIME)

    install(PROGRAMS ${AtlasWerks_DIR}/bin/GreedyWarp
      DESTINATION ${INSTALL_RUNTIME_DESTINATION}
      COMPONENT RUNTIME)
  endif()
endif()

if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
  include(${Slicer_EXTENSION_CPACK})
endif()