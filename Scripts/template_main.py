#!/usr/bin/python

import os
import time
import sys
import json 


### load configutation json

configPath=os.path.join(os.path.dirname(os.path.abspath(__file__)),"config.json")
config={}
with open(configPath,'r') as f:
	config=json.load(f)
print(json.dumps(config,indent=4))

print("\nThe current date and time are:")
print( time.strftime('%x %X %Z') )

exit(1)

print("\n=============== Main Script ================")

OutputPath= config['m_OutputPath']+"/DTIAtlas"

def DisplayErrorAndQuit ( Error ):
  print '\n\nERROR DETECTED IN WORKFLOW:',Error
  print 'ABORT'
  sys.exit(1)

if not config["m_useGridProcess"]:
	os.putenv("ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS",str(config['m_NbThreadsString'])) # to set the nb of cores to use : propagated to the other scripts via os.system()

PIDlogFile = OutputPath + "/Script/PID.log"
if os.path.isfile( PIDlogFile ) : os.remove( PIDlogFile )

if os.path.isfile( OutputPath + "/Script/DTIAtlasBuilder_Main.script" ) : os.remove( OutputPath + "/Script/DTIAtlasBuilder_Main.script" )
if os.path.isfile( OutputPath + "/Script/DTIAtlasBuilder_Preprocess.script" ) : os.remove( OutputPath + "/Script/DTIAtlasBuilder_Preprocess.script" )
if os.path.isfile( OutputPath + "/Script/DTIAtlasBuilder_AtlasBuilding.script" ) : os.remove( OutputPath + "/Script/DTIAtlasBuilder_AtlasBuilding.script" )

time1=time.time()

# Call the Preprocess script
PrePScriptCommand= config['m_PythonPath']+ " " + OutputPath+ "/Script/DTIAtlasBuilder_Preprocess.py"
print("\n=> $ " + PrePScriptCommand)
if os.system(PrePScriptCommand)!=0 : DisplayErrorAndQuit('=> Errors detected in preprocessing')

# Call the Atlas Building script
AtlasBuildingCommand= config['m_PythonPath']+ " " + OutputPath + "/Script/DTIAtlasBuilder_AtlasBuilding.py"
print("\n=> $ " + AtlasBuildingCommand)
if os.system(AtlasBuildingCommand)!=0 : DisplayErrorAndQuit('=> Errors detected in atlas building')

print("\n============ End of execution =============\n")

# Display execution time
time2=time.time()
timeTot=time2-time1
if timeTot<60 : print("| Execution time = " + str(int(timeTot)) + "s")
elif timeTot<3600 : print("| Execution time = " + str(int(timeTot)) + "s = " + str(int(timeTot/60)) + "m " + str( int(timeTot) - (int(timeTot/60)*60) ) + "s")
else : print("| Execution time = " + str(int(timeTot)) + "s = " + str(int(timeTot/3600)) + "h " + str( int( (int(timeTot) - int(timeTot/3600)*3600) /60) ) + "m " + str( int(timeTot) - (int(timeTot/60)*60) ) + "s")

if os.path.isfile( PIDlogFile ) : os.remove( PIDlogFile )

sys.exit(0)

