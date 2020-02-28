#!/usr/bin/python

import os # To run a shell command : os.system("[shell command]")
import sys # to return an exit code
import shutil # to remove a non empty directory and copy files
import time 

PIDlogFile = config['m_OutputPath']+"/DTIAtlas/Script/PID.log"
PIDfile = open( PIDlogFile, 'a') # open in Append mode
PIDfile.write( str(os.getpid()) + "\n" )
PIDfile.close()

config={}
with open(configPath,'r') as f:
  config=json.load(f)

m_OutputPath=config["m_OutputPath"]
m_ScalarMeasurement=config["m_ScalarMeasurement"]
m_GridAtlasCommand=config["m_GridAtlasCommand"]
m_RegType=config["m_RegType"]
m_Overwrite=config["m_Overwrite"]
m_useGridProcess=config["m_useGridProcess"]
m_SoftPath=config["m_SoftPath"]
m_nbLoops=config["m_nbLoops"]
m_TensTfm=config["m_TensTfm"]
m_TemplatePath=config["m_TemplatePath"]
m_BFAffineTfmMode=config["m_BFAffineTfmMode"]
m_CasesIDs=config["m_CasesIDs"]
m_CasesPath=config["m_CasesPath"]
m_CropSize=config["m_CropSize"]
m_DTIRegExtraPath=config["m_DTIRegExtraPath"]
m_DTIRegOptions=config["m_DTIRegOptions"]
m_GridAtlasCommand=config["m_GridAtlasCommand"]
m_GridGeneralCommand=config["m_GridGeneralCommand"]
m_InterpolLogOption=config["m_InterpolLogOption"]
m_InterpolType=config["m_InterpolType"]
m_NbThreadsString=config["m_NbThreadsString"]
m_NeedToBeCropped=config["m_NeedToBeCropped"]
m_PythonPath=config["m_PythonPath"]
m_TensInterpol=config["m_TensInterpol"]

print("\n============ Atlas Building =============")

# Files Paths
DeformPath= m_OutputPath+"/DTIAtlas/2_NonLinear_Registration"
AffinePath= m_OutputPath+"/DTIAtlas/1_Affine_Registration"
FinalPath= m_OutputPath+"/DTIAtlas/3_Diffeomorphic_Atlas"
FinalResampPath= m_OutputPath+"/DTIAtlas/4_Final_Resampling"

def DisplayErrorAndQuit ( Error ):
  print '\n\nERROR DETECTED IN WORKFLOW:',Error
  print 'ABORT'
  sys.exit(1)

def TestGridProcess ( FilesFolder, NbCases , NoCase1=None):
  if NoCase1 is not None:
    if NbCases>0 : print("\n| Waiting for all batches (" + str(NbCases-NoCase1) + ") to be processed on grid...")
    else : print("\n| Waiting for 1 batch to be processed on grid...")
    filesOK = 0
    OldNbFilesOK = 0
    while not filesOK :
      filesOK = 1
      if NbCases>0 :
        NbfilesOK = 0
        case = int(NoCase1) # NoCase1 is 0 or 1 (bool)
        while case < NbCases:
          if not os.path.isfile( FilesFolder + "/Case" + str(case+1) ) : filesOK = 0
          else : NbfilesOK = NbfilesOK + 1
          case += 1
        if NbfilesOK != OldNbFilesOK : print("| [" + str(NbfilesOK) + "\t / " + str(NbCases-NoCase1) + " ] cases processed")
        OldNbFilesOK=NbfilesOK  
      elif not os.path.isfile( FilesFolder + "/file" ) : filesOK = 0
      time.sleep(60) # Test only every minute\n"
    print("\n=> All files processed\n")
    shutil.rmtree(FilesFolder) # clear directory and recreate it\n"
    os.mkdir(FilesFolder)

  else:
    if NbCases>0 : print("\n| Waiting for all batches (" + str(NbCases) + ") to be processed on grid...")
    else : print("\n| Waiting for 1 batch to be processed on grid...")
    filesOK = 0
    OldNbFilesOK = 0
    while not filesOK :
      filesOK = 1
      if NbCases>0 :
        NbfilesOK = 0
        case = 0
        while case < NbCases:
          if not os.path.isfile( FilesFolder + "/Case" + str(case+1) ) : filesOK = 0
          else : NbfilesOK = NbfilesOK + 1
          case += 1

        if NbfilesOK != OldNbFilesOK : print("| [" + str(NbfilesOK) + "\t / " + str(NbCases) + " ] cases processed")
        OldNbFilesOK=NbfilesOK
      elif not os.path.isfile( FilesFolder + "/file" ) : filesOK = 0
      time.sleep(60) # Test only every minute\n"
    print("\n=> All files processed\n")
    shutil.rmtree(FilesFolder) # clear directory and recreate it\n"
    os.mkdir(FilesFolder)

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
          os.rename(OldFile.replace("AWDTI","AW"+config['m_ScalarMeasurement']), File.replace("DiffeomorphicDTI","Diffeomorphic"+config['m_ScalarMeasurement']))
          os.rename(OldFile.replace("AWDTI","AWDTI_float"), File.replace("DiffeomorphicDTI","DiffeomorphicDTI_float"))
          return 1
        else : # test other old name
          OldFile = File.replace( caseID, "Case" + str(case+1) )
          if os.path.isfile( OldFile ) : # old file exists: rename and return 1
            os.rename(OldFile, File)
            os.rename(OldFile.replace("DiffeomorphicDTI","Diffeomorphic"+config['m_ScalarMeasurement']), File.replace("DiffeomorphicDTI","Diffeomorphic"+config['m_ScalarMeasurement']))
            os.rename(OldFile.replace("DiffeomorphicDTI","DiffeomorphicDTI_float"), File.replace("DiffeomorphicDTI","DiffeomorphicDTI_float"))
            return 1
          else:
            return 0
      if "DiffeomorphicAtlasDTI" in File :
        OldFile = File.replace("DiffeomorphicAtlasDTI", "AWAtlasDTI")
        if os.path.isfile( OldFile ) : # old file exists: rename and return 1
          os.rename(OldFile, File)
          os.rename(OldFile.replace("AWAtlasDTI","AWAtlas"+config['m_ScalarMeasurement']), File.replace("DiffeomorphicAtlasDTI","DiffeomorphicAtlas"+config['m_ScalarMeasurement']))
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


GridApostrophe=""
GridProcessCmd=""
GridProcessFileExistCmd1 = ""
GridProcessCmdNoCase = ""
GridProcessCmdAverage = ""
GridProcessFileExistCmdNoCase = ""
GridProcessFileExistIndent = ""
GridProcessFileExistIndent1 = ""

#<<<<<< CURRENT
FilesFolder=""
if config["m_useGridProcess"]:
  FilesFolder= m_OutputPath + '/DTIAtlas/GridProcessingFiles'
  GridApostrophe="'"
  File=FilesFolder + "/Case" + str(case+1)
  GridProcessCmd = m_GridGeneralCommand + " " + m_PythonPath + " " + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.py " +  " " + File  + " " + GridApostrophe 
  
  GridProcessFileExistCmd1 = "    f = open( " + File + ", 'w')\n    f.close()\n" ### This is problematic 

  FileNoCase = FilesFolder + "/file"
  GridProcessCmdNoCase = m_GridGeneralCommand + " " + m_PythonPath + " " + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.py " + " " + FileNoCase + " " + GridApostrophe 
  
  GridProcessFileExistCmdNoCase = "  f = open( " + FileNoCase + ", 'w')\n  f.close()\n" ### This is problematic 
  
  GridProcessCmdAverage = m_GridAtlasCommand + " " + m_PythonPath + " " + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.py " + " " + FileNoCase + " "  + GridApostrophe 
  GridProcessFileExistIndent = "\n  "
  GridProcessFileExistIndent1 = "\n    "

# Create directory for temporary files and final
if not os.path.isdir(DeformPath):
  OldDeformPath= m_OutputPath + "/DTIAtlas/2_NonLinear_Registration_AW"
  if os.path.isdir(OldDeformPath):
    os.rename(OldDeformPath,DeformPath)
  else:
    print("\n=> Creation of the Deformation transform directory = " + DeformPath)
    os.mkdir(DeformPath)

if not os.path.isdir(FinalPath):
  OldFinalPath= m_OutputPath+"/DTIAtlas/3_AW_Atlas"
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
#alltfms = [AffinePath + "/Loop"+ m_nbLoops +"/ImageTest1_Loop1_LinearTrans.txt", AffinePath + "/Loop1/ImageTest2_Loop1_LinearTrans.txt", AffinePath + "/Loop1/ImageTest3_Loop1_LinearTrans.txt"]
alltfms=[]
for i,c in enumerate(m_CasesPath):
  alltmfs.append(AffinePath+"/Loop"+m_nbLoops+"/" m_CaseIDs[i] + "_Loop" + m_nbLoops +"_LinearTrans.txt")

allcases=[]
if m_NeedToBeCropped==1:
  for i,c in enumerate(m_CasesPath):
    allcases.append(AffinePath + "/" + m_CasesIDs[i] + "_croppedDTI.nrrd")
else:
  for i,c in enumerate(m_CasesPath):
    allcases.append(m_CasesIDs[i] + "_croppedDTI.nrrd")
#allcases = ["/work/dtiatlasbuilder/Data/Testing/ImageTest1.nrrd", "/work/dtiatlasbuilder/Data/Testing/ImageTest2.nrrd", "/work/dtiatlasbuilder/Data/Testing/ImageTest3.nrrd"]
allcasesIDs=[]
for i,c in enumerate(m_CasesIDs):
  allcasesIDs.append(m_CasesIDs[i])
#allcasesIDs = ["ImageTest1", "ImageTest2", "ImageTest3"]


#### <<<< CURRENT POSITION

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

