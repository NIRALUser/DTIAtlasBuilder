#ifndef DEF_SCRIPTWRITER
#define DEF_SCRIPTWRITER

/*std classes*/
#include <iostream>
#include <string>
#include <vector>
#include <math.h>// for the absolute value

/*itk classes*/
#include "itkImage.h"
#include "itkImageFileReader.h"

class ScriptWriter
{
  public:

/* SUB  WRITING FUNCTIONS */
    std::string pyExecuteCommandPreprocessCase ( std::string NameOfFileVarToTest, std::string NameOfCmdVarToExec, std::string ErrorTxtToDisplay, std::string SpacesBeforeFirstIf );

/*WRITING FUNCTIONS*/
    void WriteScript();
    void Preprocess();
    void AtlasBuilding();
    void MainScript();

/*SCRIPT ACCESSORS*/
    std::string getScript_Preprocess();
    std::string getScript_AtlasBuilding();
    std::string getScript_Main();

/*CHECK DATASET*/
    int setCroppingSize( bool SafetyMargin ); // returns 0 if no cropping , 1 if cropping needed
    int CheckVoxelSize(); // returns 0 if voxel size OK , otherwise 1

/*SET THE VALUES*/
    void setCasesPath(std::vector < std::string > CasesPath);
    void clearCasesPath();
    void setOutputPath(std::string OutputPath);
    void setRegType(int RegType);
    void setnbLoops(int nbLoops);
    void setTemplatePath(std::string FAAtlasrefPath);
    void setOverwrite(int Overwrite);
    void setInterpolType(std::string Type);
    void setInterpolOption(std::string Option);
    void setTensInterpol(std::string TensInterpol);
    void setInterpolLogOption(std::string InterpolLogOption);
    void setTensorTfm(std::string TensTfm);
    void setSoftPath(std::vector < std::string > SoftPath);
    void setDTIRegOptions(std::vector < std::string > DTIRegOptions);
    void setDTIRegExtraPath(std::string DTIRegExtraPath);
    void setBFAffineTfmMode(std::string BFAffineTfmMode);
    void setGridProcess(bool useGridProcess);
    void setGridCommand(std::string GridCommand);
    void setPythonPath(std::string PythonPath);
    void setNbThreads(int NbThreads);

  private:
/* VALUES */
    std::vector < std::string > m_CasesPath;
    std::vector < std::string > m_CasesIDs;
    std::string m_OutputPath;
    int m_RegType; //=0 for using template and =1 for using existing first case
    int m_nbLoops;
    std::string m_nbLoops_str;
    std::string m_TemplatePath;
    int m_CropSize [3]; //x,y,z
    int m_NeedToBeCropped; //=0 if not and =1 if need to be cropped
    int m_Overwrite; //=0 if not and =1 if overwrite on existing files
    std::string m_InterpolType;
    std::string m_InterpolOption;
    std::string m_TensInterpol;
    std::string m_InterpolLogOption;
    std::string m_TensTfm;
    std::vector < std::string > m_SoftPath; // contains the path to the softwares: 1=ImageMath, 2=ResampleDTIlogEuclidean, 3=CropDTI, 4=dtiprocess, 5=BRAINSFit, 6=AtlasWerks, 7=dtiaverage, 8=DTI-Reg, 9=unu
    std::vector < std::string > m_DTIRegOptions;
    std::string m_DTIRegExtraPath;
    std::string m_BFAffineTfmMode;
    bool m_useGridProcess;
    std::string m_GridCommand;
    std::string m_PythonPath;
    std::string m_NbThreadsString;

/* SCRIPTS */
    std::string m_Script_Preprocess; //contains the whole Script for Pre processing
    std::string m_Script_AtlasBuilding; //contains the whole Script for Atlas Building
    std::string m_Script_Main;//contains the whole Main Scrit
};
#endif

