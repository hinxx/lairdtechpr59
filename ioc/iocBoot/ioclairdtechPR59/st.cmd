#!../../bin/linux-x86_64/lairdtechPR59DemoApp

## You may have to change lairdtechPR59DemoApp to something else
## everywhere it appears in this file

< envPaths

epicsEnvSet("STREAM_PROTOCOL_PATH", "$(LAIRDTECHPR59)/db")

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/lairdtechPR59DemoApp.dbd"
lairdtechPR59DemoApp_registerRecordDeviceDriver pdbbase

#var streamDebug 1

epicsEnvSet("SERIAL_PORT", "/dev/ttyUSB0")

# drvAsynSerialPortConfigure(port, ttyName, priority, noAutoConnect, noProcessEosIn)
drvAsynSerialPortConfigure("PR59", "$(SERIAL_PORT)", 0, 0, 0)
asynSetOption("PR59", 0, "baud",   "115200")
asynSetOption("PR59", 0, "bits",   "8")
asynSetOption("PR59", 0, "parity", "none")
asynSetOption("PR59", 0, "stop",   "1")
asynSetOption("PR59", 0, "clocal", "Y")
asynSetOption("PR59", 0, "crtscts","N")

# handled by the Terminator = CR; in the proto file
#asynOctetSetInputEos( "PR59", 0, "\r")
#asynOctetSetOutputEos("PR59", 0, "\r")

asynSetTraceIOMask("PR59",0,0xff)
#asynSetTraceMask("PR59",0,0xff)

# Load record instances
dbLoadRecords("$(LAIRDTECHPR59)/db/lairdtechPR59.template","P=PR59:,R=,PORT=PR59")
dbLoadRecords("$(ASYN)/db/asynRecord.db","P=PR59:,R=asyn,PORT=PR59,ADDR=0,OMAX=100,IMAX=100")

cd "${TOP}/iocBoot/${IOC}"
iocInit
