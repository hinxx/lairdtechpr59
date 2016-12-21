#!../../bin/linux-x86_64/lairdtechPR59DemoApp

## You may have to change lairdtechPR59DemoApp to something else
## everywhere it appears in this file

< envPaths

epicsEnvSet("EPICS_CA_AUTO_ADDR_LIST", "NO")
epicsEnvSet("EPICS_CA_ADDR_LIST", "localhost")

epicsEnvSet("DEVICE",      "/dev/ttyUSB0")
epicsEnvSet("SERIAL_PORT", "LT59_SERIAL")
epicsEnvSet("PORT",        "LT59")

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/lairdtechPR59DemoApp.dbd"
lairdtechPR59DemoApp_registerRecordDeviceDriver pdbbase

# drvAsynSerialPortConfigure(port, ttyName, priority, noAutoConnect, noProcessEosIn)
drvAsynSerialPortConfigure("$(SERIAL_PORT)", "$(DEVICE)", 0, 0, 0)
asynSetOption("$(SERIAL_PORT)", 0, "baud",   "115200")
asynSetOption("$(SERIAL_PORT)", 0, "bits",   "8")
asynSetOption("$(SERIAL_PORT)", 0, "parity", "none")
asynSetOption("$(SERIAL_PORT)", 0, "stop",   "1")
asynSetOption("$(SERIAL_PORT)", 0, "clocal", "Y")
asynSetOption("$(SERIAL_PORT)", 0, "crtscts","N")

asynOctetSetInputEos("$(SERIAL_PORT)", 0, "\r\n")
asynOctetSetOutputEos("$(SERIAL_PORT)", 0, "\r")

#asynSetTraceIOMask("$(SERIAL_PORT)",0,0xff)
#asynSetTraceMask("$(SERIAL_PORT)",0,0xff)

# LTPR59Configure(const char *portName, const char *serialPort);
LTPR59Configure($(PORT), $(SERIAL_PORT))
#asynSetTraceIOMask("$(PORT)",0,0xff)
#asynSetTraceMask("$(PORT)",0,0xff)

# Load record instances
dbLoadRecords("$(LAIRDTECHPR59)/db/lairdtechPR59_main.template","P=$(PORT):,R=,PORT=$(PORT),SERIAL_PORT=$(SERIAL_PORT),ADDR=0,TIMEOUT=1")
dbLoadRecords("$(LAIRDTECHPR59)/db/lairdtechPR59_pid.template","P=$(PORT):,R=,PORT=$(PORT),ADDR=0,TIMEOUT=1")
dbLoadRecords("$(LAIRDTECHPR59)/db/lairdtechPR59_temp.template","P=$(PORT):,R=,T=1,PORT=$(PORT),ADDR=0,TIMEOUT=1")
dbLoadRecords("$(LAIRDTECHPR59)/db/lairdtechPR59_temp.template","P=$(PORT):,R=,T=4,PORT=$(PORT),ADDR=0,TIMEOUT=1")
# TODO: Add support for other temperature sensors
dbLoadRecords("$(ASYN)/db/asynRecord.db","P=$(PORT):,R=asyn,PORT=$(PORT),ADDR=0,OMAX=100,IMAX=100")

cd "${TOP}/iocBoot/${IOC}"
iocInit

# Temperature sensor 1 settings
# We have PT1000 sensor, only coefficients A, B and resistance high are used
# Values from LT_Interface tool for 'PT1000'
dbpf $(PORT):Temp1Mode "PT"
dbpf $(PORT):Temp1ResHigh 1000
dbpf $(PORT):Temp1ResMed 0
dbpf $(PORT):Temp1ResLow 0
dbpf $(PORT):Temp1CoeffA 3.90799996e-03
dbpf $(PORT):Temp1CoeffB -5.77499975e-07
dbpf $(PORT):Temp1CoeffC 0

# Temperature sensor 4 settings
# This is internal sensor, NTC type
# Values from LT_Interface tool for 'RH16-10K'
dbpf $(PORT):Temp4Mode "NTC"
dbpf $(PORT):Temp4ResHigh 2965.1389
dbpf $(PORT):Temp4ResMed 28836.7891
dbpf $(PORT):Temp4ResLow 78219.9922
dbpf $(PORT):Temp4CoeffA 6.84353872e-04
dbpf $(PORT):Temp4CoeffB 2.89854885e-04
dbpf $(PORT):Temp4CoeffC 4.39709385e-13

# Regulator settings
dbpf $(PORT):Mode "P"
dbpf $(PORT):ModeFlags "None"
dbpf $(PORT):FilterA "Off"
dbpf $(PORT):FilterB "Off"
dbpf $(PORT):StartStop "Stop"

# Get initial values
dbpf $(PORT):Retrieve 1
