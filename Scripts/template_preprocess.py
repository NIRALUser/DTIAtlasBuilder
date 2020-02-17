#!/usr/bin/python

import os # To run a shell command : os.system("[shell command]")
import sys # to return an exit code
import shutil # to remove a non empty directory
import json

configPath=os.path.join(os.path.dirname(os.path.abspath(__file__)),"config.json")
config={}
with open(configPath,'r') as f:
  config=json.load(f)

PIDlogFile = config['m_OutputPath']+"/DTIAtlas/Script/PID.log"
PIDfile = open( PIDlogFile, 'a') # open in Append mode
PIDfile.write( str(os.getpid()) + "\n" )
PIDfile.close()

print("\n============ Pre processing =============")

# Files Paths
allcases = ["/work/dtiatlasbuilder/Data/Testing/ImageTest1.nrrd", "/work/dtiatlasbuilder/Data/Testing/ImageTest2.nrrd", "/work/dtiatlasbuilder/Data/Testing/ImageTest3.nrrd"]

allcasesIDs = ["ImageTest1", "ImageTest2", "ImageTest3"]

OutputPath= "/work/DTIAtlas/1_Affine_Registration"
AtlasScalarMeasurementref= OutputPath + "/ImageTest1_FA.nrrd" #the reference will be the first case for the first loop

def DisplayErrorAndQuit ( Error ):
  print '\n\nERROR DETECTED IN WORKFLOW:',Error
  print 'ABORT'
  sys.exit(1)

# Function that checks if file exist and replace old names by new names if needed
def CheckFileExists ( File, case, caseID ) : # returns 1 if file exists or has been renamed and 0 if not
  if os.path.isfile( File ) : # file exists
    return 1
  else : # file does not exist: check if older version of file can exist (if file name has been changed)
    NamesThatHaveChanged = ["MeanImage", "DiffeomorphicDTI", "DiffeomorphicAtlasDTI", "HField", "GlobalDisplacementField"] # latest versions of the names
    if any( changedname in File for changedname in NamesThatHaveChanged ) : # if name has been changed, check if older version files exist
      if "MeanImage" in File :
        OldFile = File.replace("Mean", "Average")
        if os.path.isfile( OldFile ) : # old file exists: rename and return 1
          os.rename(OldFile, File)
          return 1
        else:
          return 0
      if "DiffeomorphicDTI" in File :
        OldFile = File.replace( caseID, "Case" + str(case+1) ).replace("DiffeomorphicDTI", "AWDTI")
        if os.path.isfile( OldFile ) : # old file exists: rename and return 1
          os.rename(OldFile, File)
          os.rename(OldFile.replace("AWDTI","AWFA"), File.replace("DiffeomorphicDTI","DiffeomorphicFA"))
          os.rename(OldFile.replace("AWDTI","AWDTI_float"), File.replace("DiffeomorphicDTI","DiffeomorphicDTI_float"))
          return 1
        else : # test other old name
          OldFile = File.replace( caseID, "Case" + str(case+1) )
          if os.path.isfile( OldFile ) : # old file exists: rename and return 1
            os.rename(OldFile, File)
            os.rename(OldFile.replace("DiffeomorphicDTI","DiffeomorphicFA"), File.replace("DiffeomorphicDTI","DiffeomorphicFA"))
            os.rename(OldFile.replace("DiffeomorphicDTI","DiffeomorphicDTI_float"), File.replace("DiffeomorphicDTI","DiffeomorphicDTI_float"))
            return 1
          else:
            return 0
      if "DiffeomorphicAtlasDTI" in File :
        OldFile = File.replace("DiffeomorphicAtlasDTI", "AWAtlasDTI")
        if os.path.isfile( OldFile ) : # old file exists: rename and return 1
          os.rename(OldFile, File)
          os.rename(OldFile.replace("AWAtlasDTI","AWAtlasFA"), File.replace("DiffeomorphicAtlasDTI","DiffeomorphicAtlasFA"))
          os.rename(OldFile.replace("AWAtlasDTI","AWAtlasDTI_float"), File.replace("DiffeomorphicAtlasDTI","DiffeomorphicAtlasDTI_float"))
          return 1
        else:
          return 0
      if "HField" in File :
        OldFile = File.replace( caseID, "Case" + str(case+1) ).replace("H", "Deformation")
        if os.path.isfile( OldFile ) : # old file exists: rename and return 1
          os.rename(OldFile, File)
          return 1
        else : # test other old name
          OldFile = File.replace( caseID, "Case" + str(case+1) )
          if os.path.isfile( OldFile ) : # old file exists: rename and return 1
            os.rename(OldFile, File)
            return 1
          else:
            return 0
      if "GlobalDisplacementField" in File :
        OldFile = File.replace( caseID, "Case" + str(case+1) ).replace("Displacement", "Deformation")
        if os.path.isfile( OldFile ) : # old file exists: rename and return 1
          os.rename(OldFile, File)
          return 1
        else : # test other old name
          OldFile = File.replace( caseID, "Case" + str(case+1) )
          if os.path.isfile( OldFile ) : # old file exists: rename and return 1
            os.rename(OldFile, File)
            return 1
          else:
            return 0
    else: # file does not exist and name has not been changed: check if the caseX version exists
      if caseID : # CaseID is empty for averages
        OldFile = File.replace( caseID, "Case" + str(case+1) )
        if os.path.isfile( OldFile ) : # old file exists: rename and return 1
          os.rename(OldFile, File)
          return 1
        else:
          return 0
      else: # for averages
        return 0


# Create directory for temporary files
if not os.path.isdir(OutputPath):
  os.mkdir(OutputPath)
  print("\n=> Creation of the affine directory = " + OutputPath)

print("")
# Creating template by processing Case 1 DTI

# Filter case 1 DTI
FilteredDTI= OutputPath + "/ImageTest1_filteredDTI.nrrd"
FilterDTICommand= "/work/dtiatlasbuilder-build/ResampleDTIlogEuclidean-install/bin/ResampleDTIlogEuclidean " + allcases[0] + " " + FilteredDTI + " --correction zero"
print("[ImageTest1] [Filter DTI] => $ " + FilterDTICommand)
if 1 :
  if os.system(FilterDTICommand)!=0 : DisplayErrorAndQuit('[ImageTest1] ResampleDTIlogEuclidean: Filter DTI to remove negative values')
else : print("=> The file \'" + FilteredDTI + "\' already exists so the command will not be executed")

# Generating case 1 FA
DTI= allcases[0]
ScalarMeasurement= OutputPath + "/ImageTest1_FA.nrrd"
GeneScalarMeasurementCommand= "/work/dtiatlasbuilder-build/DTIProcess-install/bin/dtiprocess --dti_image " + DTI + " -f " + ScalarMeasurement
print("[ImageTest1] [Generating FA] => $ " + GeneScalarMeasurementCommand)
if 1 :
  if os.system(GeneScalarMeasurementCommand)!=0 : DisplayErrorAndQuit('[ImageTest1] dtiprocess: Generating FA of DTI image')
else : print("=> The file \'" + ScalarMeasurement + "\' already exists so the command will not be executed")

print("")

# Affine Registration and Normalization Loop
n = 0
while n <= 1 :
  if not os.path.isdir(OutputPath + "/Loop" + str(n)):
    print("\n=> Creation of the Output directory for Loop " + str(n) + " = " + OutputPath + "/Loop" + str(n) + "\n")
    os.mkdir(OutputPath + "/Loop" + str(n))

  # Cases Loop
  case = (n==0) # (n==0) -> bool: =1(true) =0(false) : the first case is the reference for the first loop so it will not be normalized or registered (it is cropped and FAed before the loop)
  while case < len(allcases):

    if n==0: # Filtering and Cropping DTI and Generating FA are only part of the first loop
# Filter DTI
      # ResampleDTIlogEuclidean does by default a correction of tensor values by setting the negative values to zero
      FilteredDTI= OutputPath + "/" + allcasesIDs[case] + "_filteredDTI.nrrd"
      FilterDTICommand= "/work/dtiatlasbuilder-build/ResampleDTIlogEuclidean-install/bin/ResampleDTIlogEuclidean " + allcases[case] + " " + FilteredDTI + " --correction zero"
      print("[" + allcasesIDs[case] + "] [Filter DTI] => $ " + FilterDTICommand)
      if 1 :
        if os.system(FilterDTICommand)!=0 : DisplayErrorAndQuit('[' + allcasesIDs[case] + '] ResampleDTIlogEuclidean: Filter DTI to remove negative values')
      else : print("=> The file \'" + FilteredDTI + "\' already exists so the command will not be executed")

# Generating FA
      DTI= allcases[case]
      ScalarMeasurement= OutputPath + "/" + allcasesIDs[case] + "_FA.nrrd"
      GeneScalarMeasurementCommand= "/work/dtiatlasbuilder-build/DTIProcess-install/bin/dtiprocess --dti_image " + DTI + " -f " + ScalarMeasurement
      print("[" + allcasesIDs[case] + "] [Generating FA] => $ " + GeneScalarMeasurementCommand)
      if 1 :
        if os.system(GeneScalarMeasurementCommand)!=0 : DisplayErrorAndQuit('[' + allcasesIDs[case] + '] dtiprocess: Generating FA of DTI image')
      else : print("=> The file \'" + ScalarMeasurement + "\' already exists so the command will not be executed")

# Normalization
    ScalarMeasurement= OutputPath + "/" + allcasesIDs[case] + "_FA.nrrd"
    NormScalarMeasurement= OutputPath + "/Loop" + str(n) + "/" + allcasesIDs[case] + "_Loop" + str(n) + "_NormFA.nrrd"
    NormScalarMeasurementCommand= "/work/dtiatlasbuilder-build/niral_utilities-install/bin/ImageMath " + ScalarMeasurement + " -outfile " + NormScalarMeasurement + " -matchHistogram " + AtlasScalarMeasurementref
    print("[LOOP " + str(n) + "/1] [" + allcasesIDs[case] + "] [Normalization] => $ " + NormScalarMeasurementCommand)
    if 1 :
      if os.system(NormScalarMeasurementCommand)!=0 : DisplayErrorAndQuit('[' + allcasesIDs[case] + '] ImageMath: Normalizing FA image')
    else : print("=> The file \'" + NormScalarMeasurement + "\' already exists so the command will not be executed")

# Affine registration with BrainsFit
    NormScalarMeasurement= OutputPath + "/Loop" + str(n) + "/" + allcasesIDs[case] + "_Loop" + str(n) + "_NormFA.nrrd"
    LinearTranstfm= OutputPath + "/Loop" + str(n) + "/" + allcasesIDs[case] + "_Loop" + str(n) + "_LinearTrans.txt"
    LinearTrans= OutputPath + "/Loop" + str(n) + "/" + allcasesIDs[case] + "_Loop" + str(n) + "_LinearTrans_FA.nrrd"
    AffineCommand= "/work/dtiatlasbuilder-build/BRAINSTools-install/bin/BRAINSFit --fixedVolume " + AtlasScalarMeasurementref + " --movingVolume " + NormScalarMeasurement + " --useAffine --outputVolume " + LinearTrans + " --outputTransform " + LinearTranstfm
    InitLinearTransTxt= OutputPath + "/" + allcasesIDs[case] + "_InitLinearTrans.txt"
    InitLinearTransMat= OutputPath + "/" + allcasesIDs[case] + "_InitLinearTrans.mat"
    if n==0 and CheckFileExists( InitLinearTransMat, case, allcasesIDs[case] ) and CheckFileExists( InitLinearTransTxt, case, allcasesIDs[case] ):
      print("[WARNING] Both \'" + allcasesIDs[case] + "_InitLinearTrans.mat\' and \'" + allcasesIDs[case] + "_InitLinearTrans.txt\' have been found. The .mat file will be used.")
      AffineCommand= AffineCommand + " --initialTransform " + InitLinearTransMat
    elif n==0 and CheckFileExists( InitLinearTransMat, case, allcasesIDs[case] ) : AffineCommand= AffineCommand + " --initialTransform " + InitLinearTransMat
    elif n==0 and CheckFileExists( InitLinearTransTxt, case, allcasesIDs[case] ) : AffineCommand= AffineCommand + " --initialTransform " + InitLinearTransTxt
    else : AffineCommand= AffineCommand + " --initializeTransformMode useCenterOfHeadAlign"
    print("[LOOP " + str(n) + "/1] [" + allcasesIDs[case] + "] [Affine registration with BrainsFit] => $ " + AffineCommand)
    CheckFileExists( LinearTrans, case, allcasesIDs[case] ) # Not for checking but to rename _LinearTrans_FA if old version
    if 1 :
      if os.system(AffineCommand)!=0 : DisplayErrorAndQuit('[' + allcasesIDs[case] + '] BRAINSFit: Affine Registration of FA image')
    else : print("=> The file \'" + LinearTranstfm + "\' already exists so the command will not be executed")

# Implementing the affine registration
    LinearTranstfm= OutputPath + "/Loop" + str(n) + "/" + allcasesIDs[case] + "_Loop" + str(n) + "_LinearTrans.txt"
    LinearTransDTI= OutputPath + "/Loop" + str(n) + "/" + allcasesIDs[case] + "_Loop" + str(n) + "_LinearTrans_DTI.nrrd"
    originalDTI= allcases[case]
    ImplementCommand= "/work/dtiatlasbuilder-build/ResampleDTIlogEuclidean-install/bin/ResampleDTIlogEuclidean " + originalDTI + " " + LinearTransDTI + " -f " + LinearTranstfm + " -R " + AtlasScalarMeasurementref
    print("[LOOP " + str(n) + "/1] [" + allcasesIDs[case] + "] [Implementing the Affine registration] => $ " + ImplementCommand)
    if 1 :
      if os.system(ImplementCommand)!=0 : DisplayErrorAndQuit('[' + allcasesIDs[case] + '] ResampleDTIlogEuclidean: Implementing the Affine Registration on FA image')
    else : print("=> The file \'" + LinearTransDTI + "\' already exists so the command will not be executed")

# Generating FA of registered images
    LinearTransDTI= OutputPath + "/Loop" + str(n) + "/" + allcasesIDs[case] + "_Loop" + str(n) + "_LinearTrans_DTI.nrrd"
    if n == 1 : LoopScalarMeasurement= OutputPath + "/Loop1/" + allcasesIDs[case] + "_Loop1_FinalFA.nrrd" # the last FA will be the Final output
    else : LoopScalarMeasurement= OutputPath + "/Loop" + str(n) + "/" + allcasesIDs[case] + "_Loop" + str(n) + "_FA.nrrd"
    GeneLoopScalarMeasurementCommand= "/work/dtiatlasbuilder-build/DTIProcess-install/bin/dtiprocess --dti_image " + LinearTransDTI + " -f " + LoopScalarMeasurement
    print("[LOOP " + str(n) + "/1] [" + allcasesIDs[case] + "] [Generating FA of registered images] => $ " + GeneLoopScalarMeasurementCommand)
    if 1 :
      if os.system(GeneLoopScalarMeasurementCommand)!=0 : DisplayErrorAndQuit('[' + allcasesIDs[case] + '] dtiprocess: Generating FA of affine registered images')
    else : print("=> The file \'" + LoopScalarMeasurement + "\' already exists so the command will not be executed")

    print("")
    case += 1 # indenting cases loop


# FA Average of registered images with ImageMath
  if n != 1 : # this will not be done for the last lap
    ScalarMeasurementAverage = OutputPath + "/Loop" + str(n) + "/Loop" + str(n) + "_FAAverage.nrrd"
    if n == 0 : ScalarMeasurementforAVG= OutputPath + "/ImageTest1_FA.nrrd"
    else : ScalarMeasurementforAVG= OutputPath + "/Loop" + str(n) + "/ImageTest1_Loop" + str(n) + "_FA.nrrd"
    AverageCommand = "/work/dtiatlasbuilder-build/niral_utilities-install/bin/ImageMath " + ScalarMeasurementforAVG + " -outfile " + ScalarMeasurementAverage + " -avg "
    case = 1
    while case < len(allcases):
      ScalarMeasurementforAVG= OutputPath + "/Loop" + str(n) + "/" + allcasesIDs[case] + "_Loop" + str(n) + "_FA.nrrd "
      AverageCommand= AverageCommand + ScalarMeasurementforAVG
      case += 1
    print("[LOOP " + str(n) + "/1] [Computing FA Average of registered images] => $ " + AverageCommand)
    if 1 :
      if os.system(AverageCommand)!=0 : DisplayErrorAndQuit('[Loop ' + str(n) + '] dtiaverage: Computing FA Average of registered images')
    else :
      print("=> The file \'" + ScalarMeasurementAverage + "\' already exists so the command will not be executed")
    AtlasScalarMeasurementref = ScalarMeasurementAverage # the average becomes the reference

  print("")
  n += 1 # indenting main loop

print("\n============ End of Pre processing =============")

sys.exit(0)

