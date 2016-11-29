require lairdtechpr59

epicsEnvSet("PREFIX",   "PR59")

#var streamDebug 1

epicsEnvSet("SERIAL_PORT", "/dev/ttyUSB0")

# drvAsynSerialPortConfigure(port, ttyName, priority, noAutoConnect, noProcessEosIn)
drvAsynSerialPortConfigure("$(PREFIX)", "$(SERIAL_PORT)", 0, 0, 0)
asynSetOption("$(PREFIX)", 0, "baud",   "115200")
asynSetOption("$(PREFIX)", 0, "bits",   "8")
asynSetOption("$(PREFIX)", 0, "parity", "none")
asynSetOption("$(PREFIX)", 0, "stop",   "1")
asynSetOption("$(PREFIX)", 0, "clocal", "Y")
asynSetOption("$(PREFIX)", 0, "crtscts","N")

# handled by the Terminator = CR; in the proto file
#asynOctetSetInputEos( "$(PREFIX)", 0, "\r")
#asynOctetSetOutputEos("$(PREFIX)", 0, "\r")

asynSetTraceIOMask("$(PREFIX)",0,0xff)
#asynSetTraceMask("$(PREFIX)",0,0xff)

# Load record instances
dbLoadRecords("lairdtechPR59.template","P=$(PREFIX):,R=,PORT=$(PREFIX)")
dbLoadRecords("asynRecord.db",         "P=$(PREFIX):,R=asyn,PORT=$(PREFIX),ADDR=0,OMAX=100,IMAX=100")
