#include <iostream>
#include <string>

#include <QApplication>

#include "GUI.h"

#include "DTIAtlasBuilderCLP.h" //generated when ccmake

int main(int argc, char* argv[])
{
  PARSE_ARGS; //thanks to this line, we can use the variables entered in command line as variables of the program
  //string ParamFile, string ConfigFile, string CSVFile, bool Overwrite, bool noGUI

  QApplication app(argc, argv);

  std::string commandDirectory = app.applicationDirPath().toStdString();//itksys::SystemTools::GetFilenamePath(itksys::SystemTools::GetRealPath( argv[0] ));
  std::cout << "Executable : " << commandDirectory <<std::endl;
  GUI AtlasGUI(ParamFile, ConfigFile, CSVFile, Overwrite, noGUI, false, commandDirectory); // argv[0] is the command that the user has ran -> to search the config file in the same directory

/* Launch App */
  if(noGUI) return AtlasGUI.Compute();
  else
  {
    AtlasGUI.show();
    return app.exec();
  }

  return -1;
}

