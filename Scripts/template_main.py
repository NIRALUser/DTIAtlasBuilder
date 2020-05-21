#!/usr/bin/python
import os
import time
import sys
import json 
import argparse
import csv
import DTIAtlasBuilder_Preprocess
import DTIAtlasBuilder_AtlasBuilding

### load configutation json

def unique(list1): 
    unique_list = [] 
    for x in list1: 
        # check if exists in unique_list or not 
        if x not in unique_list: 
            unique_list.append(x) 
    return unique_list

def parse_hbuild(hb,root_path,root_node="target"): #hbuild parser to generate build sequence
    if root_node is None:
        root_node=hb['project']['target_node']
    root=hb['build'][root_node]
    seq=[]
    nodeFiles=[] ## sub node's final atlases
    # scalar=hb['config']['m_ScalarMeasurement']
    if root["type"]=="node":    
        for c in root["components"]:
            seq+=parse_hbuild(hb, root_path=root_path, root_node=c)
            nodeAtlasPath=os.path.join(root_path,"atlases/"+c+"/5_Final_Atlas/FinalAtlasDTI.nrrd")
            nodeFiles.append(nodeAtlasPath)
    elif root["type"]=="end_node":
        if root["filetype"]=="dataset":
            rows=[]
            rows_id=[]
            with open(str(root['datasetfiles']),'r') as f:
                csvreader=csv.reader(f)
                next(csvreader,None)
                for r in csvreader:
                    fpath=str(r[1])
                    fid=os.path.splitext(os.path.basename(fpath))[0]
                    rows.append(fpath)
                    rows_id.append(str(fid))

            return  [{"name" : str(root_node),
                "dataset_files" : rows,
                "dataset_ids" : rows_id,
                "project_path" : str(os.path.join(root_path,"atlases/"+root_node))
                }]
        else:
            flist=list(map(str,root["datasetfiles"]))
            fids=[]
            for e in flist:
                fid=os.path.splitext(os.path.basename(e))[0]
                fids.append(fid)

            return [{"name" : str(root_node),
                    "dataset_files" : flist,
                    "dataset_ids" : fids ,
                    "project_path" : str(os.path.join(root_path,"atlases/"+root_node))
                    }]

    # node type file reading

    seq+=[{"name" : str(root_node),
            "dataset_files" : list(map(str,nodeFiles)),
            "dataset_ids" : list(map(str,root["components"])),
            "project_path" : str(os.path.join(root_path,"atlases/"+root_node))

         }]
    seq=unique(seq)

    ## generate final buildsequence furnished with configuration


    return seq

def furnish_sequence(hb,seq):
    bs=[]
    for s in seq:
        conf=hb["config"].copy()
        conf["m_OutputPath"]=s['project_path']
        conf["m_CasesPath"]=s['dataset_files']
        conf["m_CasesIDs"]=s['dataset_ids']
        bs.append(conf)

    return bs

def generate_directories(project_path,sequence): ## from build sequence, generate directories
    atlasesPath=os.path.join(project_path,"atlases")
    finalAtlasPath=os.path.join(project_path,"final_atlas")
    if not os.path.isdir(atlasesPath):
      print("\n=> Creation of the atlas directory = " + atlasesPath)
      os.mkdir(atlasesPath)
    if not os.path.isdir(finalAtlasPath):
      print("\n=> Creation of the atlas directory = " + finalAtlasPath)
      os.mkdir(finalAtlasPath)
    for s in sequence:
        apath=os.path.join(s["m_OutputPath"])
        if not os.path.isdir(apath):
          print("\n=> Creation of the atlas directory = " + apath)
          os.mkdir(apath)
    print("Initial directories are generated")

def main(args):
    projectPath=os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)),"../"))
    scriptPath=os.path.join(projectPath,"scripts")
    commonPath=os.path.join(projectPath,'common')
    configPath=os.path.join(commonPath,"config.json")
    hbuildPath=os.path.join(commonPath,"h-build.json")
    
    ### generate build sequence
    buildSequence=[]
    if args.buildsequence is None:
        hbuild={}
        with open(hbuildPath,'r') as f:
            hbuild=json.load(f)
        config={}
        with open(configPath,'r') as f:
            config=json.load(f)

        hbuild["config"]=config
        hbuild['config']['m_GreedyAtlasParametersTemplatePath']=str(os.path.join(commonPath,'GreedyAtlasParameters.xml'))
        initSequence=parse_hbuild(hbuild,root_path=projectPath,root_node=args.node)
        buildSequence=furnish_sequence(hbuild,initSequence)

        # for s in buildSequence:
        #   print(s)
        #save sequence 
        with open(os.path.join(commonPath,'build_sequence.json'),'w') as f:
            json.dump(buildSequence,f,indent=4,sort_keys=True)

        # generate scaffolding directories 
        generate_directories(projectPath,buildSequence)
    else:
        with open(args.buildsequence,'r') as f:
            buildSequence=json.load(f)

    ### atlas build begins (to be multiprocessed)
    print("\nThe current date and time are:")
    print( time.strftime('%x %X %Z') )
    print("\n=============== Main Script ================")
    time1=time.time()
    for cfg in buildSequence:
        try:
            DTIAtlasBuilder_Preprocess.run(cfg)
        except Exception as e:
            raise Exception("Error occurred in DTIAtlasBuilder_Preprocess : " + str(e))
            

        try:
            DTIAtlasBuilder_AtlasBuilding.run(cfg)
        except Exception as e:
            raise Exception("Error occurred in DTIAtlasBuilding_DTIAtlasBuilder" + str(e))
    ### atlas build ends
    # Display execution time
    time2=time.time()
    timeTot=time2-time1
    if timeTot<60 : print("| Execution time = " + str(int(timeTot)) + "s")
    elif timeTot<3600 : print("| Execution time = " + str(int(timeTot)) + "s = " + str(int(timeTot/60)) + "m " + str( int(timeTot) - (int(timeTot/60)*60) ) + "s")
    else : print("| Execution time = " + str(int(timeTot)) + "s = " + str(int(timeTot/3600)) + "h " + str( int( (int(timeTot) - int(timeTot/3600)*3600) /60) ) + "m " + str( int(timeTot) - (int(timeTot/60)*60) ) + "s")


    

if __name__=="__main__":
    parser=argparse.ArgumentParser(description="Argument Parser")
    parser.add_argument('--node',help="node to build",type=str)
    parser.add_argument('--buildsequence',help='build sequence file, if this option is inputted then build sequence process will be skipped',type=str)
    args=parser.parse_args()


    try:
       main(args)
       sys.exit(0)
    except Exception as e:
        print(str(e))
        sys.exit(1)




