cmake_minimum_required(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)

## Remove possible existing output folder already computed to rerun all tests
message("Removing possible existing test output folder")
file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlas)

## Test 1 : Usage
add_test(NAME TestUsage COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:DTIAtlasBuilder> --help) # Should just display the Usage : ${CMAKE_CURRENT_BINARY_DIR} = DTIAtlasBuilder-build/Testing -> need to ../
set(EXECUTABLE_OUTPUT_PATH ${TestingBINdirectory}) # So the test executables go to the right place

## ITKtest Tests
if(NOT WIN32 AND NOT APPLE) # DTIAtlasBuilder pipeline does not work on Win and Mac

  # Compile the program that will be run to : run pipeline and then compare output with reference

  set(BUILD_SHARED_LIBS OFF) # for the preprocessor variable "BUILD_SHARED_LIBS" on Windows

  if(ITK_VERSION_MAJOR EQUAL 3)
    add_executable(DTIAtlasBuilderTest DTIAtlasBuilderTestITKv${ITK_VERSION_MAJOR}.cxx) # This executable will run pipeline and then compare output with reference
    target_link_libraries(DTIAtlasBuilderTest DTIAtlasBuilderLib)
    set_target_properties(DTIAtlasBuilderTest PROPERTIES LABELS DTIAtlasBuilder)
    if(BUILD_SHARED_LIBS)
      set_target_properties(DTIAtlasBuilderTest PROPERTIES COMPILE_FLAGS "-DBUILD_SHARED_LIBS") # definition of the variable for c++ preprocessor
    endif(BUILD_SHARED_LIBS)
    set(NoGUITestCommand ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:DTIAtlasBuilderTest>) # Slicer_LAUNCH_COMMAND will launch the Slicer environment # TARGET_FILE so path not given directly = no need to be known
    set(NameOfMain ModuleEntryPoint)
  elseif(ITK_VERSION_MAJOR EQUAL 4) # BRAINS/BRAINSCommonLib/BuildScripts/CMakeBRAINS3BuildMacros.cmake : macro MakeTestDriverFromSEMTool
    set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "#include \"itkTestDriverBeforeTest.inc\"")
    set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "#include \"itkTestDriverAfterTest.inc\"")
    create_test_sourcelist(DTIAtlasBuilder DTIAtlasBuilderTestDriver.cxx DTIAtlasBuilderTestITKv${ITK_VERSION_MAJOR}.cxx # Creates DTIAtlasBuilderTestDriver.cxx (= test source) into bin/Testing
      EXTRA_INCLUDE itkTestDriverIncludeRequiredIOFactories.h
      FUNCTION  ProcessArgumentsAndRegisterRequiredFactories
      )
    add_executable(DTIAtlasBuilderTestDriver DTIAtlasBuilderTestDriver.cxx DTIAtlasBuilderTestITKv${ITK_VERSION_MAJOR}.cxx)
    target_link_libraries(DTIAtlasBuilderTestDriver DTIAtlasBuilderLib ${ITKTestKernel_LIBRARIES}) # ITKTestKernel_LIBRARIES only defined in ITKv4
    if(BUILD_SHARED_LIBS)
      set_target_properties(DTIAtlasBuilderTestDriver PROPERTIES COMPILE_FLAGS "-DBUILD_SHARED_LIBS") # definition of the variable for c++ preprocessor
    endif(BUILD_SHARED_LIBS)
    set(NoGUITestCommand ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:DTIAtlasBuilderTestDriver>) # Slicer_LAUNCH_COMMAND will launch the Slicer environment # TARGET_FILE so path not given directly = no need to be known
    set(NameOfMain DTIAtlasBuilderTestITKv4)
  endif()

  set(CompareParametersImageMathFA
    --compareIntensityTolerance 200 # pixel OK if in range [value-200;value+200] => FA between 0 and 10000 (ImageMath)
    --compareRadiusTolerance 1 # pixel OK if one of neighbors in the given radius is in the tolerance range
    --compareNumberOfPixelsTolerance 50 # Set the number of pixels that are allowed to fail
    )

  set(CompareParametersdtiprocessFA
    --compareIntensityTolerance 0.2 # pixel OK if in range [value-0.1;value+0.1] => FA between 0 and 1 (dtiprocess)
    --compareRadiusTolerance 1 # pixel OK if one of neighbors in the given radius is in the tolerance range
    --compareNumberOfPixelsTolerance 50 # Set the number of pixels that are allowed to fail
    )

  set(DTIABParameters
    --nogui # no need if ITKv4 -> already in DTIAtlasBuilderTest.cxx
    --parameter_file ${TestingBINdirectory}/TestGUIparametersBRAINS.txt
    --configuration_file ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilderSoftConfig.txt # Testing config file
  )

  ## Test 2 : Affine Atlas automatic QC
  # Test with ITKtest that will compare the 2 given images after pipeline has been ran
  add_test( NAME TestnoGUIAffineOutput COMMAND ${NoGUITestCommand}
    --compare # --compare is an cmd line argument from the itk test files 
      ${TestDataFolder}/TestAffineAtlasFA.nrrd # Ref : baseline
      ${TestingBINdirectory}/DTIAtlas/1_Affine_Registration/Loop0/Loop0_FAAverage.nrrd # Affine Output
    ${CompareParametersImageMathFA}
    ${NameOfMain} # After this line, give the parameters to use in DTIAtlasBuilder
      ${DTIABParameters}
    )


  ## Test 3 : Diffeomorphic Atlas automatic QC
  # will not run the pipeline again because already ran : will just compare the images
  add_test(NAME TestnoGUIDiffeomorphicOutput COMMAND ${NoGUITestCommand}
    --compare
      ${TestDataFolder}/TestDiffeomorphicAtlasFA.nrrd # Ref : baseline
      ${TestingBINdirectory}/DTIAtlas/3_Diffeomorphic_Atlas/DiffeomorphicAtlasFA.nrrd # Diffeomorphic Output
    ${CompareParametersdtiprocessFA}
    ${NameOfMain} # After this line, give the parameters to use in DTIAtlasBuilder
     ${DTIABParameters}
    )


  ## Test 4 : Final Atlas automatic QC
  # will not run the pipeline again because already ran : will just compare the images
  add_test(NAME TestnoGUIBRAINSFinalOutput COMMAND ${NoGUITestCommand}
    --compare
      ${TestDataFolder}/TestBRAINSFinalAtlasFA.nrrd # Ref : baseline
      ${TestingBINdirectory}/DTIAtlas/4_Final_Resampling/FinalAtlasFA.nrrd # Final Output
    ${CompareParametersdtiprocessFA}
    ${NameOfMain} # After this line, give the parameters to use in DTIAtlasBuilder
      ${DTIABParameters}
    )

  ## Test 5 : ANTS Final Atlas automatic QC
  # will run the pipeline again to compute with ANTS
  add_test(NAME TestnoGUIANTSFinalOutput COMMAND ${NoGUITestCommand}
    --compare
      ${TestDataFolder}/TestANTSFinalAtlasFA.nrrd # Ref : baseline
      ${TestingBINdirectory}/DTIAtlas/4_Final_Resampling/FinalAtlasFA.nrrd # Final Output with ANTS
    ${CompareParametersdtiprocessFA}
    ${NameOfMain} # After this line, give the parameters to use in DTIAtlasBuilder
      --nogui # no need if ITKv4 -> already in DTIAtlasBuilderTest.cxx
      --overwrite # Overwrite so it is computed with ANTS
      --parameter_file ${TestingBINdirectory}/TestGUIparametersANTS.txt
      --configuration_file ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilderSoftConfig.txt # Testing config file
    )

endif(NOT WIN32 AND NOT APPLE)

## Test 6 : Open the GUI for 10 sec and close it
add_executable(DTIAtlasBuilderGUITest DTIAtlasBuilderGUITest.cxx)
target_link_libraries(DTIAtlasBuilderGUITest DTIAtlasBuilderLib)
set_target_properties(DTIAtlasBuilderGUITest PROPERTIES LABELS DTIAtlasBuilder)
add_test(NAME TestGUI COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:DTIAtlasBuilderGUITest> ${CMAKE_CURRENT_BINARY_DIR}/bin/DTIAtlasBuilderSoftConfig.txt) # The config file will be read as argv[1] (no GenerateCLP)
# Slicer_LAUNCH_COMMAND will launch the Slicer environment # TARGET_FILE so path not given directly = no need to be known


## Test 7 : Open the GUI for 10 sec with a param file, check the content of the QLineEdit and close it
add_executable(DTIAtlasBuilderGUITestLabels DTIAtlasBuilderGUITestLabels.cxx)
target_link_libraries(DTIAtlasBuilderGUITestLabels DTIAtlasBuilderLib)
set_target_properties(DTIAtlasBuilderGUITestLabels PROPERTIES LABELS DTIAtlasBuilder)
add_test(NAME TestGUILabels COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:DTIAtlasBuilderGUITestLabels> ${TestingBINdirectory}/TestGUIparametersBRAINS.txt ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilderSoftConfig.txt ${TestingBINdirectory})
# The parameter file will be read as argv[1] (no GenerateCLP) and config file argv[2] # argv[3] is the folder to check to content of LineEdit


# Configure (Replace paths) and Copy needed files
configure_file( ${TestingSRCdirectory}/TestGUIparametersBRAINS.txt.in ${TestingBINdirectory}/TestGUIparametersBRAINS.txt )
configure_file( ${TestingSRCdirectory}/TestGUIparametersANTS.txt.in ${TestingBINdirectory}/TestGUIparametersANTS.txt )
configure_file( ${TestingSRCdirectory}/TestGUIparameters.csv.in ${TestingBINdirectory}/TestGUIparameters.csv )

