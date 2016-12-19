require lairdtechpr59

epicsEnvSet("DEVICE",      "/dev/ttyUSB0")
epicsEnvSet("SERIAL_PORT", "LT59_SERIAL")
epicsEnvSet("PORT",        "LT59")

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
dbLoadRecords("lairdtechPR59_main.template",  "P=$(PORT):,R=,PORT=$(PORT),SERIAL_PORT=$(SERIAL_PORT),ADDR=0,TIMEOUT=1")
dbLoadRecords("lairdtechPR59_pid.template",   "P=$(PORT):,R=,PORT=$(PORT),ADDR=0,TIMEOUT=1")
dbLoadRecords("lairdtechPR59_temp.template",  "P=$(PORT):,R=,T=1,PORT=$(PORT),ADDR=0,TIMEOUT=1")
# TODO: Add support for other temperature sensors
dbLoadRecords("asynRecord.db","P=$(PORT):,R=asyn,PORT=$(PORT),ADDR=0,OMAX=100,IMAX=100")
