

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
	m_currentItem=m_rootNode;

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


bool CaseHierarchyModel::checkNodename(QString name){
	auto it=m_CaseHierarchy["build"].find(name.toStdString());
	if(it!=m_CaseHierarchy["build"].end()){
		return 1;
	}
	else{
		return 0;
	}
}

void CaseHierarchyModel::addNode(QString nodename){

	if(checkNodename(nodename)){
		std::cout << "It is redundant node name" << std::endl;
	}else{
		m_CaseHierarchy["build"][nodename.toStdString()]["type"]="end_node";
		m_CaseHierarchy["build"][nodename.toStdString()]["filetype"]="list";
		m_CaseHierarchy["build"][nodename.toStdString()]["components"]={};
		m_CaseHierarchy["build"][nodename.toStdString()]["datasetfiles"]={};
		QStandardItem* itm = getCurrentItem();
		QString p_nodename = getCurrentTag();
		m_CaseHierarchy["build"][p_nodename.toStdString()]["type"]=std::string("node");
		m_CaseHierarchy["build"][p_nodename.toStdString()]["components"].push_back(nodename.toStdString());
		m_CaseHierarchy["build"][p_nodename.toStdString()].erase("filetype");
		m_CaseHierarchy["build"][p_nodename.toStdString()].erase("datasetfiles");
		QStandardItem* c = new QStandardItem(nodename);
		itm->appendRow(c);
	}


}

void CaseHierarchyModel::removeNode(QStandardItem* itm){
	if(itm!=m_rootNode){
		int r= itm->row();
		QStandardItem* parent=itm->parent();
		QString nn=itm->text();
		QString pn=parent->text();
		std::vector<std::string> v;
		foreach(auto  &str, m_CaseHierarchy["build"][pn.toStdString()]["components"]){
		 	std::string s=std::string(str);
			if(nn!=QString(s.c_str())){
				//std::cout << "remainingNode " << s << std::endl;
				v.push_back(s);
			}
		}
		if(v.begin()==v.end()){
			std::cout << "No more components, changing to end_node" << std::endl;
			m_CaseHierarchy["build"][pn.toStdString()]["type"]=std::string("end_node");
		}
		m_CaseHierarchy["build"][pn.toStdString()]["components"]=v;
		// if(m_CaseHierarchy["build"][nn.toStdString()]["type"]==QString("node")){
		// 	json comps=m_CaseHierarchy["build"][nn.toStdString()]["components"];
		// 	foreach(auto )
		// }
		//m_CaseHierarchy["build"].erase(nn.toStdString());
		removeNodeRecursivelyInJson(nn);
		parent->removeRow(r);
		setCurrentTag(m_rootNode->index());	
	}else{
		std::cout << "Root node cannot be removed" << std::endl;
	}

}	

void CaseHierarchyModel::removeCurrentNode(){
	removeNode(m_currentItem);
}

void CaseHierarchyModel::removeNodeRecursivelyInJson(QString nn){
	std::string t = m_CaseHierarchy["build"][nn.toStdString()]["type"];
	if(QString(t.c_str())==QString("end_node")){
		m_CaseHierarchy["build"].erase(nn.toStdString());
	}else{
		std::vector<std::string> comps = m_CaseHierarchy["build"][nn.toStdString()]["components"];
		foreach(const std::string &str, comps){
			removeNodeRecursivelyInJson(QString(str.c_str()));
		}
		m_CaseHierarchy["build"].erase(nn.toStdString());
	}
}

void CaseHierarchyModel::setCurrentTag(QModelIndex idx){
	m_currentItem=itemFromIndex(idx);
	m_currentTag=m_currentItem->text();

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
	m_currentItem=m_rootNode;
	m_currentTag=QString("target");

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
