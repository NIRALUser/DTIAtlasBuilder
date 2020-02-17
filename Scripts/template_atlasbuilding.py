#!/usr/bin/python

import os # To run a shell command : os.system("[shell command]")
import sys # to return an exit code
import shutil # to remove a non empty directory and copy files

PIDlogFile = "/work/DTIAtlas/Script/PID.log"
PIDfile = open( PIDlogFile, 'a') # open in Append mode
PIDfile.write( str(os.getpid()) + "\n" )
PIDfile.close()

print("\n============ Atlas Building =============")

# Files Paths
DeformPath= "/work/DTIAtlas/2_NonLinear_Registration"
AffinePath= "/work/DTIAtlas/1_Affine_Registration"
FinalPath= "/work/DTIAtlas/3_Diffeomorphic_Atlas"
FinalResampPath= "/work/DTIAtlas/4_Final_Resampling"

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

# Create directory for temporary files and final
if not os.path.isdir(DeformPath):
  OldDeformPath= "/work/DTIAtlas/2_NonLinear_Registration_AW"
  if os.path.isdir(OldDeformPath):
    os.rename(OldDeformPath,DeformPath)
  else:
    print("\n=> Creation of the Deformation transform directory = " + DeformPath)
    os.mkdir(DeformPath)

if not os.path.isdir(FinalPath):
  OldFinalPath= "/work/DTIAtlas/3_AW_Atlas"
  if os.path.isdir(OldFinalPath):
    os.rename(OldFinalPath,FinalPath)
  else:
    print("\n=> Creation of the Final Atlas directory = " + FinalPath)
    os.mkdir(FinalPath)

if not os.path.isdir(FinalResampPath):
  print("\n=> Creation of the Final Resampling directory = " + FinalResampPath)
  os.mkdir(FinalResampPath)

if not os.path.isdir(FinalResampPath + "/First_Resampling"):
  print("\n=> Creation of the First Final Resampling directory = " + FinalResampPath + "/First_Resampling")
  os.mkdir(FinalResampPath + "/First_Resampling")

if not os.path.isdir(FinalResampPath + "/Second_Resampling"):
  print("\n=> Creation of the Second Final Resampling directory = " + FinalResampPath + "/Second_Resampling")
  os.mkdir(FinalResampPath + "/Second_Resampling")

if not os.path.isdir(FinalResampPath + "/FinalTensors"):
  print("\n=> Creation of the Final Tensors directory = " + FinalResampPath + "/FinalTensors")
  os.mkdir(FinalResampPath + "/FinalTensors")

if not os.path.isdir(FinalResampPath + "/FinalDeformationFields"):
  print("\n=> Creation of the Final Deformation Fields directory = " + FinalResampPath + "/FinalDeformationFields\n")
  os.mkdir(FinalResampPath + "/FinalDeformationFields")

# Cases variables
alltfms = [AffinePath + "/Loop1/ImageTest1_Loop1_LinearTrans.txt", AffinePath + "/Loop1/ImageTest2_Loop1_LinearTrans.txt", AffinePath + "/Loop1/ImageTest3_Loop1_LinearTrans.txt"]

allcases = ["/work/dtiatlasbuilder/Data/Testing/ImageTest1.nrrd", "/work/dtiatlasbuilder/Data/Testing/ImageTest2.nrrd", "/work/dtiatlasbuilder/Data/Testing/ImageTest3.nrrd"]

allcasesIDs = ["ImageTest1", "ImageTest2", "ImageTest3"]

# GreedyAtlas Command
XMLFile= DeformPath + "/GreedyAtlasParameters.xml"
ParsedFile= DeformPath + "/ParsedXML.xml"
AtlasBCommand= "/work/dtiatlasbuilder-build/AtlasWerks-build/bin/GreedyAtlas -f " + XMLFile + " -o " + ParsedFile
print("[Computing the Deformation Fields with GreedyAtlas] => $ " + AtlasBCommand)
if 1 :
  if os.system(AtlasBCommand)!=0 : DisplayErrorAndQuit('GreedyAtlas: Computing non-linear atlas from affine registered images')
  case = 0
  while case < len(allcases): # Renaming
    originalImage=DeformPath + "/" + allcasesIDs[case] + "_Loop1_FinalFADefToMean.mhd"
    originalHField=DeformPath + "/" + allcasesIDs[case] + "_Loop1_FinalFADefFieldImToMean.mhd"
    originalInvHField=DeformPath + "/" + allcasesIDs[case] + "_Loop1_FinalFADefFieldMeanToIm.mhd"
    NewImage= DeformPath + "/" + allcasesIDs[case] + "_NonLinearTrans_FA.mhd"
    NewHField=DeformPath + "/" + allcasesIDs[case] + "_HField.mhd"
    NewInvHField=DeformPath + "/" + allcasesIDs[case] + "_InverseHField.mhd"
    print("[" + allcasesIDs[case] + "] => Renaming \'" + originalImage + "\' to \'" + NewImage + "\'")
    os.rename(originalImage,NewImage)
    print("[" + allcasesIDs[case] + "] => Renaming \'" + originalHField + "\' to \'" + NewHField + "\'")
    os.rename(originalHField,NewHField)
    print("[" + allcasesIDs[case] + "] => Renaming \'" + originalInvHField + "\' to \'" + NewInvHField + "\'")
    os.rename(originalInvHField,NewInvHField)
    case += 1

# Apply deformation fields
case = 0
while case < len(allcases):
  FinalDTI= FinalPath + "/" + allcasesIDs[case] + "_DiffeomorphicDTI.nrrd"
  originalDTI= allcases[case]
  Ref = AffinePath + "/Loop0/Loop0_FAAverage.nrrd"
  HField= DeformPath + "/" + allcasesIDs[case] + "_HField.mhd"
  FinalReSampCommand="/work/dtiatlasbuilder-build/ResampleDTIlogEuclidean-install/bin/ResampleDTIlogEuclidean -R " + Ref + " -H " + HField + " -f " + alltfms[case] + " " + originalDTI + " " + FinalDTI
  FinalReSampCommand = FinalReSampCommand + " -i linear"
  FinalReSampCommand = FinalReSampCommand + " -T PPD"
  print("\n[" + allcasesIDs[case] + "] [Applying deformation fields to original DTIs] => $ " + FinalReSampCommand)
  if 1 :
    DiffeomorphicCaseScalarMeasurement = FinalPath + "/" + allcasesIDs[case] + "_DiffeomorphicFA.nrrd"
    GeneDiffeomorphicCaseScalarMeasurementCommand="/work/dtiatlasbuilder-build/DTIProcess-install/bin/dtiprocess --scalar_float --dti_image " + FinalDTI + " -f " + DiffeomorphicCaseScalarMeasurement
    CaseDbleToFloatCommand="/work/dtiatlasbuilder-build/teem-install/bin/unu convert -t float -i " + FinalDTI + " | /work/dtiatlasbuilder-build/teem-install/bin/unu save -f nrrd -e gzip -o " + FinalPath + "/" + allcasesIDs[case] + "_DiffeomorphicDTI_float.nrrd"

    if os.system(FinalReSampCommand)!=0 : DisplayErrorAndQuit('[' + allcasesIDs[case] + '] ResampleDTIlogEuclidean: Applying deformation fields to original DTIs')
    print("[" + allcasesIDs[case] + "] => $ " + GeneDiffeomorphicCaseScalarMeasurementCommand)
    if os.system(GeneDiffeomorphicCaseScalarMeasurementCommand)!=0 : DisplayErrorAndQuit('[' + allcasesIDs[case] + '] dtiprocess: Computing Diffeomorphic FA')
    print("[" + allcasesIDs[case] + "] => $ " + CaseDbleToFloatCommand + "\n")
    if os.system(CaseDbleToFloatCommand)!=0 : DisplayErrorAndQuit('[' + allcasesIDs[case] + '] unu: Converting the final DTI images from double to float DTI')
  else : print("=> The file \'" + FinalDTI + "\' already exists so the command will not be executed")
  case += 1

# DTIaverage computing
DTIAverage = FinalPath + "/DiffeomorphicAtlasDTI.nrrd"
AverageCommand = "/work/dtiatlasbuilder-build/DTIProcess-install/bin/dtiaverage "
case = 0
while case < len(allcases):
  DTIforAVG= "--inputs " + FinalPath + "/" + allcasesIDs[case] + "_DiffeomorphicDTI.nrrd "
  AverageCommand = AverageCommand + DTIforAVG
  case += 1
AverageCommand = AverageCommand + "--tensor_output " + DTIAverage
print("\n[Computing the Diffeomorphic DTI average] => $ " + AverageCommand)
if 1 : 
# Computing some images from the final DTI with dtiprocess
  FA= FinalPath + "/DiffeomorphicAtlasFA.nrrd"
  cFA= FinalPath + "/DiffeomorphicAtlasColorFA.nrrd"
  RD= FinalPath + "/DiffeomorphicAtlasRD.nrrd"
  MD= FinalPath + "/DiffeomorphicAtlasMD.nrrd"
  AD= FinalPath + "/DiffeomorphicAtlasAD.nrrd"
  GeneScalarMeasurementCommand="/work/dtiatlasbuilder-build/DTIProcess-install/bin/dtiprocess --scalar_float --dti_image " + DTIAverage + " -f " + FA + " -m " + MD + " --color_fa_output " + cFA + " --RD_output " + RD + " --lambda1_output " + AD

  DbleToFloatCommand="/work/dtiatlasbuilder-build/teem-install/bin/unu convert -t float -i " + DTIAverage + " | /work/dtiatlasbuilder-build/teem-install/bin/unu save -f nrrd -e gzip -o " + FinalPath + "/DiffeomorphicAtlasDTI_float.nrrd"

  if os.system(AverageCommand)!=0 : DisplayErrorAndQuit('dtiaverage: Computing the final DTI average')
  print("[Computing some images from the final DTI with dtiprocess] => $ " + GeneScalarMeasurementCommand)
  if os.system(GeneScalarMeasurementCommand)!=0 : DisplayErrorAndQuit('dtiprocess: Computing Diffeomorphic FA, color FA, MD, RD and AD')
  print("[Computing some images from the final DTI with dtiprocess] => $ " + DbleToFloatCommand)
  if os.system(DbleToFloatCommand)!=0 : DisplayErrorAndQuit('unu: Converting the final DTI atlas from double to float DTI')

# Computing global deformation fields
case = 0
while case < len(allcases):
  origDTI= allcases[case]
  GlobalDefField = FinalResampPath + "/First_Resampling/" + allcasesIDs[case] + "_GlobalDisplacementField.nrrd"
  FinalDef = FinalResampPath + "/First_Resampling/" + allcasesIDs[case] + "_DeformedDTI.nrrd"
  GlobalDefFieldCommand="/work/dtiatlasbuilder-build/DTI-Reg-install/bin/DTI-Reg --fixedVolume " + DTIAverage + " --movingVolume " + origDTI + " --scalarMeasurement FA --outputDisplacementField " + GlobalDefField + " --outputVolume " + FinalDef
  GlobalDefFieldCommand = GlobalDefFieldCommand + " --ProgramsPathsVector ,/work/dtiatlasbuilder-build/BRAINSTools-install/bin,/work/dtiatlasbuilder-build/DTIProcess-install/bin,/work/dtiatlasbuilder-build/ResampleDTIlogEuclidean-install/bin"
  GlobalDefFieldCommand= GlobalDefFieldCommand + " --method useScalar-BRAINS"
  GlobalDefFieldCommand= GlobalDefFieldCommand + " --BRAINSRegistrationType Diffeomorphic"
  GlobalDefFieldCommand= GlobalDefFieldCommand + " --BRAINSnumberOfPyramidLevels 5"
  GlobalDefFieldCommand= GlobalDefFieldCommand + " --BRAINSarrayOfPyramidLevelIterations 300,50,30,20,15"
  GlobalDefFieldCommand= GlobalDefFieldCommand + " --initialAffine " + alltfms[case]
  BRAINSTempTfm = FinalResampPath + "/First_Resampling/" + allcasesIDs[case] + "_FA_AffReg.txt"
  GlobalDefFieldCommand= GlobalDefFieldCommand + " --outputTransform " + BRAINSTempTfm
  print("\n[" + allcasesIDs[case] + "] [Computing global deformation fields] => $ " + GlobalDefFieldCommand)
  if 1 :
    GlobDbleToFloatCommand="/work/dtiatlasbuilder-build/teem-install/bin/unu convert -t float -i " + FinalDef + " | /work/dtiatlasbuilder-build/teem-install/bin/unu save -f nrrd -e gzip -o " + FinalResampPath + "/First_Resampling/" + allcasesIDs[case] + "_DeformedDTI_float.nrrd"

    if os.system(GlobalDefFieldCommand)!=0 : DisplayErrorAndQuit('[' + allcasesIDs[case] + '] DTI-Reg: Computing global deformation fields')
    print("\n[" + allcasesIDs[case] + "] [Converting the deformed images from double to float DTI] => $ " + GlobDbleToFloatCommand)
    if os.system(GlobDbleToFloatCommand)!=0 : DisplayErrorAndQuit('[' + allcasesIDs[case] + '] unu: Converting the deformed images from double to float DTI')
  case += 1

# dtiaverage recomputing
DTIAverage2 = FinalResampPath + "/FinalAtlasDTI.nrrd"
AverageCommand2 = "/work/dtiatlasbuilder-build/DTIProcess-install/bin/dtiaverage "
case = 0
while case < len(allcases):
  DTIforAVG2= "--inputs " + FinalResampPath + "/First_Resampling/" + allcasesIDs[case] + "_DeformedDTI.nrrd "
  AverageCommand2 = AverageCommand2 + DTIforAVG2
  case += 1
AverageCommand2 = AverageCommand2 + "--tensor_output " + DTIAverage2
print("\n[Recomputing the final DTI average] => $ " + AverageCommand2)
if 1 : 
# Computing some images from the final DTI with dtiprocess
  FA2= FinalResampPath + "/FinalAtlasFA.nrrd"
  cFA2= FinalResampPath + "/FinalAtlasColorFA.nrrd"
  RD2= FinalResampPath + "/FinalAtlasRD.nrrd"
  MD2= FinalResampPath + "/FinalAtlasMD.nrrd"
  AD2= FinalResampPath + "/FinalAtlasAD.nrrd"
  GeneScalarMeasurementCommand2="/work/dtiatlasbuilder-build/DTIProcess-install/bin/dtiprocess --scalar_float --dti_image " + DTIAverage2 + " -f " + FA2 + " -m " + MD2 + " --color_fa_output " + cFA2 + " --RD_output " + RD2 + " --lambda1_output " + AD2

  DbleToFloatCommand2="/work/dtiatlasbuilder-build/teem-install/bin/unu convert -t float -i " + DTIAverage2 + " | /work/dtiatlasbuilder-build/teem-install/bin/unu save -f nrrd -e gzip -o " + FinalResampPath + "/FinalAtlasDTI_float.nrrd"

  if os.system(AverageCommand2)!=0 : DisplayErrorAndQuit('dtiaverage: Recomputing the final DTI average')
  print("[Computing some images from the final DTI with dtiprocess] => $ " + GeneScalarMeasurementCommand2)
  if os.system(GeneScalarMeasurementCommand2)!=0 : DisplayErrorAndQuit('dtiprocess: Recomputing final FA, color FA, MD, RD and AD')
  print("[Computing some images from the final DTI with dtiprocess] => $ " + DbleToFloatCommand2)
  if os.system(DbleToFloatCommand2)!=0 : DisplayErrorAndQuit('unu: Converting the final resampled DTI atlas from double to float DTI')

# Recomputing global deformation fields
SecondResampRecomputed = [0] * len(allcases) # array of 1s and 0s to know what has been recomputed to know what to copy to final folders
case = 0
while case < len(allcases):
  origDTI2= allcases[case]
  GlobalDefField2 = FinalResampPath + "/Second_Resampling/" + allcasesIDs[case] + "_GlobalDisplacementField.nrrd"
  FinalDef2 = FinalResampPath + "/Second_Resampling/" + allcasesIDs[case] + "_FinalDeformedDTI.nrrd"
  GlobalDefFieldCommand2="/work/dtiatlasbuilder-build/DTI-Reg-install/bin/DTI-Reg --fixedVolume " + DTIAverage2 + " --movingVolume " + origDTI2 + " --scalarMeasurement FA --outputDisplacementField " + GlobalDefField2 + " --outputVolume " + FinalDef2
  GlobalDefFieldCommand2 = GlobalDefFieldCommand2 + " --ProgramsPathsVector ,/work/dtiatlasbuilder-build/BRAINSTools-install/bin,/work/dtiatlasbuilder-build/DTIProcess-install/bin,/work/dtiatlasbuilder-build/ResampleDTIlogEuclidean-install/bin"
  GlobalDefFieldCommand2 = GlobalDefFieldCommand2 + " --method useScalar-BRAINS"
  GlobalDefFieldCommand2 = GlobalDefFieldCommand2 + " --BRAINSRegistrationType Diffeomorphic"
  GlobalDefFieldCommand2 = GlobalDefFieldCommand2 + " --BRAINSnumberOfPyramidLevels 5"
  GlobalDefFieldCommand2 = GlobalDefFieldCommand2 + " --BRAINSarrayOfPyramidLevelIterations 300,50,30,20,15"
  GlobalDefFieldCommand2 = GlobalDefFieldCommand2 + " --initialAffine " + alltfms[case]
  BRAINSTempTfm2 = FinalResampPath + "/Second_Resampling/" + allcasesIDs[case] + "_FA_AffReg.txt"
  GlobalDefFieldCommand2 = GlobalDefFieldCommand2 + " --outputTransform " + BRAINSTempTfm2
  print("\n[" + allcasesIDs[case] + "] [Recomputing global deformation fields] => $ " + GlobalDefFieldCommand2)
  if 1 :
    SecondResampRecomputed[case] = 1
    DTIRegCaseScalarMeasurement = FinalResampPath + "/Second_Resampling/" + allcasesIDs[case] + "_FinalDeformedFA.nrrd"
    GeneDTIRegCaseScalarMeasurementCommand="/work/dtiatlasbuilder-build/DTIProcess-install/bin/dtiprocess --scalar_float --dti_image " + FinalDef2 + " -f " + DTIRegCaseScalarMeasurement
    GlobDbleToFloatCommand2="/work/dtiatlasbuilder-build/teem-install/bin/unu convert -t float -i " + FinalDef2 + " | /work/dtiatlasbuilder-build/teem-install/bin/unu save -f nrrd -e gzip -o " + FinalResampPath + "/Second_Resampling/" + allcasesIDs[case] + "_FinalDeformedDTI_float.nrrd"
    if os.system(GlobalDefFieldCommand2)!=0 : DisplayErrorAndQuit('[' + allcasesIDs[case] + '] DTI-Reg: Computing global deformation fields')
    print("\n[" + allcasesIDs[case] + "] [Converting the deformed images from double to float DTI] => $ " + GlobDbleToFloatCommand2)
    if os.system(GlobDbleToFloatCommand2)!=0 : DisplayErrorAndQuit('[' + allcasesIDs[case] + '] unu: Converting the deformed images from double to float DTI')
    print("\n[" + allcasesIDs[case] + "] [Computing DTIReg FA] => $ " + GeneDTIRegCaseScalarMeasurementCommand)
    if os.system(GeneDTIRegCaseScalarMeasurementCommand)!=0 : DisplayErrorAndQuit('[' + allcasesIDs[case] + '] dtiprocess: Computing DTIReg FA')
  case += 1


# Moving final images to final folders
print("\n=> Moving final images to final folders")
case = 0
while case < len(allcases):
  if SecondResampRecomputed[case] :
    GlobalDefField2 = FinalResampPath + "/Second_Resampling/" + allcasesIDs[case] + "_GlobalDisplacementField.nrrd"
    NewGlobalDefField2 = FinalResampPath + "/FinalDeformationFields/" + allcasesIDs[case] + "_GlobalDisplacementField.nrrd"
    if CheckFileExists(GlobalDefField2, case, allcasesIDs[case]) :
      shutil.copy(GlobalDefField2, NewGlobalDefField2)
    InverseGlobalDefField2 = FinalResampPath + "/Second_Resampling/" + allcasesIDs[case] + "_InverseGlobalDisplacementField.nrrd"
    NewInverseGlobalDefField2 = FinalResampPath + "/FinalDeformationFields/" + allcasesIDs[case] + "_InverseGlobalDisplacementField.nrrd"
    if CheckFileExists(InverseGlobalDefField2, case, allcasesIDs[case]) :
      shutil.copy(InverseGlobalDefField2, NewInverseGlobalDefField2)
    FinalDef2 = FinalResampPath + "/Second_Resampling/" + allcasesIDs[case] + "_FinalDeformedDTI.nrrd"
    NewFinalDef2 = FinalResampPath + "/FinalTensors/" + allcasesIDs[case] + "_FinalDeformedDTI.nrrd"
    if CheckFileExists(FinalDef2, case, allcasesIDs[case]) :
      shutil.copy(FinalDef2, NewFinalDef2)
    FinalDef2f = FinalResampPath + "/Second_Resampling/" + allcasesIDs[case] + "_FinalDeformedDTI_float.nrrd"
    NewFinalDef2f = FinalResampPath + "/FinalTensors/" + allcasesIDs[case] + "_FinalDeformedDTI_float.nrrd"
    if CheckFileExists(FinalDef2f, case, allcasesIDs[case]) :
      shutil.copy(FinalDef2f, NewFinalDef2f)
    DTIRegCaseScalarMeasurement = FinalResampPath + "/Second_Resampling/" + allcasesIDs[case] + "_FinalDeformedFA.nrrd"
    NewDTIRegCaseScalarMeasurement = FinalResampPath + "/FinalTensors/" + allcasesIDs[case] + "_FinalDeformedFA.nrrd"
    if CheckFileExists(DTIRegCaseScalarMeasurement, case, allcasesIDs[case]) :
      shutil.copy(DTIRegCaseScalarMeasurement, NewDTIRegCaseScalarMeasurement)
  case += 1

print("\n============ End of Atlas Building =============")

sys.exit(0)

