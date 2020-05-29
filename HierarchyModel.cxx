

#include "HierarchyModel.h"
#include <QFile>
#include <QTextStream>
#include <iostream>
#include <QStringList>
#include <string>
#include <vector>
#include <QMessageBox>

CaseHierarchyModel::CaseHierarchyModel(){
	CaseHierarchyModel(QString("final"));

}
CaseHierarchyModel::CaseHierarchyModel(QString project_name){
	m_projectName=project_name;
	m_rootNode=new QStandardItem(m_projectName);
	appendRow(m_rootNode);
	json obj={{"type","end_node"},{"filetype","list"},{"datasetfiles",json::array()}};
	m_CaseHierarchy["project"]["target_node"]=m_projectName.toStdString();
	m_CaseHierarchy["build"][m_projectName.toStdString()]=obj;
	m_currentTag=m_projectName;
	m_currentItem=m_rootNode;
	// QObject::connect(this, SIGNAL(dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)), this, SLOT(onItemChanged(const QModelIndex &topLeft)));

}
CaseHierarchyModel::CaseHierarchyModel(QString name, bool fromFile){ // hbuild filename
	// CaseHierarchyModel(name);
	
	// Todos
	if(fromFile){
		initializeFromFile(name);
	}else{
		CaseHierarchyModel(name);
	}
	
}
CaseHierarchyModel::~CaseHierarchyModel(){}

void CaseHierarchyModel::initializeFromFile(QString filename){
	//setHorizontalHeaderLabels(QStringList(["Nodes"]));
	loadFile(filename);
	std::string s= m_CaseHierarchy["project"]["target_node"];
	m_projectName=QString(s.c_str());
	m_rootNode=new QStandardItem(m_projectName);
	generateEntries();
}
void CaseHierarchyModel::initialize(QString project_name){
	clear();
	m_projectName=project_name;
	m_rootNode=new QStandardItem(m_projectName);
	appendRow(m_rootNode);
	m_CaseHierarchy["build"].clear();
	json obj={{"type","end_node"},{"filetype","list"},{"datasetfiles",json::array()}};
	m_CaseHierarchy["project"]["target_node"]=m_projectName.toStdString();
	m_CaseHierarchy["build"][m_projectName.toStdString()]=obj;
	m_currentTag=m_projectName;
	m_currentItem=m_rootNode;
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

QStringList CaseHierarchyModel::getRootComponents(){
	QStringList ql;
	std::string rootname=getRoot()->text().toStdString();
	std::string rootnodetype=m_CaseHierarchy["build"][rootname]["type"];
	if(QString(rootnodetype.c_str())==QString("end_node")){
		std::vector<std::string> comps=m_CaseHierarchy["build"][rootname]["datasetfiles"];
		foreach(const std::string& s, comps ){
			ql.append(QString(s.c_str()));
		}
	}else{
		std::vector<std::string> comps=m_CaseHierarchy["build"][rootname]["components"];
		foreach(const std::string& s, comps){
			ql.append(QString(s.c_str()));
		}
	}
	return ql;
}

bool CaseHierarchyModel::checkCaseExists(QString name){
	std::string tmp=m_CaseHierarchy["build"][name.toStdString()]["type"];
	QString nodeType=QString(tmp.c_str());
	if(nodeType==QString("end_node")){
	    int size = m_CaseHierarchy["build"][name.toStdString()]["datasetfiles"].size();
	    if(size>0)
	    {
	    	return true;
	    }
	}
	return false;
}
bool CaseHierarchyModel::isRoot(QString name){
	return name==m_rootNode->text();
}
bool CaseHierarchyModel::checkValidity(){
	foreach(const json &obj, m_CaseHierarchy["build"]){
		std::string s=obj["type"];
		QString nodeType=QString(s.c_str());
		if(nodeType==QString("end_node")){
			if(obj["datasetfiles"].size() ==0){
				return false;
			}
		}
	}
	return true;
}

void CaseHierarchyModel::addNode(QString nodename){

	if(checkNodename(nodename)){
		std::cout << "It is redundant node name" << std::endl;
	}else{
		m_CaseHierarchy["build"][nodename.toStdString()]["type"]="end_node";
		m_CaseHierarchy["build"][nodename.toStdString()]["filetype"]="list";
		m_CaseHierarchy["build"][nodename.toStdString()]["components"]={};
		m_CaseHierarchy["build"][nodename.toStdString()]["datasetfiles"]=json::array();
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

int CaseHierarchyModel::changeCurrentNode(QModelIndex idx,QString newName){
	int result=0;
	QString oldName=m_currentTag;
	if(checkNodename(newName)){
		std::cout << "It is redundant node name" << std::endl;
		//((QStandardItem*)idx.internalPointer())->setText(oldName);
		QStandardItemModel* m = (QStandardItemModel*)(idx.model());
		m->itemFromIndex(idx)->setText(oldName);
    	//QMessageBox::warning(this, "Failed to change the name", "There is already existing node name : ");
		result=1;

	}else{
		m_CaseHierarchy["build"][newName.toStdString()]=m_CaseHierarchy["build"][oldName.toStdString()];
		m_CaseHierarchy["build"].erase(oldName.toStdString());
		if (idx.parent()==QModelIndex()){
			std::cout << "There is no parent" << std::endl;
			m_CaseHierarchy["project"]["target_node"]=newName.toStdString();
		}else{
			QModelIndex parent=idx.parent();
			std::vector<std::string> v = m_CaseHierarchy["build"][parent.data().toString().toStdString()]["components"];
			std::vector<std::string> r;
			foreach(const std::string &str, v){
				if(QString(str.c_str())==oldName){
					r.push_back(newName.toStdString());
				}else{
					r.push_back(str);
				}
			}
			m_CaseHierarchy["build"][parent.data().toString().toStdString()]["components"]=r;
		}
	}	
	update();
	return result;
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
	m_CaseHierarchy["project"]["target_node"]=m_projectName.toStdString();

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
	json hbuild=obj;
	//std::cout << hbuild.dump() << std::endl;
	//std::cout << root->text().toStdString() << std::endl;
	// if(m_rootNode!=NULL){
	// 	delete m_rootNode;
	// }
	clear();
	std::string prj=hbuild["project"]["target_node"];
	m_projectName=QString(prj.c_str());
	m_rootNode=new QStandardItem(m_projectName);
	m_CaseHierarchy["project"]["target_node"]=m_projectName.toStdString();
	appendRow(m_rootNode);
	expandNode(m_rootNode,hbuild["build"]);
	m_currentItem=m_rootNode;
	m_currentTag=QString(m_projectName);

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

void CaseHierarchyModel::update(){
	m_projectName=m_rootNode->text();
	m_CaseHierarchy["project"]["target_node"]=m_projectName.toStdString();
}

QString CaseHierarchyModel::toString(){
	json obj=m_CaseHierarchy;
	std::string s=obj.dump(4);
	return QString(s.c_str());
}
// void CaseHierarchyModel::onItemChanged(const QModelIndex &item){
// 	std::cout << item.data().toString().toStdString() << std::endl;
// }
