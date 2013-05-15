// Qt includes
#include <QTimer>

// DTIAtlasBuildder includes
#include "GUI.h"

int main(int argc, char * argv[] )
{
  QApplication app(argc, argv); // create a QApplication

  GUI TestGUI( "", argv[1], "", false, false, true, argv[0] ); // create a GUI object : GUI( ParamFile, ConfigFile, CSVFile, Overwrite, noGUI, Testing, argv[0] )

  TestGUI.show(); // open the window

  QTimer::singleShot(2000, qApp, SLOT(quit())); // close the window after 2000ms = 2s

  return app.exec();
}
