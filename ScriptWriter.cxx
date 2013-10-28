/*std classes*/
#include <iostream>
#include <string>
#include <vector>
#include <sstream> // to convert int to std::string
#include <math.h> // for the absolute value

/*itk classes*/
#include "itkImage.h"
#include "itkImageFileReader.h"
#include <itksys/SystemTools.hxx> // for GetFilenamePath()

#include "ScriptWriter.h"

std::string IntToStr(int IntVar)
{
  std::ostringstream oss;
  oss << IntVar;
  return oss.str();
}

  /////////////////////////////////////////
 //     SUB  WRITING FUNCTIONS          //
/////////////////////////////////////////
// to avoid having to write twice the same and reduce writing functions size

std::string pyTestGridProcess ( bool NoCase1 )
{
  std::string Script = "\n# Function that tests if all batches have been processed on the grid\n";
  if( NoCase1 )
  {
    Script = Script + "def TestGridProcess ( FilesFolder, NbCases , NoCase1): # if NbCases == 0, then just search the file \'file\' (unique command)\n";
    Script = Script + "  if NbCases>0 : print(\"\\n| Waiting for all batches (\" + str(NbCases-NoCase1) + \") to be processed on grid...\") # NoCase1 is 0 or 1\n";
  }
  else
  {
    Script = Script + "def TestGridProcess ( FilesFolder, NbCases ): # if NbCases == 0, then just search the file \'file\' (unique command)\n";
    Script = Script + "  if NbCases>0 : print(\"\\n| Waiting for all batches (\" + str(NbCases) + \") to be processed on grid...\")\n";
  }
  Script = Script + "  else : print(\"\\n| Waiting for 1 batch to be processed on grid...\")\n";
  Script = Script + "  filesOK = 0\n";
  Script = Script + "  OldNbFilesOK = 0\n";
  Script = Script + "  while not filesOK :\n";
  Script = Script + "    filesOK = 1\n";
  Script = Script + "    if NbCases>0 :\n";
  Script = Script + "      NbfilesOK = 0\n";
  if( NoCase1 )
  {
    Script = Script + "      case = int(NoCase1) # NoCase1 is 0 or 1 (bool)\n";
  }
  else
  {
    Script = Script + "      case = 0\n";
  }
  Script = Script + "      while case < NbCases:\n";
  Script = Script + "        if not os.path.isfile( FilesFolder + \"/Case\" + str(case+1) ) : filesOK = 0\n";
  Script = Script + "        else : NbfilesOK = NbfilesOK + 1\n";
  Script = Script + "        case += 1\n";
  if( NoCase1 )
  {
    Script = Script + "      if NbfilesOK != OldNbFilesOK : print(\"| [\" + str(NbfilesOK) + \"\\t / \" + str(NbCases-NoCase1) + \" ] cases processed\")\n";
  }
  else
  {
    Script = Script + "      if NbfilesOK != OldNbFilesOK : print(\"| [\" + str(NbfilesOK) + \"\\t / \" + str(NbCases) + \" ] cases processed\")\n";
  }
  Script = Script + "      OldNbFilesOK=NbfilesOK\n";
  Script = Script + "    elif not os.path.isfile( FilesFolder + \"/file\" ) : filesOK = 0\n";
  Script = Script + "    time.sleep(60) # Test only every minute\n";
  Script = Script + "  print(\"\\n=> All files processed\\n\")\n";
  Script = Script + "  shutil.rmtree(FilesFolder) # clear directory and recreate it\n";
  Script = Script + "  os.mkdir(FilesFolder)\n";

  return Script;
}

std::string pyCheckFileExists ()
{
  std::string Script = "# Function that checks if file exist and replace old names by new names if needed\n";

  Script = Script + "def CheckFileExists ( File, case, caseID ) : # returns 1 if file exists or has been renamed and 0 if not\n";

  Script = Script + "  if os.path.isfile( File ) : # file exists\n";
  Script = Script + "    return 1\n";

  Script = Script + "  else : # file does not exist: check if older version of file can exist (if file name has been changed)\n";

  Script = Script + "    NamesThatHaveChanged = [\"MeanImage\", \"DiffeomorphicDTI\", \"DiffeomorphicAtlasDTI\", \"HField\", \"GlobalDisplacementField\"] # latest versions of the names\n";
  Script = Script + "    if any( changedname in File for changedname in NamesThatHaveChanged ) : # if name has been changed, check if older version files exist\n";

  Script = Script + "      if \"MeanImage\" in File :\n";
  Script = Script + "        OldFile = File.replace(\"Mean\", \"Average\")\n";
  Script = Script + "        if os.path.isfile( OldFile ) : # old file exists: rename and return 1\n";
  Script = Script + "          os.rename(OldFile, File)\n";
  Script = Script + "          return 1\n";
  Script = Script + "        else:\n";
  Script = Script + "          return 0\n";

  Script = Script + "      if \"DiffeomorphicDTI\" in File :\n";
  Script = Script + "        OldFile = File.replace( caseID, \"Case\" + str(case+1) ).replace(\"DiffeomorphicDTI\", \"AWDTI\")\n";
  Script = Script + "        if os.path.isfile( OldFile ) : # old file exists: rename and return 1\n";
  Script = Script + "          os.rename(OldFile, File)\n";
  Script = Script + "          os.rename(OldFile.replace(\"AWDTI\",\"AWFA\"), File.replace(\"DiffeomorphicDTI\",\"DiffeomorphicFA\"))\n";
  Script = Script + "          os.rename(OldFile.replace(\"AWDTI\",\"AWDTI_float\"), File.replace(\"DiffeomorphicDTI\",\"DiffeomorphicDTI_float\"))\n";
  Script = Script + "          return 1\n";
  Script = Script + "        else : # test other old name\n";
  Script = Script + "          OldFile = File.replace( caseID, \"Case\" + str(case+1) )\n";
  Script = Script + "          if os.path.isfile( OldFile ) : # old file exists: rename and return 1\n";
  Script = Script + "            os.rename(OldFile, File)\n";
  Script = Script + "            os.rename(OldFile.replace(\"DiffeomorphicDTI\",\"DiffeomorphicFA\"), File.replace(\"DiffeomorphicDTI\",\"DiffeomorphicFA\"))\n";
  Script = Script + "            os.rename(OldFile.replace(\"DiffeomorphicDTI\",\"DiffeomorphicDTI_float\"), File.replace(\"DiffeomorphicDTI\",\"DiffeomorphicDTI_float\"))\n";
  Script = Script + "            return 1\n";
  Script = Script + "          else:\n";
  Script = Script + "            return 0\n";

  Script = Script + "      if \"DiffeomorphicAtlasDTI\" in File :\n";
  Script = Script + "        OldFile = File.replace(\"DiffeomorphicAtlasDTI\", \"AWAtlasDTI\")\n";
  Script = Script + "        if os.path.isfile( OldFile ) : # old file exists: rename and return 1\n";
  Script = Script + "          os.rename(OldFile, File)\n";
  Script = Script + "          os.rename(OldFile.replace(\"AWAtlasDTI\",\"AWAtlasFA\"), File.replace(\"DiffeomorphicAtlasDTI\",\"DiffeomorphicAtlasFA\"))\n";
  Script = Script + "          os.rename(OldFile.replace(\"AWAtlasDTI\",\"AWAtlasDTI_float\"), File.replace(\"DiffeomorphicAtlasDTI\",\"DiffeomorphicAtlasDTI_float\"))\n";
  Script = Script + "          return 1\n";
  Script = Script + "        else:\n";
  Script = Script + "          return 0\n";

  Script = Script + "      if \"HField\" in File :\n";
  Script = Script + "        OldFile = File.replace( caseID, \"Case\" + str(case+1) ).replace(\"H\", \"Deformation\")\n";
  Script = Script + "        if os.path.isfile( OldFile ) : # old file exists: rename and return 1\n";
  Script = Script + "          os.rename(OldFile, File)\n";
  Script = Script + "          return 1\n";
  Script = Script + "        else : # test other old name\n";
  Script = Script + "          OldFile = File.replace( caseID, \"Case\" + str(case+1) )\n";
  Script = Script + "          if os.path.isfile( OldFile ) : # old file exists: rename and return 1\n";
  Script = Script + "            os.rename(OldFile, File)\n";
  Script = Script + "            return 1\n";
  Script = Script + "          else:\n";
  Script = Script + "            return 0\n";

  Script = Script + "      if \"GlobalDisplacementField\" in File :\n";
  Script = Script + "        OldFile = File.replace( caseID, \"Case\" + str(case+1) ).replace(\"Displacement\", \"Deformation\")\n";
  Script = Script + "        if os.path.isfile( OldFile ) : # old file exists: rename and return 1\n";
  Script = Script + "          os.rename(OldFile, File)\n";
  Script = Script + "          return 1\n";
  Script = Script + "        else : # test other old name\n";
  Script = Script + "          OldFile = File.replace( caseID, \"Case\" + str(case+1) )\n";
  Script = Script + "          if os.path.isfile( OldFile ) : # old file exists: rename and return 1\n";
  Script = Script + "            os.rename(OldFile, File)\n";
  Script = Script + "            return 1\n";
  Script = Script + "          else:\n";
  Script = Script + "            return 0\n";

  Script = Script + "    else: # file does not exist and name has not been changed: check if the caseX version exists\n";

  Script = Script + "      if caseID : # CaseID is empty for averages\n";
  Script = Script + "        OldFile = File.replace( caseID, \"Case\" + str(case+1) )\n";
  Script = Script + "        if os.path.isfile( OldFile ) : # old file exists: rename and return 1\n";
  Script = Script + "          os.rename(OldFile, File)\n";
  Script = Script + "          return 1\n";
  Script = Script + "        else:\n";
  Script = Script + "          return 0\n";
  Script = Script + "      else: # for averages\n";
  Script = Script + "        return 0\n\n";

  return Script;
}

std::string ScriptWriter::pyExecuteCommandPreprocessCase ( std::string NameOfFileVarToTest, std::string NameOfCmdVarToExec, std::string ErrorTxtToDisplay, std::string SpacesBeforeFirstIf )
{
  std::string Script="";

  if(m_Overwrite==1)
  {
    Script = Script + SpacesBeforeFirstIf + "if 1 :\n";
  }
  else
  {
    Script = Script + SpacesBeforeFirstIf + "if not CheckFileExists( " + NameOfFileVarToTest + ", case, allcasesIDs[case] ) :\n";
  }

  if( ! m_useGridProcess )
  {
    Script = Script + SpacesBeforeFirstIf + "  if os.system(" + NameOfCmdVarToExec + ")!=0 : DisplayErrorAndQuit(\'[\' + allcasesIDs[case] + \'] " + ErrorTxtToDisplay + "\')\n";
  }
  else
  {
    Script = Script + SpacesBeforeFirstIf + "  GridProcessCaseCommandsArray.append(" + NameOfCmdVarToExec + ") # Executed eventually\n";
  }

  Script = Script + SpacesBeforeFirstIf + "else : print(\"=> The file \\'\" + " + NameOfFileVarToTest + " + \"\\' already exists so the command will not be executed\")\n"; // not used if overwrite because "if 1 :"

  return Script;
}

  /////////////////////////////////////////
 //         WRITING FUNCTIONS           //
/////////////////////////////////////////

void ScriptWriter::WriteScript()
{
  std::cout<<"|"<<std::endl; // command line display
  std::cout<<"| Number of Cases: "<<m_CasesPath.size()<<std::endl; // command line display
  std::cout<<"| Output Directory : "<<m_OutputPath<<"/DTIAtlas"<<std::endl; // command line display
  if( m_useGridProcess )
  {
    std::cout<<"| Using grid processing"<<std::endl; // command line display
  }
  if(m_RegType==1)
  {
    std::cout<<"| Using Case 1 (" << m_CasesIDs[0] << ") as reference in the first Registration Loop"<<std::endl; // command line display
  }
  else
  {
    std::cout<<"| Using Template as reference for the Registration: "<<m_TemplatePath<<std::endl; // command line display
  }
  std::cout<<"| Number of loops in the Registration Loop : "<<m_nbLoops<<std::endl; // command line display
  std::cout<<"| Writing begin: "; // command line display (no endl)

  Preprocess();
  AtlasBuilding();
  MainScript();
}

void ScriptWriter::Preprocess ()
{
  std::string Script;

  std::cout<<"[Pre Processing]"; // command line display (no endl)

  Script = Script + "#!/usr/bin/python\n\n";
  Script = Script + "import os # To run a shell command : os.system(\"[shell command]\")\n";
  Script = Script + "import sys # to return an exit code\n";
  Script = Script + "import shutil # to remove a non empty directory\n\n";

  Script = Script + "PIDlogFile = \"" + m_OutputPath + "/DTIAtlas/Script/PID.log\"\n";
  Script = Script + "PIDfile = open( PIDlogFile, 'a') # open in Append mode\n";
  Script = Script + "PIDfile.write( str(os.getpid()) + \"\\n\" )\n";
  Script = Script + "PIDfile.close()\n\n";

  Script = Script + "print(\"\\n============ Pre processing =============\")\n\n";

  Script = Script + "# Files Paths\n";
  // allcases
  Script = Script + "allcases = [\"" + m_CasesPath[0];
  for (unsigned int i=1;i<m_CasesPath.size();i++)
  {
    Script = Script + "\", \"" + m_CasesPath[i];
  }
  Script = Script+ "\"]\n\n";

  // allcasesIDs
  Script = Script + "allcasesIDs = [\"" + m_CasesIDs[0];
  for (unsigned int i=1;i<m_CasesIDs.size();i++)
  {
    Script = Script + "\", \"" + m_CasesIDs[i];
  }
  Script = Script+ "\"]\n\n";

  Script = Script + "OutputPath= \"" + m_OutputPath + "/DTIAtlas/1_Affine_Registration\"\n";

  if(m_RegType==1)
  {
    Script = Script + "AtlasFAref= OutputPath + \"/" + m_CasesIDs[0] + "_FA.nrrd\" #the reference will be the first case for the first loop\n\n";
  }
  else
  {
    Script = Script + "AtlasFAref= \"" + m_TemplatePath + "\" #the reference will be the given template for the first loop\n\n";
  }

  /* Function to display error and quit */
  Script = Script + "def DisplayErrorAndQuit ( Error ):\n";
  Script = Script + "  print \'\\n\\nERROR DETECTED IN WORKFLOW:\',Error\n";
  Script = Script + "  print \'ABORT\'\n";
  Script = Script + "  sys.exit(1)\n\n";

  /* Function that checks if file exist and replace old names by new names if needed */
  Script = Script + pyCheckFileExists();

/* Call script to run command on grid */
  std::string NoCase1 = IntToStr(m_RegType); // m_RegType = 0 if template (-> NoCase1=0) |  = 1 if case1=ref (-> NoCase1=1) => NoCase1 = (string) m_RegType
  if( m_useGridProcess )
  {
    Script = Script + "\n# Call script to run command on server\n";
    Script = Script + "import time # To test existence of files only every minute\n";
    Script = Script + "FilesFolder = \"" + m_OutputPath + "/DTIAtlas/GridProcessingFiles\"\n";
    Script = Script + "if os.path.isdir(FilesFolder): shutil.rmtree(FilesFolder) # remove directory to get rid of any previous file\n";
    Script = Script + "os.mkdir(FilesFolder)\n";
    Script = Script + "print(\"\\n=> Creation of the directory for the grid processing = \" + FilesFolder)\n";

    //Test Function
    Script = Script + pyTestGridProcess( true ); // bool NoCase1
  }

/* Create directory for temporary files */
  Script = Script + "\n# Create directory for temporary files\n";
  Script = Script + "if not os.path.isdir(OutputPath):\n";
  Script = Script + "  os.mkdir(OutputPath)\n";
  Script = Script + "  print(\"\\n=> Creation of the affine directory = \" + OutputPath)\n\n";

/* Rescaling template */
  if(m_RegType==0) // with template
  {
    Script = Script + "\n# Rescaling template\n";
    Script = Script + "RescaleTemp= OutputPath + \"/FATemplate_Rescaled.nrrd\"\n";
    Script = Script + "RescaleTempCommand= \"" + m_SoftPath[0] + " \" + AtlasFAref + \" -outfile \" + RescaleTemp + \" -rescale 0,10000\"\n";

    if( m_useGridProcess )
    {
      Script = Script + "RescaleTempCommand= \"" + m_GridCommand + " " + m_PythonPath + " " + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.py \" + FilesFolder + \"/file \\'\" + RescaleTempCommand  + \"\\'\"\n";
    }
    Script = Script + "print(\"\\n[Rescaling FA template] => $ \" + RescaleTempCommand)\n";

    if(m_Overwrite==1)
    {
      Script = Script + "if 1 :\n";
    }
    else
    {
      Script = Script + "if not CheckFileExists(RescaleTemp, 0, \"\" ) :\n";
    }
    Script = Script + "  if os.system(RescaleTempCommand)!=0 : DisplayErrorAndQuit(\'ImageMath: Rescaling FA template\')\n";

    if( m_useGridProcess )
    {
      Script = Script + "  TestGridProcess( FilesFolder, 0, 0) # stays in the function until all process is done : 0 makes the function look only for \'file\'\n";
    }
    Script = Script + "else : print(\"=> The file \\'\" + RescaleTemp + \"\\' already exists so the command will not be executed\")\n";

    Script = Script + "AtlasFAref= RescaleTemp\n\n";

  } // if(m_RegType==0)

  else // if no given template, create the template by computing FA of case 1 (+ cropping if needed)
  {
    Script = Script + "print(\"\")\n";
    Script = Script + "# Creating template by processing Case 1 DTI\n";

    // Filter DTI
    Script = Script + "\n# Filter case 1 DTI\n";
    Script = Script + "FilteredDTI= OutputPath + \"/" + m_CasesIDs[0] + "_filteredDTI.nrrd\"\n";
    Script = Script + "FilterDTICommand= \"" + m_SoftPath[1] + " \" + allcases[0] + \" \" + FilteredDTI + \" --correction zero\"\n";
    Script = Script + "print(\"[" + m_CasesIDs[0] + "] [Filter DTI] => $ \" + FilterDTICommand)\n";
    if(m_Overwrite==1)
    {
      Script = Script + "if 1 :\n";
    }
    else
    {
      Script = Script + "if not CheckFileExists(FilteredDTI, 0, \"" + m_CasesIDs[0] + "\" ) :\n";
    }
    if( ! m_useGridProcess )
    {
      Script = Script + "  if os.system(FilterDTICommand)!=0 : DisplayErrorAndQuit(\'[" + m_CasesIDs[0] + "] ResampleDTIlogEuclidean: Filter DTI to remove negative values\')\n";
    }
    Script = Script + "else : print(\"=> The file \\'\" + FilteredDTI + \"\\' already exists so the command will not be executed\")\n"; // not used if overwrite because "if 1 :"

    // Cropping case 1 DTI image
    if(m_NeedToBeCropped==1)
    {
      Script = Script + "\n# Cropping case 1 DTI\n";
      Script = Script + "croppedDTI= OutputPath + \"/" + m_CasesIDs[0] + "_croppedDTI.nrrd\"\n";
      Script = Script + "CropCommand= \"" + m_SoftPath[2] + " \" + FilteredDTI + \" -o \" + croppedDTI + \" -size " + m_CropSize[0] + "," + m_CropSize[1] + "," + m_CropSize[2] + " -v\"\n";
      Script = Script + "print(\"[" + m_CasesIDs[0] + "] [Cropping DTI Image] => $ \" + CropCommand)\n";
      if(m_Overwrite==1)
      {
        Script = Script + "if 1 :\n";
      }
      else
      {
        Script = Script + "if not CheckFileExists(croppedDTI, 0, \"" + m_CasesIDs[0] + "\" ) :\n";
      }
      if( ! m_useGridProcess )
      {
        Script = Script + "  if os.system(CropCommand)!=0 : DisplayErrorAndQuit(\'[" + m_CasesIDs[0] + "] CropDTI: Cropping DTI image\')\n";
      }
      Script = Script + "else : print(\"=> The file \\'\" + croppedDTI + \"\\' already exists so the command will not be executed\")\n"; // not used if overwrite because "if 1 :"
    } //if(m_NeedToBeCropped==1)

    // Generating case 1 FA 
    Script = Script + "\n# Generating case 1 FA\n";
    if(m_NeedToBeCropped==1)
    {
      Script = Script + "DTI= OutputPath + \"/" + m_CasesIDs[0] + "_croppedDTI.nrrd\"\n";
    }
    else
    {
      Script = Script + "DTI= allcases[0]\n";
    }
    Script = Script + "FA= OutputPath + \"/" + m_CasesIDs[0] + "_FA.nrrd\"\n";
    Script = Script + "GeneFACommand= \"" + m_SoftPath[3] + " --dti_image \" + DTI + \" -f \" + FA\n";
    Script = Script + "print(\"[" + m_CasesIDs[0] + "] [Generating FA] => $ \" + GeneFACommand)\n";

    if(m_Overwrite==1)
    {
      Script = Script + "if 1 :\n";
    }
    else
    {
      Script = Script + "if not CheckFileExists(FA, 0, \"" + m_CasesIDs[0] + "\" ) :\n";
    }
    if( ! m_useGridProcess )
    {
      Script = Script + "  if os.system(GeneFACommand)!=0 : DisplayErrorAndQuit(\'[" + m_CasesIDs[0] + "] dtiprocess: Generating FA of DTI image\')\n";
    }
    Script = Script + "else : print(\"=> The file \\'\" + FA + \"\\' already exists so the command will not be executed\")\n\n"; // not used if overwrite because "if 1 :"

    // Execute commands here if grid processing (to process them together)
    if( m_useGridProcess )
    {
      Script = Script + "# Run Case1 template commands on grid\n";
      Script = Script + "if CropDTICase1Template or GeneFACase1Template :\n";
      Script = Script + "  GridCase1TemplateCommand= \"" + m_GridCommand + " " + m_PythonPath + " " + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.py \" + FilesFolder + \"/file\"\n";
      Script = Script + "  GridCase1TemplateCommand = GridCase1TemplateCommand + \" \'\" + FilterDTICommand + \"\'\"\n";
      if(m_NeedToBeCropped==1)
      {
        Script = Script + "  GridCase1TemplateCommand = GridCase1TemplateCommand + \" \'\" + CropCommand + \"\'\"\n";
      }
      Script = Script + "  GridCase1TemplateCommand = GridCase1TemplateCommand + \" \'\" + GeneFACommand + \"\'\"\n";
      Script = Script + "  print(\"[" + m_CasesIDs[0] + "] => Submitting : \" + GridCase1TemplateCommand)\n";
      Script = Script + "  if os.system(GridCase1TemplateCommand)!=0 : DisplayErrorAndQuit(\'[" + m_CasesIDs[0] + "] Grid processing script\') # Run script and collect error if so\n";
      Script = Script + "  TestGridProcess( FilesFolder, 0, 0) # stays in the function until all process is done : 0 makes the function look only for \'file\'\n\n";
    }
  } // if case 1 as reference ( else of if(m_RegType==0) )

  Script = Script + "print(\"\")\n";

/* Affine Registration and Normalization Loop */
  Script = Script + "\n# Affine Registration and Normalization Loop\n";
  Script = Script + "n = 0\n";
  Script = Script + "while n <= " + IntToStr(m_nbLoops) + " :\n";

  Script = Script + "  if not os.path.isdir(OutputPath + \"/Loop\" + str(n)):\n";
  Script = Script + "    print(\"\\n=> Creation of the Output directory for Loop \" + str(n) + \" = \" + OutputPath + \"/Loop\" + str(n) + \"\\n\")\n";
  Script = Script + "    os.mkdir(OutputPath + \"/Loop\" + str(n))\n\n";

  Script = Script + "  # Cases Loop\n";
  if(m_RegType==1)
  {
    Script = Script + "  case = (n==0) # (n==0) -> bool: =1(true) =0(false) : the first case is the reference for the first loop so it will not be normalized or registered (it is cropped and FAed before the loop)\n";
  }
  else
  {
    Script = Script + "  case = 0\n";
  }
  Script = Script + "  while case < len(allcases):\n";
  if( m_useGridProcess )
  {
    Script = Script + "    GridProcessCaseCommandsArray=[] # Empty the cmds array\n";
  }

  Script = Script + "\n    if n==0: # Filtering and Cropping DTI and Generating FA are only part of the first loop\n";
/* Filter DTI */
  Script = Script + "# Filter DTI\n";
  Script = Script + "      # ResampleDTIlogEuclidean does by default a correction of tensor values by setting the negative values to zero\n";
  Script = Script + "      FilteredDTI= OutputPath + \"/\" + allcasesIDs[case] + \"_filteredDTI.nrrd\"\n";
  Script = Script + "      FilterDTICommand= \"" + m_SoftPath[1] + " \" + allcases[case] + \" \" + FilteredDTI + \" --correction zero\"\n";
  Script = Script + "      print(\"[\" + allcasesIDs[case] + \"] [Filter DTI] => $ \" + FilterDTICommand)\n";

  Script = Script + pyExecuteCommandPreprocessCase("FilteredDTI", "FilterDTICommand", "ResampleDTIlogEuclidean: Filter DTI to remove negative values", "      ");

/* Cropping DTI image */
  if(m_NeedToBeCropped==1)
  {
    Script = Script + "\n# Cropping DTI image\n";
    Script = Script + "      croppedDTI= OutputPath + \"/\" + allcasesIDs[case] + \"_croppedDTI.nrrd\"\n";
    Script = Script + "      CropCommand= \"" + m_SoftPath[2] + " \" + FilteredDTI + \" -o \" + croppedDTI + \" -size " + m_CropSize[0] + "," + m_CropSize[1] + "," + m_CropSize[2] + " -v\"\n";
    Script = Script + "      print(\"[\" + allcasesIDs[case] + \"] [Cropping DTI Image] => $ \" + CropCommand)\n";
    
    Script = Script + pyExecuteCommandPreprocessCase("croppedDTI", "CropCommand", "CropDTI: Cropping DTI image", "      ");

  } //if(m_NeedToBeCropped==1)

/* Generating FA */
  Script = Script + "\n# Generating FA\n";
  if(m_NeedToBeCropped==1)
  {
    Script = Script + "      DTI= OutputPath + \"/\" + allcasesIDs[case] + \"_croppedDTI.nrrd\"\n";
  }
  else
  {
    Script = Script + "      DTI= allcases[case]\n";
  }
  Script = Script + "      FA= OutputPath + \"/\" + allcasesIDs[case] + \"_FA.nrrd\"\n";
  Script = Script + "      GeneFACommand= \"" + m_SoftPath[3] + " --dti_image \" + DTI + \" -f \" + FA\n";
  Script = Script + "      print(\"[\" + allcasesIDs[case] + \"] [Generating FA] => $ \" + GeneFACommand)\n";

  Script = Script + pyExecuteCommandPreprocessCase("FA", "GeneFACommand", "dtiprocess: Generating FA of DTI image", "      ");

/* Normalization */
  Script = Script + "\n# Normalization\n";
  Script = Script + "    FA= OutputPath + \"/\" + allcasesIDs[case] + \"_FA.nrrd\"\n";
  Script = Script + "    NormFA= OutputPath + \"/Loop\" + str(n) + \"/\" + allcasesIDs[case] + \"_Loop\" + str(n) + \"_NormFA.nrrd\"\n";
  Script = Script + "    NormFACommand= \"" + m_SoftPath[0] + " \" + FA + \" -outfile \" + NormFA + \" -matchHistogram \" + AtlasFAref\n";
  Script = Script + "    print(\"[LOOP \" + str(n) + \"/" + IntToStr(m_nbLoops) + "] [\" + allcasesIDs[case] + \"] [Normalization] => $ \" + NormFACommand)\n";

  Script = Script + pyExecuteCommandPreprocessCase("NormFA", "NormFACommand", "ImageMath: Normalizing FA image", "    ");

/* Affine registration with BrainsFit */
  Script = Script + "\n# Affine registration with BrainsFit\n";
  Script = Script + "    NormFA= OutputPath + \"/Loop\" + str(n) + \"/\" + allcasesIDs[case] + \"_Loop\" + str(n) + \"_NormFA.nrrd\"\n";
  Script = Script + "    LinearTranstfm= OutputPath + \"/Loop\" + str(n) + \"/\" + allcasesIDs[case] + \"_Loop\" + str(n) + \"_LinearTrans.txt\"\n";
  Script = Script + "    LinearTrans= OutputPath + \"/Loop\" + str(n) + \"/\" + allcasesIDs[case] + \"_Loop\" + str(n) + \"_LinearTrans_FA.nrrd\"\n";
  Script = Script + "    AffineCommand= \"" + m_SoftPath[4] + " --fixedVolume \" + AtlasFAref + \" --movingVolume \" + NormFA + \" --useAffine --outputVolume \" + LinearTrans + \" --outputTransform \" + LinearTranstfm\n";
  Script = Script + "    InitLinearTransTxt= OutputPath + \"/\" + allcasesIDs[case] + \"_InitLinearTrans.txt\"\n";
  Script = Script + "    InitLinearTransMat= OutputPath + \"/\" + allcasesIDs[case] + \"_InitLinearTrans.mat\"\n";
  Script = Script + "    if n==0 and CheckFileExists( InitLinearTransMat, case, allcasesIDs[case] ) and CheckFileExists( InitLinearTransTxt, case, allcasesIDs[case] ):\n";
  Script = Script + "      print(\"[WARNING] Both \\'\" + allcasesIDs[case] + \"_InitLinearTrans.mat\\' and \\'\" + allcasesIDs[case] + \"_InitLinearTrans.txt\\' have been found. The .mat file will be used.\")\n";
  Script = Script + "      AffineCommand= AffineCommand + \" --initialTransform \" + InitLinearTransMat\n";
  Script = Script + "    elif n==0 and CheckFileExists( InitLinearTransMat, case, allcasesIDs[case] ) : AffineCommand= AffineCommand + \" --initialTransform \" + InitLinearTransMat\n";
  Script = Script + "    elif n==0 and CheckFileExists( InitLinearTransTxt, case, allcasesIDs[case] ) : AffineCommand= AffineCommand + \" --initialTransform \" + InitLinearTransTxt\n";
  Script = Script + "    else : AffineCommand= AffineCommand + \" --initializeTransformMode " + m_BFAffineTfmMode + "\"\n";

  Script = Script + "    print(\"[LOOP \" + str(n) + \"/" + IntToStr(m_nbLoops) + "] [\" + allcasesIDs[case] + \"] [Affine registration with BrainsFit] => $ \" + AffineCommand)\n";
  Script = Script + "    CheckFileExists( LinearTrans, case, allcasesIDs[case] ) # Not for checking but to rename _LinearTrans_FA if old version\n";
    
  Script = Script + pyExecuteCommandPreprocessCase("LinearTranstfm", "AffineCommand", "BRAINSFit: Affine Registration of FA image", "    ");

/* Implementing the affine registration */
  Script = Script + "\n# Implementing the affine registration\n";
  Script = Script + "    LinearTranstfm= OutputPath + \"/Loop\" + str(n) + \"/\" + allcasesIDs[case] + \"_Loop\" + str(n) + \"_LinearTrans.txt\"\n";
  Script = Script + "    LinearTransDTI= OutputPath + \"/Loop\" + str(n) + \"/\" + allcasesIDs[case] + \"_Loop\" + str(n) + \"_LinearTrans_DTI.nrrd\"\n";
  if(m_NeedToBeCropped==1)
  {
    Script = Script + "    originalDTI= OutputPath + \"/\" + allcasesIDs[case] + \"_croppedDTI.nrrd\"\n";
  }
  else
  {
    Script = Script + "    originalDTI= allcases[case]\n";
  }
  Script = Script + "    ImplementCommand= \"" + m_SoftPath[1] + " \" + originalDTI + \" \" + LinearTransDTI + \" -f \" + LinearTranstfm + \" -R \" + AtlasFAref\n";
  Script = Script + "    print(\"[LOOP \" + str(n) + \"/" + IntToStr(m_nbLoops) + "] [\" + allcasesIDs[case] + \"] [Implementing the Affine registration] => $ \" + ImplementCommand)\n";

  Script = Script + pyExecuteCommandPreprocessCase("LinearTransDTI", "ImplementCommand", "ResampleDTIlogEuclidean: Implementing the Affine Registration on FA image", "    ");

/* Generating FA of registered images */
  Script = Script + "\n# Generating FA of registered images\n";
  Script = Script + "    LinearTransDTI= OutputPath + \"/Loop\" + str(n) + \"/\" + allcasesIDs[case] + \"_Loop\" + str(n) + \"_LinearTrans_DTI.nrrd\"\n";
  Script = Script + "    if n == " + IntToStr(m_nbLoops) + " : LoopFA= OutputPath + \"/Loop" + IntToStr(m_nbLoops) + "/\" + allcasesIDs[case] + \"_Loop" + IntToStr(m_nbLoops) + "_FinalFA.nrrd\" # the last FA will be the Final output\n";
  Script = Script + "    else : LoopFA= OutputPath + \"/Loop\" + str(n) + \"/\" + allcasesIDs[case] + \"_Loop\" + str(n) + \"_FA.nrrd\"\n";
  Script = Script + "    GeneLoopFACommand= \"" + m_SoftPath[3] + " --dti_image \" + LinearTransDTI + \" -f \" + LoopFA\n";
  Script = Script + "    print(\"[LOOP \" + str(n) + \"/" + IntToStr(m_nbLoops) + "] [\" + allcasesIDs[case] + \"] [Generating FA of registered images] => $ \" + GeneLoopFACommand)\n";

  Script = Script + pyExecuteCommandPreprocessCase("LoopFA", "GeneLoopFACommand", "dtiprocess: Generating FA of affine registered images", "    ");

/* Run grid process command for case X, containing all operations */
  if( m_useGridProcess )
  {
    Script = Script + "\n# Run grid process command for case X, containing all operations\n";
    Script = Script + "    if len(GridProcessCaseCommandsArray)!=0 : # There are operations to run\n";

    Script = Script + "      GridAffineCommand= \"" + m_GridCommand + " " + m_PythonPath + " " + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.py \" + FilesFolder + \"/Case\" + str(case+1)\n";

    Script = Script + "      GridCmd = 0\n";
    Script = Script + "      while GridCmd < len(GridProcessCaseCommandsArray):\n";
    Script = Script + "        GridAffineCommand = GridAffineCommand + \" \'\" + GridProcessCaseCommandsArray[GridCmd] + \"\'\"\n";
    Script = Script + "        GridCmd += 1\n";

    Script = Script + "      print(\"[LOOP \" + str(n) + \"/" + IntToStr(m_nbLoops) + "] [\" + allcasesIDs[case] + \"] => Submitting : \" + GridAffineCommand)\n";
    Script = Script + "      if os.system(GridAffineCommand)!=0 : # Run script and collect error if so\n";
    Script = Script + "        DisplayErrorAndQuit(\'[Loop \' + str(n) + \'][\' + allcasesIDs[case] + \'] Grid processing script\')\n";

    Script = Script + "    else : # No operations to run for this case\n";
    Script = Script + "      print(\"=> No operations to run for case \" + str(case+1))\n";
    Script = Script + "      f = open( FilesFolder + \"/Case\" + str(case+1), 'w')\n";
    Script = Script + "      f.close()\n";
  } // if( m_useGridProcess )

  Script = Script + "\n    print(\"\")\n";
  Script = Script + "    case += 1 # indenting cases loop\n\n";

  if( m_useGridProcess )
  {
    Script = Script + "  TestGridProcess( FilesFolder, len(allcases), " + NoCase1 + "*(n==0)) # stays in the function until all process is done\n\n";
  }

/* FA Average of registered images with ImageMath */
  Script = Script + "\n# FA Average of registered images with ImageMath\n";
  if ( m_nbLoops!=0 ) // if no looping, compute average for the only preprocessing for QC (if 1:)
  {
    Script = Script + "  if n != " + IntToStr(m_nbLoops) + " : # this will not be done for the last lap\n";
  }
  else
  {
    Script = Script + "  if 1 :\n";
  }
  Script = Script + "    FAAverage = OutputPath + \"/Loop\" + str(n) + \"/Loop\" + str(n) + \"_FAAverage.nrrd\"\n";
  if(m_RegType==1) //use case 1 as loop 1 ref
  {
    Script = Script + "    if n == 0 : FAforAVG= OutputPath + \"/" + m_CasesIDs[0] + "_FA.nrrd\"\n";
    Script = Script + "    else : FAforAVG= OutputPath + \"/Loop\" + str(n) + \"/" + m_CasesIDs[0] + "_Loop\" + str(n) + \"_FA.nrrd\"\n";
  }
  else
  {
    Script = Script + "    FAforAVG= OutputPath + \"/Loop\" + str(n) + \"/" + m_CasesIDs[0] + "_Loop\" + str(n) + \"_FA.nrrd\"\n";
  }

  Script = Script + "    AverageCommand = \"" + m_SoftPath[0] + " \" + FAforAVG + \" -outfile \" + FAAverage + \" -avg \"\n";
  Script = Script + "    case = 1\n";
  Script = Script + "    while case < len(allcases):\n";
  Script = Script + "      FAforAVG= OutputPath + \"/Loop\" + str(n) + \"/\" + allcasesIDs[case] + \"_Loop\" + str(n) + \"_FA.nrrd \"\n";
  Script = Script + "      AverageCommand= AverageCommand + FAforAVG\n";
  Script = Script + "      case += 1\n";

  if( m_useGridProcess )
  {
    Script = Script + "    AverageCommand= \"" + m_GridCommand + " " + m_PythonPath + " " + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.py \" + FilesFolder + \"/file \\'\" + AverageCommand  + \"\\'\"\n";
  }
  Script = Script + "    print(\"[LOOP \" + str(n) + \"/" + IntToStr(m_nbLoops) + "] [Computing FA Average of registered images] => $ \" + AverageCommand)\n";

  if(m_Overwrite==1)
  {
    Script = Script + "    if 1 :\n";
  }
  else
  {
    Script = Script + "    if not CheckFileExists(FAAverage, 0, \"\") :\n";
  }
  Script = Script + "      if os.system(AverageCommand)!=0 : DisplayErrorAndQuit(\'[Loop \' + str(n) + \'] dtiaverage: Computing FA Average of registered images\')\n";
  if( m_useGridProcess )
  {
    Script = Script + "      TestGridProcess( FilesFolder, 0, 0) # stays in the function until all process is done : 0 makes the function look for \'file\'\n";
  }
  Script = Script + "    else :\n"; // not used if overwrite because "if 1 :"
  Script = Script + "      print(\"=> The file \\'\" + FAAverage + \"\\' already exists so the command will not be executed\")\n";

  Script = Script + "    AtlasFAref = FAAverage # the average becomes the reference\n\n";

  Script = Script + "  print(\"\")\n";
  Script = Script + "  n += 1 # indenting main loop\n\n";

  Script = Script + "print(\"\\n============ End of Pre processing =============\")\n\n";

  Script = Script + "sys.exit(0)\n";

  m_Script_Preprocess=Script;
}

  /////////////////////////////////////////
 //           ATLAS BUILDING            //
/////////////////////////////////////////

void ScriptWriter::AtlasBuilding()
{
  std::string Script;

  std::cout<<"[AtlasBuilding]"; // command line display (no endl)

  Script = Script + "#!/usr/bin/python\n\n";
  Script = Script + "import os # To run a shell command : os.system(\"[shell command]\")\n";
  Script = Script + "import sys # to return an exit code\n";
  Script = Script + "import shutil # to remove a non empty directory and copy files\n\n";

  Script = Script + "PIDlogFile = \"" + m_OutputPath + "/DTIAtlas/Script/PID.log\"\n";
  Script = Script + "PIDfile = open( PIDlogFile, 'a') # open in Append mode\n";
  Script = Script + "PIDfile.write( str(os.getpid()) + \"\\n\" )\n";
  Script = Script + "PIDfile.close()\n\n";

  Script = Script + "print(\"\\n============ Atlas Building =============\")\n\n";

  Script = Script + "# Files Paths\n";
  Script = Script + "DeformPath= \"" + m_OutputPath + "/DTIAtlas/2_NonLinear_Registration\"\n";
  Script = Script + "AffinePath= \"" + m_OutputPath + "/DTIAtlas/1_Affine_Registration\"\n";
  Script = Script + "FinalPath= \"" + m_OutputPath + "/DTIAtlas/3_Diffeomorphic_Atlas\"\n";
  Script = Script + "FinalResampPath= \"" + m_OutputPath + "/DTIAtlas/4_Final_Resampling\"\n\n";

  /* Function to display error and quit */
  Script = Script + "def DisplayErrorAndQuit ( Error ):\n";
  Script = Script + "  print \'\\n\\nERROR DETECTED IN WORKFLOW:\',Error\n";
  Script = Script + "  print \'ABORT\'\n";
  Script = Script + "  sys.exit(1)\n\n";

  /* Function that checks if file exist and replace old names by new names if needed */
  Script = Script + pyCheckFileExists();

/* Call script to run command on grid */
  std::string GridApostrophe = "";
  std::string GridProcessCmd = "";
  std::string GridProcessFileExistCmd1 = "";
  std::string GridProcessCmdNoCase = "";
  std::string GridProcessFileExistCmdNoCase = "";
  std::string GridProcessFileExistIndent = "";
  std::string GridProcessFileExistIndent1 = "";
  if( m_useGridProcess )
  {
    Script = Script + "# Call script to run command on server\n";
    Script = Script + "import time # To test existence of files only every minute\n";
    Script = Script + "FilesFolder = \"" + m_OutputPath + "/DTIAtlas/GridProcessingFiles\"\n";

    //Test Function
    Script = Script + pyTestGridProcess( false ); // bool NoCase1

    GridApostrophe = " + \"\\'\"";
    std::string File = "FilesFolder + \"/Case\" + str(case+1)";
    GridProcessCmd = "\"" + m_GridCommand + " " + m_PythonPath + " " + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.py \" + " + File + " + \" \"" + GridApostrophe + " + ";
    GridProcessFileExistCmd1 = "    f = open( " + File + ", 'w')\n    f.close()\n"; // if the image already exists, create the "semaphore" file

    std::string FileNoCase = "FilesFolder + \"/file\""; //  for the commands executed only once = not once per case
    GridProcessCmdNoCase = "\"" + m_GridCommand + " " + m_PythonPath + " " + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.py \" + " + FileNoCase + " + \" \"" + GridApostrophe + " + ";
    GridProcessFileExistCmdNoCase = "  f = open( " + FileNoCase + ", 'w')\n  f.close()\n";

    GridProcessFileExistIndent = "\n  "; // if the file already exists, several commands in the else (create file), so change line and indent
    GridProcessFileExistIndent1 = "\n    "; // if the file already exists, several commands in the else (create file), so change line and indent (double)

  }

/* Create directory for temporary files and final */

  Script = Script + "# Create directory for temporary files and final\n";
  Script = Script + "if not os.path.isdir(DeformPath):\n";
  Script = Script + "  OldDeformPath= \"" + m_OutputPath + "/DTIAtlas/2_NonLinear_Registration_AW\"\n";
  Script = Script + "  if os.path.isdir(OldDeformPath):\n";
  Script = Script + "    os.rename(OldDeformPath,DeformPath)\n";
  Script = Script + "  else:\n";
  Script = Script + "    print(\"\\n=> Creation of the Deformation transform directory = \" + DeformPath)\n";
  Script = Script + "    os.mkdir(DeformPath)\n\n";

  Script = Script + "if not os.path.isdir(FinalPath):\n";
  Script = Script + "  OldFinalPath= \"" + m_OutputPath + "/DTIAtlas/3_AW_Atlas\"\n";
  Script = Script + "  if os.path.isdir(OldFinalPath):\n";
  Script = Script + "    os.rename(OldFinalPath,FinalPath)\n";
  Script = Script + "  else:\n";
  Script = Script + "    print(\"\\n=> Creation of the Final Atlas directory = \" + FinalPath)\n";
  Script = Script + "    os.mkdir(FinalPath)\n\n";

  Script = Script + "if not os.path.isdir(FinalResampPath):\n";
  Script = Script + "  print(\"\\n=> Creation of the Final Resampling directory = \" + FinalResampPath)\n";
  Script = Script + "  os.mkdir(FinalResampPath)\n\n";

  Script = Script + "if not os.path.isdir(FinalResampPath + \"/First_Resampling\"):\n";
  Script = Script + "  print(\"\\n=> Creation of the First Final Resampling directory = \" + FinalResampPath + \"/First_Resampling\")\n";
  Script = Script + "  os.mkdir(FinalResampPath + \"/First_Resampling\")\n\n";

  Script = Script + "if not os.path.isdir(FinalResampPath + \"/Second_Resampling\"):\n";
  Script = Script + "  print(\"\\n=> Creation of the Second Final Resampling directory = \" + FinalResampPath + \"/Second_Resampling\")\n";
  Script = Script + "  os.mkdir(FinalResampPath + \"/Second_Resampling\")\n\n";

  Script = Script + "if not os.path.isdir(FinalResampPath + \"/FinalTensors\"):\n";
  Script = Script + "  print(\"\\n=> Creation of the Final Tensors directory = \" + FinalResampPath + \"/FinalTensors\")\n";
  Script = Script + "  os.mkdir(FinalResampPath + \"/FinalTensors\")\n\n";

  Script = Script + "if not os.path.isdir(FinalResampPath + \"/FinalDeformationFields\"):\n";
  Script = Script + "  print(\"\\n=> Creation of the Final Deformation Fields directory = \" + FinalResampPath + \"/FinalDeformationFields\\n\")\n";
  Script = Script + "  os.mkdir(FinalResampPath + \"/FinalDeformationFields\")\n\n";

/* Cases variables: */
  Script = Script + "# Cases variables\n";
  // alltfms
  Script = Script + "alltfms = [AffinePath + \"/Loop" + IntToStr(m_nbLoops) + "/" + m_CasesIDs[0] + "_Loop" + IntToStr(m_nbLoops) + "_LinearTrans.txt\"";
  for (unsigned int i=1;i<m_CasesPath.size();i++) 
  {
    Script = Script + ", AffinePath + \"/Loop" + IntToStr(m_nbLoops) + "/" + m_CasesIDs[i] + "_Loop" + IntToStr(m_nbLoops) + "_LinearTrans.txt\"";
  }  
  Script = Script+ "]\n\n";

  // allcases
  if(m_NeedToBeCropped==1)
  {
    Script = Script + "allcases = [AffinePath + \"/" + m_CasesIDs[0] + "_croppedDTI.nrrd\"";
  }
  else
  {
    Script = Script + "allcases = [\"" + m_CasesPath[0] + "\"";
  }
  for (unsigned int i=1;i<m_CasesPath.size();i++) 
  {
    if(m_NeedToBeCropped==1)
    {
      Script = Script + ", AffinePath + \"/" + m_CasesIDs[i] + "_croppedDTI.nrrd\"";
    }
    else Script = Script + ", \"" + m_CasesPath[i] + "\"";
  }
  Script = Script + "]\n\n";  

  // allcasesIDs
  Script = Script + "allcasesIDs = [\"" + m_CasesIDs[0];
  for (unsigned int i=1;i<m_CasesIDs.size();i++)
  {
    Script = Script + "\", \"" + m_CasesIDs[i];
  }
  Script = Script+ "\"]\n\n";


/* GreedyAtlas Command: */
  Script = Script + "# GreedyAtlas Command\n";
  Script = Script + "XMLFile= DeformPath + \"/GreedyAtlasParameters.xml\"\n";
  Script = Script + "ParsedFile= DeformPath + \"/ParsedXML.xml\"\n";
  Script = Script + "AtlasBCommand= " + GridProcessCmdNoCase + "\"" + m_SoftPath[5] + " -f \" + XMLFile + \" -o \" + ParsedFile" + GridApostrophe + "\n";
  Script = Script + "print(\"[Computing the Deformation Fields with GreedyAtlas] => $ \" + AtlasBCommand)\n";
  if(m_Overwrite==1)
  {
    Script = Script + "if 1 :\n";
  }
  else
  {
    Script = Script + "if not CheckFileExists(DeformPath + \"/MeanImage.mhd\", 0, \"\") :\n";
  }
  Script = Script + "  if os.system(AtlasBCommand)!=0 : DisplayErrorAndQuit(\'GreedyAtlas: Computing non-linear atlas from affine registered images\')\n";
  if( m_useGridProcess )
  {
    Script = Script + "  TestGridProcess( FilesFolder, 0) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";
  }

  Script = Script + "  case = 0\n";
  Script = Script + "  while case < len(allcases): # Renaming\n";
  Script = Script + "    originalImage=DeformPath + \"/\" + allcasesIDs[case] + \"_Loop" + IntToStr(m_nbLoops) + "_FinalFADefToMean.mhd\"\n";
  Script = Script + "    originalHField=DeformPath + \"/\" + allcasesIDs[case] + \"_Loop" + IntToStr(m_nbLoops) + "_FinalFADefFieldImToMean.mhd\"\n";
  Script = Script + "    originalInvHField=DeformPath + \"/\" + allcasesIDs[case] + \"_Loop" + IntToStr(m_nbLoops) + "_FinalFADefFieldMeanToIm.mhd\"\n";
  Script = Script + "    NewImage= DeformPath + \"/\" + allcasesIDs[case] + \"_NonLinearTrans_FA.mhd\"\n";
  Script = Script + "    NewHField=DeformPath + \"/\" + allcasesIDs[case] + \"_HField.mhd\"\n";
  Script = Script + "    NewInvHField=DeformPath + \"/\" + allcasesIDs[case] + \"_InverseHField.mhd\"\n";
  Script = Script + "    print(\"[\" + allcasesIDs[case] + \"] => Renaming \\'\" + originalImage + \"\\' to \\'\" + NewImage + \"\\'\")\n";
  Script = Script + "    os.rename(originalImage,NewImage)\n";
  Script = Script + "    print(\"[\" + allcasesIDs[case] + \"] => Renaming \\'\" + originalHField + \"\\' to \\'\" + NewHField + \"\\'\")\n";
  Script = Script + "    os.rename(originalHField,NewHField)\n";
  Script = Script + "    print(\"[\" + allcasesIDs[case] + \"] => Renaming \\'\" + originalInvHField + \"\\' to \\'\" + NewInvHField + \"\\'\")\n";
  Script = Script + "    os.rename(originalInvHField,NewInvHField)\n";
  Script = Script + "    case += 1\n";
  if(m_Overwrite==0)
  {
    Script = Script + "else :\n";
    Script = Script + "  print(\"=> The file \\'\" + DeformPath + \"/MeanImage.mhd\\' already exists so the command will not be executed\")\n";
    Script = Script + "  # Renaming possible existing old named files from GreedyAtlas\n";
    Script = Script + "  case = 0\n";
    Script = Script + "  while case < len(allcases): # Updating old names if needed\n";
    Script = Script + "    NewImage= DeformPath + \"/\" + allcasesIDs[case] + \"_NonLinearTrans_FA.mhd\"\n";
    Script = Script + "    CheckFileExists(NewImage, case, allcasesIDs[case])\n";
    Script = Script + "    NewHField=DeformPath + \"/\" + allcasesIDs[case] + \"_HField.mhd\"\n";
    Script = Script + "    CheckFileExists(NewHField, case, allcasesIDs[case])\n";
    Script = Script + "    NewInvHField=DeformPath + \"/\" + allcasesIDs[case] + \"_InverseHField.mhd\"\n";
    Script = Script + "    CheckFileExists(NewInvHField, case, allcasesIDs[case])\n";
    Script = Script + "    case += 1\n";
  }

/* Apply deformation fields */
  Script = Script + "\n# Apply deformation fields\n";
  if( m_useGridProcess )
  {
    Script = Script + "GridProcessCommandsArray=[] # Empty the cmds array\n";
    Script = Script + "NbGridCommandsRan = 0\n";
  }
  Script = Script + "case = 0\n";
  Script = Script + "while case < len(allcases):\n";
  Script = Script + "  FinalDTI= FinalPath + \"/\" + allcasesIDs[case] + \"_DiffeomorphicDTI.nrrd\"\n";
  if(m_NeedToBeCropped==1)
  {
    Script = Script + "  originalDTI= AffinePath + \"/\" + allcasesIDs[case] + \"_croppedDTI.nrrd\"\n";
  }
  else
  {
    Script = Script + "  originalDTI= allcases[case]\n";
  }
  if(m_nbLoops==0)
  {
    Script = Script + "  Ref = AffinePath + \"/Loop0/Loop0_FAAverage.nrrd\"\n"; // if no looping ((m_nbLoops==0), an average is computed anyway for QC
  }
  else
  {
    Script = Script + "  Ref = AffinePath + \"/Loop" + IntToStr(m_nbLoops-1) + "/Loop" + IntToStr(m_nbLoops-1) + "_FAAverage.nrrd\"\n"; // an average image has been generated in the loops of affine reg for reference
  }
  Script = Script + "  HField= DeformPath + \"/\" + allcasesIDs[case] + \"_HField.mhd\"\n";
  Script = Script + "  FinalReSampCommand=\"" + m_SoftPath[1] + " -R \" + Ref + \" -H \" + HField + \" -f \" + alltfms[case] + \" \" + originalDTI + \" \" + FinalDTI\n";

  /* options */
  if(m_InterpolType.compare("Linear")==0) Script = Script + "  FinalReSampCommand = FinalReSampCommand + \" -i linear\"\n";
  if(m_InterpolType.compare("Nearest Neighborhoor")==0) Script = Script + "  FinalReSampCommand = FinalReSampCommand + \" -i nn\"\n";
  if(m_InterpolType.compare("Windowed Sinc")==0)
  {
    if(m_InterpolOption.compare("Hamming")==0) Script = Script + "  FinalReSampCommand = FinalReSampCommand + \" -i ws -W h\"\n";
    if(m_InterpolOption.compare("Cosine")==0) Script = Script + "  FinalReSampCommand = FinalReSampCommand + \" -i ws -W c\"\n";
    if(m_InterpolOption.compare("Welch")==0) Script = Script + "  FinalReSampCommand = FinalReSampCommand + \" -i ws -W w\"\n";
    if(m_InterpolOption.compare("Lanczos")==0) Script = Script + "  FinalReSampCommand = FinalReSampCommand + \" -i ws -W l\"\n";
    if(m_InterpolOption.compare("Blackman")==0) Script = Script + "  FinalReSampCommand = FinalReSampCommand + \" -i ws -W b\"\n";
  }
  if(m_InterpolType.compare("BSpline")==0)
  {
    std::istringstream istr(m_InterpolOption);
    int i;
    istr >> i;
    if(i>=0 && i<=5) Script = Script + "  FinalReSampCommand = FinalReSampCommand + \" -i bs -o " + m_InterpolOption + "\"\n";
  }
  if(m_TensInterpol.compare("Non Log Euclidean")==0)
  {
    if(m_InterpolLogOption.compare("Zero")==0) Script = Script + "  FinalReSampCommand = FinalReSampCommand + \" --nolog --correction zero\"\n";
    if(m_InterpolLogOption.compare("None")==0) Script = Script + "  FinalReSampCommand = FinalReSampCommand + \" --nolog --correction none\"\n";
    if(m_InterpolLogOption.compare("Absolute Value")==0) Script = Script + "  FinalReSampCommand = FinalReSampCommand + \" --nolog --correction abs\"\n";
    if(m_InterpolLogOption.compare("Nearest")==0) Script = Script + "  FinalReSampCommand = FinalReSampCommand + \" --nolog --correction nearest\"\n";
  }
  if(m_TensTfm.compare("Preservation of the Principal Direction (PPD)")==0) Script = Script + "  FinalReSampCommand = FinalReSampCommand + \" -T PPD\"\n";
  if(m_TensTfm.compare("Finite Strain (FS)")==0) Script = Script + "  FinalReSampCommand = FinalReSampCommand + \" -T FS\"\n";

  Script = Script + "  print(\"\\n[\" + allcasesIDs[case] + \"] [Applying deformation fields to original DTIs] => $ \" + FinalReSampCommand)\n";
  if(m_Overwrite==1)
  {
    Script = Script + "  if 1 :\n";
  }
  else
  {
    Script = Script + "  if not CheckFileExists(FinalDTI, case, allcasesIDs[case]) :\n";
  }

  Script = Script + "    DiffeomorphicCaseFA = FinalPath + \"/\" + allcasesIDs[case] + \"_DiffeomorphicFA.nrrd\"\n";
  Script = Script + "    GeneDiffeomorphicCaseFACommand=\"" + m_SoftPath[3] + " --scalar_float --dti_image \" + FinalDTI + \" -f \" + DiffeomorphicCaseFA\n";

  Script = Script + "    CaseDbleToFloatCommand=\"" + m_SoftPath[8] + " convert -t float -i \" + FinalDTI + \" | " + m_SoftPath[8] + " save -f nrrd -e gzip -o \" + FinalPath + \"/\" + allcasesIDs[case] + \"_DiffeomorphicDTI_float.nrrd\"\n\n";

  if( ! m_useGridProcess )
  {
    Script = Script + "    if os.system(FinalReSampCommand)!=0 : DisplayErrorAndQuit(\'[\' + allcasesIDs[case] + \'] ResampleDTIlogEuclidean: Applying deformation fields to original DTIs\')\n";

    Script = Script + "    print(\"[\" + allcasesIDs[case] + \"] => $ \" + GeneDiffeomorphicCaseFACommand)\n";
    Script = Script + "    if os.system(GeneDiffeomorphicCaseFACommand)!=0 : DisplayErrorAndQuit(\'[\' + allcasesIDs[case] + \'] dtiprocess: Computing Diffeomorphic FA\')\n";

    Script = Script + "    print(\"[\" + allcasesIDs[case] + \"] => $ \" + CaseDbleToFloatCommand + \"\\n\")\n";
    Script = Script + "    if os.system(CaseDbleToFloatCommand)!=0 : DisplayErrorAndQuit(\'[\' + allcasesIDs[case] + \'] unu: Converting the final DTI images from double to float DTI\')\n";
  }
  else // run up to 50 commands in the same script
  {
    Script = Script + "    GridProcessCommandsArray.append(FinalReSampCommand)\n";
    Script = Script + "    GridProcessCommandsArray.append(GeneDiffeomorphicCaseFACommand)\n";
    Script = Script + "    GridProcessCommandsArray.append(CaseDbleToFloatCommand)\n";

    Script = Script + "    if len(GridProcessCommandsArray)>=50 or case==len(allcases)-1 : # launch a script if more than 50 operations or if last case\n";
    Script = Script + "      GridProcessCmd= \"" + m_GridCommand + " " + m_PythonPath + " " + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.py \" + FilesFolder + \"/Case\" + str(NbGridCommandsRan+1)\n";
    Script = Script + "      GridCmd = 0\n";
    Script = Script + "      while GridCmd < len(GridProcessCommandsArray):\n";
    Script = Script + "        GridProcessCmd = GridProcessCmd + \" \'\" + GridProcessCommandsArray[GridCmd] + \"\'\"\n";
    Script = Script + "        GridCmd += 1\n";
    Script = Script + "      GridProcessCommandsArray=[] # Empty the cmds array\n";
    Script = Script + "      NbGridCommandsRan += 1\n";
    Script = Script + "      if os.system(GridProcessCmd)!=0 : # Run script and collect error if so\n";
    Script = Script + "        DisplayErrorAndQuit(\'[] Applying deformation fields to original DTIs\')\n";
  }

  Script = Script + "  else : print(\"=> The file \\'\" + FinalDTI + \"\\' already exists so the command will not be executed\")\n"; // not used if overwrite because "if 1 :"

  Script = Script + "  case += 1\n\n";

  if( m_useGridProcess )
  {
    Script = Script + "if NbGridCommandsRan!=0 : TestGridProcess( FilesFolder, NbGridCommandsRan ) # stays in the function until all process is done : 0 cmds makes the function look for \'file\'\n\n";
  }

/* dtiaverage computing */
  Script = Script + "# DTIaverage computing\n";
  Script = Script + "DTIAverage = FinalPath + \"/DiffeomorphicAtlasDTI.nrrd\"\n";
  Script = Script + "AverageCommand = \"" + m_SoftPath[6] + " \"\n";
  Script = Script + "case = 0\n";
  Script = Script + "while case < len(allcases):\n";
  Script = Script + "  DTIforAVG= \"--inputs \" + FinalPath + \"/\" + allcasesIDs[case] + \"_DiffeomorphicDTI.nrrd \"\n";
  Script = Script + "  AverageCommand = AverageCommand + DTIforAVG\n";
  Script = Script + "  case += 1\n";
  Script = Script + "AverageCommand = AverageCommand + \"--tensor_output \" + DTIAverage\n";
  Script = Script + "print(\"\\n[Computing the Diffeomorphic DTI average] => $ \" + AverageCommand)\n";
  if(m_Overwrite==1)
  {
    Script = Script + "if 1 : \n";
  }
  else
  {
    Script = Script + "if not CheckFileExists(DTIAverage, 0, \"\") : \n";
  }

/* Computing some images from the final DTI with dtiprocess */
  Script = Script + "# Computing some images from the final DTI with dtiprocess\n";
  Script = Script + "  FA= FinalPath + \"/DiffeomorphicAtlasFA.nrrd\"\n";
  Script = Script + "  cFA= FinalPath + \"/DiffeomorphicAtlasColorFA.nrrd\"\n";
  Script = Script + "  RD= FinalPath + \"/DiffeomorphicAtlasRD.nrrd\"\n"; // Radial Diffusivity
  Script = Script + "  MD= FinalPath + \"/DiffeomorphicAtlasMD.nrrd\"\n"; // Mean Diffusivity
  Script = Script + "  AD= FinalPath + \"/DiffeomorphicAtlasAD.nrrd\"\n"; // Axial Diffusivity

  Script = Script + "  GeneFACommand=\"" + m_SoftPath[3] + " --scalar_float --dti_image \" + DTIAverage + \" -f \" + FA + \" -m \" + MD + \" --color_fa_output \" + cFA + \" --RD_output \" + RD + \" --lambda1_output \" + AD\n\n";

  Script = Script + "  DbleToFloatCommand=\"" + m_SoftPath[8] + " convert -t float -i \" + DTIAverage + \" | " + m_SoftPath[8] + " save -f nrrd -e gzip -o \" + FinalPath + \"/DiffeomorphicAtlasDTI_float.nrrd\"\n\n";

  if( ! m_useGridProcess )
  {
    Script = Script + "  if os.system(AverageCommand)!=0 : DisplayErrorAndQuit(\'dtiaverage: Computing the final DTI average\')\n";

    Script = Script + "  print(\"[Computing some images from the final DTI with dtiprocess] => $ \" + GeneFACommand)\n";
    Script = Script + "  if os.system(GeneFACommand)!=0 : DisplayErrorAndQuit(\'dtiprocess: Computing Diffeomorphic FA, color FA, MD, RD and AD\')\n";

    Script = Script + "  print(\"[Computing some images from the final DTI with dtiprocess] => $ \" + DbleToFloatCommand)\n";
    Script = Script + "  if os.system(DbleToFloatCommand)!=0 : DisplayErrorAndQuit(\'unu: Converting the final DTI atlas from double to float DTI\')\n";
  }
  else // run all commands in the same time in the script
  {
    Script = Script + "  AverageGridCommand=" + GridProcessCmdNoCase + "AverageCommand + \"\\' \" + \"\\'\" + GeneFACommand + \"\\' \" + \"\\'\" + DbleToFloatCommand + \"\\'\"\n";
    Script = Script + "  if os.system(AverageGridCommand)!=0 : DisplayErrorAndQuit(\'Computing the final DTI average\')\n";
  }

  if(m_Overwrite==0)
  {
    Script = Script + "else : " + GridProcessFileExistIndent + "print(\"=> The file \\'\" + DTIAverage + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmdNoCase;
  }

  if( m_useGridProcess )
  {
    Script = Script + "TestGridProcess( FilesFolder, 0 ) # stays in the function until all process is done : 0 makes the function look for \'file\'\n";
  }

/* Computing global deformation fields */
  Script = Script + "\n# Computing global deformation fields\n";
/* Threads
  if( m_DTIRegOptions[0].compare("ANTS")==0 )
  {

  Script = Script + "import thread\n";
  Script = Script + "nbCPUs = os.sysconf(\"SC_NPROCESSORS_ONLN\")\n";
  Script = Script + "nbRunningThreads=0;\n";
  Script = Script + "def thread_executeANTS (program) : # function to use for the htread is ANTS chosen\n";
  Script = Script + "  global nbRunningThreads # to be able to use the variable nbRunningThreads as a global one\n";
  Script = Script + "  global nbCPUs # to be able to use the variable nbCPUs as a global one\n";
  Script = Script + "  nbRunningThreads = nbRunningThreads + 1\n";
//  Script = Script + "  print(str(nbRunningThreads) + \" threads running, \" + str(nbCPUs) + \" CPUs on the machine\")\n";
//  Script = Script + "  print(\"Executing: \" + program)\n";
  Script = Script + "  if os.system(program)!=0 : DisplayErrorAndQuit(\'[\' + allcasesIDs[case] + \'] DTI-Reg: Computing global deformation fields\')\n";
  Script = Script + "  nbRunningThreads = nbRunningThreads - 1;\n\n";
//  Script = Script + "  print(str(nbRunningThreads) + \" threads running, \" + str(nbCPUs) + \" CPUs on the machine\")\n";
  }
*/

/*MULTITHREADING  if( m_DTIRegOptions[0].compare("BRAINS")==0 ) Script = Script + "    if os.system(GlobalDefFieldCommand)!=0 : DisplayErrorAndQuit(\'[\' + allcasesIDs[case] + \'] DTI-Reg: Computing global deformation fields\')\n";
  if( m_DTIRegOptions[0].compare("ANTS")==0 ) //use threads for ANTS because very slow
  {
    Script = Script + "    while nbRunningThreads >= nbCPUs : pass # waiting here for a thread to be free (pass = do nothing)\n";
    Script = Script + "    try:\n";
    Script = Script + "      thread.start_new_thread( thread_executeANTS, (GlobalDefFieldCommand, ) )\n";
    Script = Script + "    except:\n";
    Script = Script + "      print \"Error: unable to start thread\"\n";
  }*/

  Script = Script + "case = 0\n";
  Script = Script + "while case < len(allcases):\n";
  if(m_NeedToBeCropped==1)
  {
    Script = Script + "  origDTI= AffinePath + \"/\" + allcasesIDs[case] + \"_croppedDTI.nrrd\"\n";
  }
  else
  {
    Script = Script + "  origDTI= allcases[case]\n";
  }
  Script = Script + "  GlobalDefField = FinalResampPath + \"/First_Resampling/\" + allcasesIDs[case] + \"_GlobalDisplacementField.nrrd\"\n";
  Script = Script + "  FinalDef = FinalResampPath + \"/First_Resampling/\" + allcasesIDs[case] + \"_DeformedDTI.nrrd\"\n";
  Script = Script + "  GlobalDefFieldCommand=\"" + m_SoftPath[7] + " --fixedVolume \" + DTIAverage + \" --movingVolume \" + origDTI + \" --outputDeformationFieldVolume \" + GlobalDefField + \" --outputVolume \" + FinalDef\n";

  // give DTI-Reg the paths to the binary directories of BRAINS (4), dtiprocess (3) and ResampleDTIlogEuclidean (1) (ANTS given in option in the GUI : DTIRegExtraPath)
  // m_SoftPath[] contains executables, and DTI-Reg only needs the paths
  std::string BRAINSExecDir = itksys::SystemTools::GetRealPath( itksys::SystemTools::GetFilenamePath(m_SoftPath[4]).c_str() );
  std::string dtiprocessExecDir = itksys::SystemTools::GetRealPath( itksys::SystemTools::GetFilenamePath(m_SoftPath[3]).c_str() );
  std::string ResampExecDir = itksys::SystemTools::GetRealPath( itksys::SystemTools::GetFilenamePath(m_SoftPath[1]).c_str() );
  Script = Script + "  GlobalDefFieldCommand = GlobalDefFieldCommand + \" --ProgramsPathsVector " + m_DTIRegExtraPath + "," + BRAINSExecDir + "," + dtiprocessExecDir + "," + ResampExecDir + "\"\n";
/* m_DTIRegOptions[]
0  RegMethod
  ANTS
  1  ARegType
  2  TfmStep
  3  Iter
  4  SimMet
  5  SimParam
  6  GSigma
  7  SmoothOff
  BRAINS
  1  BRegType
  2  TfmMode
  3  NbPyrLev
  4  PyrLevIt
*/
  if( m_DTIRegOptions[0].compare("BRAINS")==0 )
  {
   Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --method useScalar-BRAINS\"\n";
    if(m_DTIRegOptions[1].compare("GreedyDiffeo (SyN)")==0) Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --BRAINSRegistrationType GreedyDiffeo\"\n";
    else if(m_DTIRegOptions[1].compare("SpatioTempDiffeo (SyN)")==0) Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --BRAINSRegistrationType SpatioTempDiffeo\"\n";
    else Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --BRAINSRegistrationType " + m_DTIRegOptions[1] + "\"\n";
    Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --BRAINSnumberOfPyramidLevels " + m_DTIRegOptions[3] + "\"\n";
    Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --BRAINSarrayOfPyramidLevelIterations " + m_DTIRegOptions[4] + "\"\n";
    if(m_DTIRegOptions[2].compare("Use computed affine transform")==0) Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --initialAffine \" + alltfms[case]\n";
    else Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --BRAINSinitializeTransformMode " + m_DTIRegOptions[2] + "\"\n";
    Script = Script + "  BRAINSTempTfm = FinalResampPath + \"/First_Resampling/\" + allcasesIDs[case] + \"_FA_AffReg.txt\"\n"; // so that nothing is stored in the same dir than the Atlas
    Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --outputTransform \" + BRAINSTempTfm\n";
  }
  if( m_DTIRegOptions[0].compare("ANTS")==0 )
  {
    Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --method useScalar-ANTS\"\n";
    if(m_DTIRegOptions[1].compare("GreedyDiffeo (SyN)")==0) Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSRegistrationType GreedyDiffeo\"\n";
    else if(m_DTIRegOptions[1].compare("SpatioTempDiffeo (SyN)")==0) Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSRegistrationType SpatioTempDiffeo\"\n";
    else Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSRegistrationType " + m_DTIRegOptions[1] + "\"\n";
    Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSTransformationStep " + m_DTIRegOptions[2] + "\"\n";
    Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSIterations " + m_DTIRegOptions[3] + "\"\n";
    if(m_DTIRegOptions[4].compare("Cross-Correlation (CC)")==0) Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSSimilarityMetric CC\"\n";
    else if(m_DTIRegOptions[4].compare("Mutual Information (MI)")==0) Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSSimilarityMetric MI\"\n";
    else if(m_DTIRegOptions[4].compare("Mean Square Difference (MSQ)")==0) Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSSimilarityMetric MSQ\"\n";
    Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSSimilarityParameter " + m_DTIRegOptions[5] + "\"\n";
    Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSGaussianSigma " + m_DTIRegOptions[6] + "\"\n";
    if( m_DTIRegOptions[7].compare("1")==0 ) Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSGaussianSmoothingOff\"\n";
    Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --initialAffine \" + alltfms[case]\n";
    Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSUseHistogramMatching \"\n";
    Script = Script + "  ANTSTempFileBase = FinalResampPath + \"/First_Resampling/\" + allcasesIDs[case] + \"_FA_\"\n"; // so that nothing is stored in the same dir than the Atlas
    Script = Script + "  GlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSOutbase \" + ANTSTempFileBase\n"; // no --outputTfm for ANTS because --ANTSOutbase is used for the tfm
  }
  Script = Script + "  print(\"\\n[\" + allcasesIDs[case] + \"] [Computing global deformation fields] => $ \" + GlobalDefFieldCommand)\n";

  if(m_Overwrite==1)
  {
    Script = Script + "  if 1 :\n";
  }
  else
  {
    Script = Script + "  if not CheckFileExists(FinalDef, case, allcasesIDs[case]) :\n";
  }

  Script = Script + "    GlobDbleToFloatCommand=\"" + m_SoftPath[8] + " convert -t float -i \" + FinalDef + \" | " + m_SoftPath[8] + " save -f nrrd -e gzip -o \" + FinalResampPath + \"/First_Resampling/\" + allcasesIDs[case] + \"_DeformedDTI_float.nrrd\"\n\n";

  if( ! m_useGridProcess )
  {
    Script = Script + "    if os.system(GlobalDefFieldCommand)!=0 : DisplayErrorAndQuit(\'[\' + allcasesIDs[case] + \'] DTI-Reg: Computing global deformation fields\')\n";

    Script = Script + "    print(\"\\n[\" + allcasesIDs[case] + \"] [Converting the deformed images from double to float DTI] => $ \" + GlobDbleToFloatCommand)\n";
    Script = Script + "    if os.system(GlobDbleToFloatCommand)!=0 : DisplayErrorAndQuit(\'[\' + allcasesIDs[case] + \'] unu: Converting the deformed images from double to float DTI\')\n";
  }
  else // run all commands in the same time in the script
  {
    Script = Script + "    GlobDefFieldGridCommand=" + GridProcessCmd + "GlobalDefFieldCommand + \"\\' \" + \"\\'\" + GlobDbleToFloatCommand + \"\\'\"\n";
    Script = Script + "    if os.system(GlobDefFieldGridCommand)!=0 : DisplayErrorAndQuit(\'[\' + allcasesIDs[case] + \'] Computing global deformation fields\')\n";
  }

  if(m_Overwrite==0)
  {
    Script = Script + "  else : " + GridProcessFileExistIndent1 + "print(\"=> The file \\'\" + FinalDef + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmd1;
  }
  Script = Script + "  case += 1\n\n";

//if( m_DTIRegOptions[0].compare("ANTS")==0 )  Script = Script + "while nbRunningThreads > 0 : pass # waiting for all the threads to be finished\n\n";

  if( m_useGridProcess )
  {
    Script = Script + "TestGridProcess( FilesFolder, len(allcases) ) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";
  }

/* dtiaverage recomputing */
  Script = Script + "# dtiaverage recomputing\n";
  Script = Script + "DTIAverage2 = FinalResampPath + \"/FinalAtlasDTI.nrrd\"\n";
  Script = Script + "AverageCommand2 = \"" + m_SoftPath[6] + " \"\n";
  Script = Script + "case = 0\n";
  Script = Script + "while case < len(allcases):\n";
  Script = Script + "  DTIforAVG2= \"--inputs \" + FinalResampPath + \"/First_Resampling/\" + allcasesIDs[case] + \"_DeformedDTI.nrrd \"\n";
  Script = Script + "  AverageCommand2 = AverageCommand2 + DTIforAVG2\n";
  Script = Script + "  case += 1\n";
  Script = Script + "AverageCommand2 = AverageCommand2 + \"--tensor_output \" + DTIAverage2\n";
  Script = Script + "print(\"\\n[Recomputing the final DTI average] => $ \" + AverageCommand2)\n";
  if(m_Overwrite==1)
  {
    Script = Script + "if 1 : \n";
  }
  else
  {
    Script = Script + "if not CheckFileExists(DTIAverage2, 0, \"\") : \n";
  }

/* Computing some images from the final DTI with dtiprocess */
  Script = Script + "# Computing some images from the final DTI with dtiprocess\n";
  Script = Script + "  FA2= FinalResampPath + \"/FinalAtlasFA.nrrd\"\n";
  Script = Script + "  cFA2= FinalResampPath + \"/FinalAtlasColorFA.nrrd\"\n";
  Script = Script + "  RD2= FinalResampPath + \"/FinalAtlasRD.nrrd\"\n"; // Radial Diffusivity
  Script = Script + "  MD2= FinalResampPath + \"/FinalAtlasMD.nrrd\"\n"; // Mean Diffusivity
  Script = Script + "  AD2= FinalResampPath + \"/FinalAtlasAD.nrrd\"\n"; // Axial Diffusivity
  Script = Script + "  GeneFACommand2=\"" + m_SoftPath[3] + " --scalar_float --dti_image \" + DTIAverage2 + \" -f \" + FA2 + \" -m \" + MD2 + \" --color_fa_output \" + cFA2 + \" --RD_output \" + RD2 + \" --lambda1_output \" + AD2\n\n";

  Script = Script + "  DbleToFloatCommand2=\"" + m_SoftPath[8] + " convert -t float -i \" + DTIAverage2 + \" | " + m_SoftPath[8] + " save -f nrrd -e gzip -o \" + FinalResampPath + \"/FinalAtlasDTI_float.nrrd\"\n\n";

  if( ! m_useGridProcess )
  {
    Script = Script + "  if os.system(AverageCommand2)!=0 : DisplayErrorAndQuit(\'dtiaverage: Recomputing the final DTI average\')\n";

    Script = Script + "  print(\"[Computing some images from the final DTI with dtiprocess] => $ \" + GeneFACommand2)\n";
    Script = Script + "  if os.system(GeneFACommand2)!=0 : DisplayErrorAndQuit(\'dtiprocess: Recomputing final FA, color FA, MD, RD and AD\')\n";

    Script = Script + "  print(\"[Computing some images from the final DTI with dtiprocess] => $ \" + DbleToFloatCommand2)\n";
    Script = Script + "  if os.system(DbleToFloatCommand2)!=0 : DisplayErrorAndQuit(\'unu: Converting the final resampled DTI atlas from double to float DTI\')\n";
  }
  else // run all commands in the same time in the script
  {
    Script = Script + "  Average2GridCommand=" + GridProcessCmdNoCase + "AverageCommand2 + \"\\' \" + \"\\'\" + GeneFACommand2 + \"\\' \" + \"\\'\" + DbleToFloatCommand2 + \"\\'\"\n";
    Script = Script + "  if os.system(Average2GridCommand)!=0 : DisplayErrorAndQuit(\'Recomputing the final DTI average\')\n";
  }

  if(m_Overwrite==0)
  {
    Script = Script + "else : " + GridProcessFileExistIndent + "print(\"=> The file \\'\" + DTIAverage2 + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmdNoCase;
  }

  if( m_useGridProcess )
  {
    Script = Script + "TestGridProcess( FilesFolder, 0 ) # stays in the function until all process is done : 0 makes the function look for \'file\'\n";
  }

/* Recomputing global deformation fields */
  Script = Script + "\n# Recomputing global deformation fields\n";
  Script = Script + "SecondResampRecomputed = [0] * len(allcases) # array of 1s and 0s to know what has been recomputed to know what to copy to final folders\n";
  Script = Script + "case = 0\n";
  Script = Script + "while case < len(allcases):\n";
  if(m_NeedToBeCropped==1)
  {
    Script = Script + "  origDTI2= AffinePath + \"/\" + allcasesIDs[case] + \"_croppedDTI.nrrd\"\n";
  }
  else
  {
    Script = Script + "  origDTI2= allcases[case]\n";
  }
  Script = Script + "  GlobalDefField2 = FinalResampPath + \"/Second_Resampling/\" + allcasesIDs[case] + \"_GlobalDisplacementField.nrrd\"\n";
  Script = Script + "  FinalDef2 = FinalResampPath + \"/Second_Resampling/\" + allcasesIDs[case] + \"_FinalDeformedDTI.nrrd\"\n";
  Script = Script + "  GlobalDefFieldCommand2=\"" + m_SoftPath[7] + " --fixedVolume \" + DTIAverage2 + \" --movingVolume \" + origDTI2 + \" --outputDeformationFieldVolume \" + GlobalDefField2 + \" --outputVolume \" + FinalDef2\n";

  // give DTI-Reg the paths to the binary directories of ANTS (6), BRAINS (5) and ResampleDTIlogEuclidean (2) | m_DTIRegExtraPath is supposed to contain ANTS
  // ANTSExecDir, BRAINSExecDir, ResampExecDir defined in the previous resampling
  Script = Script + "  GlobalDefFieldCommand2 = GlobalDefFieldCommand2 + \" --ProgramsPathsVector " + m_DTIRegExtraPath + "," + BRAINSExecDir + "," + dtiprocessExecDir + "," + ResampExecDir + "\"\n";

/* m_DTIRegOptions[]
0  RegMethod
  ANTS
  1  ARegType
  2  TfmStep
  3  Iter
  4  SimMet
  5  SimParam
  6  GSigma
  7  SmoothOff
  BRAINS
  1  BRegType
  2  TfmMode
  3  NbPyrLev
  4  PyrLevIt
*/
  if( m_DTIRegOptions[0].compare("BRAINS")==0 )
  {
    Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --method useScalar-BRAINS\"\n";
    if(m_DTIRegOptions[1].compare("GreedyDiffeo (SyN)")==0) Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --BRAINSRegistrationType GreedyDiffeo\"\n";
    else if(m_DTIRegOptions[1].compare("SpatioTempDiffeo (SyN)")==0) Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --BRAINSRegistrationType SpatioTempDiffeo\"\n";
    else Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --BRAINSRegistrationType " + m_DTIRegOptions[1] + "\"\n";
    Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --BRAINSnumberOfPyramidLevels " + m_DTIRegOptions[3] + "\"\n";
    Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --BRAINSarrayOfPyramidLevelIterations " + m_DTIRegOptions[4] + "\"\n";
    if(m_DTIRegOptions[2].compare("Use computed affine transform")==0) Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --initialAffine \" + alltfms[case]\n";
    else Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --BRAINSinitializeTransformMode " + m_DTIRegOptions[2] + "\"\n";
    Script = Script + "  BRAINSTempTfm2 = FinalResampPath + \"/Second_Resampling/\" + allcasesIDs[case] + \"_FA_AffReg.txt\"\n"; // so that nothing is stored in the same dir than the Atlas
    Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --outputTransform \" + BRAINSTempTfm2\n";
  }
  if( m_DTIRegOptions[0].compare("ANTS")==0 )
  {
    Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --method useScalar-ANTS\"\n";
    if(m_DTIRegOptions[1].compare("GreedyDiffeo (SyN)")==0) Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSRegistrationType GreedyDiffeo\"\n";
    else if(m_DTIRegOptions[1].compare("SpatioTempDiffeo (SyN)")==0) Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSRegistrationType SpatioTempDiffeo\"\n";
    else Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSRegistrationType " + m_DTIRegOptions[1] + "\"\n";
    Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSTransformationStep " + m_DTIRegOptions[2] + "\"\n";
    Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSIterations " + m_DTIRegOptions[3] + "\"\n";
    if(m_DTIRegOptions[4].compare("Cross-Correlation (CC)")==0) Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSSimilarityMetric CC\"\n";
    else if(m_DTIRegOptions[4].compare("Mutual Information (MI)")==0) Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSSimilarityMetric MI\"\n";
    else if(m_DTIRegOptions[4].compare("Mean Square Difference (MSQ)")==0) Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSSimilarityMetric MSQ\"\n";
    Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSSimilarityParameter " + m_DTIRegOptions[5] + "\"\n";
    Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSGaussianSigma " + m_DTIRegOptions[6] + "\"\n";
    if( m_DTIRegOptions[7].compare("1")==0 ) Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSGaussianSmoothingOff\"\n";
    Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --initialAffine \" + alltfms[case]\n";
    Script = Script + "  ANTSTempFileBase2 = FinalResampPath + \"/Second_Resampling/\" + allcasesIDs[case] + \"_FA_\"\n"; // so that nothing is stored in the same dir than the Atlas
    Script = Script + "  GlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSOutbase \" + ANTSTempFileBase2\n"; // no --outputTfm for ANTS because --ANTSOutbase is used for the tfm
  }
  Script = Script + "  print(\"\\n[\" + allcasesIDs[case] + \"] [Recomputing global deformation fields] => $ \" + GlobalDefFieldCommand2)\n";
  if(m_Overwrite==1)
  {
    Script = Script + "  if 1 :\n";
  }
  else
  {
    Script = Script + "  if not CheckFileExists(FinalDef2, case, allcasesIDs[case]) :\n";
  }

  Script = Script + "    SecondResampRecomputed[case] = 1\n";
  Script = Script + "    DTIRegCaseFA = FinalResampPath + \"/Second_Resampling/\" + allcasesIDs[case] + \"_FinalDeformedFA.nrrd\"\n";
  Script = Script + "    GeneDTIRegCaseFACommand=\"" + m_SoftPath[3] + " --scalar_float --dti_image \" + FinalDef2 + \" -f \" + DTIRegCaseFA\n";

  Script = Script + "    GlobDbleToFloatCommand2=\"" + m_SoftPath[8] + " convert -t float -i \" + FinalDef2 + \" | " + m_SoftPath[8] + " save -f nrrd -e gzip -o \" + FinalResampPath + \"/Second_Resampling/\" + allcasesIDs[case] + \"_FinalDeformedDTI_float.nrrd\"\n";

  if( ! m_useGridProcess )
  {
    Script = Script + "    if os.system(GlobalDefFieldCommand2)!=0 : DisplayErrorAndQuit(\'[\' + allcasesIDs[case] + \'] DTI-Reg: Computing global deformation fields\')\n";

    Script = Script + "    print(\"\\n[\" + allcasesIDs[case] + \"] [Converting the deformed images from double to float DTI] => $ \" + GlobDbleToFloatCommand2)\n";
    Script = Script + "    if os.system(GlobDbleToFloatCommand2)!=0 : DisplayErrorAndQuit(\'[\' + allcasesIDs[case] + \'] unu: Converting the deformed images from double to float DTI\')\n";

    Script = Script + "    print(\"\\n[\" + allcasesIDs[case] + \"] [Computing DTIReg FA] => $ \" + GeneDTIRegCaseFACommand)\n";
    Script = Script + "    if os.system(GeneDTIRegCaseFACommand)!=0 : DisplayErrorAndQuit(\'[\' + allcasesIDs[case] + \'] dtiprocess: Computing DTIReg FA\')\n";
  }
  else // run all commands in the same time in the script
  {
    Script = Script + "    GlobDefField2GridCommand=" + GridProcessCmd + "GlobalDefFieldCommand2 + \"\\' \" + \"\\'\" + GlobDbleToFloatCommand2 + \"\\' \" + \"\\'\" + GeneDTIRegCaseFACommand + \"\\'\"\n";
    Script = Script + "    if os.system(GlobDefField2GridCommand)!=0 : DisplayErrorAndQuit(\'[\' + allcasesIDs[case] + \'] Recomputing global deformation fields\')\n";
  }

  if(m_Overwrite==0)
  {
    Script = Script + "  else : " + GridProcessFileExistIndent1 + "print(\"=> The file \\'\" + FinalDef2 + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmd1;
  }
  Script = Script + "  case += 1\n\n";

  if( m_useGridProcess )
  {
    Script = Script + "TestGridProcess( FilesFolder, len(allcases) ) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";
  }

/* Moving final images to final folders */
  Script = Script + "\n# Moving final images to final folders\n";
  Script = Script + "print(\"\\n=> Moving final images to final folders\")\n";
  Script = Script + "case = 0\n";
  Script = Script + "while case < len(allcases):\n";
  Script = Script + "  if SecondResampRecomputed[case] :\n";
  Script = Script + "    GlobalDefField2 = FinalResampPath + \"/Second_Resampling/\" + allcasesIDs[case] + \"_GlobalDisplacementField.nrrd\"\n";
  Script = Script + "    NewGlobalDefField2 = FinalResampPath + \"/FinalDeformationFields/\" + allcasesIDs[case] + \"_GlobalDisplacementField.nrrd\"\n";
  Script = Script + "    if CheckFileExists(GlobalDefField2, case, allcasesIDs[case]) :\n";
  Script = Script + "      shutil.copy(GlobalDefField2, NewGlobalDefField2)\n";
  Script = Script + "    FinalDef2 = FinalResampPath + \"/Second_Resampling/\" + allcasesIDs[case] + \"_FinalDeformedDTI.nrrd\"\n";
  Script = Script + "    NewFinalDef2 = FinalResampPath + \"/FinalTensors/\" + allcasesIDs[case] + \"_FinalDeformedDTI.nrrd\"\n";
  Script = Script + "    if CheckFileExists(FinalDef2, case, allcasesIDs[case]) :\n";
  Script = Script + "      shutil.copy(FinalDef2, NewFinalDef2)\n";
  Script = Script + "    FinalDef2f = FinalResampPath + \"/Second_Resampling/\" + allcasesIDs[case] + \"_FinalDeformedDTI_float.nrrd\"\n";
  Script = Script + "    NewFinalDef2f = FinalResampPath + \"/FinalTensors/\" + allcasesIDs[case] + \"_FinalDeformedDTI_float.nrrd\"\n";
  Script = Script + "    if CheckFileExists(FinalDef2f, case, allcasesIDs[case]) :\n";
  Script = Script + "      shutil.copy(FinalDef2f, NewFinalDef2f)\n";
  Script = Script + "    DTIRegCaseFA = FinalResampPath + \"/Second_Resampling/\" + allcasesIDs[case] + \"_FinalDeformedFA.nrrd\"\n";
  Script = Script + "    NewDTIRegCaseFA = FinalResampPath + \"/FinalTensors/\" + allcasesIDs[case] + \"_FinalDeformedFA.nrrd\"\n";
  Script = Script + "    if CheckFileExists(DTIRegCaseFA, case, allcasesIDs[case]) :\n";
  Script = Script + "      shutil.copy(DTIRegCaseFA, NewDTIRegCaseFA)\n";
  Script = Script + "  case += 1\n\n";

  Script = Script + "print(\"\\n============ End of Atlas Building =============\")\n\n";

  Script = Script + "sys.exit(0)\n";

  m_Script_AtlasBuilding=Script;
}

void ScriptWriter::MainScript()
{
  std::string Script;

  std::cout<<"[Main]"<<std::endl; // command line display (no endl)

  Script = Script + "#!/usr/bin/python\n\n";
  Script = Script + "import os\n"; ///// To run a shell command : os.system("[shell command]")
  Script = Script + "import time\n"; // to compute the execution time
  Script = Script + "import sys\n\n"; // to return an exit code

  // Display python version used
//  Script = Script + "print(\"\\n[] Script running with Python version:\")\n";
//  Script = Script + "os.system(\"which python\")\n\n";

  // Display current date and time (for log file)
  Script = Script + "print(\"\\nThe current date and time are:\")\n";
  Script = Script + "print( time.strftime(\'%x %X %Z\') )\n\n";

  Script = Script + "print(\"\\n=============== Main Script ================\")\n\n";

  Script = Script + "OutputPath= \"" + m_OutputPath + "/DTIAtlas\"\n\n";

  /* Function to display error and quit */
  Script = Script + "def DisplayErrorAndQuit ( Error ):\n";
  Script = Script + "  print \'\\n\\nERROR DETECTED IN WORKFLOW:\',Error\n";
  Script = Script + "  print \'ABORT\'\n";
  Script = Script + "  sys.exit(1)\n\n";

  if( ! m_useGridProcess ) // if grid process, var defined in the grid script
  {
    Script = Script + "os.putenv(\"ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS\",\"" + m_NbThreadsString + "\") # to set the nb of cores to use : propagated to the other scripts via os.system()\n\n";
  }

  Script = Script + "PIDlogFile = OutputPath + \"/Script/PID.log\"\n";
  Script = Script + "if os.path.isfile( PIDlogFile ) : os.remove( PIDlogFile )\n\n";

  // remove possibly existing old versions of scripts
  Script = Script + "if os.path.isfile( OutputPath + \"/Script/DTIAtlasBuilder_Main.script\" ) : os.remove( OutputPath + \"/Script/DTIAtlasBuilder_Main.script\" )\n";
  Script = Script + "if os.path.isfile( OutputPath + \"/Script/DTIAtlasBuilder_Preprocess.script\" ) : os.remove( OutputPath + \"/Script/DTIAtlasBuilder_Preprocess.script\" )\n";
  Script = Script + "if os.path.isfile( OutputPath + \"/Script/DTIAtlasBuilder_AtlasBuilding.script\" ) : os.remove( OutputPath + \"/Script/DTIAtlasBuilder_AtlasBuilding.script\" )\n\n";

  Script = Script + "time1=time.time()\n\n";

/* Call the other scripts */
  Script = Script + "# Call the Preprocess script\n";
  Script = Script + "PrePScriptCommand= \"" + m_PythonPath + " \" + OutputPath + \"/Script/DTIAtlasBuilder_Preprocess.py\"\n"; // PythonPath contains already a space after the command
  Script = Script + "print(\"\\n=> $ \" + PrePScriptCommand)\n";
  Script = Script + "if os.system(PrePScriptCommand)!=0 : DisplayErrorAndQuit(\'=> Errors detected in preprocessing\')\n\n";

  Script = Script + "# Call the Atlas Building script\n";
  Script = Script + "AtlasBuildingCommand= \"" + m_PythonPath + " \" + OutputPath + \"/Script/DTIAtlasBuilder_AtlasBuilding.py\"\n";
  Script = Script + "print(\"\\n=> $ \" + AtlasBuildingCommand)\n";
  Script = Script + "if os.system(AtlasBuildingCommand)!=0 : DisplayErrorAndQuit(\'=> Errors detected in atlas building\')\n\n";

  Script = Script + "print(\"\\n============ End of execution =============\\n\")\n\n";

  Script = Script + "# Display execution time\n";
  Script = Script + "time2=time.time()\n";
  Script = Script + "timeTot=time2-time1\n";
  Script = Script + "if timeTot<60 : print(\"| Execution time = \" + str(int(timeTot)) + \"s\")\n";
  Script = Script + "elif timeTot<3600 : print(\"| Execution time = \" + str(int(timeTot)) + \"s = \" + str(int(timeTot/60)) + \"m \" + str( int(timeTot) - (int(timeTot/60)*60) ) + \"s\")\n";
  Script = Script + "else : print(\"| Execution time = \" + str(int(timeTot)) + \"s = \" + str(int(timeTot/3600)) + \"h \" + str( int( (int(timeTot) - int(timeTot/3600)*3600) /60) ) + \"m \" + str( int(timeTot) - (int(timeTot/60)*60) ) + \"s\")\n\n";

  Script = Script + "if os.path.isfile( PIDlogFile ) : os.remove( PIDlogFile )\n\n";

  Script = Script + "sys.exit(0)\n";

  m_Script_Main=Script;
}

  /////////////////////////////////////////
 //         SCRIPT ACCESSORS            //
/////////////////////////////////////////

std::string ScriptWriter::getScript_Preprocess()
{
  return m_Script_Preprocess;
}

std::string ScriptWriter::getScript_AtlasBuilding()
{
  return m_Script_AtlasBuilding;
}

std::string ScriptWriter::getScript_Main()
{
  return m_Script_Main;
}

  /////////////////////////////////////////
 //           CHECK DATASET             //
/////////////////////////////////////////

int ScriptWriter::setCroppingSize( bool SafetyMargin ) // returns 0 if no cropping , 1 if cropping needed
{  
  m_NeedToBeCropped=0;

  int MaxSize [3] = {-1,-1,-1};

/////////itk type definitions
  typedef itk::Image < double , 4 > ImageType; //itk type for image
  typedef itk::ImageFileReader <ImageType> ReaderType; //itk reader class to open an image
  ReaderType::Pointer reader=ReaderType::New();
  ImageType::RegionType region;

//////////Testing all the cases
  for (unsigned int i=0;i<m_CasesPath.size();i++) // read the headers of all files
  {
    reader->SetFileName( m_CasesPath[i] ); //Label is a path => open the image
    reader->UpdateOutputInformation(); // get the informations in the header
    region = reader->GetOutput()->GetLargestPossibleRegion();

    if( (int)region.GetSize()[0] != MaxSize[0] ) //x coordinate
    {
      if( (int)region.GetSize()[0] > MaxSize[0] )
      {
        MaxSize[0]=region.GetSize()[0];
      }
      if( i > 0 )
      {
        m_NeedToBeCropped=1;
      }
    }
    if( (int)region.GetSize()[1] != MaxSize[1] ) //y coordinate
    {
      if( (int)region.GetSize()[1] > MaxSize[1] )
      {
        MaxSize[1]=region.GetSize()[1];
      }
      if( i > 0 )
      {
        m_NeedToBeCropped=1;
      }
    }
    if( (int)region.GetSize()[2] != MaxSize[2] ) //z coordinate
    {
      if( (int)region.GetSize()[2] > MaxSize[2] )
      {
        MaxSize[2]=region.GetSize()[2];
      }
      if( i > 0 )
      {
        m_NeedToBeCropped=1;
      }
    }
  }

  if(SafetyMargin) // bool SafetyMargin (CropDTI will add 2 voxels each size : place the image in the center)
  {
    m_NeedToBeCropped=1; 
    MaxSize[0] = MaxSize[0] + 4; // add 2 voxels each side in each direction to the computed cropping size
    MaxSize[1] = MaxSize[1] + 4; // add 2 voxels each side in each direction to the computed cropping size
    MaxSize[2] = MaxSize[2] + 4; // add 2 voxels each side in each direction to the computed cropping size
  }

  if(m_NeedToBeCropped==1)
  {
    m_CropSize[0] = IntToStr(MaxSize[0]);
    m_CropSize[1] = IntToStr(MaxSize[1]);
    m_CropSize[2] = IntToStr(MaxSize[2]);

    std::cout<<"| Crop size computed : ["<<m_CropSize[0]<<";"<<m_CropSize[1]<<";"<<m_CropSize[2]<<"]"<<std::endl;
    return 1;
  }

  else return 0;
}

int ScriptWriter::CheckVoxelSize() // returns 0 if voxel size OK , otherwise 1
{  
/////////itk definitions
  typedef itk::Image < double , 4 > ImageType; //itk type for image
  typedef itk::ImageFileReader <ImageType> ReaderType; //itk reader class to open an image

  ReaderType::Pointer reader=ReaderType::New();
  reader->SetFileName( m_CasesPath[0] ); //Label is a path => open the image
  reader->UpdateOutputInformation();
  const ImageType::SpacingType& sp = reader-> GetOutput()->GetSpacing();
  double RefSpacing [3]; // the spacing of the first case is the reference for this computation
  RefSpacing[0]=sp[0];
  RefSpacing[1]=sp[1];
  RefSpacing[2]=sp[2];

//////////Testing all the cases
  for (unsigned int i=1;i<m_CasesPath.size();i++) // read the headers of all files
  {
    reader->SetFileName( m_CasesPath[i] ); //Label is a path => open the image
    reader->UpdateOutputInformation(); // get the informations in the header
    const ImageType::SpacingType& sp = reader-> GetOutput() -> GetSpacing();

    if( fabs(sp[0]-RefSpacing[0])/RefSpacing[0] > 0.05 ) return 1;
    if( fabs(sp[1]-RefSpacing[1])/RefSpacing[1] > 0.05 ) return 1;
    if( fabs(sp[2]-RefSpacing[2])/RefSpacing[2] > 0.05 ) return 1;
  }

  return 0;
}

  /////////////////////////////////////////
 //           SET THE VALUES            //
/////////////////////////////////////////

void ScriptWriter::setCasesPath(std::vector < std::string > CasesPath)
{
  for (unsigned int i=0;i<CasesPath.size();i++)
  {
    m_CasesPath.push_back( CasesPath[i] );
    m_CasesIDs.push_back( itksys::SystemTools::GetFilenameWithoutExtension( CasesPath[i] ) );
  }
}
void ScriptWriter::clearCasesPath()
{
  m_CasesPath.clear();
}

void ScriptWriter::setOutputPath(std::string OutputPath)
{
  m_OutputPath = OutputPath;
}

void ScriptWriter::setRegType(int RegType)
{
  m_RegType = RegType;
}

void ScriptWriter::setnbLoops(int nbLoops)
{
  m_nbLoops = nbLoops;
}

void ScriptWriter::setTemplatePath(std::string TemplatePath)
{
  m_TemplatePath = TemplatePath;
}

void ScriptWriter::setOverwrite(int Overwrite)
{
  m_Overwrite = Overwrite;
}

void ScriptWriter::setInterpolType(std::string Type)
{
  m_InterpolType = Type;
}

void ScriptWriter::setInterpolOption(std::string Option)
{
  m_InterpolOption = Option;
}

void ScriptWriter::setTensInterpol(std::string TensInterpol)
{
  m_TensInterpol = TensInterpol;
}

void ScriptWriter::setInterpolLogOption(std::string InterpolLogOption)
{
  m_InterpolLogOption = InterpolLogOption;
}

void ScriptWriter::setTensorTfm(std::string TensTfm)
{
  m_TensTfm = TensTfm;
}

void ScriptWriter::setSoftPath(std::vector < std::string > SoftPath) // 1=ImageMath, 2=ResampleDTIlogEuclidean, 3=CropDTI, 4=dtiprocess, 5=BRAINSFit, 6=GreedyAtlas, 7=dtiaverage, 8=DTI-Reg, 9=unu
{
  m_SoftPath.clear();
  for (unsigned int i=0;i<SoftPath.size();i++) m_SoftPath.push_back( SoftPath[i] );
}

void ScriptWriter::setDTIRegOptions(std::vector < std::string > DTIRegOptions)
{
  m_DTIRegOptions.clear();
  for (unsigned int i=0;i<DTIRegOptions.size();i++) m_DTIRegOptions.push_back( DTIRegOptions[i] );
/*
  RegMethod
  ANTS
    ARegType
    TfmStep
    Iter
    SimMet
    SimParam
    GSigma
    SmoothOff

  BRAINS
    BRegType
    TfmMode
    NbPyrLev
    PyrLevIt
*/
}

void ScriptWriter::setDTIRegExtraPath(std::string DTIRegExtraPath)
{
  m_DTIRegExtraPath=DTIRegExtraPath;
}

void ScriptWriter::setBFAffineTfmMode(std::string BFAffineTfmMode)
{
  m_BFAffineTfmMode=BFAffineTfmMode;
}

void ScriptWriter::setGridProcess(bool useGridProcess)
{
  m_useGridProcess = useGridProcess;
}

void ScriptWriter::setGridCommand(std::string GridCommand)
{
  m_GridCommand = GridCommand;
}

void ScriptWriter::setPythonPath(std::string PythonPath)
{
  m_PythonPath = PythonPath;
}

void ScriptWriter::setNbThreads(int NbThreads) // if( NbThreadsSpinBox->value() != 0 ) // 0 <=> automatic = no limit option given
{
  m_NbThreadsString = IntToStr(NbThreads);
/*Multi threading option used to set env var at the beginning of the main script */
}

