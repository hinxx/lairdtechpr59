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

#asynOctetSetInputEos("$(SERIAL_PORT)", 0, "> ")
asynOctetSetInputEos("$(SERIAL_PORT)", 0, "\r\n")
asynOctetSetOutputEos("$(SERIAL_PORT)", 0, "\r")

#asynSetTraceIOMask("$(SERIAL_PORT)",0,0xff)
#asynSetTraceMask("$(SERIAL_PORT)",0,0xff)

# LTPR59Configure(const char *portName, const char *serialPort);
LTPR59Configure($(PORT), $(SERIAL_PORT))
asynSetTraceIOMask("$(PORT)",0,0xff)
asynSetTraceMask("$(PORT)",0,0xff)

# Load record instances
dbLoadRecords("$(LAIRDTECHPR59)/db/lairdtechPR59_main.template","P=$(PORT):,R=,PORT=$(PORT),SERIAL_PORT=$(SERIAL_PORT),ADDR=0,TIMEOUT=1")
#dbLoadRecords("$(LAIRDTECHPR59)/db/lairdtechPR59_pid.template","P=$(PORT):,R=,PORT=$(PORT)")
#dbLoadRecords("$(LAIRDTECHPR59)/db/lairdtechPR59_temp.template","P=$(PORT):,R=,PORT=$(PORT)")
dbLoadRecords("$(ASYN)/db/asynRecord.db","P=$(PORT):,R=asyn,PORT=$(PORT),ADDR=0,OMAX=100,IMAX=100")

cd "${TOP}/iocBoot/${IOC}"
iocInit
