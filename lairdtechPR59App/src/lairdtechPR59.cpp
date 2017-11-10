/*
 * lairdtechPR59.cpp
 *
 *  Created on: Dec 6, 2016
 *      Author: hinkokocevar
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>

#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsExit.h>
#include <epicsExport.h>
#include <iocsh.h>

#include "lairdtechPR59.h"

static const char *driverName = "LTPR59";


static void taskC(void *drvPvt) {
	LTPR59 *pPvt = (LTPR59 *)drvPvt;
	pPvt->dataTask();
}

static void exitHandler(void *drvPvt) {
	LTPR59 *pPvt = (LTPR59 *)drvPvt;
	delete pPvt;
}


#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS 8
#endif
void LTPR59::hexdump(void *mem, unsigned int len) {
	unsigned int i, j;

	for (i = 0; i < len + ((len % HEXDUMP_COLS) ?
							(HEXDUMP_COLS - len % HEXDUMP_COLS) : 0); i++) {
		/* print offset */
		if (i % HEXDUMP_COLS == 0) {
			printf("0x%06x: ", i);
		}

		/* print hex data */
		if (i < len) {
			printf("%02x ", 0xFF & ((char*) mem)[i]);
		} else {
			/* end of block, just aligning for ASCII dump */
			printf("   ");
		}

		/* print ASCII dump */
		if (i % HEXDUMP_COLS == (HEXDUMP_COLS - 1)) {
			for (j = i - (HEXDUMP_COLS - 1); j <= i; j++) {
				if (j >= len) {
					/* end of block, not really printing */
					putchar(' ');
				} else if (isprint(((char*) mem)[j])) {
					/* printable char */
					putchar(0xFF & ((char*) mem)[j]);
				} else {
					/* other char */
					putchar('.');
				}
			}
			putchar('\n');
		}
	}
}

asynStatus LTPR59::serialPortWriteRead(double timeout) {
	int eomReason;
	asynStatus status;

//	printf("%s: request (%ld bytes):\n", __func__, mReqSz);
//	hexdump(mReq, mReqSz);

	status = pasynOctetSyncIO->writeRead(mAsynUserCommand,
			mReq, mReqSz, mResp, mRespSz,
			timeout, &mReqActSz, &mRespActSz, &eomReason);

//	printf("%s: response (%ld bytes):\n", __func__, mRespActSz);
//	hexdump(mResp, mRespActSz);

	if (status) {
		asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
				"%s:%s, status=%d, eomReason=%d\n",
				driverName, __func__, status, eomReason);
	} else {
		asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
			  "%s:%s: status=%d, eomReason=%d\n",
			  driverName, __func__, status, eomReason);
	}

	return status;
}

asynStatus LTPR59::serialPortWrite(double timeout) {
	asynStatus status;

//	printf("%s: request (%ld bytes):\n", __func__, mReqSz);
//	hexdump(mReq, mReqSz);

	status = pasynOctetSyncIO->write(mAsynUserCommand,
			mReq, mReqSz,
			timeout, &mReqActSz);

//	printf("%s: request actual size %ld bytes\n", __func__, mReqActSz);
	if (status) {
		asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
				"%s:%s, status=%d\n",
				driverName, __func__, status);
	} else {
		asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
			  "%s:%s: status=%d\n",
			  driverName, __func__, status);
	}

	return status;
}

asynStatus LTPR59::serialPortRead(double timeout) {
	int eomReason;
	asynStatus status;

	status = pasynOctetSyncIO->read(mAsynUserCommand,
			mResp, mRespSz,
			timeout, &mRespActSz, &eomReason);


//	printf("%s: response (%ld bytes):\n", __func__, mRespActSz);
//	hexdump(mResp, mRespActSz);

	if (status) {
		asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
				"%s:%s, status=%d\n",
				driverName, __func__, status);
	} else {
		asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
			  "%s:%s: status=%d, eomReason=%d\n",
			  driverName, __func__, status, eomReason);
	}

	return status;
}

void LTPR59::updateStatus(const char *msg) {
    memcpy(mStatusMsg, msg, strlen(msg));
//	printf("Status message: '%s'\n", mStatusMsg);
	setStringParam(LTStatusMessage, mStatusMsg);

	/* Do callbacks so higher layers see any changes */
	callParamCallbacks(0);
}

asynStatus LTPR59::convToString(epicsInt32 val, char *buf, unsigned int *len) {
	if (buf == NULL) {
		return asynError;
	}
	if (len == NULL) {
		return asynError;
	}
	if (*len > LTPR59_MAX_MSG_SZ) {
		return asynError;
	}
	*len = snprintf(buf, *len, "%d", val);
	return asynSuccess;
}

asynStatus LTPR59::convToString(epicsFloat64 val, char *buf, unsigned int *len) {
	if (buf == NULL) {
		return asynError;
	}
	if (len == NULL) {
		return asynError;
	}
	if (*len > LTPR59_MAX_MSG_SZ) {
		return asynError;
	}
	*len = snprintf(buf, *len, "%f", val);
	return asynSuccess;
}

asynStatus LTPR59::convFromString(epicsInt32 *val) {
	int v;
	int n;

	n = sscanf(mResp, "%d", &v);
	if (n != 1) {
		updateStatus("Failed to parse value");
	    return asynError;
	}

	*val = v;

	return asynSuccess;
}

asynStatus LTPR59::convFromString(epicsFloat64 *val) {
	float v;
	int n;

	n = sscanf(mResp, "%f", &v);
	if (n != 1) {
		updateStatus("Failed to parse value");
	    return asynError;
	}

	*val = v;

	return asynSuccess;
}

asynStatus LTPR59::xfer(unsigned int reqType, const char *cmd,
		unsigned int reg, const char *data, bool readData, double timeout) {
	asynStatus status = asynSuccess;
	unsigned int l, len;
	char buf[LTPR59_MAX_MSG_SZ] = {0};

	memset(mReq, 0, LTPR59_MAX_MSG_SZ);
	memset(mResp, 0, LTPR59_MAX_MSG_SZ);
	mReqActSz = 0;
	mRespActSz = 0;
	mReqSz = 0;
	mRespSz = 0;

	len = 0;
	l = strlen(cmd);
	len += l;
	memcpy(buf, cmd, l);

	if (reqType == LTPR59_REQ_TYPE_CMD) {
		// nothing to do here ..
	} else if (reqType == LTPR59_REQ_TYPE_READ) {
		l = snprintf(&buf[len], 5, "%d", reg);
		len += l;
		buf[len] = '?';
		len++;
	} else if (reqType == LTPR59_REQ_TYPE_WRITE) {
		if (data == NULL) {
			return asynError;
		}

		l = snprintf(&buf[len], 5, "%d", reg);
		len += l;
		buf[len] = '=';
		len++;
		l = strlen(data);
		memcpy(&buf[len], data, l);
		len += l;
	} else {
		return asynError;
	}

	if (len > LTPR59_MAX_MSG_SZ) {
		return asynError;
	}
	if (len == 0) {
		return asynError;
	}

	memcpy(mReq, buf, len);
	mReqActSz = 0;
	mRespActSz = 0;
	mReqSz = len;
	mRespSz = LTPR59_MAX_MSG_SZ;

	/* Send request and read back the echoed request line. */
	status = serialPortWriteRead(timeout);
	if (status) {
		return status;
	}
	if (readData) {
		/* Read the actual response. */
		status = serialPortRead(timeout);
		if (status) {
			return status;
		}
	}

	if (mRespActSz == 0) {
	    updateStatus("No response");
		return asynError;
	}
	if (mResp[0] == '?') {
	    updateStatus("Unknown command");
	    return asynError;
	}

	return status;
}

asynStatus LTPR59::readDataFloat(unsigned int reg, epicsFloat64 *val) {
	asynStatus status = asynSuccess;

	status = xfer(LTPR59_REQ_TYPE_READ, "$R", reg, NULL);
	if (status) {
		return status;
	}
	status = convFromString(val);

	return status;
}

asynStatus LTPR59::readDataInt(unsigned int reg, epicsInt32 *val) {
	asynStatus status = asynSuccess;

	status = xfer(LTPR59_REQ_TYPE_READ, "$R", reg, NULL);
	if (status) {
		return status;
	}
	status = convFromString(val);

	return status;
}

asynStatus LTPR59::readString(const char *cmd, char *val, unsigned int *len) {
	asynStatus status = asynSuccess;

	status = xfer(LTPR59_REQ_TYPE_CMD, cmd, 0, NULL);
	if (status) {
		return status;
	}

	if (val && len) {
		memcpy(val, mResp, mRespActSz);
		*len = mRespActSz;
	}

	return status;
}

asynStatus LTPR59::writeDataFloat(unsigned int reg, const epicsFloat64 val) {
	asynStatus status = asynSuccess;
	char buf[LTPR59_MAX_MSG_SZ] = {0};
	unsigned int len = LTPR59_MAX_MSG_SZ;

	status = convToString(val, buf, &len);
	if (status) {
		return status;
	}
	/* no additional data is read back apart from echoed request */
	status = xfer(LTPR59_REQ_TYPE_WRITE, "$R", reg, buf, false);

	return status;
}

asynStatus LTPR59::writeDataInt(unsigned int reg, const epicsInt32 val) {
	asynStatus status = asynSuccess;
	char buf[LTPR59_MAX_MSG_SZ] = {0};
	unsigned int len = LTPR59_MAX_MSG_SZ;

	status = convToString(val, buf, &len);
	if (status) {
		return status;
	}
	/* no additional data is read back apart from echoed request */
	status = xfer(LTPR59_REQ_TYPE_WRITE, "$R", reg, buf, false);

	return status;
}

/**
 * Do data continuous readout.
 */
void LTPR59::dataTask(void) {
	asynStatus status = asynSuccess;
	epicsInt32 contLog = 0;
	int n;
	int iv[20];
	float fv[20];
	char buf[LTPR59_MAX_MSG_SZ];

	sleep(2);

	printf("%s:%s: Data thread started...\n", driverName, __func__);

	this->lock();

	while (1) {

		// Wait for event from main thread to signal that data acquisition has started.
		this->unlock();
		epicsEventWait(mDataEvent);
		asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
				"%s:%s:, got data event\n", driverName, __func__);

		if (this->mFinish) {
			asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
					"%s:%s: Stopping thread!\n", driverName, __func__);
			break;
		}
		this->lock();

		// Sanity check that main thread thinks we are acquiring data
		while (mAcquiringData) {
			this->unlock();
			status = serialPortRead(0.1);
			this->lock();

			if (status) {
				break;
			}

			if (mResp[0] != '[') {
				printf("%s: No '[' in response..\n", __func__);
				continue;
			}

			getIntegerParam(LTLoggingMode, &contLog);
			if (contLog == 0) {
				break;
			} else if (contLog == 1) {
				/* $A1
				 * View Ain Vin F2c T1 T2 T3 Tfet Curr V12 F1c a12 a13 iTc iF1 iF2
				 * [1 0 311 605 0 1023 1022 1022 691 127 597 0 0 0 0 1023 1023]
				 *
				 * XXX: 16 labels, 17 values; doc 1.6c has only 13 listed fields?!
				 */
				n = sscanf(mResp, "[%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]",
					&iv[0], &iv[1], &iv[2],  &iv[3],  &iv[4],  &iv[5],  &iv[6],  &iv[7],
					&iv[8], &iv[9], &iv[10], &iv[11], &iv[12], &iv[13], &iv[14], &iv[15], &iv[16]);
//				printf("%s: Parsed %d values..\n", __func__, n);

			} else if (contLog == 2) {
				/* $A2
				 * View ERR Mode Temp1 TcOut F1out F2out
				 * [2 100AC001 00 1023 0.0000 0.0000 0.0000]
				 */
				n = sscanf(mResp, "[%d %X %X %d %f %f %f]",
					&iv[0], &iv[1], &iv[2], &iv[3], &fv[0], &fv[1], &fv[2]);
//				printf("%s: Parsed %d values..\n", __func__, n);

			} else if (contLog == 3) {
				/* $A3
				 * View ERR Mode Tc Ta1 Ta2 Tr Ta Tp Ti Td TLP_A TLP_B
				 * [3 100AC001 00 0.0000 -999.9000 -999.9000 20.0000 -999.9000 0.0000 0.0000 0.0000 20.0000 36.6650]
				 */
				n = sscanf(mResp, "[%d %X %X %f %f %f %f %f %f %f %f %f %f]",
					&iv[0], &iv[1], &iv[2], &fv[0], &fv[1], &fv[2], &fv[3], &fv[4],
					&fv[5], &fv[6], &fv[7], &fv[8], &fv[9]);
//				printf("%s: Parsed %d values..\n", __func__, n);
				if (n == 13) {
					sprintf(buf, "%04X %04X", (iv[1] >> 16) & 0xFFFF, (iv[1] & 0xFFFF));
					setStringParam(LTStatus, buf);
//					printf("%s::%s: alarms 0x%X, errors 0x%X\n", driverName, __func__, (iv[1] >> 16) & 0xFFFF, (iv[1] & 0xFFFF));
					setUIntDigitalParam(LTStatusAlarm, iv[1] >> 16, 0xFFFF);
					setUIntDigitalParam(LTStatusError, iv[1], 0xFFFF);
					setDoubleParam(LTTcOutput, fv[0]);
					setDoubleParam(LTTemp1, fv[1]);
					// skip temp2
					// skip set point
					setDoubleParam(LTPIDTa, fv[4]);
					setDoubleParam(LTPIDTp, fv[5]);
					setDoubleParam(LTPIDTi, fv[6]);
					setDoubleParam(LTPIDTd, fv[7]);
					setDoubleParam(LTPIDTLPa, fv[8]);
					setDoubleParam(LTPIDTLPb, fv[9]);
					/* Do callbacks so higher layers see any changes */
					callParamCallbacks();
				}

			} else if (contLog == 4) {
				/* $A4
				 * View ERR Mode Tc Tr Curr
				 * [4 100AC001 00 0.0000 20.0000 127]
				 */
				n = sscanf(mResp, "[%d %X %X %f %f %d]",
					&iv[0], &iv[1], &iv[2], &fv[0], &fv[1], &iv[3]);
//				printf("%s: Parsed %d values..\n", __func__, n);

			} else if (contLog == 5) {
				/* $A5
				 * View ERR Mode TrExt Tr Ta
				 * [5 100AC001 00 0.0000 20.0000 -999.9000]
				 */
				n = sscanf(mResp, "[%d %X %X %f %f %f]",
					&iv[0], &iv[1], &iv[2], &fv[0], &fv[1], &fv[2]);
//				printf("%s: Parsed %d values..\n", __func__, n);

			} else if (contLog == 6) {
				/* $A6
				 * View RUNTIMEDATA
				 * [6 91E4 100AC001 00 00 0.0000 0.0000 0.0000 -999.9000 -999.9000 -999.9000 67.7612 0.0000 20.0000 -999.9000 36.6650 0.0000 0.0000 0.0000 20.0000 20.0377 0.0000 -12.1233]
				 *
				 * Partial decoded values from comparing to other $Ax outputs and registers:
				 * 0 - mode
				 * 1 - event counter in hex
				 * 2 - status alarms and errors in hex
				 * 3 - regulator mode in hex (?)
				 * 4 - ? in hex (?)
				 * 5 - Tc $R106
				 * 6 - ?
				 * 7 - ?
				 * 8 - Temp 1 $R55
				 * 9 - Temp 2 $R56
				 * 10 - Temp 3 $R57
				 * 11 - Temp 4 $R58
				 * 12 - ?
				 * 13 - TRef $R105
				 * 14 - Ta $R110
				 * 15 - Te $R111
				 * 16 - Tp $R112
				 * 17 - Ti $R113
				 * 18 - Td $R114
				 * 19 - TLP_A $R117
				 * 20 - TLP_B $R118
				 * 21 - ?
				 * 22 - ?
				 *
				 * XXX: To be decoded fully.. contact company?
				 */
				n = sscanf(mResp, "[%d %X %X %X %X %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f]",
					&iv[0], &iv[1], &iv[2], &iv[3], &iv[4],
					&fv[0], &fv[1], &fv[2], &fv[3], &fv[4], &fv[5], &fv[6], &fv[7],
					&fv[8], &fv[9], &fv[10], &fv[11], &fv[12], &fv[13], &fv[14],
					&fv[15], &fv[16], &fv[17]);
//				printf("%s: Parsed %d values..\n", __func__, n);
				if (n == 23) {
					setIntegerParam(LTEventCounter, iv[1]);
					sprintf(buf, "%04X %04X", (iv[2] >> 16) & 0xFFFF, (iv[2] & 0xFFFF));
					setStringParam(LTStatus, buf);
//					printf("%s::%s: alarms 0x%X, errors 0x%X\n", driverName, __func__, (iv[2] >> 16) & 0xFFFF, (iv[2] & 0xFFFF));
					setUIntDigitalParam(LTStatusAlarm, iv[2] >> 16, 0xFFFF);
					setUIntDigitalParam(LTStatusError, iv[2], 0xFFFF);
//					printf("%s::%s: regulator mode %d\n", driverName, __func__, iv[3]);
					setDoubleParam(LTTcOutput, fv[0]);
					setDoubleParam(LTTemp1, fv[3]);
					// skip temp2
					// skip temp3
					setDoubleParam(LTTemp4, fv[6]);
					setDoubleParam(LTTRef, fv[8]);
					setDoubleParam(LTPIDTa, fv[9]);
					setDoubleParam(LTPIDTe, fv[10]);
					setDoubleParam(LTPIDTp, fv[11]);
					setDoubleParam(LTPIDTi, fv[12]);
					setDoubleParam(LTPIDTd, fv[13]);
					setDoubleParam(LTPIDTLPa, fv[14]);
					setDoubleParam(LTPIDTLPb, fv[15]);
					/* Do callbacks so higher layers see any changes */
					callParamCallbacks();
				}

			} else if (contLog == 7) {
				/* $A7
				 * View NEW RUNTIMEDATA
				 * [7 9A1F 100AC001 00 00 00000000 00000000 00000000 C479F99A C479F99A C479F99A 428785C8 00000000 41A00000 C479F99A 4212A8F6 00000000 00000000 00000000 41A00000 41A04D46 00000000 00000000 00000000 03FF 0000]
				 *
				 * XXX: To be decoded.. contact company?
				 */

			} else {
				printf("%s: Unsupported logging mode %d\n", __func__, contLog);
			}
		}

		// Now clear main thread flag
		mAcquiringData = 0;

		status = pasynOctetSyncIO->flush(mAsynUserCommand);
		status = serialPortRead(0.1);

	} // End of while(1) loop

	printf("Data thread is down!\n");
}

asynStatus LTPR59::writeInt32(asynUser *pasynUser, epicsInt32 value) {

	int function = pasynUser->reason;
	int addr = 0;
	asynStatus status = asynSuccess;

	status = getAddress(pasynUser, &addr);
	if (status != asynSuccess) {
		return(status);
	}

//	printf("%s: function %d, addr %d, value %d\n", __func__, function, addr, value);
	status = setIntegerParam(addr, function, value);

	if (function == LTLoggingMode) {
		status = controlLogging((bool)value);
	} else if (function == LTStatusClear) {
		status = readString("$SC", NULL, NULL);
	} else if (function == LTStartStop) {
		if (value) {
			status = readString("$W", NULL, NULL);
		} else {
			status = readString("$Q", NULL, NULL);
		}
	} else if (function == LTClearEEPROM) {
		status = readString("$RC", NULL, NULL);
	} else if (function == LTWriteEEPROM) {
		status = readString("$RW", NULL, NULL);
	} else if (function == LTRetrieve) {
		status = readAllRegisterParams();
	} else if (function == LTSend) {
		status = writeAllRegisterParams();
	}

	/* Do callbacks so higher layers see any changes */
	callParamCallbacks(addr, addr);

	if (status) {
		asynPrint(pasynUser, ASYN_TRACE_ERROR,
			  "%s:%s: error, status=%d function=%d, addr=%d, value=%d\n",
			  driverName, __func__, status, function, addr, value);
	} else {
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
			  "%s:%s: function=%d, addr=%d, value=%d\n",
			  driverName, __func__, function, addr, value);
	}

	return status;
}

asynStatus LTPR59::controlLogging(const bool active) {
	epicsInt32 contLog = 0;
	char cmd[4] = {'$', 'A', 0, 0};
	asynStatus status = asynSuccess;

	/* if active then get the logging mode value and send it to the regulator,
	 * otherwise logging will be disabled.. */
	if (active) {
		getIntegerParam(LTLoggingMode, &contLog);
		cmd[2] = '0' + contLog;
	}

	status = xfer(LTPR59_REQ_TYPE_CMD, cmd, 0, NULL, false);
	if (contLog > 0) {
		mAcquiringData = 1;
	} else {
		mAcquiringData = 0;
	}

	if (mAcquiringData) {
		asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
				"%s:%s:, Sending mDataEvent to dataTask ...\n", driverName,
				__func__);
		printf("%s:%s:, Sending mDataEvent to dataTask ...\n", driverName,
				__func__);
		// Also signal the data readout thread
		epicsEventSignal(mDataEvent);
	}

	/* logging needs to start / finish before we can access the regulator
	 * with other commands */
	usleep(300000);

	return status;
}

asynStatus LTPR59::createRegisterParam(const int num, const int mask,
		const char *name, asynParamType type, int *index) {
	struct LTPR59Register *reg;
	asynStatus status = asynSuccess;

	if (mRegsIndex >= LTPR59_MAX_REGISTERS) {
		return asynError;
	}

	reg = &mRegs[mRegsIndex];

	status = createParam(name, type, index);
	if (status) {
		return status;
	}
	reg->param = *index;
	reg->num = num;
	reg->mask = mask;
	reg->type = type;
	/* registers 99 and above are read-only, with register 9 being exception */
	if ((reg->num < 99) && (reg->num != 9)) {
		reg->write = 1;
	} else {
		reg->write = 0;
	}

	mRegsIndex++;

	return asynSuccess;
}

asynStatus LTPR59::findRegister(const int num, struct LTPR59Register **reg) {
	struct LTPR59Register *r;
	int i;

	for (i = 0; i < mRegsIndex; i++) {
		r = &mRegs[i];
		if (r->num == num) {
			*reg = &mRegs[i];
			return asynSuccess;
		}
	}

	return asynError;
}

asynStatus LTPR59::readAllRegisterParams(void) {
	struct LTPR59Register *reg;
	int i;
	unsigned int len, n;
	char value[LTPR59_MAX_MSG_SZ];
	unsigned int alarms, errors;
	asynStatus status = asynSuccess;

	status = controlLogging(false);
	if (status) {
		return asynError;
	}

	len = LTPR59_MAX_MSG_SZ;
	status = readString("$LI", value, &len);
	value[len] = 0;
	status = setStringParam(LTID, value);

	len = LTPR59_MAX_MSG_SZ;
	status = readString("$v", value, &len);
	value[len] = 0;
	status = setStringParam(LTVersion, value);

	len = LTPR59_MAX_MSG_SZ;
	status = readString("$S", value, &len);
	value[len] = 0;

	n = sscanf(value, "%X %X %*s", &alarms, &errors);
	if (n != 2) {
		status = asynError;
	} else {
//		printf("%s::%s: alarms 0x%X, errors 0x%X\n", driverName, __func__, alarms, errors);
		sprintf(value, "%04X %04X", alarms, errors);
		status = setStringParam(LTStatus, value);
		status = setUIntDigitalParam(LTStatusAlarm, alarms, 0xFFFF);
		status = setUIntDigitalParam(LTStatusError, errors, 0xFFFF);
	}

	for (i = 0; i < mRegsIndex; i++) {
		reg = &mRegs[i];
		/* this PV does not have a register to be read */
		if (reg->num == -1) {
			continue;
		}

		if (reg->type == asynParamInt32) {
			epicsInt32 value;
			status = readDataInt(reg->num, &value);
			if (status == asynSuccess) {
				status = setIntegerParam(reg->param, value);
			}
		} else if (reg->type == asynParamFloat64) {
			epicsFloat64 value;
			status = readDataFloat(reg->num, &value);
			if (status == asynSuccess) {
				status = setDoubleParam(reg->param, value);
			}
		} else if (reg->type == asynParamUInt32Digital) {
			epicsUInt32 value;
			status = readDataInt(reg->num, (epicsInt32 *)&value);
			if (reg->param == LTTemp1Mode) {
				/* Temp1 has zoom mode */
				value &= ~0x08;
			}
			if (status == asynSuccess) {
				status = setUIntDigitalParam(reg->param, value, reg->mask);
			}
		} else {
			printf("%s::%s: Unsupported type %d for index %d!\n",
					driverName, __func__, reg->type, reg->param);
		}
		if (status) {
			printf("%s::%s: Failed to read from register %d\n",
					driverName, __func__, reg->num);
		}

		/* let the regulator breathe */
		usleep(10000);
	}

	status = controlLogging(true);
	if (status) {
		return asynError;
	}

	return status;
}

asynStatus LTPR59::writeAllRegisterParams(void) {
	struct LTPR59Register *reg, *modeReg;
	int i;
	epicsUInt32 bits;
	epicsInt32 mode = 0;
	asynStatus status = asynSuccess;

	status = controlLogging(false);
	if (status) {
		return asynError;
	}

	status = findRegister(13, &modeReg);
	if (status) {
		return asynError;
	}
	getUIntDigitalParam(LTMode, &bits, 0xF);
	mode = bits;
	getUIntDigitalParam(LTModeFlags, &bits, 0x3F0);
	mode |= bits;
	getUIntDigitalParam(LTFilterA, &bits, 0x3000);
	mode |= bits;
	getUIntDigitalParam(LTFilterB, &bits, 0xC000);
	mode |= bits;

	/* first set mode WITH 'download parameters' flag! */
	mode |= 0x40;
	status = writeDataInt(modeReg->num, mode);

	for (i = 0; i < mRegsIndex; i++) {
		reg = &mRegs[i];
		/* this PV does not have a register to be written */
		if (reg->num == -1) {
			continue;
		}
		/* this register is not writable */
		if (reg->write == 0) {
			continue;
		}
		/* this PV should not be written to a register */
		if (reg->param == LTMode ||
			reg->param == LTModeFlags ||
			reg->param == LTFilterA ||
			reg->param == LTFilterB) {
			continue;
		}

		if (reg->type == asynParamInt32) {
			epicsInt32 value;
			status = getIntegerParam(reg->param, &value);
			if (status == asynSuccess) {
				status = writeDataInt(reg->num, value);
			}
		} else if (reg->type == asynParamFloat64) {
			epicsFloat64 value;
			status = getDoubleParam(reg->param, &value);
			if (status == asynSuccess) {
				status = writeDataFloat(reg->num, value);
			}
		} else if (reg->type == asynParamUInt32Digital) {
			epicsUInt32 value;
			status = getUIntDigitalParam(reg->param, &value, reg->mask);
			if (reg->param == LTTemp1Mode) {
				/* Temp1 has zoom mode */
				value |= 0x08;
			}
			if (status == asynSuccess) {
				status = writeDataInt(reg->num, (epicsInt32)value);
			}
		} else {
			printf("%s::%s: Unsupported type %d for index %d!\n",
					driverName, __func__, reg->type, reg->param);
		}
		if (status) {
			printf("%s::%s: Failed to write to register %d\n",
					driverName, __func__, reg->num);
		}

		/* let the regulator breathe */
		usleep(10000);
	}

	/* set mode again WITHOUT 'download parameters' flag! */
	mode &= ~0x40;
	status = writeDataInt(modeReg->num, mode);

	status = controlLogging(true);
	if (status) {
		return asynError;
	}

	return status;
}

void LTPR59::report(FILE *fp, int details) {

	fprintf(fp, "LTPR59 %s\n", this->portName);
	if (details > 0) {
	}
	/* Invoke the base class method */
	asynPortDriver::report(fp, details);
}

/** Constructor for the LTPR59 class.
  * Calls constructor for the asynPortDriver base class.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] serialPort The name of the serials port
  */
LTPR59::LTPR59(const char *portName, const char *serialPort)
   : asynPortDriver(portName,
		   1,
		   asynInt32Mask | asynFloat64Mask | asynOctetMask | asynUInt32DigitalMask | asynDrvUserMask,
		   asynInt32Mask | asynFloat64Mask | asynOctetMask | asynUInt32DigitalMask,
		   ASYN_CANBLOCK, /* asynFlags: ASYN_CANBLOCK=1, ASYN_MULTIDEVICE=0*/
		   1, /* autoConnect: YES */
		   0, /* priority: assigned by asynManager */
		   0) /* stackSize: assigned by asynManager*/
{
	int status = asynSuccess;

//	printf("%s::%s: drvAsynSeriaPort '%s'\n", driverName, __func__, serialPort);

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

	/* General commands */
	createParam(LTStatusMessageString,		asynParamOctet,			&LTStatusMessage);
	createParam(LTIDString,					asynParamOctet,			&LTID);
	createParam(LTVersionString,			asynParamOctet,			&LTVersion);
	createParam(LTStatusString,				asynParamOctet,			&LTStatus);
	createParam(LTStatusAlarmString,		asynParamUInt32Digital,	&LTStatusAlarm);
	createParam(LTStatusErrorString,		asynParamUInt32Digital,	&LTStatusError);
	createParam(LTLoggingModeString,		asynParamInt32,			&LTLoggingMode);
	createParam(LTStatusClearString,		asynParamInt32,			&LTStatusClear);
	createParam(LTStartStopString,			asynParamInt32,			&LTStartStop);
	createParam(LTWriteEEPROMString,		asynParamInt32,			&LTWriteEEPROM);
	createParam(LTClearEEPROMString,		asynParamInt32,			&LTClearEEPROM);
	createParam(LTSendString,				asynParamInt32,			&LTSend);
	createParam(LTRetrieveString,			asynParamInt32,			&LTRetrieve);

	/* Regulator parameters */
	mRegsIndex = 0;
	createRegisterParam(13,		0xF, 	LTModeString,				asynParamUInt32Digital,	&LTMode);
	createRegisterParam(13,		0x3F0, 	LTModeFlagsString,			asynParamUInt32Digital,	&LTModeFlags);
	createRegisterParam(13,		0x3000,	LTFilterAString,			asynParamUInt32Digital,	&LTFilterA);
	createRegisterParam(13,		0xC000,	LTFilterBString,			asynParamUInt32Digital,	&LTFilterB);
	createRegisterParam(99,		0x0, 	LTEventCounterString,		asynParamInt32,			&LTEventCounter);
	createRegisterParam(0,		0x0, 	LTTrSetPointString,			asynParamFloat64,		&LTTrSetPoint);
	createRegisterParam(6,		0x0, 	LTTcLimitString,			asynParamFloat64,		&LTTcLimit);
	createRegisterParam(7,		0x0, 	LTTcDeadBandString,			asynParamFloat64,		&LTTcDeadBand);
	createRegisterParam(9,		0x0, 	LTSampleRateString,			asynParamInt32,			&LTSampleRate);
	createRegisterParam(10,		0x0, 	LTTcCoolGainString,			asynParamFloat64,		&LTTcCoolGain);
	createRegisterParam(11,		0x0, 	LTTcHeatGainString,			asynParamFloat64,		&LTTcHeatGain);
	createRegisterParam(14,		0x0, 	LTAlgoDeadBandString,		asynParamFloat64,		&LTAlgoDeadBand);
	createRegisterParam(15,		0x0, 	LTAlgoHysteresisString,		asynParamFloat64,		&LTAlgoHysteresis);
	createRegisterParam(105,	0x0, 	LTTRefString,				asynParamFloat64,		&LTTRef);
	createRegisterParam(106,	0x0, 	LTTcOutputString,			asynParamFloat64,		&LTTcOutput);
	createRegisterParam(150,	0x0, 	LTInputVoltageString,		asynParamFloat64,		&LTInputVoltage);
	createRegisterParam(151,	0x0, 	LTInternal12VString,		asynParamFloat64,		&LTInternal12V);
	createRegisterParam(152,	0x0, 	LTMainCurrentString,		asynParamFloat64,		&LTMainCurrent);

	/* Temperature 1 parameters */
	createRegisterParam(100,	0x0, 	LTTemp1String,				asynParamFloat64,		&LTTemp1);
	createRegisterParam(55,		0x1F, 	LTTemp1ModeString,			asynParamUInt32Digital,	&LTTemp1Mode);
	createRegisterParam(35,		0x0, 	LTTemp1GainString,			asynParamFloat64,		&LTTemp1Gain);
	createRegisterParam(36,		0x0, 	LTTemp1OffsetString,		asynParamFloat64,		&LTTemp1Offset);
	createRegisterParam(59,		0x0, 	LTTemp1CoeffAString,		asynParamFloat64,		&LTTemp1CoeffA);
	createRegisterParam(60,		0x0, 	LTTemp1CoeffBString,		asynParamFloat64,		&LTTemp1CoeffB);
	createRegisterParam(61,		0x0, 	LTTemp1CoeffCString,		asynParamFloat64,		&LTTemp1CoeffC);
	createRegisterParam(79,		0x0, 	LTTemp1ResHighString,		asynParamFloat64,		&LTTemp1ResHigh);
	createRegisterParam(80,		0x0, 	LTTemp1ResMedString,		asynParamFloat64,		&LTTemp1ResMed);
	createRegisterParam(81,		0x0, 	LTTemp1ResLowString,		asynParamFloat64,		&LTTemp1ResLow);
	createRegisterParam(-1,		0x0, 	LTTemp1TempHighString,		asynParamFloat64,		&LTTemp1TempHigh);
	createRegisterParam(-1,		0x0, 	LTTemp1TempMedString,		asynParamFloat64,		&LTTemp1TempMed);
	createRegisterParam(-1,		0x0, 	LTTemp1TempLowString,		asynParamFloat64,		&LTTemp1TempLow);

	/* TODO: Temperature 2 parameters */
	/* TODO: Temperature 3 parameters */

	/* Temperature 4 parameters */
	createRegisterParam(103,	0x0, 	LTTemp4String,				asynParamFloat64,		&LTTemp4);
	createRegisterParam(58,		0x1F, 	LTTemp4ModeString,			asynParamUInt32Digital,	&LTTemp4Mode);
	createRegisterParam(41,		0x0, 	LTTemp4GainString,			asynParamFloat64,		&LTTemp4Gain);
	createRegisterParam(42,		0x0, 	LTTemp4OffsetString,		asynParamFloat64,		&LTTemp4Offset);
	createRegisterParam(68,		0x0, 	LTTemp4CoeffAString,		asynParamFloat64,		&LTTemp4CoeffA);
	createRegisterParam(69,		0x0, 	LTTemp4CoeffBString,		asynParamFloat64,		&LTTemp4CoeffB);
	createRegisterParam(70,		0x0, 	LTTemp4CoeffCString,		asynParamFloat64,		&LTTemp4CoeffC);
	createRegisterParam(88,		0x0, 	LTTemp4ResHighString,		asynParamFloat64,		&LTTemp4ResHigh);
	createRegisterParam(89,		0x0, 	LTTemp4ResMedString,		asynParamFloat64,		&LTTemp4ResMed);
	createRegisterParam(90,		0x0, 	LTTemp4ResLowString,		asynParamFloat64,		&LTTemp4ResLow);
	createRegisterParam(-1,		0x0, 	LTTemp4TempHighString,		asynParamFloat64,		&LTTemp4TempHigh);
	createRegisterParam(-1,		0x0, 	LTTemp4TempMedString,		asynParamFloat64,		&LTTemp4TempMed);
	createRegisterParam(-1,		0x0, 	LTTemp4TempLowString,		asynParamFloat64,		&LTTemp4TempLow);

	/* PID parameters */
	createRegisterParam(1,		0x0, 	LTPIDKpString,				asynParamFloat64,		&LTPIDKp);
	createRegisterParam(2,		0x0, 	LTPIDKiString,				asynParamFloat64,		&LTPIDKi);
	createRegisterParam(3,		0x0, 	LTPIDKdString,				asynParamFloat64,		&LTPIDKd);
	createRegisterParam(4,		0x0, 	LTPIDKLPaString,			asynParamFloat64,		&LTPIDKLPa);
	createRegisterParam(5,		0x0, 	LTPIDKLPbString,			asynParamFloat64,		&LTPIDKLPb);
	createRegisterParam(8,		0x0, 	LTPIDILimitString,			asynParamFloat64,		&LTPIDILimit);
	createRegisterParam(12,		0x0, 	LTPIDDecayString,			asynParamFloat64,		&LTPIDDecay);
	createRegisterParam(110,	0x0, 	LTPIDTaString,				asynParamFloat64,		&LTPIDTa);
	createRegisterParam(111,	0x0, 	LTPIDTeString,				asynParamFloat64,		&LTPIDTe);
	createRegisterParam(112,	0x0, 	LTPIDTpString,				asynParamFloat64,		&LTPIDTp);
	createRegisterParam(113,	0x0, 	LTPIDTiString,				asynParamFloat64,		&LTPIDTi);
	createRegisterParam(114,	0x0, 	LTPIDTdString,				asynParamFloat64,		&LTPIDTd);
	createRegisterParam(117,	0x0, 	LTPIDTLPaString,			asynParamFloat64,		&LTPIDTLPa);
	createRegisterParam(118,	0x0, 	LTPIDTLPbString,			asynParamFloat64,		&LTPIDTLPb);
//	printf("%s::%s: Created %d registers..\n", driverName, __func__, mRegsIndex);

	setStringParam(LTStatusMessage,			"");
	setStringParam(LTID,					"");
	setStringParam(LTVersion,				"");
	setStringParam(LTStatus,				"");
	setIntegerParam(LTLoggingMode,		     0);
	setUIntDigitalParam(LTStatusAlarm,		 0, 0xFFFF);
	setUIntDigitalParam(LTStatusError,		 0, 0xFFFF);

	mAcquiringData = 0;
	mFinish = 0;

	/* Connect to desired IP port */
	status = pasynOctetSyncIO->connect(serialPort, 0, &mAsynUserCommand, NULL);
	if (status) {
		printf("%s:%s: pasynOctetSyncIO->connect failure\n", driverName, __func__);
		printf("%s:%s: init FAIL!\n", driverName, __func__);
		return;
	}

	/* stop continuous logging if in progress! */
	status = controlLogging(false);
	if (status) {
		printf("%s:%s: device not present?\n", driverName, __func__);
		printf("%s:%s: init FAIL!\n", driverName, __func__);
		return;
	}

	/* Use this to signal the data acquisition task that acquisition has started. */
	this->mDataEvent = epicsEventMustCreate(epicsEventEmpty);
	if (!this->mDataEvent) {
		printf("%s:%s epicsEventCreate failure for data event\n", driverName, __func__);
		printf("%s:%s: init FAIL!\n", driverName, __func__);
		return;
	}

	/* Create the thread that continuous logged data readout */
	status = (epicsThreadCreate("LTPR59Task",
			epicsThreadPriorityMedium,
			epicsThreadGetStackSize(epicsThreadStackMedium),
			(EPICSTHREADFUNC) taskC, this) == NULL);
	if (status) {
		printf("%s:%s: epicsThreadCreate failure for data task\n", driverName, __func__);
		printf("%s:%s: init FAIL!\n", driverName, __func__);
		return;
	}

	printf("%s:%s: init complete OK!\n", driverName, __func__);
}

LTPR59::~LTPR59() {
	printf("%s: shutting down ...\n", __func__);

	pasynOctetSyncIO->disconnect(mAsynUserCommand);

	this->lock();
	/* make sure threads are cleanly stopped */
	printf("Waiting for threads...\n");
	this->mFinish = 1;
	epicsEventSignal(mDataEvent);
	sleep(1);
	printf("Threads are down!\n");

	this->unlock();

	printf("%s: shutdown complete!\n", __func__);
}



/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int LTPR59Configure(const char *portName, const char *serialPort) {
	new LTPR59(portName, serialPort);
	return(asynSuccess);
}

/* EPICS iocsh shell commands */

static const iocshArg initArg0 = { "portName",			iocshArgString};
static const iocshArg initArg1 = { "serialPort",		iocshArgString};
static const iocshArg * const initArgs[] = {&initArg0,
											&initArg1};
static const iocshFuncDef initFuncDef = {"LTPR59Configure", 2, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	LTPR59Configure(args[0].sval, args[1].sval);
}

void LTPR59Register(void) {
	iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(LTPR59Register);

} /* extern "C" */
