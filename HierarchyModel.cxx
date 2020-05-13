

#include "HierarchyModel.h"
#include <QFile>
#include <QTextStream>
#include <iostream>
#include <QStringList>
#include <string>

CaseHierarchyModel::CaseHierarchyModel(){

}
CaseHierarchyModel::CaseHierarchyModel(QString filename){ // hbuild filename
	CaseHierarchyModel();
	
	// Todos
	initialize(filename);
}
CaseHierarchyModel::~CaseHierarchyModel(){
}


void CaseHierarchyModel::initialize(QString filename){
	//setHorizontalHeaderLabels(QStringList(["Nodes"]));
	loadFile(filename);
	generateEntries();
}
void CaseHierarchyModel::initialize(){

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
	std::string str=m_CaseHierarchy.dump();
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
		// std::cout << "End Node" << std::endl;
		// QStandardItem* c_node = new QStandardItem(p_node->text());
		// p_node->appendRow(c_node);
	}

}

void CaseHierarchyModel::generateEntries(json obj){
	json hbuild=obj["build"];
	//std::cout << hbuild.dump() << std::endl;
	QStandardItem *root=new QStandardItem(QString("target"));
	appendRow(root);
	//std::cout << root->text().toStdString() << std::endl;
	expandNode(root,hbuild);

}
void CaseHierarchyModel::generateEntries(){
	generateEntries(m_CaseHierarchy);
}

