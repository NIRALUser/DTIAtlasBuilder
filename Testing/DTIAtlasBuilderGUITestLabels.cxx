// Qt includes
#include <QTimer>

// DTIAtlasBuildder includes
#include "GUI.h"

int main(int argc, char * argv[] ) // called with $ DTIAtlasBuilderGUITestLabels (paramFileConfigured) (configFile) (TestBinFolder)
{
  if( argc < 4 ) return -1;

  QApplication app(argc, argv); // create a QApplication

  GUI TestGUI( argv[1], argv[2], "", false, false, true, argv[0] ); // create a GUI object : GUI( ParamFile, ConfigFile, CSVFile, Overwrite, noGUI, Testing, argv[0] )

  TestGUI.show(); // open the window

  QTimer::singleShot(2000, qApp, SLOT(quit())); // close the window after 2000ms = 2s

  if(TestGUI.OutputFolderLineEdit->text().toStdString() != argv[3]) return -1;

  return app.exec();
}
