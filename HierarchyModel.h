#ifndef __HIERARCHICALMODEL_H_
#define __HIERARCHICALMODEL_H_

#include <QStandardItemModel>
#include <QStandardItem>
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

	QStringList getFileList(QString node); // get the list of files of a node
protected:
	void expandNode(QStandardItem*,json);	
	QStringList readCSV(QString filename);// load csv file
private:
	json m_CaseHierarchy;  // hierarchy json object 
	QStandardItem* m_rootNode; //root node ("target")
};
#endif
