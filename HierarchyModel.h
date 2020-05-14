#ifndef __HIERARCHICALMODEL_H_
#define __HIERARCHICALMODEL_H_

#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include <QString>
#include "nlohmann/json.hpp"

//
// treemodel for hierarchical build
//
using json = nlohmann::json;

class CaseHierarchyModel : public QStandardItemModel{


public:
	CaseHierarchyModel();
	CaseHierarchyModel(QString filename);
	~CaseHierarchyModel();

	void initialize();
	void initialize(QString filename);
	void loadFile(QString filename); // load json file to store json object 
	void saveFile(QString filename); // save caseHiearchy to file


	void generateEntries(json ha); // build entries from hierarchy json obj
	void generateEntries(); // regenerate from cseHierarchy

	void addNode(QString); // add node to current node (m_currentItem)

	void removeNode(QStandardItem* ); // remove node
	void removeCurrentNode(); // remove current Node

	void removeNodeRecursivelyInJson(QString node); // remove a node and its child
	
	bool checkNodename(QString); //check node name exists in the build file (1 : exists, 0 : not existing) 

	QStringList getFileList(QString node); // get the list of files of a node
	QStandardItem* getRoot(){return m_rootNode;};
	void setCurrentTag(QModelIndex i);  
	QString getCurrentTag(){return m_currentTag;};
	QString getCurrentType();
	QStandardItem* getCurrentItem(){return m_currentItem;};

	void setFiles(QString nodename, QStringList ql);

protected:
	void expandNode(QStandardItem*,json);	
	QStringList readCSV(QString filename);// load csv file
private:
	json m_CaseHierarchy;  // hierarchy json object 
	QStandardItem* m_rootNode; //root node ("target")
	QString m_currentTag; // current tag of a node
	QStandardItem* m_currentItem; //current selected node's item
};
#endif
