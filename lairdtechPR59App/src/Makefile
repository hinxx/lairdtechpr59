TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += lairdtechPR59
PROD_NAME = lairdtechPR59

# install lairdtechPR59.dbd into <top>/dbd
DBD += lairdtechPR59Support.dbd
#$(PROD_NAME)_DBD += base.dbd
#$(PROD_NAME)_DBD += stream.dbd
#$(PROD_NAME)_DBD += drvAsynSerialPort.dbd
#$(PROD_NAME)_DBD += asyn.dbd

# specify all source files to be compiled and added to the library
#lairdtechPR59_SRCS += xxx
# No sources to compile here..

#PROD_LIBS += stream
#PROD_LIBS += asyn

$(PROD_NAME)_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

