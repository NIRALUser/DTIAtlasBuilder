

#include "HierarchyModel.h"
#include <QFile>
#include <QTextStream>
#include <iostream>
#include <QStringList>
#include <string>
#include <vector>


CaseHierarchyModel::CaseHierarchyModel(){
	m_rootNode=new QStandardItem(QString("target"));
	appendRow(m_rootNode);
	json obj={{"type","end_node"},{"filetype","list"},{"datasetfiles",{}}};
	m_CaseHierarchy["build"]["target"]=obj;
	m_currentTag="target";

}
CaseHierarchyModel::CaseHierarchyModel(QString filename){ // hbuild filename
	CaseHierarchyModel();
	
	// Todos
	initialize(filename);
}
CaseHierarchyModel::~CaseHierarchyModel(){}

void CaseHierarchyModel::initialize(QString filename){
	//setHorizontalHeaderLabels(QStringList(["Nodes"]));
	loadFile(filename);
	generateEntries();
}
void CaseHierarchyModel::initialize(){

}


QString CaseHierarchyModel::getCurrentType(){
	std::string s=m_CaseHierarchy["build"][m_currentTag.toStdString()]["type"];
	return QString(s.c_str());
}


void CaseHierarchyModel::loadFile(QString filename){
	QFile file(filename);
	file.open(QIODevice::ReadOnly);
	QTextStream s(&file);
	QString str=s.readAll();
	m_CaseHierarchy=json::parse(str.toStdString());
	generateEntries();
}

void CaseHierarchyModel::saveFile(QString filename){
	QFile file(filename);
	file.open(QIODevice::WriteOnly);
	QTextStream fs(&file);
	std::string str=m_CaseHierarchy.dump(4);
	fs << QString(str.c_str());
}

void CaseHierarchyModel::expandNode(QStandardItem* p_node,json hbuild){
	QString tag=p_node->text();
	json obj=hbuild[tag.toStdString()];
	//std::cout << obj.dump() << std::endl;

	
	std::string _type=obj["type"];
	//std::cout << _type << std::endl;

	if(QString(_type.c_str())==QString("node")){
		json components=obj["components"];
		for(auto it=components.begin(); it!=components.end();++it){
			//std::cout << it.value() << std::endl;
			std::string t=it.value();
			QStandardItem* c_node = new QStandardItem(QString(t.c_str()));
			p_node->appendRow(c_node);
			expandNode(c_node,hbuild);
		}

	}else{
		//End node, read file list
		std::string _ft=obj["filetype"];
		if(QString(_ft.c_str())==QString("list")){
			std::vector<std::string> fl= obj["datasetfiles"];
			// foreach(const std::string &str, fl){
			// 	//std::cout << str << std::endl;
			// }
		}else if(QString(_ft.c_str())==QString("dataset")){
			std::string s=obj["datasetfiles"];
			QStringList ql=readCSV(QString(s.c_str()));
			std::vector<std::string> fl;
			foreach(const QString &str, ql){
				//std::cout << str.toStdString() << std::endl;
				fl.push_back(str.toStdString());
			}
			m_CaseHierarchy["build"][tag.toStdString()]["filetype"]=std::string("list");
			m_CaseHierarchy["build"][tag.toStdString()]["datasetfiles"]=fl;
		}else{

		}
	}

}

void CaseHierarchyModel::generateEntries(json obj){
	json hbuild=obj["build"];
	//std::cout << hbuild.dump() << std::endl;
	//std::cout << root->text().toStdString() << std::endl;
	delete m_rootNode;
	clear();
	m_rootNode=new QStandardItem(QString("target"));
	appendRow(m_rootNode);
	expandNode(m_rootNode,hbuild);

}
void CaseHierarchyModel::generateEntries(){
	generateEntries(m_CaseHierarchy);
}

void CaseHierarchyModel::setFiles(QString nodename, QStringList ql){
	std::vector<std::string> v;
	foreach(const QString& str, ql){
		//std::cout << "In HM : " << str.toStdString() << std::endl;
		v.push_back(str.toStdString());
	}
	m_CaseHierarchy["build"][nodename.toStdString()]["filetype"]=std::string("list");
	m_CaseHierarchy["build"][nodename.toStdString()]["datasetfiles"]=v;
}

QStringList CaseHierarchyModel::getFileList(QString nodestr){
	QStringList ql;
	foreach(const std::string &str, m_CaseHierarchy["build"][nodestr.toStdString()]["datasetfiles"]){
		ql.append(str.c_str());
	}
	return ql;
}

QStringList CaseHierarchyModel::readCSV(QString CSVfile)
{
  QStringList CSVCaseList;
  if(!CSVfile.isEmpty())
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
            QStringList list = line.split(QString(","));
            if( list.at(0) != "id" && list.at(0) != "")
            {
              CSVCaseList.append( list.at(1) );
            }
          }
        }

        std::cout<<"DONE"<<std::endl; // command line display
        return CSVCaseList;
  	   }
	}
}
