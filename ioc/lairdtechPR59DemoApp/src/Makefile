TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

# The following gets rid of the -fno-implicit-templates flag on vxWorks, 
# so we get automatic template instantiation.
# This is what we want for miscellaneous/asynPortDriver.cpp
ifeq (vxWorks,$(findstring vxWorks, $(T_A)))
CODE_CXXFLAGS=
endif

PROD_NAME = lairdtechPR59DemoApp
PROD_IOC = $(PROD_NAME)

# <name>.dbd will be created from <name>Include.dbd
DBD += $(PROD_NAME).dbd
$(PROD_NAME)_DBD += lairdtechPR59Support.dbd
$(PROD_NAME)_DBD += base.dbd
$(PROD_NAME)_DBD += drvAsynSerialPort.dbd
$(PROD_NAME)_DBD += asyn.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
PROD_SRCS_DEFAULT += $(PROD_NAME)_registerRecordDeviceDriver.cpp $(PROD_NAME)Main.cpp
PROD_SRCS_vxWorks += $(PROD_NAME)_registerRecordDeviceDriver.cpp

PROD_LIBS += lairdtechPR59
PROD_LIBS += asyn

$(PROD_NAME)_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

