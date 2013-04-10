cmake_minimum_required(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)

option(InnerBuildCMakeLists OFF) # Not even useful
mark_as_advanced(InnerBuildCMakeLists)

if(InnerBuildCMakeLists) # For the DTIAB external project

  include(${CMAKE_CURRENT_SOURCE_DIR}/DTIAtlasBuilder.cmake)

else(InnerBuildCMakeLists) # Default = Superbuild CMakeLists

  include(${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake)

endif(InnerBuildCMakeLists)
