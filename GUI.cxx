/*Qt classes*/
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QShortcut>
#include <QCloseEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QSignalMapper>
#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include <QLineEdit>
#include <QTimer>
#include <QDebug>
#include <QItemSelection>


/*std classes*/
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iomanip>
#include <iostream>

/*itk classes*/
#include "itkImage.h"
#include "itkImageFileReader.h"
#include <itksys/SystemTools.hxx> // for FindProgram() and GetFilenamePath()


/* ITK modes = same values than the C variables: F_OK W_OK R_OK X_OK */
mode_t ITKmode_F_OK = 0;
mode_t ITKmode_W_OK = 2;
mode_t ITKmode_R_OK = 4;
mode_t ITKmode_X_OK = 1;

#ifndef DTIAtlasBuilder_VERSION
#define DTIAtlasBuilder_VERSION "Unknown version"
#endif

/* DTIAB version check:
Version will be written in a log file when computing
to display a warning if a new compute is done afterwards with a newer version
1.0 : No version file
1.1 : Introducing version file
      Renaming AW folder to Diffeomorphic folders (folders renamed in python script)
      Renaming GreedyAtlas' Def Fields to HField
      Renaming Final Def Fields to GlobalDisplacementField
*/

/* homemade */
#include "GUI.h"
#include "ScriptWriter.h"

// Q_OS_WIN32 Q_OS_LINUX Q_OS_MAC are Qt macros
#if defined Q_OS_WIN32
#define Platform "win"
#elif defined Q_OS_MAC
#define Platform "mac"
#elif defined Q_OS_LINUX
#define Platform "linux"
#endif

//We want to search the extension path first, and then we look for the tools on the system
std::string FindProgram( const char* name , std::vector< std::string > m_FindProgramDTIABExecDirVec )
{
  std::string path ;

  path = itksys::SystemTools::FindProgram( name , m_FindProgramDTIABExecDirVec , true ) ;

  if( path.empty() )
    {
    path = itksys::SystemTools::FindProgram( name ) ;
    }
  return path ;
}

template< class T > std::string IntOrDoubleToStr(T IntOrDoubleVar) // T is int or double
{
  std::ostringstream oss;
  oss << IntOrDoubleVar;
  return oss.str();
}

  /////////////////////////////////////////
 //            CONSTRUCTOR              //
/////////////////////////////////////////
  
// bool Testing: If automatic testing (constructor called from DTIAtllasBuilderGUITest.cxx), this flag will prevent any pop up window to show
GUI::GUI(std::string ParamFile, std::string ConfigFile, std::string CSVFile, bool overwrite, bool noGUI, bool Testing, std::string commandRan) : QMainWindow()
{
  setupUi(this);
  QStringList version = QString(DTIAtlasBuilder_VERSION).split("|");
  setWindowTitle( "DTI Atlas Builder: " + version[0] ) ;
  this->setAcceptDrops(true);

  m_ErrorDetectedInConstructor=false;

/* Hierarchy model */
  m_HierarchyModel= new CaseHierarchyModel(QString(""));


/* Script writing object */
  m_scriptwriter = new ScriptWriter; // delete in "void GUI::ExitProgram()"

/* Variables */
  m_ExecutableDir = commandRan;
  m_scriptwriter->setExecutableDir(m_ExecutableDir); 

  m_CSVseparator = QString(",");
  m_ParamSaved=1;
  m_lastCasePath="";
  m_noGUI=noGUI;
  m_Testing = Testing;
  if( overwrite ) OverwritecheckBox->setChecked(true);
  m_ScriptRunning=false;
  m_ScriptQProcess = new QProcess;
  QObject::connect(m_ScriptQProcess, SIGNAL(finished(int)), this, SLOT(ScriptQProcessDone(int)));
  m_ScriptRunningQTimer = new QTimer(this);
  QObject::connect(m_ScriptRunningQTimer, SIGNAL(timeout()), this, SLOT(UpdateScriptRunningGUIDisplay()));

/* Error message if windows or mac */
  // if ( !m_Testing )
  // {
  //   if( (std::string)Platform == "mac" || (std::string)Platform == "win")
  //   {
  //     if(DTIAtlasBuilder_BUILD_SLICER_EXTENSION) QMessageBox::critical(this, "DTIAtlasBuilder not working", "This program is currently not working as it should on this platform.\nPlease do not hit the compute button, or the process will fail.\nYou can find other useful tools in Slicer:\n> DTI-Reg\n>Resample DTI Volume - log euclidean");
  //     else QMessageBox::critical(this, "DTIAtlasBuilder not working", "This program is currently not working as it should on this platform.\nPlease do not hit the compute button, or the process will fail.\nIf the package was compiled or downloaded, you will find other useful tools in the specified install directory.");
  //   }
  // }

/* Initialize the options */
  InitOptions();

  if(!m_noGUI)
  {

/* Objects connections */
    QObject::connect(ComputepushButton, SIGNAL(clicked()), this, SLOT(Compute()));
    // Compute button triggered when user presses ENTER key
    QShortcut* EnterShortcut = new QShortcut(QKeySequence(Qt::Key_Return), this); // ENTER on main keyboard (RETURN)
    QObject::connect(EnterShortcut, SIGNAL(activated()), ComputepushButton, SLOT(click()));
    QShortcut* NumPadEnterShortcut = new QShortcut(QKeySequence(Qt::Key_Enter), this); // ENTER on num pad
    QObject::connect(NumPadEnterShortcut, SIGNAL(activated()), ComputepushButton, SLOT(click()));

    // Hierarchical objects & Treeview set
    QObject::connect(openHierarchyButton,SIGNAL(clicked()), this, SLOT(openHierarchyFile()));
    QObject::connect(saveHierarchyButton,SIGNAL(clicked()), this, SLOT(saveHierarchyFile()));
    QObject::connect(addNodeButton,SIGNAL(clicked()), this, SLOT(addNode()));
    QObject::connect(removeNodeButton,SIGNAL(clicked()), this, SLOT(removeNode()));

    caseHierarchyTreeView->setModel(m_HierarchyModel);
    enableTreeViewWidget(false);
    QObject::connect(caseHierarchyTreeView, SIGNAL(clicked(const QModelIndex)), this, SLOT(treeViewItemSelected(const QModelIndex)));
    QObject::connect(m_HierarchyModel, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)),this,SLOT(treeViewItemChanged(const QModelIndex)));
    //std::cout << b << std::endl;
  
    //
    
    //QModelIndex idx= m_HierarchyModel->item(0)->index();
    //caseHierarchyTreeView->clicked(idx);
    //caseHierarchyTreeView->selectionModel()->select(QItemSelection(idx,idx), QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    enableCaseWidget(0);


    QObject::connect(StoppushButton, SIGNAL(clicked()), this, SLOT(KillScriptQProcess()));
    QObject::connect(BrowseCSVPushButton, SIGNAL(clicked()), this, SLOT(ReadCSVSlot()));
    QObject::connect(SaveCSVPushButton, SIGNAL(clicked()), this, SLOT(SaveCSVDatasetBrowse()));
    QObject::connect(BrowseOutputPushButton, SIGNAL(clicked()), this, SLOT(OpenOutputBrowseWindow()));
    QObject::connect(BrowseDTIRegExtraPathPushButton, SIGNAL(clicked()), this, SLOT(OpenDTIRegExtraPathBrowseWindow()));
    QObject::connect(TemplateBrowsePushButton, SIGNAL(clicked()), this, SLOT(OpenTemplateBrowseWindow()));
    QObject::connect(AddPushButton, SIGNAL(clicked()), this, SLOT(OpenAddCaseBrowseWindow()));
    // Remove button triggered when user presses PLUS key
    QShortcut* PlusShortcut = new QShortcut(QKeySequence(Qt::Key_Plus), this);
    QObject::connect(PlusShortcut, SIGNAL(activated()), AddPushButton, SLOT(click()));

    QObject::connect(RemovePushButton, SIGNAL(clicked()), this, SLOT(RemoveSelectedCases()));
    // Remove button triggered when user presses DEL or MINUS key
    QShortcut* DelShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this); // DELETE key
    QObject::connect(DelShortcut, SIGNAL(activated()), RemovePushButton, SLOT(click()));
    QShortcut* MinusShortcut = new QShortcut(QKeySequence(Qt::Key_Minus), this); // MINUS key
    QObject::connect(MinusShortcut, SIGNAL(activated()), RemovePushButton, SLOT(click()));

    RemovePushButton->setEnabled(false);
    ComputepushButton->setEnabled(false);
    StoppushButton->setEnabled(false);
    CleanOutputPushButton->setEnabled(false);
    DisableQC(); // will be enable when cases are loaded

    
    QObject::connect(actionNew_Project, SIGNAL(triggered()), this, SLOT(newHierarchyProject()));
    QObject::connect(actionOpen_Project_File, SIGNAL(triggered()), this, SLOT(openHierarchyFile()));
    QObject::connect(actionSave_Project_File, SIGNAL(triggered()), this, SLOT(saveHierarchyFile()));
    QObject::connect(actionOpen_Project_Directory,SIGNAL(triggered()),this,SLOT(openProjectDirectorySlot()));

    QObject::connect(actionLoad_parameters, SIGNAL(triggered()), this, SLOT(LoadParametersSlot()));
    QObject::connect(actionSave_parameters, SIGNAL(triggered()), this, SLOT(SaveParametersSlot()));
    QObject::connect(actionExit, SIGNAL(triggered()), this, SLOT(ExitProgram()));
    QObject::connect(actionLoad_Software_Configuration, SIGNAL(triggered()), this, SLOT(LoadConfigSlot()));
    QObject::connect(actionSave_Software_Configuration, SIGNAL(triggered()), this, SLOT(SaveConfig()));
    QObject::connect(actionRead_Me, SIGNAL(triggered()), this, SLOT(ReadMe()));
    QObject::connect(actionKeyboard_Shortcuts, SIGNAL(triggered()), this, SLOT(KeyShortcuts()));

    //Tools actions
    QObject::connect(actionGenerate_Project_Directory,SIGNAL(triggered()),this,SLOT(GenerateProjectDirectorySlot()));

    QObject::connect(InterpolTypeComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(InterpolTypeComboBoxChanged(int)));
    QObject::connect(TensInterpolComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(TensorInterpolComboBoxChanged(int)));
    QObject::connect(RegMethodcomboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(RegMethodComboBoxChanged(int)));

    QObject::connect(DefaultButton, SIGNAL(clicked()), this, SLOT(ConfigDefault()));
    QObject::connect(GAPath, SIGNAL(editingFinished()), this, SLOT(testGA())); // test the version of GreedyAtlas automatically when the text is changed manually ( not by a setText() )
    QObject::connect(DTIRegPath, SIGNAL(editingFinished()), this, SLOT(testDTIReg())); // test the version of DTI-Reg automatically when the text is changed manually ( not by a setText() )

    QObject::connect(GridProcesscheckBox, SIGNAL(stateChanged(int)), this, SLOT(GridProcesscheckBoxHasChanged(int)));
    QObject::connect(OutputFolderLineEdit, SIGNAL(textChanged(QString)), this, SLOT(OutputFolderLineEditHasChanged(QString)));

    QObject::connect(AffineQCButton, SIGNAL(clicked()), this, SLOT(DisplayAffineQC()));
    QObject::connect(DeformQCButton, SIGNAL(clicked()), this, SLOT(DisplayDeformQC()));
    QObject::connect(ResampQCButton, SIGNAL(clicked()), this, SLOT(DisplayResampQC()));
    QObject::connect(CleanOutputPushButton, SIGNAL(clicked()), this, SLOT(CleanOutputFolder()));

/* Browse software path Buttons */
    QSignalMapper *SoftButtonMapper = new QSignalMapper();
    QObject::connect(SoftButtonMapper, SIGNAL(mapped(int)), this, SLOT( BrowseSoft(int) ));

    QObject::connect(ImagemathButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
    SoftButtonMapper->setMapping(ImagemathButton,1);
    QObject::connect(ResampButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
    SoftButtonMapper->setMapping(ResampButton,2);
    QObject::connect(CropDTIButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
    SoftButtonMapper->setMapping(CropDTIButton,3);
    QObject::connect(dtiprocButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
    SoftButtonMapper->setMapping(dtiprocButton,4);
    QObject::connect(BRAINSFitButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
    SoftButtonMapper->setMapping(BRAINSFitButton,5);
    QObject::connect(GAButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
    SoftButtonMapper->setMapping(GAButton,6);
    QObject::connect(dtiavgButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
    SoftButtonMapper->setMapping(dtiavgButton,7);
    QObject::connect(DTIRegButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
    SoftButtonMapper->setMapping(DTIRegButton,8);
    QObject::connect(unuButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
    SoftButtonMapper->setMapping(unuButton,9);
    QObject::connect(MriWatcherButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
    SoftButtonMapper->setMapping(MriWatcherButton,10);
    QObject::connect(ITKTransformToolsButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
    SoftButtonMapper->setMapping(ITKTransformToolsButton,11);

/* Reset software path Buttons */
    QSignalMapper *ResetSoftButtonMapper = new QSignalMapper();
    QObject::connect(ResetSoftButtonMapper, SIGNAL(mapped(int)), this, SLOT( ResetSoft(int) ));

    QObject::connect(ImagemathResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
    ResetSoftButtonMapper->setMapping(ImagemathResetButton,1);
    QObject::connect(ResampResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
    ResetSoftButtonMapper->setMapping(ResampResetButton,2);
    QObject::connect(CropDTIResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
    ResetSoftButtonMapper->setMapping(CropDTIResetButton,3);
    QObject::connect(dtiprocResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
    ResetSoftButtonMapper->setMapping(dtiprocResetButton,4);
    QObject::connect(BRAINSFitResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
    ResetSoftButtonMapper->setMapping(BRAINSFitResetButton,5);
    QObject::connect(GAResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
    ResetSoftButtonMapper->setMapping(GAResetButton,6);
    QObject::connect(dtiavgResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
    ResetSoftButtonMapper->setMapping(dtiavgResetButton,7);
    QObject::connect(DTIRegResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
    ResetSoftButtonMapper->setMapping(DTIRegResetButton,8);
    QObject::connect(unuResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
    ResetSoftButtonMapper->setMapping(unuResetButton,9);
    QObject::connect(MriWatcherResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
    ResetSoftButtonMapper->setMapping(MriWatcherResetButton,10);
    QObject::connect(ITKTransformToolsResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
    ResetSoftButtonMapper->setMapping(ITKTransformToolsResetButton,11);

/* When any value changes, the value of m_ParamSaved is set to 0 */
    QObject::connect(TemplateLineEdit, SIGNAL(textChanged(QString)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(OutputFolderLineEdit, SIGNAL(textChanged(QString)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(SafetyMargincheckBox, SIGNAL(stateChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(BFAffineTfmModecomboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(OverwritecheckBox, SIGNAL(stateChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(SL4checkBox, SIGNAL(stateChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(SL2checkBox, SIGNAL(stateChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(SL1checkBox, SIGNAL(stateChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(NbLoopsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(SL4spinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(nbIter4SpinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(alpha4DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(beta4DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(gamma4DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(maxPerturbation4DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(SL2spinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(nbIter2SpinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(alpha2DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(beta2DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(gamma2DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(maxPerturbation2DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(SL1spinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(nbIter1SpinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(alpha1DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(beta1DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(gamma1DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(maxPerturbation1DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(TensTfmComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(InterpolTypeComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(TensInterpolComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(DTIRegExtraPathlineEdit, SIGNAL(textChanged(QString)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(RegMethodcomboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(m_windowComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(m_BSplineComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(m_nologComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(m_BRegTypeComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(m_TfmModeComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(m_NbPyrLevSpin, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(m_PyrLevItLine, SIGNAL(textChanged(QString)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(m_ARegTypeComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(m_TfmStepLine, SIGNAL(textChanged(QString)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(m_IterLine, SIGNAL(textChanged(QString)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(m_SimMetComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(m_SimParamDble, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(m_GSigmaDble, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(m_SmoothOffCheck, SIGNAL(stateChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(NbThreadsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
    QObject::connect(GridProcesscheckBox, SIGNAL(stateChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
  } //if(!m_noGUI)

  m_FromConstructor=1; // do not test GA path if 'Default' called from constructor -> test at the end of constructor

/* SET the soft config */
  // std::vector

  std::string DTIABExecutablePath = itksys::SystemTools::GetRealPath( itksys::SystemTools::GetFilenamePath(commandRan).c_str() );
  if( DTIABExecutablePath=="" ) // if DTIAtlasBuilder called without path (e.g. $ DTIAtlasBuilder) => either found in the PATH or in the current dir => FindProgram will find it
  {
    DTIABExecutablePath = itksys::SystemTools::GetFilenamePath( itksys::SystemTools::FindProgram( itksys::SystemTools::GetFilenameName(commandRan).c_str() ).c_str() );
    // get the name of the executable used (i.e. DTIAtlasBuilder, DTIAtlasBuilder_vX.X (GetFilenameName), then search it in the PATH or current dir (FindProgram) and get the parent dir (GetFilenamePath)
  }

  // set the path to the executable directory for FindProgram
  m_FindProgramDTIABExecDirVec.push_back(DTIABExecutablePath); // FindProgram will search in the executable directory too

  // If DTIAB is built as an SlicerExtension, give the path to the folder containing external non cli tools
  // If no SicerExtension, find_program will just search there and find nothing -> not an issue
  m_DTIABSlicerExtensionExternalBinDir = DTIABExecutablePath + "/../ExternalBin";
  m_FindProgramDTIABExecDirVec.push_back(m_DTIABSlicerExtensionExternalBinDir);

  // On Mac if Slicer Ext, paths to BRAINS (compiled within Slicer) are not set in the PATH env var, so add it to the search vector
  if(DTIAtlasBuilder_BUILD_SLICER_EXTENSION && (std::string)Platform=="mac")
  {
    std::string SlicerExtMacBRAINSPath = m_DTIABSlicerExtensionExternalBinDir + "/../../../cli_modules";
    m_FindProgramDTIABExecDirVec.push_back(SlicerExtMacBRAINSPath);
  }

  // Because ANTS is in m_DTIABSlicerExtensionExternalBinDir when Slicer Ext
  if(DTIAtlasBuilder_BUILD_SLICER_EXTENSION)
  {
    // There are always 2 elements at least in m_FindProgramDTIABExecDirVec when build as an extension: DTIABExecutablePath and DTIABExecutablePath + "/../ExternalBin"
    QString listPath = m_FindProgramDTIABExecDirVec[ 0 ].c_str() ;
    for( size_t i = 1 ; i < m_FindProgramDTIABExecDirVec.size() ; i++ )
    {
      listPath  += QString(",") + m_FindProgramDTIABExecDirVec[i].c_str() ;
    }
    DTIRegExtraPathlineEdit->setText( listPath ) ;
  }

  // m_FindProgramDTIABExecDirVec.push_back(commandRan + "/../../../../ResampleDTIlogEuclidean/" + std::string(Slicer_CLIMODULES_BIN_DIR));
  // m_FindProgramDTIABExecDirVec.push_back(commandRan + "/../../../../ResampleDTIlogEuclidean/" + std::string(Slicer_CLIMODULES_BIN_DIR) + "/../ExternalBin");
  // m_FindProgramDTIABExecDirVec.push_back(commandRan + "/../../../../DTI-Reg/" + std::string(Slicer_CLIMODULES_BIN_DIR));
  // m_FindProgramDTIABExecDirVec.push_back(commandRan + "/../../../../DTI-Reg/" + std::string(Slicer_CLIMODULES_BIN_DIR) + "/../ExternalBin");
  // m_FindProgramDTIABExecDirVec.push_back(commandRan + "/../../../../DTIProcess/" + std::string(Slicer_CLIMODULES_BIN_DIR));
  // m_FindProgramDTIABExecDirVec.push_back(commandRan + "/../../../../DTIProcess/" + std::string(Slicer_CLIMODULES_BIN_DIR) + "/../ExternalBin");
  m_FindProgramDTIABExecDirVec.push_back(commandRan + "/ResampleDTIlogEuclidean/bin/");
  m_FindProgramDTIABExecDirVec.push_back(commandRan + "/DTI-Reg/bin/" );
  m_FindProgramDTIABExecDirVec.push_back(commandRan + "/DTIProcess/bin/");
  m_FindProgramDTIABExecDirVec.push_back(commandRan + "/niral_utilities/bin/");
  m_FindProgramDTIABExecDirVec.push_back(commandRan + "/AtlasWerks/bin/" );
  m_FindProgramDTIABExecDirVec.push_back(commandRan + "/teem/bin/" );
  m_FindProgramDTIABExecDirVec.push_back(commandRan + "/BRAINSTools/bin/" );
  m_FindProgramDTIABExecDirVec.push_back(commandRan + "/MriWatcher/bin/");
  m_FindProgramDTIABExecDirVec.push_back(commandRan + "/ITKTransformTools/bin/");
  
  // look for the programs with the itk function
  ConfigDefault(commandRan);

  // Look for the config file in the executable directory
  //std::string SoftConfigPath= DTIABExecutablePath + "/DTIAtlasBuilderSoftConfig.txt";
  std::string SoftConfigPath= m_ExecutableDir + "/DTIAtlasBuilderSoftConfig.txt";
  std::cout << m_ExecutableDir;

  if( itksys::SystemTools::GetPermissions(SoftConfigPath.c_str(),ITKmode_F_OK) )
  {
    if( LoadConfig(QString( SoftConfigPath.c_str() )) == -1 )
    {
      m_ErrorDetectedInConstructor=true; // if file exists
    }
  }

  // Look for the config file in the current directory
  std::string CurrentPath = itksys::SystemTools::GetRealPath( itksys::SystemTools::GetCurrentWorkingDirectory().c_str() ); //GetRealPath() to remove symlinks
  SoftConfigPath = CurrentPath + "/DTIAtlasBuilderSoftConfig.txt";

  if( itksys::SystemTools::GetPermissions( SoftConfigPath.c_str() , ITKmode_F_OK) )
  {
    if( LoadConfig(QString( SoftConfigPath.c_str() )) == -1 )
    {
      m_ErrorDetectedInConstructor=true; // if file exists
    }
  }

  // Look for the config file in the env variable
  const char * value = itksys::SystemTools::GetEnv("DTIAtlasBuilderSoftPath"); // C function = const char * getenv(const char *)
  if (value!=NULL) 
  {
    printf ("| Environment variable read. The config file is \'%s\'\n",value);
    if( LoadConfig(QString(value)) == -1 )
    {
      m_ErrorDetectedInConstructor=true; // replace the paths by the paths given in the config file
    }
  }
  else
  {
    std::cout<<"| No environment variable found"<<std::endl;
  }

/* Look for the parameter file in the current directory */ // Load only if no param file given and if CSV file is given, only load the given CSV file.
  std::string ParamPath = CurrentPath + "/DTIAtlasBuilderParameters.txt";
  if( ParamFile.empty() && itksys::SystemTools::GetPermissions( ParamPath.c_str() , ITKmode_F_OK) )
  {
    if( LoadParameters(QString( ParamPath.c_str() ), !CSVFile.empty()) == -1 )
    {
      m_ErrorDetectedInConstructor=true; 
    }
  }

/* Load Parameters from Command Line => cmd line arguments a taking into account at last and change the parameters at last because they have priority */
  if( !ParamFile.empty() )
  {
    if( LoadParameters(QString(ParamFile.c_str()), !CSVFile.empty()) == -1 )
    {
      m_ErrorDetectedInConstructor=true;
    }
  }
  else if(m_noGUI) // no parameters and nogui => not possible
  {
    std::cout<<"| Please give a parameter file"<<std::endl; // command line display
    m_ErrorDetectedInConstructor=true;
  }

  if( !CSVFile.empty() )
  {
    if( ReadCSV( QString(CSVFile.c_str())) == -1 )
    {
      m_ErrorDetectedInConstructor=true;
    }
  }
  if( !ConfigFile.empty() )
  {
    if( LoadConfig( QString(ConfigFile.c_str())) == -1 )
    {
      m_ErrorDetectedInConstructor=true;
    }
  }

  m_FromConstructor=0;

/* NOW that all the files have been loaded => test if all the paths are here */
  bool GAFound=true;
  bool DTIRegFound=true;
  std::string notFound;

  if(ImagemathPath->text().isEmpty()) notFound = notFound + "> ImageMath\n";
  if(ResampPath->text().isEmpty())    notFound = notFound + "> ResampleDTIlogEuclidean\n";
  if(CropDTIPath->text().isEmpty())   notFound = notFound + "> CropDTI\n";
  if(dtiprocPath->text().isEmpty())   notFound = notFound + "> dtiprocess\n";
  if(BRAINSFitPath->text().isEmpty()) notFound = notFound + "> BRAINSFit\n";
  if(GAPath->text().isEmpty())
  {
    notFound = notFound + "> GreedyAtlas\n";
    GAFound=false; // so it will not test the version
  }
  if(dtiavgPath->text().isEmpty()) notFound = notFound + "> dtiaverage\n";
  if(DTIRegPath->text().isEmpty())
  {
    notFound = notFound + "> DTI-Reg\n";
    DTIRegFound=false; // so it will not test the version
  }
  if(unuPath->text().isEmpty()) notFound = notFound + "> unu\n";
  if(ITKTransformToolsPath->text().isEmpty()) notFound = notFound + "> ITKTransformTools\n";
  if(MriWatcherPath->text().isEmpty()) notFound = notFound + "> MriWatcher (Program will work, but QC will not be available)\n";

  if( !notFound.empty() )
  {
    if(!m_noGUI && !m_Testing) 
    {
      std::string text = "The following programs have not been found.\nPlease enter the path manually or open a configuration file:\n" + notFound;
      QMessageBox::warning(this, "Program missing", QString(text.c_str()) );
    }
    else
    {
      std::cout<<"| The following programs have not been found. Please give a configuration file or modify it or enter the path manually in the GUI:\n"<< notFound <<std::endl;
    }
  }

  if(GAFound)
  {
    testGA(); // test the version of GreedyAtlas only if found
  }
  if(DTIRegFound)
  {
    testDTIReg(); // test the version of DTI-Reg only if found
  }
}

  /////////////////////////////////////////
 //           INITIALIZATIONS           //
/////////////////////////////////////////

void GUI::InitOptions()
{
/* Init options for Final AtlasBuilding param : Interpolation algo */
  m_optionStackLayout = new QStackedWidget;
  optionHLayout->addWidget(m_optionStackLayout);

  QWidget *emptyWidget= new QWidget;
  m_optionStackLayout->addWidget(emptyWidget);

  QLabel *windowLabel = new QLabel("Type:", this);
  m_windowComboBox = new QComboBox(this);
  m_windowComboBox->addItem("Hamming");
  m_windowComboBox->addItem("Cosine");
  m_windowComboBox->addItem("Welch");
  m_windowComboBox->addItem("Lanczos");
  m_windowComboBox->addItem("Blackman");
  m_windowComboBox->setCurrentIndex(1);
  QHBoxLayout *windowLayout= new QHBoxLayout;
  windowLayout->addWidget(windowLabel);
  windowLayout->addWidget(m_windowComboBox);
  QWidget *windowWidget= new QWidget;
  windowWidget->setLayout(windowLayout);
  m_optionStackLayout->addWidget(windowWidget);

  QLabel *BSplineLabel = new QLabel("Order:", this);
  m_BSplineComboBox = new QComboBox(this);
  m_BSplineComboBox->addItem("0");
  m_BSplineComboBox->addItem("1");
  m_BSplineComboBox->addItem("2");
  m_BSplineComboBox->addItem("3");
  m_BSplineComboBox->addItem("4");
  m_BSplineComboBox->addItem("5");
  m_BSplineComboBox->setCurrentIndex(3);
  QHBoxLayout *BSplineLayout= new QHBoxLayout;
  BSplineLayout->addWidget(BSplineLabel);
  BSplineLayout->addWidget(m_BSplineComboBox);
  QWidget *BSplineWidget= new QWidget;
  BSplineWidget->setLayout(BSplineLayout);
  m_optionStackLayout->addWidget(BSplineWidget);

  m_optionStackLayout->setCurrentIndex(0);

/* Init options for Final AtlasBuilding param : Tensor Interpolation */
  m_logOptionStackLayout = new QStackedWidget;
  logOptionHLayout->addWidget(m_logOptionStackLayout);

  QWidget *logEmptyWidget= new QWidget;
  m_logOptionStackLayout->addWidget(logEmptyWidget);

  QLabel *nologLabel = new QLabel("Correction:", this);
  m_nologComboBox = new QComboBox(this);
  m_nologComboBox->addItem("Zero");
  m_nologComboBox->addItem("None");
  m_nologComboBox->addItem("Absolute Value");
  m_nologComboBox->addItem("Nearest");
  m_nologComboBox->setCurrentIndex(1);
  QHBoxLayout *nologLayout= new QHBoxLayout;
  nologLayout->addWidget(nologLabel);
  nologLayout->addWidget(m_nologComboBox);
  QWidget *nologWidget= new QWidget;
  nologWidget->setLayout(nologLayout);
  m_logOptionStackLayout->addWidget(nologWidget);

  m_logOptionStackLayout->setCurrentIndex(0);

/* Init options for DTI-Reg: Reg Method */
  m_DTIRegOptionStackLayout = new QStackedWidget;
  DTIRegOptionsVLayout->addWidget(m_DTIRegOptionStackLayout);

  /*BRAINS*/
  QHBoxLayout *BRAINSHLayout = new QHBoxLayout;
  QVBoxLayout *BRAINSLabelVLayout = new QVBoxLayout;
  BRAINSHLayout->addLayout(BRAINSLabelVLayout);
  QVBoxLayout *BRAINSWidgetVLayout = new QVBoxLayout;
  BRAINSHLayout->addLayout(BRAINSWidgetVLayout);

  QLabel *BRegTypeLabel = new QLabel("Registration Type:", this);
  BRAINSLabelVLayout->addWidget(BRegTypeLabel);
  m_BRegTypeComboBox = new QComboBox(this);
  m_BRegTypeComboBox->addItem("None");
  m_BRegTypeComboBox->addItem("Rigid");
  m_BRegTypeComboBox->addItem("BSpline");
  m_BRegTypeComboBox->addItem("Diffeomorphic");
  m_BRegTypeComboBox->addItem("Demons");
  m_BRegTypeComboBox->addItem("FastSymmetricForces");
  m_BRegTypeComboBox->setCurrentIndex(3);
  BRAINSWidgetVLayout->addWidget(m_BRegTypeComboBox);

  QLabel *TfmModeLabel = new QLabel("Transform Mode:", this);
  BRAINSLabelVLayout->addWidget(TfmModeLabel);
  m_TfmModeComboBox = new QComboBox(this);
  m_TfmModeComboBox->addItem("Off");
  m_TfmModeComboBox->addItem("useMomentsAlign");
  m_TfmModeComboBox->addItem("useCenterOfHeadAlign");
  m_TfmModeComboBox->addItem("useGeometryAlign");
  m_TfmModeComboBox->addItem("Use computed affine transform");
  m_TfmModeComboBox->setCurrentIndex(4);
  BRAINSWidgetVLayout->addWidget(m_TfmModeComboBox);

  QLabel *NbPyrLevLabel = new QLabel("Number Of Pyramid Levels:", this);
  BRAINSLabelVLayout->addWidget(NbPyrLevLabel);
  m_NbPyrLevSpin = new QSpinBox(this);
  m_NbPyrLevSpin->setValue(5);
  BRAINSWidgetVLayout->addWidget(m_NbPyrLevSpin);

  QLabel *PyrLevItLabel = new QLabel("Number Of Iterations for the Pyramid Levels:", this);
  BRAINSLabelVLayout->addWidget(PyrLevItLabel);
  m_PyrLevItLine = new QLineEdit(this);
  m_PyrLevItLine->setText("300,50,30,20,15");
  BRAINSWidgetVLayout->addWidget(m_PyrLevItLine);

  QWidget *BRAINSWidget = new QWidget;
  BRAINSWidget->setLayout(BRAINSHLayout);
  m_DTIRegOptionStackLayout->addWidget(BRAINSWidget);

  /*ANTS*/
  QHBoxLayout *ANTSHLayout = new QHBoxLayout;
  QVBoxLayout *ANTSLabelVLayout = new QVBoxLayout;
  ANTSHLayout->addLayout(ANTSLabelVLayout);
  QVBoxLayout *ANTSWidgetVLayout = new QVBoxLayout;
  ANTSHLayout->addLayout(ANTSWidgetVLayout);

  QLabel *ARegTypeLabel = new QLabel("Registration Type:", this);
  ANTSLabelVLayout->addWidget(ARegTypeLabel);
  m_ARegTypeComboBox = new QComboBox(this);
  m_ARegTypeComboBox->addItem("None");
  m_ARegTypeComboBox->addItem("Rigid");
  m_ARegTypeComboBox->addItem("Elast");
  m_ARegTypeComboBox->addItem("Exp");
  m_ARegTypeComboBox->addItem("GreedyExp");
  m_ARegTypeComboBox->addItem("GreedyDiffeo (SyN)");
  m_ARegTypeComboBox->addItem("SpatioTempDiffeo (SyN)");
  m_ARegTypeComboBox->setCurrentIndex(5);
  QObject::connect(m_ARegTypeComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(ANTSRegTypeChanged( int )));
  ANTSWidgetVLayout->addWidget(m_ARegTypeComboBox);

  QLabel *TfmStepLabel = new QLabel("Transformation Step:", this);
  ANTSLabelVLayout->addWidget(TfmStepLabel);
  m_TfmStepLine = new QLineEdit(this);
  m_TfmStepLine->setText("0.25");
  ANTSWidgetVLayout->addWidget(m_TfmStepLine);

  QLabel *IterLabel = new QLabel("Iterations:", this);
  ANTSLabelVLayout->addWidget(IterLabel);
  m_IterLine = new QLineEdit(this);
  m_IterLine->setText("100x50x25");
  ANTSWidgetVLayout->addWidget(m_IterLine);

  QLabel *SimMetLabel = new QLabel("Similarity Metric:", this);
  ANTSLabelVLayout->addWidget(SimMetLabel);
  m_SimMetComboBox = new QComboBox(this);
  m_SimMetComboBox->addItem("Cross-Correlation (CC)");
  m_SimMetComboBox->addItem("Mutual Information (MI)");
  m_SimMetComboBox->addItem("Mean Square Difference (MSQ)");
  m_SimMetComboBox->setCurrentIndex(0);
  QObject::connect(m_SimMetComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(SimMetChanged( int )));
  ANTSWidgetVLayout->addWidget(m_SimMetComboBox);

  m_SimParamLabel = new QLabel("Region Radius:", this);
  ANTSLabelVLayout->addWidget(m_SimParamLabel);
  m_SimParamDble = new QDoubleSpinBox(this);
  m_SimParamDble->setValue(2);
  ANTSWidgetVLayout->addWidget(m_SimParamDble);

  QLabel *GSigmaLabel = new QLabel("Gaussian Sigma:", this);
  ANTSLabelVLayout->addWidget(GSigmaLabel);
  m_GSigmaDble = new QDoubleSpinBox(this);
  m_GSigmaDble->setValue(3);
  ANTSWidgetVLayout->addWidget(m_GSigmaDble);

  m_SmoothOffCheck = new QCheckBox("Gaussian Smoothing Off", this);
  m_SmoothOffCheck->setChecked(false);
  m_SmoothOffCheck->setLayoutDirection(Qt::RightToLeft);
  ANTSLabelVLayout->addWidget(m_SmoothOffCheck);
  QLabel *emptyL = new QLabel("", this);
  ANTSWidgetVLayout->addWidget(emptyL);

  QWidget *ANTSWidget = new QWidget;
  ANTSWidget->setLayout(ANTSHLayout);
  m_DTIRegOptionStackLayout->addWidget(ANTSWidget);


  m_DTIRegOptionStackLayout->setCurrentIndex(0); //BRAINS is the default
}

  /////////////////////////////////////////
 //                Hierarchy            //
/////////////////////////////////////////

void GUI::newHierarchyProject(){
  bool ok;
  QString name= QInputDialog::getText(this, QString("Project Name"), QString("Project Name : "), QLineEdit::Normal, "FinalAtlas", &ok);
  if(ok){
    m_HierarchyModel->initialize(name);
    QModelIndex idx=m_HierarchyModel->getCurrentItem()->index();
    caseHierarchyTreeView->setModel(m_HierarchyModel);
    enableTreeViewWidget(true);
    caseHierarchyTreeView->clicked(idx);
    caseHierarchyTreeView->selectionModel()->select(QItemSelection(idx,idx),QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    caseHierarchyTreeView->setExpanded(m_HierarchyModel->getCurrentItem()->index(),1);
    SelectCasesLabel->setText(QString(""));
    //std::cout << m_HierarchyModel->toString().toStdString() << std::endl;
  }
}
void GUI::openHierarchyFile() /*SLOT*/
{
  QString fileBrowse=QFileDialog::getOpenFileName(this, "Open Case Hierarchy File", QString(), "JSON File (*.json);;All Files (*.*)");
  if(!fileBrowse.isEmpty())
  {
    //load and set the caseHierarchyTreeView
    try{
      m_HierarchyModel->loadFile(fileBrowse);
      QModelIndex idx=m_HierarchyModel->getCurrentItem()->index();
      enableTreeViewWidget(true);
      caseHierarchyTreeView->clicked(idx);
      caseHierarchyTreeView->selectionModel()->select(QItemSelection(idx,idx),QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
      caseHierarchyTreeView->setExpanded(m_HierarchyModel->getCurrentItem()->index(),1);
      SelectCasesLabel->setText(fileBrowse);
    }catch(const std::exception &e){
      m_HierarchyModel->clear();
      QMessageBox::warning(this,"Failed to load","Failed to load hierarchy file.");
    }
    //std::cout << m_HierarchyModel->toString().toStdString() << std::endl;
  }
}

void GUI::openProjectDirectorySlot() // 
{
  QString CurrentOutputFolder = "";
  if( !OutputFolderLineEdit->text().isEmpty() )
  {
    CurrentOutputFolder = OutputFolderLineEdit->text();
  }

  QString OutputBrowse = QFileDialog::getExistingDirectory( this, "Find Directory", CurrentOutputFolder );
  if(!OutputBrowse.isEmpty())
  {
    OutputFolderLineEdit->setText(OutputBrowse);
    //Load hiearchical build file
    try{
      QString filename = OutputBrowse + "/common/h-build.json";
      m_HierarchyModel->loadFile(filename);
      QModelIndex idx=m_HierarchyModel->getCurrentItem()->index();
      enableTreeViewWidget(true);
      caseHierarchyTreeView->clicked(idx);
      caseHierarchyTreeView->selectionModel()->select(QItemSelection(idx,idx),QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
      caseHierarchyTreeView->setExpanded(m_HierarchyModel->getCurrentItem()->index(),1);
      SelectCasesLabel->setText(filename);
    }catch(const std::exception &e){
      m_HierarchyModel->clear();
      QMessageBox::warning(this,"Failed to load","Failed to load hierarchy file.");
    }
    //Load parameter file
    try{
      QString paramfilename=OutputBrowse+"/common/DTIAtlasBuilderParameters.txt";
      LoadParameters(paramfilename,false);
    }catch(const std::exception &e){
      QMessageBox::warning(this,"Failed to load","Failed to load parameter file.");
    }

  }
}

void GUI::saveHierarchyFile() /*SLOT*/
{
  QString fileBrowse=QFileDialog::getSaveFileName(this, "Save Case Hierarchy File", QString(), "JSON File (*.json);;All Files (*.*)");
  if(!fileBrowse.isEmpty())
  {
    //load and set the caseHierarchyTreeView
    m_HierarchyModel->saveFile(fileBrowse);
    SelectCasesLabel->setText(fileBrowse);
    //std::cout << m_HierarchyModel->toString().toStdString() << std::endl;
  }
}

void GUI::addNode(){
  std::cout << " Adding node" << std::endl;
  // first to change the node type to 'node' to current selected item
  // second to choose the node type of generated node. (node / end_node)

  //production codes

  bool ok;
  QString prefix="";
  QString currentTag=m_HierarchyModel->getCurrentTag();
  QString currentType=m_HierarchyModel->getCurrentType();
  int dlg_ret=QMessageBox::Yes;

  if(m_HierarchyModel->checkCaseExists(currentTag))
  {
      dlg_ret=QMessageBox::warning(this,"Containing filed","This operation removes the current node's case list, are you sure to proceed?",QMessageBox::No|QMessageBox::Yes);
  }
  if(dlg_ret==QMessageBox::Yes){
      if(m_HierarchyModel->getCurrentItem()!=m_HierarchyModel->getRoot()){
        prefix=m_HierarchyModel->getCurrentTag() + QString("-");
      }
      QString name= QInputDialog::getText(this, QString("Input the name of new atlas node"), QString("New atlas name : ") + prefix, QLineEdit::Normal, "myatlas", &ok);

      if(ok){
        if(m_HierarchyModel->checkNodename(prefix+name)){
          QMessageBox::warning(this, "Failed to add a node", QString("There is an existing node with the name of ")+name);
        }else{
         m_HierarchyModel->addNode(prefix+name);
         caseHierarchyTreeView->setExpanded(m_HierarchyModel->getCurrentItem()->index(),1);
         caseHierarchyTreeView->clicked(m_HierarchyModel->getCurrentItem()->index());     
        }   
      }
  }
}

void GUI::removeNode(){
  //std::cout << " Current node " << m_HierarchyModel->getCurrentItem()->text().toStdString() << std::endl;
  bool ok;
  QString currentTag=m_HierarchyModel->getCurrentTag();
  if(m_HierarchyModel->isRoot(currentTag)){
    QMessageBox::warning(this,"Warning","Root node cannot be removed. If you want to start a new project, select New project in menu");
  }else{
    int ret=QMessageBox::warning(this,"Remove Node","Are you sure to delete the node :  " + currentTag + " ?",QMessageBox::No | QMessageBox::Yes);
    if(ret==QMessageBox::Yes){
        m_HierarchyModel->removeCurrentNode();
        QModelIndexList idxl=caseHierarchyTreeView->selectionModel()->selectedIndexes();
        foreach(const QModelIndex &idx, idxl){
          //std::cout << idx.data().toString().toStdString() << std::endl;
          m_HierarchyModel->setCurrentTag(idx);
        }
        caseHierarchyTreeView->clicked(m_HierarchyModel->getCurrentItem()->index());
      }
  }
}

void GUI::treeViewItemSelected(const QModelIndex idx){

  //std::cout << "item clicked (" << idx.row()<<","<<idx.column() << ")" <<std::endl;
  QString tag = idx.data().toString();
  //std::cout << tag.toStdString() << std::endl;
  m_HierarchyModel->setCurrentTag(idx);
  QString _type=m_HierarchyModel->getCurrentType();
  CaseListWidget->clear();
  QStringList CL=m_HierarchyModel->getFileList(tag);
  AddCasesToListWidget(CL,QString(""));
  if(_type==QString("node")){
    enableCaseWidget(0);
  }else{
    enableCaseWidget(1);
  }

}

void GUI::treeViewItemChanged(const QModelIndex idx){
  QString tag = idx.data().toString();
  int res=m_HierarchyModel->changeCurrentNode(idx,tag);
  m_HierarchyModel->update();

  //std::cout << m_HierarchyModel->toString().toStdString() << std::endl;
}


void GUI::updateHierarchyFiles(){
  QStringList ql;
  QString nodename = m_HierarchyModel->getCurrentTag();
  for(int i=0; i < CaseListWidget->count() ;i++){
      QString s=CaseListWidget->item(i)->text().split(QString(": ")).at(1);
      //std::cout << s.toStdString() << std::endl;
      ql.append(s);
  }
  m_HierarchyModel->setFiles(nodename,ql);
}

void GUI::enableCaseWidget(bool tf){
  CaseListWidget->setEnabled(tf);
  AddPushButton->setEnabled(tf);
  RemovePushButton->setEnabled(tf);
  BrowseCSVPushButton->setEnabled(tf);
  SaveCSVPushButton->setEnabled(tf);
}

void GUI::enableTreeViewWidget(bool tf){
  caseHierarchyTreeView->setEnabled(tf);
  addNodeButton->setEnabled(tf);
  removeNodeButton->setEnabled(tf);
  saveHierarchyButton->setEnabled(tf);
  ComputepushButton->setEnabled(tf);
  StoppushButton->setEnabled(tf);
  AffineQCButton->setEnabled(tf);
  DeformQCButton->setEnabled(tf);
  CleanOutputPushButton->setEnabled(tf);
  ResampQCButton->setEnabled(tf);
}


  /////////////////////////////////////////
 //                CASES                //

/////////////////////////////////////////

bool GUI::FileIsParameterFile( std::string filepath ) // TODO
{
  if( itksys::SystemTools::GetFilenameLastExtension( filepath ) != ".txt" )
  {
    return false;
  }

  QFile file(filepath.c_str());
  if (file.open(QFile::ReadOnly))
  {
    QTextStream stream(&file);
    QString line = stream.readLine();
    QStringList list = line.split("=");
    if( ! list.at(0).contains("DTIAtlasBuilderParameterFileVersion") )
    {
      return false;
    }
  }

  return true;
}

bool GUI::FileIsSoftConfigFile( std::string filepath ) // TODO
{
  if( itksys::SystemTools::GetFilenameLastExtension( filepath ) != ".txt" )
  {
    return false;
  }

  QFile file(filepath.c_str());
  if (file.open(QFile::ReadOnly))
  {
    QTextStream stream(&file);

    QString line = stream.readLine();
    QStringList list = line.split("=");
    if( ! list.at(0).contains("ImageMath") )
    {
      return false;
    }

    line = stream.readLine();
    list = line.split("=");
    if( ! list.at(0).contains("ResampleDTIlogEuclidean") )
    {
      return false;
    }
  }

  return true;
}

void GUI::dragEnterEvent(QDragEnterEvent *event)
{
  event->acceptProposedAction();
}

void GUI::dropEvent(QDropEvent* event)
{
  const QMimeData* mimeData = event->mimeData();

  // check for needed mime type, a file or a list of files
  if( mimeData->hasUrls() )
  {
    QStringList pathList;
    QList<QUrl> urlList = mimeData->urls();
 
    // extract the local paths of the files
    for (int i = 0; i < urlList.size(); ++i)
    {
      QString CurrentFile = urlList.at(i).toLocalFile();
      if ( itksys::SystemTools::GetFilenameLastExtension( CurrentFile.toStdString() ) == ".csv" ) // load possible csv file(s)
      {
        ReadCSV( CurrentFile );
      }
      else if ( FileIsParameterFile( CurrentFile.toStdString() ) ) // load possible parameter file(s)
      {
        LoadParameters(CurrentFile,false);
      }
      else if ( FileIsSoftConfigFile( CurrentFile.toStdString() ) ) // load possible soft config file(s)
      {
        LoadConfig(CurrentFile);
      }
      else // other file -> maybe image: will be check in AddCasesToListWidget()
      {
        pathList.append( CurrentFile );
      }
    }
 
    AddCasesToListWidget(pathList, QString(""));
    updateHierarchyFiles();
  } // if mimedata hasurls

} // dropEvent

void GUI::OpenAddCaseBrowseWindow() /*SLOT*/
{
  QStringList CaseListBrowse = QFileDialog::getOpenFileNames(this, "Open Cases", m_lastCasePath, "ITK format Images (*.nrrd *.nhdr *.nii *.nii.gz *.*)");
  AddCasesToListWidget(CaseListBrowse, QString(""));
  updateHierarchyFiles();
}

void GUI::AddCasesToListWidget(QStringList CaseList, QString CSVfiletext) // called when drag&dropping any file or adding files with the + button or loading CSV file
{
  bool SomeFilesAreNotImages = false;

  // Add REAL PATH of cases to list
  for(int i=0; i<CaseList.count() ; i++)
  {
    std::string CaseRealPath = itksys::SystemTools::GetRealPath( CaseList.at(i).toStdString().c_str() );
    if( CheckCase( CaseRealPath, true ) ) // true for no display if error: just not add file
    {
      CaseListWidget->addItem( QString( CaseRealPath.c_str() ) );
    }
    else
    {
      SomeFilesAreNotImages = true; // to display warning
    }
  }

  if( SomeFilesAreNotImages && !m_noGUI && !m_Testing )
  {
    QMessageBox::warning(this, "Not DTI Images", "Some of the files added were not DTI images.\nThese files have not been loaded into the list." );
  }

  if(!CaseList.isEmpty())
  {
    if( CaseListWidget->count()>0 )
    {
      RemovePushButton->setEnabled(true);
      ComputepushButton->setEnabled(true);

      if( ! OutputFolderLineEdit->text().isEmpty() )
      {
        EnableQC();
      }
    }
    m_ParamSaved=0;
    //SelectCasesLabel->setText( CSVfiletext );
    m_lastCasePath = CaseList.last();

    CheckCasesIndex();
  }
  updateHierarchyFiles();
}

void GUI::RemoveSelectedCases() /*SLOT*/
{
  int NbOfSelectedItems = (CaseListWidget->selectedItems()).size();
  int NB=NbOfSelectedItems;
  int ItemRow;
  while(NbOfSelectedItems>0)
  {
    ItemRow = CaseListWidget->row( CaseListWidget->selectedItems().at(0) );
    CaseListWidget->takeItem(ItemRow);
    delete CaseListWidget->selectedItems().at(0);
    NbOfSelectedItems--;
  }
  if( NB>0 ) // only if some items were removed
  {
    if ( CaseListWidget->count()==0 )    
    {
      RemovePushButton->setEnabled(false);
      //ComputepushButton->setEnabled(false);
      //DisableQC();
    }
    m_ParamSaved=0;
    SelectCasesLabel->setText( QString("") );

    CheckCasesIndex();
  }
  updateHierarchyFiles();
}

void GUI::CheckCasesIndex() /* Check duplicates Change ids at the begining of the lines */
{
  // Check IDs at the beginning of the lines
  std::string text;
  for(int i=0; i < CaseListWidget->count() ;i++)
  {
    if( CaseListWidget->item(i)->text().contains(": ") ) text = IntOrDoubleToStr<int>(i+1) + ": " + CaseListWidget->item(i)->text().toStdString().substr( CaseListWidget->item(i)->text().split(":").at(0).size()+2 ); // from pos to the end
    else text = IntOrDoubleToStr<int>(i+1) + ": " + CaseListWidget->item(i)->text().toStdString();

    CaseListWidget->item(i)->setText( QString( text.c_str() ) );
  }

  // Check and remove possible duplicates in the list
  std::vector< std::string > UniqueCaseVector; // contains only one of each
  unsigned int NbUniqueCases = CaseListWidget->count();
  bool RemoveDuplicatesWarningDisplayed = false; // to display popup only once
  for(unsigned int i=0; i<NbUniqueCases; i++)
  {
    std::string CurrentCase = CaseListWidget->item(i)->text().toStdString().substr( CaseListWidget->item(i)->text().split(":").at(0).size()+2 );
    // substr: keep only what is after "id: " (.size() returns the nb of digits of the id and +2 is for ": " after the id number)
 
    // search and remove
    if( std::find( UniqueCaseVector.begin(), UniqueCaseVector.end(), CurrentCase ) != UniqueCaseVector.end() ) // not the only one: if found => != end
    {
      if( !m_noGUI && !m_Testing && !RemoveDuplicatesWarningDisplayed ) // for the first duplicate found, ask if want to remove, if no break
      {
        int ret = QMessageBox::warning(this,"Remove duplicates","Duplicated cases have been detected in the list.\nDo you want to remove these duplicates?",QMessageBox::No | QMessageBox::Yes);
        if(ret == QMessageBox::No) break; // if no remove break the for loop HERE so nothing removed
        RemoveDuplicatesWarningDisplayed = true;
      }

      CaseListWidget->takeItem(i); //remove current
      i--;
      NbUniqueCases--;
    }
    else // first one
    {
      UniqueCaseVector.push_back( CurrentCase );
    }
  }

} // CheckCasesIndex()

  /////////////////////////////////////////
 //               OUTPUT                //
/////////////////////////////////////////

void GUI::OpenOutputBrowseWindow() /*SLOT*/
{
  // If already an output folder, browse from this location
  QString CurrentOutputFolder = "";
  if( !OutputFolderLineEdit->text().isEmpty() )
  {
    CurrentOutputFolder = OutputFolderLineEdit->text();
  }

  QString OutputBrowse = QFileDialog::getExistingDirectory( this, "Find Directory", CurrentOutputFolder );
  if(!OutputBrowse.isEmpty())
  {
    OutputFolderLineEdit->setText(OutputBrowse);
  }
}

void GUI::CleanOutputFolder() /*SLOT*/
{
  // Remove all useless temporary images

  std::string PopupText = "This action will remove the content of all these folders:\n" + OutputFolderLineEdit->text().toStdString() + "/atlases\n" + OutputFolderLineEdit->text().toStdString() + "/final_atlas\n" + "Do you really want to remove them?";
  int ret = QMessageBox::warning(this, "Remove Files", QString( PopupText.c_str() ), QMessageBox::No | QMessageBox::Yes );

  if (ret == QMessageBox::Yes) 
  {
    std::cout<<"| Cleaning output folder...";

    std::vector< std::string > PathsToRemove;
    PathsToRemove.push_back(OutputFolderLineEdit->text().toStdString() + "/atlases");
    PathsToRemove.push_back(OutputFolderLineEdit->text().toStdString() + "/final_atlas");
    PathsToRemove.push_back(OutputFolderLineEdit->text().toStdString() + "/displacement_fields");

    for( unsigned int i = 0 ; i < PathsToRemove.size() ; ++i )
    {
      if( itksys::SystemTools::GetPermissions( PathsToRemove[i].c_str(), ITKmode_F_OK ) )
      {
        itksys::SystemTools::RemoveADirectory( PathsToRemove[i].c_str() );
      }
    }

//  itksys::SystemTools::RemoveFile(const char* source);

    std::cout<<"DONE"<<std::endl;

  } //   if (ret == QMessageBox::Yes) 
}

  /////////////////////////////////////////
 //               DTI-Reg               //
/////////////////////////////////////////

void GUI::OpenDTIRegExtraPathBrowseWindow() /*SLOT*/
{
  QString DTIRegExtraPathBrowse=QFileDialog::getExistingDirectory(this);
  if(!DTIRegExtraPathBrowse.isEmpty())
  {
    DTIRegExtraPathlineEdit->setText(DTIRegExtraPathBrowse);
  }
}

  /////////////////////////////////////////
 //              TEMPLATE               //
/////////////////////////////////////////

void GUI::OpenTemplateBrowseWindow() /*SLOT*/
{
  QString TemplateBrowse=QFileDialog::getOpenFileName(this, "Open Atlas Template", QString(), "ITK format Images (*.nrrd *.nhdr *.nii *.nii.gz *.*)");
  if(!TemplateBrowse.isEmpty())
  {
    TemplateLineEdit->setText(TemplateBrowse);
  }
}

  /////////////////////////////////////////
 //           QUALITY CONTROL           //
/////////////////////////////////////////

void GUI::DisplayAffineQC() /*SLOT*/
{
  std::string program = MriWatcherPath->text().toStdString() + " --viewAll";
  std::string pathList="";
  std::string path;
  for(int i=0; i < CaseListWidget->count() ;i++) 
  {
    path = OutputFolderLineEdit->text().toStdString() + "/final_atlas/1_Affine_Registration/Loop" + IntOrDoubleToStr<int>(NbLoopsSpinBox->value()) + "/" + itksys::SystemTools::GetFilenameWithoutExtension( CaseListWidget->item(i)->text().toStdString() ) + "_Loop" + IntOrDoubleToStr<int>(NbLoopsSpinBox->value()) + "_Final" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd";
    if( itksys::SystemTools::GetPermissions(path.c_str(), ITKmode_F_OK) ) pathList = pathList + " " + path;
  }
  if(NbLoopsSpinBox->value()>0)
  {
    path = OutputFolderLineEdit->text().toStdString() + "/final_atlas/1_Affine_Registration/Loop" + IntOrDoubleToStr<int>(NbLoopsSpinBox->value()-1) + "/Loop" + IntOrDoubleToStr<int>(NbLoopsSpinBox->value()-1) + "_" + ScalarMeasurementComboBox->currentText().toStdString() + "Average.nrrd";
  }
  else // if no looping (NbLoopsSpinBox->value()==0), an average is computed anyway for QC
  {
    path = OutputFolderLineEdit->text().toStdString() + "/final_atlas/1_Affine_Registration/Loop0/Loop0_" + ScalarMeasurementComboBox->currentText().toStdString() + "Average.nrrd";
  }
  if( itksys::SystemTools::GetPermissions(path.c_str(), ITKmode_F_OK) ) pathList = pathList + " " + path;

  if( pathList != "" )
  {
    program = program + pathList;
    std::cout<<"| Starting: "<<program<<std::endl;

    QProcess* QCQProcess = new QProcess;
    QCQProcess->start(program.c_str());
  }
  else
  {
    if(!m_noGUI && !m_Testing) 
    {
      std::string text = "None of the images to be displayed have been found.\nPlease check that these files exist:\n" + OutputFolderLineEdit->text().toStdString() + "/final_atlas/1_Affine_Registration/Loop" + IntOrDoubleToStr<int>(NbLoopsSpinBox->value()) + "/*_Loop" + IntOrDoubleToStr<int>(NbLoopsSpinBox->value()) + "_Final" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd";
      QMessageBox::warning(this, "No images found", text.c_str() );
    }
    else
    {
      std::cout<<"| None of the images to be displayed have been found. Please check that these files exist: "<< OutputFolderLineEdit->text().toStdString() << "/final_atlas/1_Affine_Registration/Loop" << IntOrDoubleToStr<int>(NbLoopsSpinBox->value()) << "/*_Loop" << IntOrDoubleToStr<int>(NbLoopsSpinBox->value()) << "_Final" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd" <<std::endl;
    }
  }
}

void GUI::DisplayDeformQC() /*SLOT*/
{
  std::string program = MriWatcherPath->text().toStdString() + " --viewAll";
  std::string pathList="";
  std::string path;
  for(int i=0; i < CaseListWidget->count() ;i++) 
  {
    path = OutputFolderLineEdit->text().toStdString() + "/final_atlas/3_Diffeomorphic_Atlas/" + itksys::SystemTools::GetFilenameWithoutExtension( CaseListWidget->item(i)->text().toStdString() ) + "_Diffeomorphic" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd";
    if( itksys::SystemTools::GetPermissions(path.c_str(), ITKmode_F_OK) ) pathList = pathList + " " + path;
  }
  path = OutputFolderLineEdit->text().toStdString() + "/final_atlas/3_Diffeomorphic_Atlas/DiffeomorphicAtlas" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd";
  if( itksys::SystemTools::GetPermissions(path.c_str(), ITKmode_F_OK) ) pathList = pathList + " " + path;

  if( pathList != "" )
  {
    program = program + pathList;
    std::cout<<"| Starting: "<<program<<std::endl;

    QProcess* QCQProcess = new QProcess;
    QCQProcess->start(program.c_str());
  }
  else
  {
    if(!m_noGUI && !m_Testing) 
    {
      std::string text = "None of the images to be displayed have been found.\nPlease check that these files exist:\n" + OutputFolderLineEdit->text().toStdString() + "/final_atlas/3_Diffeomorphic_Atlas/*_Diffeomorphic" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd";
      QMessageBox::warning(this, "No images found", text.c_str() );
    }
    else
    {
      std::cout<<"| None of the images to be displayed have been found. Please check that these files exist: "<< OutputFolderLineEdit->text().toStdString() << "/final_atlas/3_Diffeomorphic_Atlas/*_Diffeomorphic" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd" <<std::endl;
    }
  }
}


void GUI::DisplayResampQC() /*SLOT*/
{
  std::string program = MriWatcherPath->text().toStdString() + " --viewAll";
  std::string pathList="";
  std::string path;
  QStringList ql=m_HierarchyModel->getRootComponents();

  foreach(const QString &q, ql)
  {
    path = OutputFolderLineEdit->text().toStdString() + "/final_atlas/4_Final_Resampling/FinalTensors/" + itksys::SystemTools::GetFilenameWithoutExtension( q.toStdString() ) + "_FinalDeformed" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd";
    if( itksys::SystemTools::GetPermissions(path.c_str(), ITKmode_F_OK) ) pathList = pathList + " " + path;
  }
  // for(int i=0; i < CaseListWidget->count() ;i++) 
  // {
  //   path = OutputFolderLineEdit->text().toStdString() + "/final_atlas/4_Final_Resampling/FinalTensors/" + itksys::SystemTools::GetFilenameWithoutExtension( CaseListWidget->item(i)->text().toStdString() ) + "_FinalDeformed" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd";
  //   if( itksys::SystemTools::GetPermissions(path.c_str(), ITKmode_F_OK) ) pathList = pathList + " " + path;
  // }
  path = OutputFolderLineEdit->text().toStdString() + "/final_atlas/4_Final_Resampling/FinalAtlas" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd";
  if( itksys::SystemTools::GetPermissions(path.c_str(), ITKmode_F_OK) ) pathList = pathList + " " + path;

  if( pathList != "" )
  {
    program = program + pathList;
    std::cout<<"| Starting: "<<program<<std::endl;

    QProcess* QCQProcess = new QProcess;
    QCQProcess->start(program.c_str());
  }
  else
  {
    if(!m_noGUI && !m_Testing) 
    {
      std::string text = "None of the images to be displayed have been found.\nPlease check that these files exist:\n" + OutputFolderLineEdit->text().toStdString() + "/final_atlas/4_Final_Resampling/FinalTensors/*_FinalDeformed" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd";
      QMessageBox::warning(this, "No images found", text.c_str() );
    }
    else
    {
      std::cout<<"| None of the images to be displayed have been found. Please check that these files exist: "<< OutputFolderLineEdit->text().toStdString() << "/final_atlas/4_Final_Resampling/FinalTensors/*_FinalDeformed" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd" <<std::endl;
    }
  }
}

// void GUI::DisplayResampQC() /*SLOT*/
// {
//   std::string program = MriWatcherPath->text().toStdString() + " --viewAll";
//   std::string pathList="";
//   std::string path;
//   for(int i=0; i < CaseListWidget->count() ;i++) 
//   {
//     path = OutputFolderLineEdit->text().toStdString() + "/final_atlas/4_Final_Resampling/FinalTensors/" + itksys::SystemTools::GetFilenameWithoutExtension( CaseListWidget->item(i)->text().toStdString() ) + "_FinalDeformed" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd";
//     if( itksys::SystemTools::GetPermissions(path.c_str(), ITKmode_F_OK) ) pathList = pathList + " " + path;
//   }
//   path = OutputFolderLineEdit->text().toStdString() + "/final_atlas/4_Final_Resampling/FinalAtlas" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd";
//   if( itksys::SystemTools::GetPermissions(path.c_str(), ITKmode_F_OK) ) pathList = pathList + " " + path;

//   if( pathList != "" )
//   {
//     program = program + pathList;
//     std::cout<<"| Starting: "<<program<<std::endl;

//     QProcess* QCQProcess = new QProcess;
//     QCQProcess->start(program.c_str());
//   }
//   else
//   {
//     if(!m_noGUI && !m_Testing) 
//     {
//       std::string text = "None of the images to be displayed have been found.\nPlease check that these files exist:\n" + OutputFolderLineEdit->text().toStdString() + "/final_atlas/4_Final_Resampling/FinalTensors/*_FinalDeformed" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd";
//       QMessageBox::warning(this, "No images found", text.c_str() );
//     }
//     else
//     {
//       std::cout<<"| None of the images to be displayed have been found. Please check that these files exist: "<< OutputFolderLineEdit->text().toStdString() << "/final_atlas/4_Final_Resampling/FinalTensors/*_FinalDeformed" + ScalarMeasurementComboBox->currentText().toStdString() + ".nrrd" <<std::endl;
//     }
//   }
// }

void GUI::DisableQC() // disable QC buttons
{
  AffineQCButton->setEnabled(false);
  DeformQCButton->setEnabled(false);
  ResampQCButton->setEnabled(false);
}

void GUI::EnableQC() // enable QC buttons
{
  AffineQCButton->setEnabled(true);
  DeformQCButton->setEnabled(true);
  ResampQCButton->setEnabled(true);
}

  /////////////////////////////////////////
 //                EXIT                 //
/////////////////////////////////////////

void GUI::ExitProgram() /*SLOT*/
{
  std::cout<<"| End of the program"<<std::endl; // command line display
  delete m_scriptwriter;
  qApp->quit(); //end of Application: close the main window
}

void GUI::closeEvent(QCloseEvent* event)
{
  if(m_ScriptRunning)
  {
    int ret = QMessageBox::question(this,"Quit","The script is still running. If you exit now, the script will be aborted.\nDo you want to exit anyway?",QMessageBox::No | QMessageBox::Yes);
    if (ret == QMessageBox::No) 
    {
      event->ignore();
      return;
    }

    KillScriptQProcess();
  }

  else
  {
    while(m_ParamSaved==0)
    {
      int ret = QMessageBox::question(this,"Quit","Last parameters have not been saved.\nDo you want to save the last parameters?",QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes);
      if (ret == QMessageBox::Yes) SaveParametersSlot();
      else if (ret == QMessageBox::No) break;
      else if (ret == QMessageBox::Cancel) 
      {
        event->ignore();
        return;
      }
    }
  }

  delete m_scriptwriter;
  event->accept();
}

  /////////////////////////////////////////
 //               DATASET               //
/////////////////////////////////////////

void GUI::ReadCSVSlot() /*SLOT*/
{  
  QString CSVBrowse=QFileDialog::getOpenFileName(this, "Open CSV File", QString(), ".csv Files (*.csv)");

  if(!CSVBrowse.isEmpty())
  {
    ReadCSV(CSVBrowse);
  }
}

int GUI::ReadCSV(QString CSVfile)
{
  int ret=QMessageBox::Yes; // <=> replace current dataset by new
  if( CaseListWidget->count()!=0 )
  {
    ret = QMessageBox::question(this,"Case list not empty","There are already some cases listed.\nDo you want to replace them by the new dataset?\nIf No, the new dataset will be added to the existing cases.",QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes);

    if (ret == QMessageBox::Yes) CaseListWidget->clear();
    else if (ret == QMessageBox::Cancel) return 0;
  }

  QStringList CSVCaseList;

  if(!CSVfile.isEmpty())
  {
    if( itksys::SystemTools::GetPermissions(CSVfile.toStdString().c_str(), ITKmode_F_OK) ) // Test if the csv file exists => itksys::SystemTools::GetPermissions() returns true if ITKmode_F(file)_OK
    {
      QFile file(CSVfile);

      if (file.open(QFile::ReadOnly))
      {
        std::cout<<"| Loading csv file \'"<< CSVfile.toStdString() <<"\'..."; // command line display

        QTextStream stream(&file);
        while(!stream.atEnd()) //read all the lines
        {
          QString line = stream.readLine();
          if(line != "")
          {
            QStringList list = line.split(m_CSVseparator);
            if( list.at(0) != "id" && list.at(0) != "")
            {
              CSVCaseList.append( list.at(1) );
            }
          }
        }
  
        // if (ret == QMessageBox::Yes) AddCasesToListWidget(CSVCaseList, QString("Current CSV file : ") + CSVfile);
        // else AddCasesToListWidget(CSVCaseList, QString(""));
        if (ret == QMessageBox::Yes) AddCasesToListWidget(CSVCaseList, QString("Current CSV file : ") + CSVfile);
        else AddCasesToListWidget(CSVCaseList, QString(""));


        std::cout<<"DONE"<<std::endl; // command line display
      } 
      else
      {
        qDebug( "Could not open csv file");
        return -1;
      }
    }
    else
    {
      std::cout<<"| The given CSV file does not exist"<<std::endl; // command line display
      return -1;
    }
  }

  return 0;
}

void GUI::SaveCSVDatasetBrowse() /*SLOT*/
{  
  if(CaseListWidget->count()==0)
  {
    QMessageBox::critical(this, "No Dataset", "No Dataset");
    return;
  }

  QString CSVBrowseName = QFileDialog::getSaveFileName(this, tr("Save Dataset"),"./DTIAtlasBuilderDataSet.csv",tr("CSV File (*.csv)"));

  if(!CSVBrowseName.isEmpty())
  {

  QFile file(CSVBrowseName);

  if ( file.open( QFile::WriteOnly ) )
  {
    std::cout<<"| Generating Dataset csv file..."; // command line display

    QTextStream stream( &file );
    stream << QString("id") << m_CSVseparator << QString("Original DTI Image") << endl;
    for(int i=0; i < CaseListWidget->count() ;i++) stream << i+1 << m_CSVseparator << CaseListWidget->item(i)->text().remove(0, CaseListWidget->item(i)->text().split(":").at(0).size()+2 ) << endl;
    std::cout<<"DONE"<<std::endl; // command line display
  
    //SelectCasesLabel->setText( QString("Current CSV file : ") + CSVBrowseName );
    QMessageBox::information(this, "Saving succesful", "Dataset has been succesfully saved at" + CSVBrowseName);    
  }
  else qDebug( "Could not create file");

  }
}

void GUI::SaveCSVResults(int Crop, int nbLoops) // Crop = 0 if no cropping , 1 if cropping needed
{  

  QString csvPath;
  csvPath = m_OutputPath + QString("/common/DTIAtlasBuilderResults.csv");
  QFile file(csvPath);

  if ( file.open( QFile::WriteOnly ) )
  {
    std::cout<<"| Generating Results csv file..."; // command line display

    QTextStream stream( &file );

    stream << QString("id") << m_CSVseparator << QString("Original DTI Image");
    if(Crop==1) stream << m_CSVseparator << QString("Cropped DTI");
    stream << m_CSVseparator << ScalarMeasurementComboBox->currentText() << QString(" from original") << m_CSVseparator << QString("Affine transform") << m_CSVseparator << QString("Affine registered DTI") << m_CSVseparator << QString("Affine Registered ") << ScalarMeasurementComboBox->currentText() << m_CSVseparator << QString("Diffeomorphic Deformed ") << ScalarMeasurementComboBox->currentText() << m_CSVseparator << QString("Diffeomorphic Deformation field to Affine space") << m_CSVseparator << QString("Diffeomorphic Inverse Deformation field to Affine space") << m_CSVseparator << QString("Diffeomorphic DTI") << m_CSVseparator << QString("Diffeomorphic Deformation field to Original space") << m_CSVseparator << QString("DTI-Reg Final DTI") << endl;

    for(int i=0; i < CaseListWidget->count() ;i++) // for all cases
    {
      QString CaseID = QString( itksys::SystemTools::GetFilenameWithoutExtension( CaseListWidget->item(i)->text().toStdString() ).c_str() );

      stream << i+1 << m_CSVseparator << CaseListWidget->item(i)->text().remove(0, CaseListWidget->item(i)->text().split(":").at(0).size()+2 ); // Original DTI Image
      if(Crop==1) stream << m_CSVseparator << m_OutputPath + QString("/final_atlas/1_Affine_Registration/") << CaseID << QString("_croppedDTI.nrrd"); // Cropped DTI
      stream << m_CSVseparator << m_OutputPath + QString("/final_atlas/1_Affine_Registration/") << CaseID << QString("_") << ScalarMeasurementComboBox->currentText() << QString(".nrrd"); // Scalar measurement (FA or MD) from original
      stream << m_CSVseparator << m_OutputPath + QString("/final_atlas/1_Affine_Registration/Loop") << nbLoops << QString("/") << CaseID << QString("_Loop ") << nbLoops << QString("_LinearTrans.txt"); // Affine transform
      stream << m_CSVseparator << m_OutputPath + QString("/final_atlas/1_Affine_Registration/Loop") << nbLoops << QString("/") << CaseID << QString("_Loop ") << nbLoops << QString("_LinearTrans_DTI.nrrd"); // Affine registered DTI
      stream << m_CSVseparator << m_OutputPath + QString("/final_atlas/1_Affine_Registration/Loop") << nbLoops << QString("/") << CaseID << QString("_Loop ") << nbLoops << QString("_Final") << ScalarMeasurementComboBox->currentText() << QString(".nrrd"); // Affine Registered scalar measurement (FA or MD)
      stream << m_CSVseparator << m_OutputPath + QString("/final_atlas/2_NonLinear_Registration/") << CaseID << QString("_NonLinearTrans_") << ScalarMeasurementComboBox->currentText() << QString(".mhd"); // Diffeomorphic Deformed scalar measurement (FA or MD)
      stream << m_CSVseparator << m_OutputPath + QString("/final_atlas/2_NonLinear_Registration/") << CaseID << QString("_HField.mhd"); // Diffeomorphic Deformation H field to Affine space
      stream << m_CSVseparator << m_OutputPath + QString("/final_atlas/2_NonLinear_Registration/") << CaseID << QString("_InverseHField.mhd"); // Diffeomorphic Inverse Deformation H field to Affine space
      stream << m_CSVseparator << m_OutputPath + QString("/final_atlas/3_Diffeomorphic_Atlas/") << CaseID << QString("_DiffeomorphicDTI.nrrd"); // Diffeomorphic DTI
      stream << m_CSVseparator << m_OutputPath + QString("/final_atlas/4_Final_Resampling/FinalDeformationFields/") << CaseID << QString("_GlobalDisplacementField.nrrd"); // Diffeomorphic Deformation Displacement field to Original space
      stream << m_CSVseparator << m_OutputPath + QString("/final_atlas/4_Final_Resampling/FinalTensors/") << CaseID << QString("_FinalDeformedDTI.nrrd"); // DTI-Reg Final DTI
      stream << endl;
    }

    std::cout<<"DONE"<<std::endl; // command line display
    
  }
  else qDebug( "Could not create file");
}

  /////////////////////////////////////////
 //             PARAMETERS              //
/////////////////////////////////////////

void GUI::SaveParametersSlot() /*SLOT*/
{
  if(CaseListWidget->count()==0)
  {
    QMessageBox::critical(this, "No Dataset", "No Dataset");
    return;
  }

  QString ParamBrowseName=QFileDialog::getSaveFileName(this, tr("Save Parameter File"),"./DTIAtlasBuilderParameters.txt",tr("Text File (*.txt)"));

  if(!ParamBrowseName.isEmpty())
  {
    QString CSVFileName = ParamBrowseName.split(".").at(0) + QString(".csv"); // [Name].txt => [Name].csv
    SaveParameters(ParamBrowseName,CSVFileName);

    QMessageBox::information(this, "Saving succesful", "Parameters have been succesfully saved at" + ParamBrowseName);
    m_ParamSaved=1;
  }
}

void GUI::SaveParameters(QString ParamBrowseName,QString CSVFileName)
{
  if(!ParamBrowseName.isEmpty())
  {

  QFile file(ParamBrowseName);
  if ( file.open( QFile::WriteOnly ) )
  {
    std::cout<<"| Saving Parameters file..."; // command line display

    QTextStream stream( &file );

    stream << "DTIAtlasBuilderParameterFileVersion" << "=4" << endl;
    stream << "Output Folder=" << OutputFolderLineEdit->text() << endl;
    stream << "Atlas Template=" << TemplateLineEdit->text() << endl;
    stream << "Scalar Measurement=" << ScalarMeasurementComboBox->currentText() << endl;
    if(SafetyMargincheckBox->isChecked()) stream << "SafetyMargin=true" << endl;
    else  stream << "SafetyMargin=false" << endl;
    stream << "Loops for the registration=" << NbLoopsSpinBox->value() << endl;
    stream << "BRAINSFit Affine Tfm Mode=" << BFAffineTfmModecomboBox->currentText() << endl;
    if(OverwritecheckBox->isChecked()) stream << "Overwrite=true" << endl;
    else  stream << "Overwrite=false" << endl;

    int SL4,SL2,SL1;
    if(SL4checkBox->isChecked()) SL4=1;
    else SL4=0;
    stream << "Scale Level =" << SL4 << ","  << SL4spinBox->value() << ","  << nbIter4SpinBox->value() << "," << alpha4DoubleSpinBox->value() << "," << beta4DoubleSpinBox->value() << "," << gamma4DoubleSpinBox->value() << "," << maxPerturbation4DoubleSpinBox->value() << endl;
    if(SL2checkBox->isChecked()) SL2=1;
    else SL2=0;
    stream << "Scale Level =" << SL2 << ","  << SL2spinBox->value() << "," << nbIter2SpinBox->value() << "," << alpha2DoubleSpinBox->value() << "," << beta2DoubleSpinBox->value() << "," << gamma2DoubleSpinBox->value() << "," << maxPerturbation2DoubleSpinBox->value() << endl;
    if(SL1checkBox->isChecked()) SL1=1;
    else SL1=0;
    stream << "Scale Level =" << SL1 << ","  << SL1spinBox->value() << "," << nbIter1SpinBox->value() << "," << alpha1DoubleSpinBox->value() << "," << beta1DoubleSpinBox->value() << "," << gamma1DoubleSpinBox->value() << "," << maxPerturbation1DoubleSpinBox->value() << endl;

    stream << "Resampling Interpolation Algorithm=" << InterpolTypeComboBox->currentText() ;
    if( InterpolTypeComboBox->currentText()==QString("Windowed Sinc") ) stream << "=" << m_windowComboBox->currentText() << endl;
    else if( InterpolTypeComboBox->currentText()==QString("BSpline") ) stream << "=" << m_BSplineComboBox->currentText() << endl;
    else stream << endl;

    stream << "Tensor interpolation=" << TensInterpolComboBox->currentText() ;
    if( TensInterpolComboBox->currentText()==QString("Non Log Euclidean") ) stream << "=" << m_nologComboBox->currentText() << endl;
    else stream << endl;

    stream << "Tensor transformation=" << TensTfmComboBox->currentText()<< endl;

    stream << "DTIReg Extra Path=" << DTIRegExtraPathlineEdit->text()<< endl;

    stream << "DTIRegMethod=" << RegMethodcomboBox->currentText() ;
    if( RegMethodcomboBox->currentText()==QString("ANTS") ) 
    {
      stream << "=" << m_ARegTypeComboBox->currentText() << ";" << m_TfmStepLine->text() << ";" << m_IterLine->text() << ";" << m_SimMetComboBox->currentText() << ";" << m_SimParamDble->value() << ";" << m_GSigmaDble->value() ;
      if(m_SmoothOffCheck->isChecked()) stream << ";1" << endl;
      else stream << ";0" << endl;
    }
    else stream << "=" << m_BRegTypeComboBox->currentText() << ";" << m_TfmModeComboBox->currentText() << ";" << m_NbPyrLevSpin->value() << ";" << m_PyrLevItLine->text() << endl;

    if( GridProcesscheckBox->isChecked() )
    {
      stream << "GridProcessing=" << GridProcessCmdLineEdit->text() <<endl;
      stream << "GridAtlasProcessing=" << GridProcessAtlasCmdLineEdit->text() <<endl;
    }
    else
    {
      stream << "GridProcessing="<<endl;
      stream << "GridAtlasProcessing="<<endl;
    }

    stream << "NbThreads=" << NbThreadsSpinBox->value() << endl;

    //stream << "CSV Dataset File=" << CSVFileName << endl;
    stream << "nbLoopsDTIReg=" << NbLoopsDTIRegSpinBox->value() <<endl;
    std::cout<<"DONE"<<std::endl; // command line display

    // QFile filecsv(CSVFileName);
    // if ( filecsv.open( QFile::WriteOnly ) )
    // {
    //   std::cout<<"| Generating Dataset csv file..."; // command line display

    //   QTextStream streamcsv( &filecsv );
    //   streamcsv << QString("id") << m_CSVseparator << QString("Original DTI Image") << endl;
    //   for(int i=0; i < CaseListWidget->count() ;i++) streamcsv << i+1 << m_CSVseparator << CaseListWidget->item(i)->text().remove(0, CaseListWidget->item(i)->text().split(":").at(0).size()+2 ) << endl;
    //   std::cout<<"DONE"<<std::endl; // command line display
    
    //   //SelectCasesLabel->setText( QString("Current CSV file : ") + CSVFileName );
    // }
    // else 
    // {
    //   std::cout<<"FAILED"<<std::endl; // command line display
    //   qDebug( "Could not create csv file");
    // }

  }
  else qDebug( "Could not create parameter file");

  }
}



void GUI::LoadParametersSlot() /*SLOT*/
{
  QString ParamBrowse=QFileDialog::getOpenFileName(this, "Open Parameter File", QString(), ".txt Files (*.txt);;All Files (*.*)");

  if(!ParamBrowse.isEmpty())
  {
    LoadParameters(ParamBrowse,false);
  }
}

bool GUI::LoadParametersReturnTrueIfCorrupted(QString ReadParameter, const char* Parameter )
{
  if( ! ReadParameter.contains( QString(Parameter) ) )
  {
    if(!m_noGUI && !m_Testing) 
    {
      QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted.\nIt may have been modified manually.\nPlease save it again.");
      std::cout<<"FAILED"<<std::endl; // command line display
    }
    else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
    return true;
  }

  return false;
}

bool GUI::LoadParametersLoadScaleLevelReturnTrueIfCorrupted(  QStringList nbrs, 
                                                              QCheckBox* SLcheckBox, // & = reference : so we can change the value
                                                              QSpinBox* SLspinBox,
                                                              QSpinBox* nbIterSpinBox,
                                                              QDoubleSpinBox*& alphaDoubleSpinBox,
                                                              QDoubleSpinBox*& betaDoubleSpinBox,
                                                              QDoubleSpinBox*& gammaDoubleSpinBox,
                                                              QDoubleSpinBox*& maxPerturbationDoubleSpinBox )
{
  if( nbrs.size()!=7 )
  {
    if(!m_noGUI && !m_Testing) 
    {
      QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted.\nIt may have been modified manually.\nPlease save it again.");
      std::cout<<"FAILED"<<std::endl; // command line display
    }
    else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
    return true;
  }
  if(nbrs.at(0).toInt()==1) // if scale level true
  {
    if( (nbrs.at(1).toInt()==0) || (nbrs.at(2).toInt()==0) || (nbrs.at(3).toDouble()==0) || (nbrs.at(4).toDouble()==0) || (nbrs.at(5).toDouble()==0) || (nbrs.at(6).toDouble()==0))
    {
      if(!m_noGUI && !m_Testing) 
      {
        QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
        std::cout<<"FAILED"<<std::endl; // command line display
      }
      else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
      return true;
    }
    SLcheckBox->setChecked(true);
    SLspinBox->setValue(nbrs.at(1).toInt());
    nbIterSpinBox->setValue(nbrs.at(2).toInt());
    alphaDoubleSpinBox->setValue(nbrs.at(3).toDouble());
    betaDoubleSpinBox->setValue(nbrs.at(4).toDouble());
    gammaDoubleSpinBox->setValue(nbrs.at(5).toDouble());
    maxPerturbationDoubleSpinBox->setValue(nbrs.at(6).toDouble());
  }
  else SLcheckBox->setChecked(false);

  return false;
}

int GUI::LoadParameters(QString paramFile, bool DiscardParametersCSV) // DiscardParametersCSV used only in constructor: if CSV file is given, only load the given CSV file.
{
  if( ! itksys::SystemTools::GetPermissions(paramFile.toStdString().c_str(), ITKmode_F_OK) ) // Test if the csv file exists => itksys::SystemTools::GetPermissions() returns true if ITKmode_F(file)_OK
  {
    std::cout<<"| The given parameter file does not exist"<<std::endl; // command line display
    return -1;
  }

  QFile file(paramFile);

  if (file.open(QFile::ReadOnly))
  {
    QTextStream stream(&file);

    QString line = stream.readLine();
    QStringList list = line.split("=");
    if( ! list.at(0).contains("DTIAtlasBuilderParameterFileVersion") )
    {
      if(!m_noGUI && !m_Testing) QMessageBox::critical(this, "No parameter file", "This file is not a parameter file\nfor this program");
      else std::cout<<"| This file is not a parameter file for this program"<<std::endl;
      return -1;
    }
    int ParamFileVersion = list.at(1).toInt();

    std::cout<<"| Loading Parameters file \'"<< paramFile.toStdString() <<"\'..."; // command line display

/* Other Parameters */
    line = stream.readLine();
    list = line.split("=");
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"Output Folder") ) return -1;
    OutputFolderLineEdit->setText(list.at(1));

    line = stream.readLine();
    list = line.split("=");
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"Atlas Template") ) return -1;
    TemplateLineEdit->setText(list.at(1));

    if( ParamFileVersion >= 3 )
    {
      line = stream.readLine() ;
      list = line.split( "=" ) ;
      int comboBoxIndex = ScalarMeasurementComboBox->findText( list.at( 1 ) ) ;
      if( comboBoxIndex == -1 )
      {
        // The value found for ScalarMeasurementComboBox in the parameter file is not valid.
        // We "corrupt" the parameter file such that 'LoadParametersReturnTrueIfCorrupted' returns and error message.
        list[ 0 ] = "" ;
      }
      if( LoadParametersReturnTrueIfCorrupted( list.at( 0 ) , "Scalar Measurement" ) )
      {
        return -1 ;
      }
      ScalarMeasurementComboBox->setCurrentIndex( comboBoxIndex ) ;
    }

    line = stream.readLine();
    list = line.split("=");
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"SafetyMargin") ) return -1;
    if( list.at(1) == QString("true") ) SafetyMargincheckBox->setChecked(true);

    line = stream.readLine();
    list = line.split("=");
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"Loops for the registration") ) return -1;
    NbLoopsSpinBox->setValue( list.at(1).toInt() );

    line = stream.readLine();
    list = line.split("=");
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"BRAINSFit Affine Tfm Mode") ) return -1;

    if( list.at(1).contains(QString("Off")) ) BFAffineTfmModecomboBox->setCurrentIndex(0);
    else if( list.at(1).contains(QString("useMomentsAlign")) ) BFAffineTfmModecomboBox->setCurrentIndex(1);
    else if( list.at(1).contains(QString("useCenterOfHeadAlign")) ) BFAffineTfmModecomboBox->setCurrentIndex(2);
    else if( list.at(1).contains(QString("useGeometryAlign")) ) BFAffineTfmModecomboBox->setCurrentIndex(3);

    line = stream.readLine();
    list = line.split("=");
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"Overwrite") ) return -1;
    if( list.at(1) == QString("true") ) OverwritecheckBox->setChecked(true);

/* Scale Levels */
    line = stream.readLine();
    list = line.split("=");
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"Scale Level") ) return -1;

    if( LoadParametersLoadScaleLevelReturnTrueIfCorrupted(list.at(1).split(",") , SL4checkBox , SL4spinBox , nbIter4SpinBox , alpha4DoubleSpinBox , beta4DoubleSpinBox , gamma4DoubleSpinBox , maxPerturbation4DoubleSpinBox) ) return -1; // (list of parameters, variables to fill..)

    line = stream.readLine();
    list = line.split("=");
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"Scale Level") ) return -1;

    if( LoadParametersLoadScaleLevelReturnTrueIfCorrupted(list.at(1).split(",") , SL2checkBox , SL2spinBox , nbIter2SpinBox , alpha2DoubleSpinBox , beta2DoubleSpinBox , gamma2DoubleSpinBox , maxPerturbation2DoubleSpinBox) ) return -1; // (list of parameters, variables to fill..)

    line = stream.readLine();
    list = line.split("=");
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"Scale Level") ) return -1;

    if( LoadParametersLoadScaleLevelReturnTrueIfCorrupted(list.at(1).split(",") , SL1checkBox , SL1spinBox , nbIter1SpinBox , alpha1DoubleSpinBox , beta1DoubleSpinBox , gamma1DoubleSpinBox , maxPerturbation1DoubleSpinBox) ) return -1; // (list of parameters, variables to fill..)


/* Final Atlas Building parameters */
    line = stream.readLine();
    list = line.split("=");
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"Resampling Interpolation Algorithm") ) return -1;

    if( list.at(1).contains(QString("Linear")) ) InterpolTypeComboBox->setCurrentIndex(0);
    else if( list.at(1).contains(QString("Nearest Neighborhoor")) ) InterpolTypeComboBox->setCurrentIndex(1);
    else if( list.at(1).contains(QString("Windowed Sinc")) ) 
    { 
      InterpolTypeComboBox->setCurrentIndex(2);
      if( list.at(2).contains(QString("Hamming")) ) m_windowComboBox->setCurrentIndex(0);
      else if( list.at(2).contains(QString("Cosine")) ) m_windowComboBox->setCurrentIndex(1);
      else if( list.at(2).contains(QString("Welch")) ) m_windowComboBox->setCurrentIndex(2);
      else if( list.at(2).contains(QString("Lanczos")) ) m_windowComboBox->setCurrentIndex(3);
      else if( list.at(2).contains(QString("Blackman")) ) m_windowComboBox->setCurrentIndex(4);
      else
      {
        if(!m_noGUI && !m_Testing) 
        {
          QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
          std::cout<<"FAILED"<<std::endl; // command line display
        }
        else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
        return -1;
      }
    }
    else if( list.at(1).contains(QString("BSpline")) ) 
    { 
      InterpolTypeComboBox->setCurrentIndex(3);
      if( list.at(2).toInt()>=0 && list.at(2).toInt()<=5 ) m_BSplineComboBox->setCurrentIndex( list.at(2).toInt()-1 );
      else
      {
      if(!m_noGUI && !m_Testing) 
      {
        QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
        std::cout<<"FAILED"<<std::endl; // command line display
      }
      else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
        return -1;
      }
    }
    else
    {
      if(!m_noGUI && !m_Testing) 
      {
        QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
        std::cout<<"FAILED"<<std::endl; // command line display
      }
      else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
      return -1;
    }

    line = stream.readLine();
    list = line.split("=");
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"Tensor interpolation") ) return -1;

    if( list.at(1).contains(QString("Non Log Euclidean")) )
    { 
      TensInterpolComboBox->setCurrentIndex(1);
      if( list.at(2).contains(QString("Zero")) ) m_nologComboBox->setCurrentIndex(0);
      else if( list.at(2).contains(QString("None")) ) m_nologComboBox->setCurrentIndex(1);
      else if( list.at(2).contains(QString("Absolute Value")) ) m_nologComboBox->setCurrentIndex(2);
      else if( list.at(2).contains(QString("Nearest")) ) m_nologComboBox->setCurrentIndex(3);
      else
      {
        if(!m_noGUI && !m_Testing) 
        {
          QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
          std::cout<<"FAILED"<<std::endl; // command line display
        }
        else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
        return -1;
      }
    }
    else if( list.at(1).contains(QString("Log Euclidean")) ) TensInterpolComboBox->setCurrentIndex(0);
    else
    {
      if(!m_noGUI && !m_Testing) 
      {
        QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
        std::cout<<"FAILED"<<std::endl; // command line display
      }
      else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
      return -1;
    }

    line = stream.readLine();
    list = line.split("=");
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"Tensor transformation") ) return -1;

    if( list.at(1).contains(QString("Preservation of the Principal Direction (PPD)")) ) TensTfmComboBox->setCurrentIndex(0);
    else if( list.at(1).contains(QString("Finite Strain (FS)")) ) TensTfmComboBox->setCurrentIndex(1);
    else
    {
      if(!m_noGUI && !m_Testing) 
      {
        QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
        std::cout<<"FAILED"<<std::endl; // command line display
      }
      else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
      return -1;
    }

/* Final Resampling parameters */
    line = stream.readLine();
    list = line.split("=");
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"DTIReg Extra Path") ) return -1;
    DTIRegExtraPathlineEdit->setText(list.at(1));

    line = stream.readLine();
    list = line.split("=");
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"DTIRegMethod") ) return -1;

    if( list.at(1).contains(QString("BRAINS")) )
    {
      RegMethodcomboBox->setCurrentIndex(0);
      QStringList param= list.at(2).split(";");
      if( param.size()!=4 )
      {
        if(!m_noGUI && !m_Testing) 
        {
          QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
          std::cout<<"FAILED"<<std::endl; // command line display
        }
        else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
        return -1;
      }

      if( param.at(0).contains(QString("None")) ) m_BRegTypeComboBox->setCurrentIndex(0);
      else if( param.at(0).contains(QString("Rigid")) ) m_BRegTypeComboBox->setCurrentIndex(1);
      else if( param.at(0).contains(QString("BSpline")) ) m_BRegTypeComboBox->setCurrentIndex(2);
      else if( param.at(0).contains(QString("Diffeomorphic")) ) m_BRegTypeComboBox->setCurrentIndex(3);
      else if( param.at(0).contains(QString("Demons")) ) m_BRegTypeComboBox->setCurrentIndex(4);
      else if( param.at(0).contains(QString("FastSymmetricForces")) ) m_BRegTypeComboBox->setCurrentIndex(5);
      else
      {
        if(!m_noGUI && !m_Testing) 
        {
          QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
          std::cout<<"FAILED"<<std::endl; // command line display
        }
        else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
        return -1;
      }

      if( param.at(1).contains(QString("Off")) ) m_TfmModeComboBox->setCurrentIndex(0);
      else if( param.at(1).contains(QString("useMomentsAlign")) ) m_TfmModeComboBox->setCurrentIndex(1);
      else if( param.at(1).contains(QString("useCenterOfHeadAlign")) ) m_TfmModeComboBox->setCurrentIndex(2);
      else if( param.at(1).contains(QString("useGeometryAlign")) ) m_TfmModeComboBox->setCurrentIndex(3);
      else if( param.at(1).contains(QString("Use computed affine transform")) ) m_TfmModeComboBox->setCurrentIndex(4);
      else
      {
        if(!m_noGUI && !m_Testing) 
        {
          QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
          std::cout<<"FAILED"<<std::endl; // command line display
        }
        else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
        return -1;
      }

      m_NbPyrLevSpin->setValue( param.at(2).toInt() );
      m_PyrLevItLine->setText( param.at(3) );
    }
    else if( list.at(1).contains(QString("ANTS")) )
    {
      RegMethodcomboBox->setCurrentIndex(1);
      QStringList param= list.at(2).split(";");
      if( param.size()!=7 )
      {
        if(!m_noGUI && !m_Testing) 
        {
          QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
          std::cout<<"FAILED"<<std::endl; // command line display
        }
        else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
        return -1;
      }

      if( param.at(0).contains(QString("None")) ) m_ARegTypeComboBox->setCurrentIndex(0);
      else if( param.at(0).contains(QString("Rigid")) ) m_ARegTypeComboBox->setCurrentIndex(1);
      else if( param.at(0).contains(QString("Elast")) ) m_ARegTypeComboBox->setCurrentIndex(2);
      else if( param.at(0).contains(QString("Exp")) ) m_ARegTypeComboBox->setCurrentIndex(3);
      else if( param.at(0).contains(QString("GreedyExp")) ) m_ARegTypeComboBox->setCurrentIndex(4);
      else if( param.at(0).contains(QString("GreedyDiffeo (SyN)")) ) m_ARegTypeComboBox->setCurrentIndex(5);
      else if( param.at(0).contains(QString("SpatioTempDiffeo (SyN)")) ) m_ARegTypeComboBox->setCurrentIndex(6);
      else
      {
        if(!m_noGUI && !m_Testing) 
        {
          QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
          std::cout<<"FAILED"<<std::endl; // command line display
        }
        else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
        return -1;
      }

      m_TfmStepLine->setText( param.at(1) );
      m_IterLine->setText( param.at(2) );

      if( param.at(3).contains(QString("Cross-Correlation (CC)")) ) m_SimMetComboBox->setCurrentIndex(0);
      else if( param.at(3).contains(QString("Mutual Information (MI)")) ) m_SimMetComboBox->setCurrentIndex(1);
      else if( param.at(3).contains(QString("Mean Square Difference (MSQ)")) ) m_SimMetComboBox->setCurrentIndex(2);
      else
      {
        if(!m_noGUI && !m_Testing) 
        {
          QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
          std::cout<<"FAILED"<<std::endl; // command line display
        }
        else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
        return -1;
      }

      m_SimParamDble->setValue( param.at(4).toDouble() );
      m_GSigmaDble->setValue( param.at(5).toDouble() );
      if ( param.at(6).toInt()==1 ) m_SmoothOffCheck->setChecked(true);
    }
    else
    {
      if(!m_noGUI && !m_Testing) 
      {
        QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
        std::cout<<"FAILED"<<std::endl; // command line display
      }
      else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
      return -1;
    }

/* Grid Processing */
    line = stream.readLine();
    list = line.split("=");
    GridProcesscheckBox->setChecked(false);//both for GridProcessing and GridAtlasProcessing
    if( LoadParametersReturnTrueIfCorrupted(list.at(0),"GridProcessing") ) return -1;

    if( !list.at(1).isEmpty() )
    {
      list.removeFirst() ;
      QString concatenatedList = list.join("=") ;//This line likely contains "=" signs. The grid command line would then be split. We need to concatenate it back to its normal form!
      GridProcesscheckBox->setChecked(true);
      GridProcessCmdLineEdit->setText( concatenatedList );
    }
    else
    {
      GridProcessCmdLineEdit->setText( "" );
    }
/* Atlas command line for grid processing */
    if( ParamFileVersion >= 3 )
    {
      line = stream.readLine();
      list = line.split("=");
      if( LoadParametersReturnTrueIfCorrupted(list.at(0),"GridAtlasProcessing") )
      {
        return -1;
      }
      if( !list.at(1).isEmpty() )
      {
        list.removeFirst() ;
        QString concatenatedList = list.join("=") ;//This line likely contains "=" signs. The grid command line would then be split. We need to concatenate it back to its normal form!
        GridProcessAtlasCmdLineEdit->setText( concatenatedList );
      }
      else
      {
        GridProcessAtlasCmdLineEdit->setText( "" );
      }
    }
    else
    {
      GridProcessAtlasCmdLineEdit->setText( GridProcessCmdLineEdit->text() );
    }
/* Nb Threads*/
    if( ParamFileVersion >= 2 )
    {
      line = stream.readLine();
      list = line.split("=");
      if( LoadParametersReturnTrueIfCorrupted(list.at(0),"NbThreads") ) return -1;
      NbThreadsSpinBox->setValue( list.at(1).toInt() );
    }

    if (ParamFileVersion >=4){
      line = stream.readLine();
      list = line.split("=");
      if( LoadParametersReturnTrueIfCorrupted(list.at(0),"nbLoopsDTIReg") ) return -1;
      NbLoopsDTIRegSpinBox->setValue( list.at(1).toInt() ); 
    }


    std::cout<<"DONE"<<std::endl; // command line display

/* Opening CSV File */
    // if( ! DiscardParametersCSV )
    // {
    //   line = stream.readLine();
    //   list = line.split("=");
    //   if(!list.at(0).contains(QString("CSV Dataset File")))
    //   {
    //     if(!m_noGUI && !m_Testing) 
    //     {
    //       QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted\nIt may have been modified manually.\nPlease save it again.");
    //       std::cout<<"FAILED"<<std::endl; // command line display
    //     }
    //     else std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
    //     return -1;
    //   }
    //   QString CSVpath = list.at(1);
    //   // CaseListWidget->clear();
    //   ReadCSV(CSVpath);
    // } // if( ! DiscardParametersCSV )

    m_ParamSaved=1;
  } 
  else if ( !paramFile.isEmpty() ) qDebug( "Could not open file");

  return 0;
}

  /////////////////////////////////////////
 //              XML FILE               //
/////////////////////////////////////////

void GUI::GenerateXMLForGA()
{
  QString  xmlFileName = m_OutputPath + QString("/common/GreedyAtlasParameters.xml");

  if( ! itksys::SystemTools::GetPermissions((m_OutputPath.toStdString() + "/common").c_str(), ITKmode_F_OK) ) // Test if the main folder does not exists => itksys::SystemTools::GetPermissions() returns true if ITKmode_F(file)_OK
  {
    if( itksys::SystemTools::GetPermissions((m_OutputPath.toStdString() + "/common").c_str(), ITKmode_F_OK) ) // old version of the folder => put the xml in the old folder : will be renamed in the python script to the new name
    {
      xmlFileName = m_OutputPath + QString("common/GreedyAtlasParameters.xml");
    }
    else
    {
      std::cout<<"| Creating Non Linear Registration directory..."<<std::endl; // command line display
      std::string Dir = m_OutputPath.toStdString() + "/common";
      itksys::SystemTools::MakeDirectory( Dir.c_str() );
    }
  }

  QFile file(xmlFileName);
  if ( file.open( QFile::WriteOnly ) )
  {
    std::cout<<"| Saving XML file for GreedyAtlas..."; // command line display
    QTextStream stream( &file );

    stream <<"<!--top-level node-->"<< endl;
    stream <<"<ParameterFile>"<< endl;
    stream <<"  <WeightedImageSet>"<< endl;
    stream <<"    <ScaleImageWeights val=\"true\"/>"<< endl;
    stream <<"    <InputImageFormatString>"<< endl;
    stream <<"      <FormatString val=\"\" />"<< endl;
    stream <<"      <Base val=\"0\" />"<< endl;
    stream <<"      <NumFiles val=\"" << IntOrDoubleToStr<int>(m_CasesPath.size()).c_str() << "\" />"<< endl;
    stream <<"      <Weight val=\"1\" />"<< endl;
    stream <<"    </InputImageFormatString>"<< endl;
    // for (unsigned int i=0;i<m_CasesPath.size();i++)
    // {
    //   stream <<"    <WeightedImage>"<< endl;
    //   stream <<"      <Filename val=\"" << m_OutputPath << "/final_atlas/1_Affine_Registration/Loop" << IntOrDoubleToStr<int>(NbLoopsSpinBox->value()).c_str() << "/" << itksys::SystemTools::GetFilenameWithoutExtension( m_CasesPath[i] ).c_str() << "_Loop" << IntOrDoubleToStr<int>(NbLoopsSpinBox->value()).c_str() << "_Final" << ScalarMeasurementComboBox->currentText() << ".nrrd\" />"<< endl;
    //   stream <<"      <ItkTransform val=\"1\" />"<< endl;
    //   stream <<"    </WeightedImage>"<< endl;
    // }
    stream <<"  </WeightedImageSet>"<< endl;

/* Scale Levels */
    if(SL4checkBox->isChecked())
    {
      stream <<"  <GreedyScaleLevel>"<< endl;
      stream <<"    <ScaleLevel>"<< endl;
      stream <<"      <!--factor by which to downsample images-->"<< endl;
      stream <<"      <DownsampleFactor val=\"" << IntOrDoubleToStr<int>(SL4spinBox->value()).c_str() << "\" />"<< endl;
      stream <<"    </ScaleLevel>"<< endl;
      stream <<"    <!--Scale factor on the maximum velocity in a given deformation for computing delta-->"<< endl;
      stream <<"    <NIterations val=\"" << IntOrDoubleToStr<int>(nbIter4SpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"    <Iterator>"<< endl;
      stream <<"      <MaxPert val=\"" << IntOrDoubleToStr<double>(maxPerturbation4DoubleSpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"      <DiffOper>"<< endl;
      stream <<"        <Alpha val=\"" << IntOrDoubleToStr<double>(alpha4DoubleSpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"        <Beta val=\"" << IntOrDoubleToStr<double>(beta4DoubleSpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"        <Gamma val=\"" << IntOrDoubleToStr<double>(gamma4DoubleSpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"      </DiffOper>"<< endl;
      stream <<"    </Iterator>"<< endl;
      stream <<"  </GreedyScaleLevel>"<< endl;
    }

    if(SL2checkBox->isChecked())
    {
      stream <<"  <GreedyScaleLevel>"<< endl;
      stream <<"    <ScaleLevel>"<< endl;
      stream <<"      <!--factor by which to downsample images-->"<< endl;
      stream <<"      <DownsampleFactor val=\"" << IntOrDoubleToStr<int>(SL2spinBox->value()).c_str() << "\" />"<< endl;
      stream <<"    </ScaleLevel>"<< endl;
      stream <<"    <!--Scale factor on the maximum velocity in a given deformation for computing delta-->"<< endl;
      stream <<"    <NIterations val=\"" << IntOrDoubleToStr<int>(nbIter2SpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"    <Iterator>"<< endl;
      stream <<"      <MaxPert val=\"" << IntOrDoubleToStr<double>(maxPerturbation2DoubleSpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"      <DiffOper>"<< endl;
      stream <<"        <Alpha val=\"" << IntOrDoubleToStr<double>(alpha2DoubleSpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"        <Beta val=\"" << IntOrDoubleToStr<double>(beta2DoubleSpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"        <Gamma val=\"" << IntOrDoubleToStr<double>(gamma2DoubleSpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"      </DiffOper>"<< endl;
      stream <<"    </Iterator>"<< endl;
      stream <<"  </GreedyScaleLevel>"<< endl;
    }

    if(SL1checkBox->isChecked())
    {
      stream <<"  <GreedyScaleLevel>"<< endl;
      stream <<"    <ScaleLevel>"<< endl;
      stream <<"      <!--factor by which to downsample images-->"<< endl;
      stream <<"      <DownsampleFactor val=\"" << IntOrDoubleToStr<int>(SL1spinBox->value()).c_str() << "\" />"<< endl;
      stream <<"    </ScaleLevel>"<< endl;
      stream <<"    <!--Scale factor on the maximum velocity in a given deformation for computing delta-->"<< endl;
      stream <<"    <NIterations val=\"" << IntOrDoubleToStr<int>(nbIter1SpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"    <Iterator>"<< endl;
      stream <<"      <MaxPert val=\"" << IntOrDoubleToStr<double>(maxPerturbation1DoubleSpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"      <DiffOper>"<< endl;
      stream <<"        <Alpha val=\"" << IntOrDoubleToStr<double>(alpha1DoubleSpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"        <Beta val=\"" << IntOrDoubleToStr<double>(beta1DoubleSpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"        <Gamma val=\"" << IntOrDoubleToStr<double>(gamma1DoubleSpinBox->value()).c_str() << "\" />"<< endl;
      stream <<"      </DiffOper>"<< endl;
      stream <<"    </Iterator>"<< endl;
      stream <<"  </GreedyScaleLevel>"<< endl;
    }

    stream <<"  <!--number of threads to use, 0=one per processor (only for CPU computation)-->"<< endl;
    if( GridProcesscheckBox->isChecked() )
    {
      stream <<"  <nThreads val=\"1\" />"<< endl; // to prevent GreedyAtlas from using too many cores on the cluster
    }
    else
    {
      stream <<"  <nThreads val=\""<< IntOrDoubleToStr<int>(NbThreadsSpinBox->value()).c_str() <<"\" />"<< endl;
    }

    stream <<"  <OutputPrefix val=\"" << m_OutputPath << "/final_atlas/2_NonLinear_Registration/\" />"<< endl;
    stream <<"  <OutputSuffix val=\"mhd\" />"<< endl;
    stream <<"</ParameterFile>"<< endl;

    std::cout<<"DONE"<<std::endl; // command line display
  } // if ( file.open() )
  else qDebug( "Could not create xml file");

}

  /////////////////////////////////////////
 //         SOFT CONFIGURATION          //
/////////////////////////////////////////

void GUI::LoadConfigSlot() /*SLOT*/
{
  QString ConfigBrowse=QFileDialog::getOpenFileName(this, "Open Configuration File", QString(), ".txt Files (*.txt)");

  if(!ConfigBrowse.isEmpty())
  {
    LoadConfig(ConfigBrowse);
  }
}


bool GUI::LoadConfigReturnTrueIfCorrupted(QString ReadProgram, const char* Program )
{
  if( ! ReadProgram.contains( QString(Program) ) )
  {
    if(!m_noGUI && !m_Testing) 
    {
      QMessageBox::critical(this, "Corrupt File", "This config file is corrupted.\nIt may have been modified manually.\nPlease save it again.");
      std::cout<<"FAILED"<<std::endl; // command line display
    }
    else std::cout<<"FAILED"<<std::endl<<"| This config file is corrupted. It may have been modified manually. Please save it again."<<std::endl;
    return true;
  }

  return false;
}

int GUI::LoadConfig(QString configFile) // returns -1 if fails, otherwise 0
{
  if( itksys::SystemTools::GetPermissions(configFile.toStdString().c_str(), ITKmode_F_OK) ) // Test if the config file exists => itksys::SystemTools::GetPermissions() returns true if ITKmode_F(file)_OK
  {
    std::cout<<"| Loading Configuration file \'"<< configFile.toStdString() <<"\'..."; // command line display

    std::string notFound;

    ///get the values from file
    QFile file(configFile);
    if (file.open(QFile::ReadOnly))
    {
      QTextStream stream(&file);

      QString line = stream.readLine();
      QStringList list = line.split("=");
      if( LoadConfigReturnTrueIfCorrupted(list.at(0),"ImageMath") ) return -1;
      if(!list.at(1).isEmpty()) ImagemathPath->setText(list.at(1));
      else if(ImagemathPath->text().isEmpty()) notFound = notFound + "> ImageMath\n";  

      line = stream.readLine();
      list = line.split("=");
      if( LoadConfigReturnTrueIfCorrupted(list.at(0),"ResampleDTIlogEuclidean") ) return -1;
      if(!list.at(1).isEmpty()) ResampPath->setText(list.at(1));
      else if(ResampPath->text().isEmpty()) notFound = notFound + "> ResampleDTIlogEuclidean\n";

      line = stream.readLine();
      list = line.split("=");
      if( LoadConfigReturnTrueIfCorrupted(list.at(0),"CropDTI") ) return -1;
      if(!list.at(1).isEmpty()) CropDTIPath->setText(list.at(1));
      else if(CropDTIPath->text().isEmpty()) notFound = notFound + "> CropDTI\n";

      line = stream.readLine();
      list = line.split("=");
      if( LoadConfigReturnTrueIfCorrupted(list.at(0),"dtiprocess") ) return -1;
      if(!list.at(1).isEmpty()) dtiprocPath->setText(list.at(1));
      else if(dtiprocPath->text().isEmpty()) notFound = notFound + "> dtiprocess\n";

      line = stream.readLine();
      list = line.split("=");
      if( LoadConfigReturnTrueIfCorrupted(list.at(0),"BRAINSFit") ) return -1;
      if(!list.at(1).isEmpty()) BRAINSFitPath->setText(list.at(1));
      else if(BRAINSFitPath->text().isEmpty()) notFound = notFound + "> BRAINSFit\n";


      line = stream.readLine();
      list = line.split("=");
      if( LoadConfigReturnTrueIfCorrupted(list.at(0),"GreedyAtlas") ) return -1;
      bool GAToTest=false;
      if( !list.at(1).isEmpty() )
      {
        GAToTest=true; // call testGA after the display of "DONE"
        GAPath->setText(list.at(1));
      }
      else if(GAPath->text().isEmpty()) notFound = notFound + "> GreedyAtlas\n";

      line = stream.readLine();
      list = line.split("=");
      if( LoadConfigReturnTrueIfCorrupted(list.at(0),"dtiaverage") ) return -1;
      if(!list.at(1).isEmpty()) dtiavgPath->setText(list.at(1));
      else if(dtiavgPath->text().isEmpty()) notFound = notFound + "> dtiaverage\n";

      line = stream.readLine();
      list = line.split("=");
      if( LoadConfigReturnTrueIfCorrupted(list.at(0),"DTI-Reg") ) return -1;
      bool DTIRegToTest=false;
      if( !list.at(1).isEmpty() )
      {
        DTIRegToTest=true; // call testDTIReg after the display of "DONE"
        DTIRegPath->setText(list.at(1));
      }
      else if(DTIRegPath->text().isEmpty()) notFound = notFound + "> DTI-Reg\n";

      line = stream.readLine();
      list = line.split("=");
      if( LoadConfigReturnTrueIfCorrupted(list.at(0),"unu") ) return -1;
      if(!list.at(1).isEmpty()) unuPath->setText(list.at(1));
      else if(unuPath->text().isEmpty()) notFound = notFound + "> unu\n";

      line = stream.readLine();
      list = line.split("=");
      if( LoadConfigReturnTrueIfCorrupted(list.at(0),"MriWatcher") ) return -1;
      if(!list.at(1).isEmpty()) MriWatcherPath->setText(list.at(1));
      else if(MriWatcherPath->text().isEmpty()) notFound = notFound + "> MriWatcher (Program will work, but QC will not be available)\n";

      line = stream.readLine();
      list = line.split("=");
      if( LoadConfigReturnTrueIfCorrupted(list.at(0),"ITKTransformTools") ) return -1;
      if(!list.at(1).isEmpty()) ITKTransformToolsPath->setText(list.at(1));
      else if(ITKTransformToolsPath->text().isEmpty()) notFound = notFound + "> ITKTransformTools\n";

      std::cout<<"DONE"<<std::endl; // command line display

      if(m_FromConstructor!=1) // do not test when from constructor -> test at the end of it
      {
        if( !notFound.empty() )
        {
          if(!m_noGUI && !m_Testing) 
          {
            std::string text = "The following programs are missing.\nPlease enter the path manually:\n" + notFound;
            QMessageBox::warning(this, "Program missing", QString(text.c_str()) );
          }
          else
          {
            std::cout<<"| The following programs have not been found. Please give a configuration file or modify it or enter the path manually in the GUI:\n"<< notFound <<std::endl;
            return -1;
          }
        }

        if(GAToTest) testGA();  // do not test GA path if 'LoadConfig' called from constructor -> test at the end of constructor
        if(DTIRegToTest) testDTIReg();  // do not test DTIReg path if 'LoadConfig' called from constructor -> test at the end of constructor
      }

    } 
    else qDebug( "Could not open file");
  }
  else std::cout<<"| The given configuration file does not exist"<<std::endl; // command line display

  return 0;
}

void GUI::SaveConfig() /*SLOT*/
{
  QString ConfigBrowseName=QFileDialog::getSaveFileName(this, tr("Save Configuration File"),"./DTIAtlasBuilderSoftConfiguration.txt",tr("Text File (*.txt)"));

  if(!ConfigBrowseName.isEmpty())
  {
/* getting the values and generating the config file */

    QFile file(ConfigBrowseName);
    if ( file.open( QFile::WriteOnly ) )
    {
      std::cout<<"| Generating config file..."; // command line display

      QTextStream stream( &file );

      stream << "ImageMath=" << ImagemathPath->text() << endl;
      stream << "ResampleDTIlogEuclidean=" << ResampPath->text() << endl;
      stream << "CropDTI=" << CropDTIPath->text() << endl;
      stream << "dtiprocess=" << dtiprocPath->text() << endl;
      stream << "BRAINSFit=" << BRAINSFitPath->text() << endl;
      stream << "GreedyAtlas=" << GAPath->text() << endl;
      stream << "dtiaverage=" << dtiavgPath->text() << endl;
      stream << "DTI-Reg=" << DTIRegPath->text() << endl;
      stream << "unu=" << unuPath->text() << endl;
      stream << "MriWatcher=" << MriWatcherPath->text() << endl;
      stream << "ITKTransformTools=" << ITKTransformToolsPath->text() << endl;

      std::cout<<"DONE"<<std::endl; // command line display
    }
    else 
    {
      std::cout<<"FAILED"<<std::endl; // command line display
      qDebug( "Could not create config file");
    }
  }
}

void GUI::ConfigDefault(std::string commandRan) /*SLOT*/
{
  std::cout<<"| Searching the softwares..."; // command line display

  std::string program;
  std::string notFound;

  // get the directory where the running executable is
  //std::string DTIAtlasBuilderPath = itksys::SystemTools::GetRealPath(itksys::SystemTools::GetParentDirectory(commandRan));
  std::string DTIAtlasBuilderPath=m_ExecutableDir;
  std::vector< std::string > tools_paths_hints;

  if(commandRan.compare("") != 0){
    tools_paths_hints.push_back("DTI-Reg");
    tools_paths_hints.push_back("DTIProcess");
    tools_paths_hints.push_back("ResampleDTIlogEuclidean");

    if(DTIAtlasBuilderPath.find("DTIAtlasBuilder") != std::string::npos){
      std::cout<<DTIAtlasBuilderPath<<std::endl;
      std::cout<<DTIAtlasBuilderPath.size()<<std::endl;

      std::size_t firstch = DTIAtlasBuilderPath.rfind("DTIAtlasBuilder");

      for(int i = 0; i < tools_paths_hints.size(); i++){
        std::string fullpath = DTIAtlasBuilderPath;
        tools_paths_hints[i] = fullpath.replace(firstch, std::string("DTIAtlasBuilder").size(), tools_paths_hints[i]);
        std::cout<<tools_paths_hints[i]<<std::endl;
      }
      std::string dtireghint = tools_paths_hints[0];
      if(dtireghint.rfind("cli-modules") != std::string::npos){
        tools_paths_hints.push_back(dtireghint.replace(dtireghint.rfind("cli-modules"), std::string("cli-modules").size(), "ExternalBin"));
        std::cout<<tools_paths_hints[tools_paths_hints.size() - 1]<<std::endl;
      }
      
    }

    m_FindProgramDTIABExecDirVec.insert(m_FindProgramDTIABExecDirVec.end(), tools_paths_hints.begin(), tools_paths_hints.end());
  }

  program = FindProgram("ImageMath",m_FindProgramDTIABExecDirVec);
  if(program.empty()) { if(ImagemathPath->text().isEmpty()) notFound = notFound + "> ImageMath\n"; }
  else ImagemathPath->setText(QString(program.c_str()));

  program = FindProgram("ResampleDTIlogEuclidean",m_FindProgramDTIABExecDirVec);
  if(program.empty()) { if(ResampPath->text().isEmpty()) notFound = notFound + "> ResampleDTIlogEuclidean\n"; }
  else ResampPath->setText(QString(program.c_str()));

  program = FindProgram("CropDTI",m_FindProgramDTIABExecDirVec);
  if(program.empty()) { if(CropDTIPath->text().isEmpty()) notFound = notFound + "> CropDTI\n"; }
  else CropDTIPath->setText(QString(program.c_str()));

  program = FindProgram("dtiprocess",m_FindProgramDTIABExecDirVec);
  if(program.empty()) { if(dtiprocPath->text().isEmpty()) notFound = notFound + "> dtiprocess\n"; }
  else dtiprocPath->setText(QString(program.c_str()));

  program = FindProgram("BRAINSFit",m_FindProgramDTIABExecDirVec);
  if(program.empty()) { if(BRAINSFitPath->text().isEmpty()) notFound = notFound + "> BRAINSFit\n"; }
  else BRAINSFitPath->setText(QString(program.c_str()));

  bool GAToTest=false;
  program = FindProgram("GreedyAtlas",m_FindProgramDTIABExecDirVec);
  if(program.empty()) { if(GAPath->text().isEmpty()) notFound = notFound + "> GreedyAtlas\n"; }
  else
  {
    GAToTest=true; // call testGA after the display of "DONE"
    GAPath->setText(QString(program.c_str()));  
  }

  program = FindProgram("dtiaverage",m_FindProgramDTIABExecDirVec);
  if(program.empty())
  {
    if( dtiavgPath->text().isEmpty() )
    {
      notFound = notFound + "> dtiaverage\n" ;
    }
  }
  else
  {
    dtiavgPath->setText( QString( program.c_str() ) ) ;
  }
  bool DTIRegToTest=false;
  program = FindProgram( "DTI-Reg" , m_FindProgramDTIABExecDirVec ) ;
  if( program.empty() )
  {
     if( DTIRegPath->text().isEmpty() )
     {
       notFound = notFound + "> DTI-Reg\n" ;
     }
  }
  else
  {
    DTIRegToTest=true; // call testDTIReg after the display of "DONE"
    DTIRegPath->setText( QString(program.c_str() ) ) ;  
  }
  program = FindProgram("unu",m_FindProgramDTIABExecDirVec);
  if(program.empty()) { if(unuPath->text().isEmpty()) notFound = notFound + "> unu\n"; }
  else unuPath->setText(QString(program.c_str()));

  program = FindProgram("ITKTransformTools",m_FindProgramDTIABExecDirVec);
  if(program.empty()) { if(ITKTransformToolsPath->text().isEmpty()) notFound = notFound + "> ITKTransformTools\n"; }
  else ITKTransformToolsPath->setText(QString(program.c_str()));


  program = FindProgram("MriWatcher",m_FindProgramDTIABExecDirVec);
  if(program.empty()) { if(MriWatcherPath->text().isEmpty()) notFound = notFound + "> MriWatcher (Program will work, but QC will not be available)\n"; }
  else MriWatcherPath->setText(QString(program.c_str()));


  std::cout<<"DONE"<<std::endl; // command line display

  if(m_FromConstructor!=1) // do not test when from constructor -> test at the end of it
  {
    if( !notFound.empty() )
    {
      if(!m_noGUI && !m_Testing) 
      {
        std::string text = "The following programs have not been found.\nPlease enter the path manually or open a configuration file:\n" + notFound;
        QMessageBox::warning(this, "Program missing", QString(text.c_str()) );
      }
      else std::cout<<"| The following programs have not been found. Please give a configuration file or modify it or enter the path manually in the GUI:\n"<< notFound <<std::endl;
    }

    if(GAToTest) testGA();  // do not test GA path if 'Default' called from constructor -> test at the end of constructor
    if(DTIRegToTest) testDTIReg();  // do not test DTIReg path if 'Default' called from constructor -> test at the end of constructor
  }
}

void GUI::BrowseSoft(int soft)  /*SLOT*/ //softwares: 1=ImageMath, 2=ResampleDTIlogEuclidean, 3=CropDTI, 4=dtiprocess, 5=BRAINSFit, 6=GreedyAtlas, 7=dtiaverage, 8=DTI-Reg, 9=unu, 10=MriWatcher , 11=ITKTransformTools
{
  QString SoftBrowse = QFileDialog::getOpenFileName(this, "Open Software", QString(), "Executable Files (*)");

  if(!SoftBrowse.isEmpty())
  {
    switch (soft)
    {
    case 1: ImagemathPath->setText(SoftBrowse);
      break;
    case 2: ResampPath->setText(SoftBrowse);
      break;
    case 3: CropDTIPath->setText(SoftBrowse);
      break;
    case 4: dtiprocPath->setText(SoftBrowse);
      break;
    case 5: BRAINSFitPath->setText(SoftBrowse);
      break;
    case 6: {
      GAPath->setText(SoftBrowse);
      testGA();
      }
      break;
    case 7: dtiavgPath->setText(SoftBrowse);
      break;
    case 8: {
      DTIRegPath->setText(SoftBrowse);
      testDTIReg();
      }
      break;
    case 9: unuPath->setText(SoftBrowse);
      break;
    case 10: MriWatcherPath->setText(SoftBrowse);
      break;
    case 11: ITKTransformToolsPath->setText(SoftBrowse);
      break;
    }
  }
}

//Converts software number from string to vector<int>
//Splits string at '.'
//eg: "1.1.3" to vector<int>={1,1,3}
std::vector<int> GUI::ConvertStringVersionToVector( std::string version )
{
  std::vector<int> vecVersion ;
  std::size_t posOrig = 0 ;
  std::size_t posDot ;
  do
  {
    posDot = version.find( "." , posOrig ) ;
    if( posDot == std::string::npos )
    {
      posDot = version.size() ;
    }
    std::istringstream extractString( version.substr( posOrig , posDot - posOrig ) ) ;
    int convert ;
    extractString >> convert ;
    if( !extractString )
    {
      convert = -1 ;
    }
    vecVersion.push_back( convert ) ;
    posOrig = posDot + 1 ;
  } while( posDot != version.size() ) ;
  return vecVersion ;
}

//Compares 2 softare version number and returns 'true' if
//givenVersion is larger than comparedVersion
//eg: givenVersion = 1.3
//    comparedVersion = 1.0.1
//    returns true
//returns true if versions are equal
//eg: givenVersion = 2.0
//    comparedVersion = 2.0
//    returns true
//returns false if comparedVersion is larger than givenVersion
//eg: givenVersion = 3.2
//    comparedVersion = 4.5.2
//    returns false
bool GUI::testVersion( std::string givenVersion , std::string comparedVersion )
{
  std::vector<int> givenVersionVec = ConvertStringVersionToVector( givenVersion ) ;
  std::vector<int> comparedVersionVec = ConvertStringVersionToVector( comparedVersion ) ;
  std::size_t max = std::max( givenVersionVec.size() , comparedVersionVec.size() ) ;
  givenVersionVec.resize( max , 0 ) ;
  comparedVersionVec.resize( max , 0 ) ;
  for( std::size_t i = 0 ; i < max ; i++ )
  {
    if( givenVersionVec[ i ] > comparedVersionVec[ i ] )
    {
      return true ;
    }
    else if( givenVersionVec[ i ] < comparedVersionVec[ i ] )
    {
      return false ;
    }
  }
  return true ; //If we are here, the 2 versions are equal
}

void GUI::ResetSoft(int softindex) /*SLOT*/ //softwares: 1=ImageMath, 2=ResampleDTIlogEuclidean, 3=CropDTI, 4=dtiprocess, 5=BRAINSFit, 6=GreedyAtlas, 7=dtiaverage, 8=DTI-Reg, 9=unu, 10=MriWatcher
{
  std::string soft;

  switch (softindex)
  {
  case 1: soft="ImageMath";
    break;
  case 2: soft="ResampleDTIlogEuclidean";
    break;
  case 3: soft="CropDTI";
    break;
  case 4: soft="dtiprocess";
    break;
  case 5:  soft="BRAINSFit";
    break;
  case 6: soft="GreedyAtlas";
    break;
  case 7: soft="dtiaverage";
    break;
  case 8: soft="DTI-Reg";
    break;
  case 9: soft="unu";
    break;
  case 10: soft="MriWatcher";
    break;
  case 11: soft="ITKTransformTools";
    break;
  }

  std::cout<<"| Searching the software \'"<< soft <<"\'..."; // command line display

  std::string program = FindProgram(soft.c_str(),m_FindProgramDTIABExecDirVec);

  bool GAToTest=false;
  bool DTIRegToTest=false;
  if(program.empty()) 
  {
    std::string text = "The program \'" + soft + "\' is missing.\nPlease enter the path manually.\n";
    QMessageBox::warning(this, "Program missing", QString(text.c_str()) );
  }
  else 
  {
    if(softindex==1) ImagemathPath->setText(QString(program.c_str()));
    else if(softindex==2) ResampPath->setText(QString(program.c_str()));
    else if(softindex==3) CropDTIPath->setText(QString(program.c_str()));
    else if(softindex==4) dtiprocPath->setText(QString(program.c_str()));
    else if(softindex==5) BRAINSFitPath->setText(QString(program.c_str()));
    else if(softindex==6)
    {
      GAPath->setText(QString(program.c_str()));
      GAToTest=true; // call testGA after the display of "DONE"
    }
    else if(softindex==7) dtiavgPath->setText(QString(program.c_str()));
    else if(softindex==8)
    {
      DTIRegPath->setText(QString(program.c_str()));
      DTIRegToTest=true; // call testDTIReg after the display of "DONE"
    }
    else if(softindex==9) unuPath->setText(QString(program.c_str()));
    else if(softindex==10) MriWatcherPath->setText(QString(program.c_str()));
    else if(softindex==11) ITKTransformToolsPath->setText(QString(program.c_str()));
  }

  std::cout<<"DONE"<<std::endl; // command line display

  if(m_FromConstructor!=1) // do not test paths if 'Default' called from constructor -> test at the end of constructor
  {
    if(GAToTest) testGA();
    if(DTIRegToTest) testDTIReg();
  }
}

int GUI::testGA() /*SLOT*/ // returns 0 if version ok, -1 if bad version
{
  QProcess * Process = new QProcess;
  std::string program;
  program = GAPath->text().toStdString() + " --version";

  std::cout<<"| Checking the version of GreedyAtlas...";

  Process->setProcessChannelMode(QProcess::MergedChannels);
  Process->start( program.c_str() ); // try to find the version => returns nothing if not the right version
  Process->waitForFinished();
  Process->waitForReadyRead();
  QByteArray BA =  Process->readAllStandardOutput(); // int BA.size() // std::string s = BA.data(); std::cout<<s<<std::endl;

  std::cout<<"DONE"<<std::endl;

  if(BA.size()==0) // if nothing displayed in std output, '--version' does not exists so it is not the right version
  {
    if(!m_noGUI && !m_Testing) 
    {
      std::string text = "The version of GreedyAtlas \'" + GAPath->text().toStdString() + "\' is not the right one.\nPlease give a version supporting a XML file (--paramFile).\n";
      QMessageBox::warning(this, "Wrong version", QString(text.c_str()) );
    }
    else std::cout<<"| The version of GreedyAtlas \'" << GAPath->text().toStdString() << "\' is not the right one. Please give a version supporting a XML file (--paramFile)."<<std::endl;

    return -1;
  }

  return 0;
}

int GUI::testDTIReg() /*SLOT*/ // returns 0 if version ok, -1 if bad version
{
  QProcess * Process = new QProcess;
  std::string program;
  program = DTIRegPath->text().toStdString() + " --version";

  std::cout<<"| Checking the version of DTI-Reg...";

  Process->setProcessChannelMode(QProcess::MergedChannels);
  Process->start( program.c_str() ); // try to find the version => returns nothing if not the right version
  Process->waitForFinished();
  Process->waitForReadyRead();
  QByteArray BA =  Process->readAllStandardOutput();
  QString text = QString( BA.data() );
  if( BA.size() == 0 )//If no version number, we call it version 0
  {
      text = "0" ;
  }
  std::cout << "DONE" << std::endl ;
  std::string currentVersion = text.toStdString() ;
  std::size_t pos = currentVersion.find( "version:" ) + 9 ;//+9 is to remove "version: ", it should keep just the version number
  bool errorReadingVersion = false ;
  std::string minDTIRegVersion = "1.1.9" ;//Set which minimum version of DTI-Reg to use
  //Making sure we are not trying to go to far in the string containing the version number
  if( currentVersion.size() < pos )
  {
      errorReadingVersion = true ;
  }
  else
  {
    currentVersion = currentVersion.substr( pos , currentVersion.size() - pos ) ;
    currentVersion.erase( std::remove( currentVersion.begin() , currentVersion.end() , '\n' ) , currentVersion.end() ) ;//remove newlines
  }
  if( errorReadingVersion || !testVersion( currentVersion , minDTIRegVersion )  ) //old version -> Not OK ->print error message
  {
    if(!m_noGUI && !m_Testing)
    {
      std::string text = "The version of DTI-Reg \'" + DTIRegPath->text().toStdString() + "\' is not the right one.\nPlease give the version " + minDTIRegVersion +" or newer";
      QMessageBox::warning(this, "Wrong version", QString(text.c_str()) );
    }
    else
    {
      std::cout<<"| The version of DTI-Reg \'" << DTIRegPath->text().toStdString() << "\' is not the right one. Please give the version " + minDTIRegVersion +" or newer"<<std::endl;
    }
    return -1;
  }
  return 0;
}

  /////////////////////////////////////////
 //               READ ME               //
/////////////////////////////////////////

void GUI::ReadMe()  /*SLOT*/ /////to UPDATE
{
  QDialog *dlg = new QDialog(this);
  dlg->setWindowTitle ("Read Me");

  std::string info = "<p><b>DTIAtlasBuilder: "+std::string(DTIAtlasBuilder_VERSION)+"</b><br>";
  info = info + "<i>A tool to create an atlas from several DTI images</i><br><br>";
  info = info + "Homepage: <a href=\"http://www.nitrc.org/projects/dtiatlasbuilder\" target=\"_blank\">On NITRC</a><br><br>";
  info = info + "For any question, suggestion or remark, please contact <a href=\"mailto:sangkyoon_park@med.unc.edu\" target=\"_blank\">sangkyoon_park@med.unc.edu</a>.<br><br>";
  info = info + "Read the <a href=\"http://github.com/NIRALUser/DTIAtlasBuilder/blob/master/README.md\" target=\"_blank\">full Read Me</a>.</p>";

  QLabel *InfoLabel = new QLabel ( info.c_str(), this );
  QVBoxLayout *VLayout = new QVBoxLayout();
  VLayout->addWidget(InfoLabel);

  dlg->setLayout(VLayout);

  dlg->setVisible(!dlg->isVisible()); // display the window
}

void GUI::KeyShortcuts()  /*SLOT*/
{
  QDialog *dlg = new QDialog(this);
  dlg->setWindowTitle ("Keyboard Shortcuts");

  std::string info = "<p>Keyboard Shortcuts:<br><br>";
  info = info + "<b>RETURN</b> or <b>ENTER</b> : Compute<br>";
  info = info + "<b>PLUS</b> or <b>Drag & Drop</b> : Add cases<br>";
  info = info + "<b>MINUS</b> or <b>DELETE</b> : Remove cases<br><br>";
  info = info + "It is also possible to <b>drag and drop</b> dataset <b>CSV</b> files,<br>";
  info = info + "<b>parameters</b> files and <b>software configuration</b> files.</p>";

  QLabel *InfoLabel = new QLabel ( info.c_str(), this );
  QVBoxLayout *VLayout = new QVBoxLayout();
  VLayout->addWidget(InfoLabel);

  dlg->setLayout(VLayout);

  dlg->setVisible(!dlg->isVisible()); // display the window
}

  /////////////////////////////////////////
 //         FINAL RESAMP PARAM          //
/////////////////////////////////////////

void GUI::InterpolTypeComboBoxChanged(int index)  /*SLOT*/ // index: 0=Linear, 1=Nearest Neighborhoor, 2=Windowed Sinc, 3=BSpline
{
  switch (index)
  {
  case 0:  m_optionStackLayout->setCurrentIndex(0);
    break;
  case 1:  m_optionStackLayout->setCurrentIndex(0);
    break;
  case 2:  m_optionStackLayout->setCurrentIndex(1);
    break;
  case 3:  m_optionStackLayout->setCurrentIndex(2);
    break;
  }
}

void GUI::TensorInterpolComboBoxChanged(int index) /*SLOT*/ // 0= log, 1= nolog
{
  switch (index)
  {
  case 0:  m_logOptionStackLayout->setCurrentIndex(0);
    break;
  case 1:  m_logOptionStackLayout->setCurrentIndex(1);
    break;
  }
}

  /////////////////////////////////////////
 //               DTI-REG               //
/////////////////////////////////////////

void GUI::RegMethodComboBoxChanged(int index) /*SLOT*/
{
  switch (index)
  {
  case 0:  m_DTIRegOptionStackLayout->setCurrentIndex(0);
    break;
  case 1:  m_DTIRegOptionStackLayout->setCurrentIndex(1);
    break;
  }
}

void GUI::SimMetChanged(int index) /*SLOT*/
{
  switch (index)
  {
  case 0:  {
    m_SimParamDble->setValue(2); //CC
    m_SimParamLabel->setText(QString("Region Radius:"));
    }
    break;
  case 1:  {
    m_SimParamDble->setValue(32); //MI
    m_SimParamLabel->setText(QString("Number of bins:"));
    }
    break;
  case 2: m_SimParamLabel->setText(QString("Similarity Parameter:")); //MSQ
    break;
  }
}

void GUI::ANTSRegTypeChanged(int index) /*SLOT*/
{
  switch (index)
  {
  case 2:  m_TfmStepLine->setText("0.25"); //GreedyDiffeo
    break;
  case 3:  m_TfmStepLine->setText("0.25,5,0.01"); //SpatioTempDiffeo
    break;
  }
}

  /////////////////////////////////////////
 //         WIDGET CHANGE SLOT          //
/////////////////////////////////////////

void GUI::WidgetHasChangedParamNoSaved() /*SLOT*/ //called when any widget is changed
{
  m_ParamSaved=0;
}

void GUI::GridProcesscheckBoxHasChanged(int state) /*SLOT*/ // called when Gid Processing is checked
{
/* State=0 if false, State=2 if true */
  if(state==2) NbThreadsSpinBox->setEnabled(false);
  else  NbThreadsSpinBox->setEnabled(true);
}

void GUI::OutputFolderLineEditHasChanged( QString NewOutputFolder )
{
  if( NewOutputFolder!="" )
  {
    if( CaseListWidget->count()>0 )
    {
      EnableQC();
    }
    CleanOutputPushButton->setEnabled(true);
  }
  else
  {
    DisableQC();
    CleanOutputPushButton->setEnabled(false);
  }
}

  /////////////////////////////////////////
 //           CHECK IMAGE OK            //
/////////////////////////////////////////

int GUI::checkImage(std::string Image) // returns 1 if not an image, 2 if not a dti, otherwise 0
{
  typedef itk::Image < double , 4 > ImageType; //itk type for image
  typedef itk::ImageFileReader <ImageType> ReaderType; //itk reader class to open an image
  ReaderType::Pointer reader=ReaderType::New();
  ImageType::RegionType region;

  reader->SetFileName( Image );

  try{
    reader->UpdateOutputInformation();
  }
  catch(itk::ExceptionObject & err)
  {
    return 1; // file is not an image
  }

  itk::ImageIOBase::IOPixelType pixel  = reader->GetImageIO()->GetPixelType() ;
  if( pixel == itk::ImageIOBase::SYMMETRICSECONDRANKTENSOR || pixel == itk::ImageIOBase::DIFFUSIONTENSOR3D || pixel == itk::ImageIOBase::VECTOR ) return 0; // test if DTI
  return 2;
}

  /////////////////////////////////////////
 //           MAIN FUNCTIONS            //
/////////////////////////////////////////

int GUI::Compute() /*SLOT*/
{
  if( m_ErrorDetectedInConstructor && m_noGUI )
  {
    ExitProgram();
    return -1;
  }
  else
  {

    if(!m_HierarchyModel->checkValidity())
    {
      if(!m_noGUI && !m_Testing) QMessageBox::critical(this, "No Cases", "There might be some node having no cases");
      else std::cout<<"| No Cases: Please give at least one case"<<std::endl;
    }
    else // OK Case
    {
  
      if(OutputFolderLineEdit->text().isEmpty())
      {
        if(!m_noGUI && !m_Testing) QMessageBox::critical(this, "No Output Folder", "Please give an output folder");
        else std::cout<<"| No Output Folder: Please give an output folder"<<std::endl;
      }
      else // OK Output
      {
        int WritingOutStatus = LaunchScriptWriter();

        std::cout<<"| Clearing previous cases in vectors..."; // command line display
        m_CasesPath.clear();
        m_scriptwriter->clearCasesPath();
        std::cout<<"DONE"<<std::endl; // command line display

        if(WritingOutStatus==-1) 
        {
          if(m_noGUI) ExitProgram(); // no possibility to change options because no GUI so QUIT
          return -1;
        }

        return LaunchScriptRunner(); // else

      } // else of if(OutputFolderLineEdit->text().isEmpty())

    } // else of if[Case]

    if(m_noGUI) ExitProgram(); // Only 1 compute in nogui mode
    return -1;

  } // else of if( m_ErrorDetectedInConstructor && m_noGUI )
}


void GUI::GenerateProjectDirectorySlot(){
    if(GenerateProjectDirectoryOnly()){
      QMessageBox::information(this,"Success","Project Generated in " + m_OutputPath);
    }
}

int GUI::GenerateProjectDirectoryOnly(){
  if( m_ErrorDetectedInConstructor && m_noGUI )
  {
    ExitProgram();
    return -1;
  }
  else
  {

    if(!m_HierarchyModel->checkValidity())
    {
      if(!m_noGUI && !m_Testing) QMessageBox::critical(this, "No Cases", "There might be some node having no cases");
      else std::cout<<"| No Cases: Please give at least one case"<<std::endl;
    }
    else // OK Case
    {
  
      if(OutputFolderLineEdit->text().isEmpty())
      {
        if(!m_noGUI && !m_Testing) QMessageBox::critical(this, "No Output Folder", "Please give an output folder");
        else std::cout<<"| No Output Folder: Please give an output folder"<<std::endl;
      }
      else // OK Output
      {
        int WritingOutStatus = LaunchScriptWriter();

        std::cout<<"| Clearing previous cases in vectors..."; // command line display
        m_CasesPath.clear();
        m_scriptwriter->clearCasesPath();
        std::cout<<"DONE"<<std::endl; // command line display

        if(WritingOutStatus==-1) 
        {
          if(m_noGUI) ExitProgram(); // no possibility to change options because no GUI so QUIT
          return -1;
        }

        return 1; //success

      } // else of if(OutputFolderLineEdit->text().isEmpty())

    } // else of if[Case]

    if(m_noGUI) ExitProgram(); // Only 1 compute in nogui mode
    return -1;

  } // else of if( m_ErrorDetectedInConstructor && m_noGUI )
}

bool GUI::CheckCase( std::string CasePath, bool NoErrorPopup ) // called when hitting compute and loading files
{
  // check if file exists
  if( ! itksys::SystemTools::GetPermissions(CasePath.c_str(), ITKmode_F_OK) ) // Test if the case files exist => itksys::SystemTools::GetPermissions() returns true if ITKmode_F(file)_OK
  {
    if(!m_noGUI && !m_Testing && !NoErrorPopup)
    {
      std::string text = "This file does not exist :\n" + CasePath;
      QMessageBox::critical(this, "Case does not exist", QString(text.c_str()) );
    }
    else if(!NoErrorPopup) std::cout<<"| This file does not exist : " << CasePath <<std::endl;
    return false;
  }

  // check if image
  int checkIm = checkImage(CasePath); // returns 1 if not an image, 2 if not a dti, otherwise 0
  if( checkIm == 1 ) // returns 1 if not an image, 2 if not a dti, otherwise 0
  {
    if(!m_noGUI && !m_Testing && !NoErrorPopup)
    {
      std::string text = "This file is not an image :\n" + CasePath;
      QMessageBox::critical(this, "Not Image", QString(text.c_str()) );
    }
    else if(!NoErrorPopup) std::cout<<"| This file is not an image : " << CasePath <<std::endl;
    return false;
  }
  if( checkIm == 2 ) // returns 1 if not an image, 2 if not a dti, otherwise 0
  {
    if(!m_noGUI && !m_Testing && !NoErrorPopup)
    {
      std::string text = "This image is not a DTI :\n" + CasePath;
      QMessageBox::critical(this, "Not DTI Image", QString(text.c_str()) );
    }
    else if(!NoErrorPopup) std::cout<<"| This image is not a DTI : " << CasePath <<std::endl;
    return false;
  }

  return true;

} // CheckCase
bool GUI::CheckAllCases()
{
  QStringList ql=m_HierarchyModel->getAllCasePaths();

  foreach(const QString &s, ql) 
  {
    //std::string CurrentCase = CaseListWidget->item(i)->text().toStdString().substr( CaseListWidget->item(i)->text().split(":").at(0).size()+2 );
    std::string CurrentCase= s.toStdString();
    // substr: keep only what is after "id: " (.size() returns the nb of digits of the id and +2 is for ": " after the id number)

    if( ! CheckCase( CurrentCase, false ) ) // returns false if bad file
    {
      return false;
    }
    m_CasesPath.push_back( CurrentCase );
  }

  //m_scriptwriter->setCasesPath(m_CasesPath); // m_CasesPath is a vector

  return true;
}

// bool GUI::CheckAllCases()
// {
//   for(int i=0; i < CaseListWidget->count() ;i++) 
//   {
//     std::string CurrentCase = CaseListWidget->item(i)->text().toStdString().substr( CaseListWidget->item(i)->text().split(":").at(0).size()+2 );
//     // substr: keep only what is after "id: " (.size() returns the nb of digits of the id and +2 is for ": " after the id number)

//     if( ! CheckCase( CurrentCase, false ) ) // returns false if bad file
//     {
//       return false;
//     }
//     m_CasesPath.push_back( CurrentCase );
//   }

//   m_scriptwriter->setCasesPath(m_CasesPath); // m_CasesPath is a vector

//   return true;
// }

bool GUI::CheckOutput( bool& FirstComputeInOutputFolder )
{
  m_OutputPath=OutputFolderLineEdit->text();
  if( ! itksys::SystemTools::GetPermissions(m_OutputPath.toStdString().c_str(), ITKmode_F_OK) ) // Test if the folder exists => itksys::SystemTools::GetPermissions() returns true if ITKmode_F(file)_OK
  {
    if(!m_noGUI && !m_Testing)
    {
      int ret = QMessageBox::question(this,"No Output Folder","The output folder does not exist. Do you want to create it?",QMessageBox::No | QMessageBox::Yes);
      if (ret == QMessageBox::Yes) // create folder
      {
        std::cout<<"| Creating Output folder..."<<std::endl; // command line display
        itksys::SystemTools::MakeDirectory( m_OutputPath.toStdString().c_str() );
      }
      else // no creation -> return
      {
        return false;
      }
    }
    else // if no gui, output folder not exist == exit
    {
      std::cout<<"| The output folder does not exist. Please create it."<<std::endl;
      return false;
    }
  }
  if( ! itksys::SystemTools::GetPermissions(m_OutputPath.toStdString().c_str(), ITKmode_W_OK) ) // Test if the program can write in the output folder => itksys::SystemTools::GetPermissions() returns true if ITKmode_F(file)_OK
  {
    if(!m_noGUI && !m_Testing) QMessageBox::critical(this, "Output Folder Unwritable", "Please give an output folder authorized in reading");
    else std::cout<<"| Please give an output folder authorized in reading"<<std::endl;
    return false;
  }
  m_scriptwriter->setOutputPath(m_OutputPath.toStdString());

  if( ! itksys::SystemTools::GetPermissions((m_OutputPath.toStdString() + "/common").c_str(), ITKmode_F_OK) ) // Test if the main folder does not exists => itksys::SystemTools::GetPermissions() returns true if ITKmode_F(file)_OK
  {
    std::cout<<"| Creating Configuration directory..."<<std::endl; // command line display
    std::string Dir = m_OutputPath.toStdString() + "/common";
    itksys::SystemTools::MakeDirectory( Dir.c_str() );
  }

  if( ! itksys::SystemTools::GetPermissions((m_OutputPath.toStdString() + "/scripts").c_str(), ITKmode_F_OK) ) // Test if the script folder does not exists => itksys::SystemTools::GetPermissions() returns true if ITKmode_F(file)_OK
  {
    std::cout<<"| Creating Script directory..."<<std::endl; // command line display
    std::string Dir = m_OutputPath.toStdString() + "/scripts";
    itksys::SystemTools::MakeDirectory( Dir.c_str() );
    FirstComputeInOutputFolder  = true;
  }

  return true;
} // CheckOutput

bool GUI::CheckTemplate()
{
  if (!TemplateLineEdit->text().isEmpty()) 
  {
    if( ! itksys::SystemTools::GetPermissions(TemplateLineEdit->text().toStdString().c_str(), ITKmode_F_OK) ) // Test if the case files exist => itksys::SystemTools::GetPermissions() returns true if ITKmode_F(file)_OK
    {
      if(!m_noGUI && !m_Testing)
      {
        std::string text = "This template file does not exist :\n" + TemplateLineEdit->text().toStdString();
        QMessageBox::critical(this, "Template does not exist", QString(text.c_str()) );
      }
      else std::cout<<"| This template file does not exist : " << TemplateLineEdit->text().toStdString() <<std::endl;
      return false;
    }
    int checkImTemp = checkImage(TemplateLineEdit->text().toStdString()); // returns 1 if not an image, 2 if not a dti, otherwise 0
    if( checkImTemp == 1 ) // returns 1 if not an image, 2 if not a dti, otherwise 0
    {
      if(!m_noGUI && !m_Testing)
      {
        std::string text = "This template file is not an image :\n" + TemplateLineEdit->text().toStdString();
        QMessageBox::critical(this, "No image", QString(text.c_str()) );
      }
      else std::cout<<"| This template file is not an image : " << TemplateLineEdit->text().toStdString() <<std::endl;
      return false;
    }
    if( checkImTemp == 0 ) // returns 1 if not an image, 2 if not a dti, otherwise 0 //the template has to be a FA or an MD image !!
    {
      if(!m_noGUI && !m_Testing)
      {
        std::string text = "This template image is not a scalar volume (FA or MD):\n" + TemplateLineEdit->text().toStdString();
        QMessageBox::critical(this, "No FA", QString(text.c_str()) );
      }
      else std::cout<<"| This template image is not a scalar volume (FA or MD): " << TemplateLineEdit->text().toStdString() <<std::endl;
      return false;
    }
    m_scriptwriter->setRegType(0);
    m_scriptwriter->setTemplatePath(TemplateLineEdit->text().toStdString());
  }

  else m_scriptwriter->setRegType(1); // default

  return true;
} // CheckTemplate

void GUI::SetResampOptions()
{
  m_scriptwriter->setInterpolType(InterpolTypeComboBox->currentText().toStdString());
  if( InterpolTypeComboBox->currentText()==QString("Windowed Sinc") ) m_scriptwriter->setInterpolOption(m_windowComboBox->currentText().toStdString());
  else if( InterpolTypeComboBox->currentText()==QString("BSpline") ) m_scriptwriter->setInterpolOption(m_BSplineComboBox->currentText().toStdString());

  m_scriptwriter->setTensInterpol(TensInterpolComboBox->currentText().toStdString());
  if( TensInterpolComboBox->currentText()==QString("Non Log Euclidean") ) m_scriptwriter->setInterpolLogOption(m_nologComboBox->currentText().toStdString());

  m_scriptwriter->setTensorTfm(TensTfmComboBox->currentText().toStdString());

/* Final Resamp options */

  m_scriptwriter->setDTIRegExtraPath(DTIRegExtraPathlineEdit->text().toStdString());

  std::vector < std::string > DTIRegOptions;

  DTIRegOptions.push_back(RegMethodcomboBox->currentText().toStdString());

  if( RegMethodcomboBox->currentText()==QString("ANTS") ) 
  {
    DTIRegOptions.push_back(m_ARegTypeComboBox->currentText().toStdString());
    DTIRegOptions.push_back(m_TfmStepLine->text().toStdString());
    DTIRegOptions.push_back(m_IterLine->text().toStdString());
    DTIRegOptions.push_back(m_SimMetComboBox->currentText().toStdString());

    DTIRegOptions.push_back( IntOrDoubleToStr<double>(m_SimParamDble->value()) );
    DTIRegOptions.push_back( IntOrDoubleToStr<double>(m_GSigmaDble->value()) );

    if(m_SmoothOffCheck->isChecked()) DTIRegOptions.push_back("1");
    else DTIRegOptions.push_back("0");
  }
  else //BRAINS
  {
    DTIRegOptions.push_back(m_BRegTypeComboBox->currentText().toStdString());
    DTIRegOptions.push_back(m_TfmModeComboBox->currentText().toStdString());

    DTIRegOptions.push_back( IntOrDoubleToStr<int>(m_NbPyrLevSpin->value()) );

    DTIRegOptions.push_back(m_PyrLevItLine->text().toStdString());
  }

  m_scriptwriter->setDTIRegOptions(DTIRegOptions);
  DTIRegOptions.clear();
} // SetResampOptions

void GUI::CheckProgram( std::string ProgramName, QLineEdit *ProgramPath, std::string& notFound )
{
  if( ProgramPath->text().isEmpty() )
  {
    std::string FoundprogramPath = FindProgram( ProgramName.c_str(), m_FindProgramDTIABExecDirVec );
    if(FoundprogramPath.empty()) notFound = notFound + "> " + ProgramName + "\n";
    else ProgramPath->setText(QString(FoundprogramPath.c_str()));
  }
} // CheckProgram

bool GUI::CheckPrograms()
{
  if(  ImagemathPath->text().isEmpty()
    || ResampPath->text().isEmpty()
    || CropDTIPath->text().isEmpty()
    || dtiprocPath->text().isEmpty()
    || BRAINSFitPath->text().isEmpty()
    || GAPath->text().isEmpty()
    || dtiavgPath->text().isEmpty()
    || DTIRegPath->text().isEmpty()
    || unuPath->text().isEmpty()
    || ITKTransformToolsPath->text().isEmpty()
    || MriWatcherPath->text().isEmpty() ) // if any path is missing => check in the config file and in the PATH
  {
    const char * value = itksys::SystemTools::GetEnv("DTIAtlasBuilderSoftPath"); // C function = const char * getenv(const char *)
    if (value)
    {
      LoadConfig( QString(value) ); // replace the paths by the paths given in the config file
    }

    std::string notFound;

    CheckProgram( "ImageMath", ImagemathPath, notFound );
    CheckProgram( "ResampleDTIlogEuclidean", ResampPath, notFound );
    CheckProgram( "CropDTI", CropDTIPath, notFound );
    CheckProgram( "dtiprocess", dtiprocPath, notFound );
    CheckProgram( "BRAINSFit", BRAINSFitPath, notFound );
    CheckProgram( "GreedyAtlas", GAPath, notFound );
    CheckProgram( "dtiaverage", dtiavgPath, notFound );
    CheckProgram( "DTI-Reg", DTIRegPath, notFound );
    CheckProgram( "unu", unuPath, notFound );
    CheckProgram( "ITKTransformTools", ITKTransformToolsPath, notFound );

    if(MriWatcherPath->text().isEmpty()) // MriWatcher special because not in notFound -> just warning no QC
    {
      std::string programPath = FindProgram("MriWatcher",m_FindProgramDTIABExecDirVec);
      if(programPath.empty()) DisableQC(); // notFound = notFound + "> MriWatcher (Program will work, but QC will not be available)\n";
      else MriWatcherPath->setText(QString(programPath.c_str()));
    }

    if( !notFound.empty() )
    {
      if(!m_noGUI && !m_Testing)
      {
        std::string text = "The following programs are missing.\nPlease enter the path manually:\n" + notFound;
        QMessageBox::critical(this, "Program missing", QString(text.c_str()) );
      }
      else std::cout<<"| The following programs are missing. Please modify the configuration file or enter the path manually in the GUI:\n" << notFound <<std::endl;
      return false;
    }
  }

  if(testGA()==-1) return false;
  if(testDTIReg()==-1) return false;

  return true;
} // CheckPrograms

bool GUI::CheckProgramExecutable( QLineEdit *ProgramPath )
{
  if(! itksys::SystemTools::GetPermissions(ProgramPath->text().toStdString().c_str(), ITKmode_X_OK))
  {
    if(!m_noGUI && !m_Testing)
    {
      std::string text = "The file \'" + ProgramPath->text().toStdString() + "\' is not executable";
      QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
    }
    else std::cout<<"| The file \'" << ProgramPath->text().toStdString() << "\' is not executable" << std::endl;
    return false;
  }
  return true;
} // CheckProgramExecutable


bool GUI::CheckProgramsExecutable()
{
  if( ! CheckProgramExecutable( ImagemathPath                ) ) return false;
  if( ! CheckProgramExecutable( ResampPath                   ) ) return false;
  if( ! CheckProgramExecutable( CropDTIPath                  ) ) return false;
  if( ! CheckProgramExecutable( dtiprocPath                  ) ) return false;
  if( ! CheckProgramExecutable( BRAINSFitPath                ) ) return false;
  if( ! CheckProgramExecutable( GAPath                       ) ) return false;
  if( ! CheckProgramExecutable( dtiavgPath                   ) ) return false;
  if( ! CheckProgramExecutable( DTIRegPath                   ) ) return false;
  if( ! CheckProgramExecutable( unuPath                      ) ) return false;
  if( ! CheckProgramExecutable( ITKTransformToolsPath        ) ) return false;

  // MriWatcher special because no critical -> just warning no QC
  if(MriWatcherPath->text().isEmpty()) DisableQC();
  else if(! itksys::SystemTools::GetPermissions(MriWatcherPath->text().toStdString().c_str(), ITKmode_X_OK) != 0)
  {
    if(!m_noGUI && !m_Testing)
    {
      std::string text = "The file \'" + MriWatcherPath->text().toStdString() + "\' is not executable: program will work, but QC will not be available";
      QMessageBox::warning(this, "Non executable File", QString(text.c_str()) );
    }
    else std::cout<<"| The file \'" << MriWatcherPath->text().toStdString() << "\' is not executable: program will work, but QC will not be available" << std::endl;
    DisableQC(); // return false;
  }

  std::vector < std::string > SoftPath;

  SoftPath.push_back(ImagemathPath->text().toStdString());
  SoftPath.push_back(ResampPath->text().toStdString());
  SoftPath.push_back(CropDTIPath->text().toStdString());
  SoftPath.push_back(dtiprocPath->text().toStdString());
  SoftPath.push_back(BRAINSFitPath->text().toStdString());
  SoftPath.push_back(GAPath->text().toStdString());
  SoftPath.push_back(dtiavgPath->text().toStdString());
  SoftPath.push_back(DTIRegPath->text().toStdString());
  SoftPath.push_back(unuPath->text().toStdString());
  SoftPath.push_back(ITKTransformToolsPath->text().toStdString());

  m_scriptwriter->setSoftPath(SoftPath);
  SoftPath.clear();

  return true;
} // CheckProgramsExecutable

bool GUI::FindPython()
{
  // m_PythonPath member because needed in LaunchScriptRunner()

  m_PythonPath = ""; // in case it has been used before
  if(DTIAtlasBuilder_BUILD_SLICER_EXTENSION)
  {
    // Use the python executable copied from the Slicer build tree (python-build/bin/python or customPython) to the ExternalBin folder
    if((std::string)Platform == "linux" || (std::string)Platform == "mac")
    {
      m_PythonPath = m_DTIABSlicerExtensionExternalBinDir + "/SlicerPython";
    }
    else if((std::string)Platform == "win")
    {
      m_PythonPath = m_DTIABSlicerExtensionExternalBinDir + "/SlicerPython.exe";
    }

    if(! itksys::SystemTools::GetPermissions(m_PythonPath.c_str(), ITKmode_X_OK) ) // if still not here: use default python
    {
      m_PythonPath=""; // If Slicer Extension and testing, ExternalBin folder does not exist so just leave it // itksys::SystemTools::GetPermissions() returns true if ok, -1 if not ok
    }

  } // if(DTIAtlasBuilder_BUILD_SLICER_EXTENSION)

  if( m_PythonPath.empty() ) // Find Python version used // If not slicer ext -> empty, if slicer ext and test and not found -> empty
  {
    m_PythonPath = FindProgram("python",m_FindProgramDTIABExecDirVec);
    if(m_PythonPath.empty())
    {
      std::string PythonNotFoundtext = "Python has not been found. Please install python before running DTIAtlasBuilder.";
      if(!m_noGUI && !m_Testing)
      {
        QMessageBox::critical(this, "Python not found", QString(PythonNotFoundtext.c_str()) );
      }
      else std::cout<<"| "<< PythonNotFoundtext << std::endl;
      return false;
    }
  }

  // Check python version (need > 2.5)
  QProcess * Process = new QProcess;
  std::string program = m_PythonPath + " --version";
  std::cout<<"| Python path: " << m_PythonPath << std::endl ;
  std::cout<<"| Checking the version of python...";
  Process->setProcessChannelMode(QProcess::MergedChannels);
  Process->start( program.c_str(), QIODevice::ReadWrite  );
  Process->waitForFinished();
  Process->waitForReadyRead();
  QString PythonVersion = QString( Process->readAllStandardOutput().data() ).split(" ").at(1).split("\n").at(0); // "Python X.X.X\n"
  std::cout<<"DONE"<<std::endl;

  if( PythonVersion.isEmpty() || PythonVersion.split(".").at(0).toInt()<2 || (PythonVersion.split(".").at(0).toInt()<3 && PythonVersion.split(".").at(1).toInt()<5) )
  {
    std::string PythonBadVersiontext = "The version of python found (" + m_PythonPath + ", version " + PythonVersion.toStdString() + ") is too old. DTIAtlasBuilder needs a version older than 2.5.";
    if(!m_noGUI && !m_Testing)
    {
      QMessageBox::critical(this, "Python version issue", QString(PythonBadVersiontext.c_str()) );
    }
    else std::cout<<"| "<< PythonBadVersiontext << std::endl;
    return false;
  }

  std::cout<<"| Found python version "<< PythonVersion.toStdString() << std::endl;
  m_scriptwriter->setPythonPath(m_PythonPath);
  return true;
} // FindPython

bool GUI::CheckVoxelSizeAndCropping()
{
  QStringList casePaths=m_HierarchyModel->getAllCasePaths();

  try{

      if(m_scriptwriter->CheckVoxelSize(casePaths)==1) 
      {
        if(!m_noGUI && !m_Testing) QMessageBox::critical(this, "Different Voxel Sizes", "Error: The voxel size of the images\nare not the same,\nplease change dataset"); // returns 0 if voxel size OK , otherwise 1
        else std::cout<<"| Error: The voxel size of the images are not the same, please change dataset" << std::endl;
        return false;
      }
      m_NeedToBeCropped=m_scriptwriter->setCroppingSize( SafetyMargincheckBox->isChecked() ); // returns 0 if no cropping , 1 if cropping needed
      if( m_NeedToBeCropped==1 && !SafetyMargincheckBox->isChecked() )
      {
        if(!m_noGUI && !m_Testing) QMessageBox::warning(this, "Cropping", "Warning: The images do not have the same size, \nso some of them will be cropped");
        else std::cout<<"| Warning: The images do not have the same size, so some of them will be cropped" << std::endl;
      }
  }catch(const std::exception &e){
    std::cout << "Exception occurred in GUI::CheckVoxelSizeAndCropping" << std::endl;
    return false;
  }

  return true;
} // CheckVoxelSizeAndCropping

void GUI::SetOtherOptions()
{
  if(OverwritecheckBox->isChecked()) m_scriptwriter->setOverwrite(1);
  else m_scriptwriter->setOverwrite(0);

  m_scriptwriter->setnbLoops(NbLoopsSpinBox->value());
  m_scriptwriter->setnbLoopsDTIReg(NbLoopsDTIRegSpinBox->value());
  m_scriptwriter->setBFAffineTfmMode(BFAffineTfmModecomboBox->currentText().toStdString());

  m_scriptwriter->setGridProcess( GridProcesscheckBox->isChecked() ); // isChecked() returns true or false
  m_scriptwriter->setGridGeneralCommand( GridProcessCmdLineEdit->text().toStdString() );
  m_scriptwriter->setGridAtlasCommand( GridProcessAtlasCmdLineEdit->text().toStdString() );

  if( GridProcesscheckBox->isChecked() ) m_scriptwriter->setNbThreads(1); // Not used in script, but fct needs to be called to initialize var
  else m_scriptwriter->setNbThreads( NbThreadsSpinBox->value() );
  m_scriptwriter->setScalarMeasurement( ScalarMeasurementComboBox->currentText().toStdString() ) ;
}  // SetOtherOptions


void GUI::GenerateScriptFile( std::string ScriptName, std::string ScriptText )
{

  std::cout<<"| Generating " << ScriptName << " script file..."; // command line display

  QString ScriptPath = m_OutputPath + QString("/scripts/DTIAtlasBuilder_") + QString(ScriptName.c_str()) + QString(".py");
  QFile file(ScriptPath);

  if ( file.open( QFile::WriteOnly ) )
  {
    //file.setPermissions(QFile::ExeOwner); //make the file executable for the owner
    QTextStream stream( &file );
    stream << QString( ScriptText.c_str() ) << endl;
    std::cout<<"DONE"<<std::endl; // command line display

    itksys::SystemTools::SetPermissions( (ScriptPath.toStdString()).c_str(), ITKmode_X_OK );
  }
  else qDebug( "Could not create file");

} // GenerateScriptFile

void GUI::GenerateServerScript()
{
    std::cout<<"| Generating Server script file..."; // command line display

    QString ScriptPath = m_OutputPath + QString("/scripts/RunCommandOnServer.py");
    QFile fileMain(ScriptPath);

    if ( fileMain.open( QFile::WriteOnly ) )
    {
      QTextStream stream( &fileMain );

      stream << "#!/usr/bin/python" << endl << endl;

      stream << "import os" << endl ;
      stream << "import sys # to get the arguments" << endl << endl;

      stream << "# arguments: [file to create] [commands to execute]" << endl;
      stream << "# example : RunCommandOnServer.py file.txt \"ls\" \"du -sh\" \"nautilus .\"" << endl << endl;

      stream << "os.putenv(\"ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS\",\"1\") # to prevent programs from using too many cores on the cluster" << endl << endl;

      stream << "i=2 # sys.argv[0] is the name of the executable and sys.argv[1] is the name of the file to create" << endl;
      stream << "while i < len(sys.argv):" << endl;
        stream << "  Command = sys.argv[i]" << endl;
        stream << "  print(\"Running Command : \" + Command)" << endl;
        stream << "  os.system(Command)" << endl;
        stream << "  i += 1"<< endl <<endl;

      stream << "FileName=sys.argv[1]" << endl ;
      stream << "print(\"Creating file : \" + FileName + \"\\n\")" << endl ;
      stream << "f = open(FileName,'w')" << endl ;
      stream << "f.close()" << endl ;

      std::cout<<"DONE"<<std::endl; // command line display

      itksys::SystemTools::SetPermissions( (ScriptPath.toStdString()).c_str(), ITKmode_X_OK );
    }
    else qDebug( "Could not create file");

} // GenerateServerScript

/*
    MASTER FUNCTION
*/
int GUI::LaunchScriptWriter()
{
  bool FirstComputeInOutputFolder = false; // to avoid confusion with absence of version file (normal if first compute)

/* Checking and Setting the values */
  if( ! CheckAllCases() )
  {
    return -1;
  }

  if( ! CheckOutput( FirstComputeInOutputFolder ) )
  {
    return -1;
  }

  if( ! CheckTemplate() )
  {
    return -1;
  }

  SetResampOptions();

/* Software paths */
  if( ! CheckPrograms() ) // Checking if all the programs have been given
  {
    return -1;
  }

  if( ! CheckProgramsExecutable() ) // Checking if the given files are executable
  {
    return -1;
  }

/* Find Python */
  if( ! FindPython() )
  {
    return -1;
  }

  bool chk;
  try{
    chk=CheckVoxelSizeAndCropping();
  }catch(const std::exception &e){
    std::cout << "Exception while CheckVoxelSizeAndCropping" << std::endl;
  }
  
  if( ! chk )
  {
    return -1;
  }

  SetOtherOptions();

/* Launch writing */
  //m_scriptwriter->WriteScript(); // Master Function : get pid to send a signal to Qt process to move progress bar
  m_scriptwriter->setHierarchy(m_HierarchyModel->getHierarchy());
  m_scriptwriter->WriteScriptFromTemplate(m_HierarchyModel->getAllCasePaths()); //New Writer from template python source file

  GenerateXMLForGA();

  //SaveCSVResults(m_NeedToBeCropped,NbLoopsSpinBox->value());
  SaveParameters(m_OutputPath + QString("/common/DTIAtlasBuilderParameters.txt"), m_OutputPath + QString("/common/DTIAtlasBuilderDataset.csv"));

  GenerateScriptFile( "Preprocess", m_scriptwriter->getScript_Preprocess() );
  GenerateScriptFile( "AtlasBuilding", m_scriptwriter->getScript_AtlasBuilding() );
  GenerateScriptFile( "Main", m_scriptwriter->getScript_Main() );
  GenerateScriptFile( "Utilities", m_scriptwriter->getScript_Utilities() );

  if( GridProcesscheckBox->isChecked() )
  {
    GenerateServerScript();
  }

  return 0;
} // LaunchScriptWriter()

int GUI::LaunchScriptRunner()
{

  ComputepushButton->setEnabled(false);
  StoppushButton->setEnabled(true);
  CleanOutputPushButton->setEnabled(false);
  m_ScriptRunning=true;
  progressBar->setMaximum(0); // Setting max and min to zero to behave as a busy indicator

/* Running the Script: */ // python path found before writing script : contains already a space after the command
  std::string program;
  program = m_PythonPath + " " + m_OutputPath.toStdString() + "/scripts/DTIAtlasBuilder_Main.py"; // 
  std::cout<<"| Starting: " << program << std::endl;

  std::cout<<"| Script Running..."<< std::endl; // command line display

  std::string LogFilePath = m_OutputPath.toStdString() + "/scripts/DTIAtlasBuilder.log";
  m_ScriptQProcess->setStandardOutputFile(LogFilePath.c_str(), QIODevice::Truncate); // Truncate = overwrite // Append= write after what's written

  if(m_noGUI) // !! no log file in the case of nogui (execute will write the stdout in the console, as it is the same process)
  {
    m_ScriptQProcess->execute( program.c_str() ); // execute will stay stuck in the program and wait for the script to be done
  }
  else
  {
    m_ScriptQProcess->start( program.c_str() ); // start will just start the program in another process
    m_ScriptRunningQTimer->start(1000); // To update display in cmd line every second (display dots moving)
  }
  // If needed, m_ScriptQProcess->pid()

  std::string text = "Script Running";
  ScriptRunningDisplayQLabel->setText( QString( text.c_str() ) );

  return 0;
}

void GUI::KillScriptQProcess() /* SLOT */
{
  StoppushButton->setEnabled(false);
  m_ScriptQProcess->kill(); // kill the running process
  std::cout<<"| Main Script Aborted."<<std::endl;

/* Kill other processes by  PID : read PID.log file */
  if((std::string)Platform == "mac" || (std::string)Platform == "linux") // Kill PID
  {
    QString PIDlogFilePath = m_OutputPath + QString("/scripts/PID.log");
    if( itksys::SystemTools::GetPermissions(PIDlogFilePath.toStdString().c_str(), ITKmode_F_OK) ) // PID file exists -> read it
    {
      QFile PIDfile(PIDlogFilePath);
      if( PIDfile.open(QFile::ReadOnly) )
      {
        QTextStream PIDstream(&PIDfile);
        QString PID = PIDstream.readLine();
        while( ! PID.isEmpty() ) // end of file = PID empty
        {
          std::cout<<"| Killing: "<< PID.toStdString() <<std::endl;
          std::string KillCommand = "kill " + PID.toStdString();
          QProcess * KillProcess = new QProcess;
          KillProcess->execute( KillCommand.c_str() );

          PID = PIDstream.readLine();
        } // while file not at end
      } // open file in reading
    } // if file exists
  } // if mac or windows
}

void GUI::UpdateScriptRunningGUIDisplay() /*SLOT*/ // called by QTimer every second
{
  if(ScriptRunningDisplayQLabel->text().size()>=19)
  {
    std::string text = "Script Running";
    ScriptRunningDisplayQLabel->setText( QString( text.c_str() ) ); // display only 5 dots at a time ("Script Running" = 14)
  }

  QString ScriptRunningDisplay = ScriptRunningDisplayQLabel->text() + ".";
  ScriptRunningDisplayQLabel->setText(ScriptRunningDisplay);

  /* TODO: Search the log file for keywords to know at what step the processing is */

}

void GUI::ScriptQProcessDone(int ExitCode) /*SLOT*/ // called
{
  if(!m_noGUI) // no timer if no GUI
  {
    m_ScriptRunningQTimer->stop();
  }

  progressBar->setMaximum(100); // Stop behaving as a busy indicator
  progressBar->setValue(100);
  ComputepushButton->setEnabled(true);
  StoppushButton->setEnabled(false);
  CleanOutputPushButton->setEnabled(true);
  m_ScriptRunning=false;

  if(ExitCode==0) RunningCompleted();
  else RunningFailed();

  if(!m_noGUI)
  {
    std::string LogFileText = "| Log file written in: " + m_OutputPath.toStdString() + "/scripts/DTIAtlasBuilder.log";
    std::cout<<LogFileText<<std::endl;
    ScriptRunningDisplayQLabel->setText(ScriptRunningDisplayQLabel->text() + QString(LogFileText.c_str()));
  }
}

void GUI::RunningCompleted()
{
  ScriptRunningDisplayQLabel->setText("Running Completed ");

  std::cout<< "| Running Completed !"<<std::endl;
  std::cout<< "| Final Atlas is in: " << m_OutputPath.toStdString() << "/final_atlas/FinalAtlasDTI.nrrd"<<std::endl;
  std::cout<< "| Final Displacement Fields for each case are in: " << m_OutputPath.toStdString() << "/final_atlas/FinalDeformationFields/<case>_GlobalDisplacementField.nrrd" <<std::endl;

  if(!m_noGUI && !m_Testing)
  {
    std::string RunningCompletedTextPopup = "Running Completed !\n\nFinal Atlas is in:\n" + m_OutputPath.toStdString() + "/final_atlas/FinalAtlasDTI.nrrd\n\nFinal Displacement Fields for each case are in:\n" + m_OutputPath.toStdString() + "/final_atlas/FinalDeformationFields/<case>_GlobalDisplacementField.nrrd";
    QMessageBox::information(this, "Running Completed", QString(RunningCompletedTextPopup.c_str()));
  }
}

void GUI::RunningFailed()
{
  ScriptRunningDisplayQLabel->setText("Running Failed ");

  if(!m_noGUI && !m_Testing) QMessageBox::information(this, "Running Failed", "Running Failed...");
  std::cout<<"| Running Failed..."<<std::endl; // command line display
}

