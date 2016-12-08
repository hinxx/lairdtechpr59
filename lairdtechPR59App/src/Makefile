TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += lairdtechPR59
PROD_NAME = lairdtechPR59

# install lairdtechPR59Support.dbd into <top>/dbd
DBD += lairdtechPR59Support.dbd
$(PROD_NAME)_DBD += lairdtechPR59SupportInclude.dbd

# specify all source files to be compiled and added to the library
$(PROD_NAME)_SRCS += lairdtechPR59.cpp

PROD_LIBS += asyn

$(PROD_NAME)_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

