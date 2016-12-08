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

asynStatus LTPR59::trimResponse(char *buf, unsigned int *len) {
	char *s, *e, *b;
	unsigned int l;

	s = &mResp[0];
	e = &mResp[mRespActSz-1];
	while (*s && (*s != '\n')) {
		s++;
	}
	while (*e && (*e != '\r')) {
		e--;
	}
	assert(*s != 0);
	assert(*e != 0);
	s++;
	b = buf;
	l = 0;
	do {
		*b = *s;
		b++;
		s++;
		l++;
	} while ((s != e) && (l < (*len - 1)));
	*len = l;
	*b = '\0';
	printf("%s: Final string [%d]: '%s'\n", __func__, *len, buf);

	if (*buf == '?') {
	    updateStatus("Unknown command");
	    return asynError;
	}

	return asynSuccess;
}

asynStatus LTPR59::convFromString(epicsInt32 *val) {
	asynStatus status = asynSuccess;
	int v;
	int n;
	char buf[LTPR59_MAX_MSG_SZ] = {0};
	unsigned int len = LTPR59_MAX_MSG_SZ;

	status = trimResponse(buf, &len);
	if (status) {
		return status;
	}
	n = sscanf(buf, "%d", &v);
	if (n != 1) {
		updateStatus("Failed to parse value");
	    return asynError;
	}

	*val = v;

	return asynSuccess;
}

asynStatus LTPR59::convFromString(epicsFloat64 *val) {
	asynStatus status = asynSuccess;
	float v;
	int n;
	char buf[LTPR59_MAX_MSG_SZ] = {0};
	unsigned int len = LTPR59_MAX_MSG_SZ;

	status = trimResponse(buf, &len);
	if (status) {
		return status;
	}
	n = sscanf(buf, "%f", &v);
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

	if (readData) {
		// used for all regular commands and register access
		status = serialPortWriteRead(timeout);
	} else {
		// used when starting continuous logging
		status = serialPortWrite(timeout);
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
	status = trimResponse(val, len);
	if (status) {
		return status;
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
	status = xfer(LTPR59_REQ_TYPE_WRITE, "$R", reg, buf);

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
	status = xfer(LTPR59_REQ_TYPE_WRITE, "$R", reg, buf);

	return status;
}


/**
 * Do data readout from the detector. Meant to be run in own thread.
 */
void LTPR59::dataTask(void) {
	asynStatus status = asynSuccess;
	epicsInt32 contLog = 0;
	char *p;
	int n;
	int iv[20];
	float fv[20];

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
			status = serialPortRead(1.0);
			this->lock();

			if (status) {
				break;
			}

			p = mResp;
			n = mRespActSz;
			while (n && (*p != '[')) {
				p++;
				n--;
			}
			if (!n) {
				printf("%s: Failed to find '['..\n", __func__);
				continue;
			}

			getIntegerParam(LTLoggingMode, &contLog);
			if (contLog == 1) {
				/* $A1
				 * View Ain Vin F2c T1 T2 T3 Tfet Curr V12 F1c a12 a13 iTc iF1 iF2
				 * [1 0 311 605 0 1023 1022 1022 691 127 597 0 0 0 0 1023 1023]
				 *
				 * XXX: 16 labels, 17 values; doc 1.6c has only 13 listed fields?!
				 */
				n = sscanf(p, "[%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]",
					&iv[0], &iv[1], &iv[2],  &iv[3],  &iv[4],  &iv[5],  &iv[6],  &iv[7],
					&iv[8], &iv[9], &iv[10], &iv[11], &iv[12], &iv[13], &iv[14], &iv[15], &iv[16]);
				printf("%s: Parsed %d values..\n", __func__, n);

			} else if (contLog == 2) {
				/* $A2
				 * View ERR Mode Temp1 TcOut F1out F2out
				 * [2 100AC001 00 1023 0.0000 0.0000 0.0000]
				 */
				n = sscanf(p, "[%d %X %X %d %f %f %f]",
					&iv[0], &iv[1], &iv[2], &iv[3], &fv[0], &fv[1], &fv[2]);
				printf("%s: Parsed %d values..\n", __func__, n);

			} else if (contLog == 3) {
				/* $A3
				 * View ERR Mode Tc Ta1 Ta2 Tr Ta Tp Ti Td TLP_A TLP_B
				 * [3 100AC001 00 0.0000 -999.9000 -999.9000 20.0000 -999.9000 0.0000 0.0000 0.0000 20.0000 36.6650]
				 */
				n = sscanf(p, "[%d %X %X %f %f %f %f %f %f %f %f %f %f]",
					&iv[0], &iv[1], &iv[2], &fv[0], &fv[1], &fv[2], &fv[3], &fv[4],
					&fv[5], &fv[6], &fv[7], &fv[8], &fv[9]);
				printf("%s: Parsed %d values..\n", __func__, n);
				if (n == 13) {
					setDoubleParam(LTOutputTc, fv[0]);
					setDoubleParam(LTTemp1, fv[1]);
					setDoubleParam(LTTemp2, fv[2]);
					//setDoubleParam(LTSetPoint, fv[3]);
					setDoubleParam(LTPIDTa, fv[4]);
					setDoubleParam(LTPIDTp, fv[5]);
					setDoubleParam(LTPIDTi, fv[6]);
					setDoubleParam(LTPIDTd, fv[7]);
					setDoubleParam(LTPIDTLPa, fv[8]);
					setDoubleParam(LTPIDTLPb, fv[9]);
				}

			} else if (contLog == 4) {
				/* $A4
				 * View ERR Mode Tc Tr Curr
				 * [4 100AC001 00 0.0000 20.0000 127]
				 */
				n = sscanf(p, "[%d %X %X %f %f %d]",
					&iv[0], &iv[1], &iv[2], &fv[0], &fv[1], &iv[3]);
				printf("%s: Parsed %d values..\n", __func__, n);

			} else if (contLog == 5) {
				/* $A5
				 * View ERR Mode TrExt Tr Ta
				 * [5 100AC001 00 0.0000 20.0000 -999.9000]
				 */
				n = sscanf(p, "[%d %X %X %f %f %f]",
					&iv[0], &iv[1], &iv[2], &fv[0], &fv[1], &fv[2]);
				printf("%s: Parsed %d values..\n", __func__, n);

			} else if (contLog == 6) {
				/* $A6
				 * View RUNTIMEDATA
				 * [6 91E4 100AC001 00 00 0.0000 0.0000 0.0000 -999.9000 -999.9000 -999.9000 67.7612 0.0000 20.0000 -999.9000 36.6650 0.0000 0.0000 0.0000 20.0000 20.0377 0.0000 -12.1233]
				 *
				 * XXX: To be decoded.. contact company?
				 */

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
		status = serialPortRead(1.0);

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

	printf("%s: function %d, addr %d, value %d\n", __func__, function, addr, value);
	status = setIntegerParam(addr, function, value);

	if (function == LTLoggingMode) {
		char cmd[4] = {'$', 'A', 0, 0};
		cmd[2] = '0' + value;
		//status = xfer(LTPR59_REQ_TYPE_CMD, cmd, 0, NULL, false);
		status = xfer(LTPR59_REQ_TYPE_CMD, cmd, 0, NULL);
		if (value > 0) {
			mAcquiringData = 1;
		} else {
			mAcquiringData = 0;
		}
	}

	/* Do callbacks so higher layers see any changes */
	callParamCallbacks(addr, addr);

	if (mAcquiringData) {
		asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
				"%s:%s:, Sending mDataEvent to dataTask ...\n", driverName,
				__func__);
		printf("%s:%s:, Sending mDataEvent to dataTask ...\n", driverName,
				__func__);
		// Also signal the data readout thread
		epicsEventSignal(mDataEvent);
	}

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

asynStatus LTPR59::readInt32(asynUser *pasynUser, epicsInt32 *value) {

	int function = pasynUser->reason;
	int addr = 0;
	asynStatus status = asynSuccess;

	status = getAddress(pasynUser, &addr);
	if (status != asynSuccess) {
		return(status);
	}

	if (0) {

	} else {
		status = asynPortDriver::readInt32(pasynUser, value);
	}
	printf("%s: function %d, addr %d, value %d\n", __func__, function, addr, *value);

	/* Do callbacks so higher layers see any changes */
//	callParamCallbacks(addr, addr);

	if (status) {
		asynPrint(pasynUser, ASYN_TRACE_ERROR,
			  "%s:%s: error, status=%d function=%d, addr=%d, value=%d\n",
			  driverName, __func__, status, function, addr, *value);
	} else {
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
			  "%s:%s: function=%d, addr=%d, value=%d\n",
			  driverName, __func__, function, addr, *value);
	}

	return status;

}

asynStatus LTPR59::writeUInt32Digital(asynUser *pasynUser, epicsUInt32 value, epicsUInt32 mask) {

	int function = pasynUser->reason;
	int addr = 0;
	asynStatus status = asynSuccess;

	status = getAddress(pasynUser, &addr);
	if (status != asynSuccess) {
		return(status);
	}

	printf("%s: function %d, addr %d, value %d, mask 0x%X\n", __func__, function, addr, value, mask);
	status = setUIntDigitalParam(addr, function, value, mask);

	if (function == LTRegulatorMode ||
		function == LTRegulatorModeFlags ||
		function == LTRegulatorFilterA ||
		function == LTRegulatorFilterB) {
		epicsUInt32 bits;
		epicsInt32 regVal = 0;
		getUIntDigitalParam(LTRegulatorMode, &bits, 0xF);
		regVal = bits;
		getUIntDigitalParam(LTRegulatorModeFlags, &bits, 0x3F0);
		regVal |= bits;
		getUIntDigitalParam(LTRegulatorFilterA, &bits, 0x3000);
		regVal |= bits;
		getUIntDigitalParam(LTRegulatorFilterB, &bits, 0xC000);
		regVal |= bits;
		status = writeDataInt(13, regVal);
	} else if (function == LTTemp1Mode) {
		status = writeDataInt(55, (epicsInt32)value);
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

asynStatus LTPR59::readUInt32Digital(asynUser *pasynUser, epicsUInt32 *value, epicsUInt32 mask) {

	int function = pasynUser->reason;
	int addr = 0;
	asynStatus status = asynSuccess;

	status = getAddress(pasynUser, &addr);
	if (status != asynSuccess) {
		return(status);
	}

	if (function == LTRegulatorMode ||
		function == LTRegulatorModeFlags ||
		function == LTRegulatorFilterA ||
		function == LTRegulatorFilterB) {
		status = readDataInt(13, (epicsInt32 *)value);
	} else if (function == LTTemp1Mode) {
		status = readDataInt(55, (epicsInt32 *)value);
	} else {
		status = asynPortDriver::readUInt32Digital(pasynUser, value, mask);
	}
	printf("%s: function %d, addr %d, value %d, mask 0x%X\n", __func__, function, addr, *value, mask);

	/* Do callbacks so higher layers see any changes */
//	callParamCallbacks(addr, addr);

	if (status) {
		asynPrint(pasynUser, ASYN_TRACE_ERROR,
			  "%s:%s: error, status=%d function=%d, addr=%d, value=%d\n",
			  driverName, __func__, status, function, addr, *value);
	} else {
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
			  "%s:%s: function=%d, addr=%d, value=%d\n",
			  driverName, __func__, function, addr, *value);
	}

	return status;

}

asynStatus LTPR59::writeFloat64(asynUser *pasynUser, epicsFloat64 value) {

	int function = pasynUser->reason;
	int addr = 0;
	asynStatus status = asynSuccess;

	status = getAddress(pasynUser, &addr);
	if (status != asynSuccess) {
		return(status);
	}

	printf("%s: function %d, addr %d, value %f\n", __func__, function, addr, value);
	status = setDoubleParam(addr, function, value);

	if (function == LTSetPointTr) {
		status = writeDataFloat(0, value);
	}

	/* Do callbacks so higher layers see any changes */
	callParamCallbacks(addr, addr);

	if (status) {
		asynPrint(pasynUser, ASYN_TRACE_ERROR,
			  "%s:%s: error, status=%d function=%d, addr=%d, value=%f\n",
			  driverName, __func__, status, function, addr, value);
	} else {
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
			  "%s:%s: function=%d, addr=%d, value=%f\n",
			  driverName, __func__, function, addr, value);
	}

	return status;
}

asynStatus LTPR59::readFloat64(asynUser *pasynUser, epicsFloat64 *value) {

	int function = pasynUser->reason;
	int addr = 0;
	asynStatus status = asynSuccess;

	status = getAddress(pasynUser, &addr);
	if (status != asynSuccess) {
		return(status);
	}

	if (function == LTSetPointTr) {
		status = readDataFloat(0, value);
	} else if (function == LTTemp1) {
		status = readDataFloat(100, value);
	} else if (function == LTTemp2) {
		status = readDataFloat(101, value);
	} else if (function == LTTemp3) {
		status = readDataFloat(102, value);
	} else if (function == LTTemp4) {
		status = readDataFloat(103, value);
	} else if (function == LTPIDTa) {
		status = readDataFloat(110, value);
	} else if (function == LTPIDTp) {
		status = readDataFloat(112, value);
	} else if (function == LTPIDTi) {
		status = readDataFloat(113, value);
	} else if (function == LTPIDTd) {
		status = readDataFloat(114, value);
	} else if (function == LTPIDTLPa) {
		status = readDataFloat(117, value);
	} else if (function == LTPIDTLPb) {
		status = readDataFloat(118, value);
	} else if (function == LTInputVoltage) {
		status = readDataFloat(150, value);
	} else if (function == LTInternal12V) {
		status = readDataFloat(151, value);
	} else if (function == LTMainCurrent) {
		status = readDataFloat(152, value);
	} else if (function == LTFan1Current) {
		status = readDataFloat(153, value);
	} else if (function == LTFan2Current) {
		status = readDataFloat(154, value);
	} else {
		status = asynPortDriver::readFloat64(pasynUser, value);
	}

	printf("%s: function %d, addr %d, value %f\n", __func__, function, addr, *value);

	/* Do callbacks so higher layers see any changes */
//	callParamCallbacks(addr, addr);

	if (status) {
		asynPrint(pasynUser, ASYN_TRACE_ERROR,
			  "%s:%s: error, status=%d function=%d, addr=%d, value=%f\n",
			  driverName, __func__, status, function, addr, *value);
	} else {
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
			  "%s:%s: function=%d, addr=%d, value=%f\n",
			  driverName, __func__, function, addr, *value);
	}

	return status;

}

asynStatus LTPR59::readOctet(asynUser *pasynUser, char *value, size_t maxChars,
                                    size_t *nActual, int *eomReason) {

	int function = pasynUser->reason;
	int addr = 0;
	unsigned int len, n;
	unsigned int alarms, errors;
	asynStatus status = asynSuccess;

	status = getAddress(pasynUser, &addr);
	if (status != asynSuccess) {
		return(status);
	}

	if (function == LTID) {
		len = maxChars;
		status = readString("$LI", value, &len);
		*nActual = len;
		*eomReason = 0;
	} else if (function == LTStatus) {
		len = maxChars;
		status = readString("$S", value, &len);
		*nActual = len;
		*eomReason = 0;
		n = sscanf(value, "%X %X %*s", &alarms, &errors);
		if (n != 2) {
			status = asynError;
		} else {
			printf("%s: alarms 0x%X, errors 0x%X\n", __func__, alarms, errors);
			setUIntDigitalParam(LTStatusAlarm, alarms, 0xFFFF);
			setUIntDigitalParam(LTStatusError, errors, 0xFFFF);
			/* Do callbacks so higher layers see any changes */
			callParamCallbacks(addr, addr);
		}
	} else {
		status = asynPortDriver::readOctet(pasynUser, value, maxChars, nActual, eomReason);
	}
	printf("%s: function %d, addr %d, value %s\n", __func__, function, addr, value);

	/* Do callbacks so higher layers see any changes */
//	callParamCallbacks(addr, addr);

	if (status) {
		asynPrint(pasynUser, ASYN_TRACE_ERROR,
			  "%s:%s: error, status=%d function=%d, addr=%d, value=%s\n",
			  driverName, __func__, status, function, addr, value);
	} else {
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
			  "%s:%s: function=%d, addr=%d, value=%s\n",
			  driverName, __func__, function, addr, value);
	}

	return status;
}

asynStatus LTPR59::serialPortWriteRead(double timeout) {
	int eomReason;
	asynStatus status;

	printf("%s: request (%ld bytes):\n", __func__, mReqSz);
	hexdump(mReq, mReqSz);

	status = pasynOctetSyncIO->writeRead(mAsynUserCommand,
			mReq, mReqSz, mResp, mRespSz,
			timeout, &mReqActSz, &mRespActSz, &eomReason);

	printf("%s: response (%ld bytes):\n", __func__, mRespActSz);
	hexdump(mResp, mRespActSz);

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

	printf("%s: request (%ld bytes):\n", __func__, mReqSz);
	hexdump(mReq, mReqSz);

	status = pasynOctetSyncIO->write(mAsynUserCommand,
			mReq, mReqSz,
			timeout, &mReqActSz);

	printf("%s: request actual size %ld bytes\n", __func__, mReqActSz);
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


	printf("%s: response (%ld bytes):\n", __func__, mRespActSz);
	hexdump(mResp, mRespActSz);

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
	printf("Status message: '%s'\n", mStatusMsg);
	setStringParam(LTStatusMessage, mStatusMsg);

	/* Do callbacks so higher layers see any changes */
	callParamCallbacks(0);
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
		   NUM_LTPR59_PARAMS,
		   asynInt32Mask | asynFloat64Mask | asynOctetMask | asynUInt32DigitalMask | asynDrvUserMask,
		   asynInt32Mask | asynFloat64Mask | asynOctetMask | asynUInt32DigitalMask,
		   ASYN_CANBLOCK, /* asynFlags: ASYN_CANBLOCK=1, ASYN_MULTIDEVICE=0*/
		   1, /* autoConnect: YES */
		   0, /* priority: assigned by asynManager */
		   0) /* stackSize: assigned by asynManager*/
{
	int status = asynSuccess;

	mSerialPort = strdup(serialPort);
	printf("%s: Serial port %s\n", __func__, mSerialPort);

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

	createParam(LTStatusMessageString,		asynParamOctet,			&LTStatusMessage);
	createParam(LTIDString,					asynParamOctet,			&LTID);
	createParam(LTStatusString,				asynParamOctet,			&LTStatus);
	createParam(LTStatusAlarmString,		asynParamUInt32Digital,	&LTStatusAlarm);
	createParam(LTStatusErrorString,		asynParamUInt32Digital,	&LTStatusError);
	createParam(LTRegulatorModeString,		asynParamUInt32Digital,	&LTRegulatorMode);
	createParam(LTRegulatorModeFlagsString,	asynParamUInt32Digital,	&LTRegulatorModeFlags);
	createParam(LTRegulatorFilterAString,	asynParamUInt32Digital,	&LTRegulatorFilterA);
	createParam(LTRegulatorFilterBString,	asynParamUInt32Digital,	&LTRegulatorFilterB);
	createParam(LTTemp1ModeString,			asynParamUInt32Digital,	&LTTemp1Mode);
	createParam(LTSetPointTrString,			asynParamFloat64,		&LTSetPointTr);
	createParam(LTLoggingModeString,		asynParamInt32,			&LTLoggingMode);
	createParam(LTTemp1String,				asynParamFloat64,		&LTTemp1);
	createParam(LTTemp2String,				asynParamFloat64,		&LTTemp2);
	createParam(LTTemp3String,				asynParamFloat64,		&LTTemp3);
	createParam(LTTemp4String,				asynParamFloat64,		&LTTemp4);
	createParam(LTOutputTcString,			asynParamFloat64,		&LTOutputTc);
	createParam(LTPIDTaString,				asynParamFloat64,		&LTPIDTa);
	createParam(LTPIDTpString,				asynParamFloat64,		&LTPIDTp);
	createParam(LTPIDTiString,				asynParamFloat64,		&LTPIDTi);
	createParam(LTPIDTdString,				asynParamFloat64,		&LTPIDTd);
	createParam(LTPIDTLPaString,			asynParamFloat64,		&LTPIDTLPa);
	createParam(LTPIDTLPbString,			asynParamFloat64,		&LTPIDTLPb);
	createParam(LTInputVoltageString,		asynParamFloat64,		&LTInputVoltage);
	createParam(LTInternal12VString,		asynParamFloat64,		&LTInternal12V);
	createParam(LTMainCurrentString,		asynParamFloat64,		&LTMainCurrent);
	createParam(LTFan1CurrentString,		asynParamFloat64,		&LTFan1Current);
	createParam(LTFan2CurrentString,		asynParamFloat64,		&LTFan2Current);

	setStringParam(LTStatusMessage,			"");
	setStringParam(LTID,					"");
	setStringParam(LTStatus,				"");
	setIntegerParam(LTLoggingMode,		     0);

	mAcquiringData = 0;
	mFinish = 0;

	/* Connect to desired IP port */
	status = pasynOctetSyncIO->connect(mSerialPort, 0, &mAsynUserCommand, NULL);
	if (status) {
		printf("%s:%s: pasynOctetSyncIO->connect failure\n", driverName, __func__);
		printf("%s: init FAIL!\n", __func__);
		return;
	}

	/* stop continuous logging if in progress! */
	char cmd[4] = {'$', 'A', 0, 0};
	status = xfer(LTPR59_REQ_TYPE_CMD, cmd, 0, NULL);

	/* Use this to signal the data acquisition task that acquisition has started. */
	this->mDataEvent = epicsEventMustCreate(epicsEventEmpty);
	if (!this->mDataEvent) {
		printf("%s:%s epicsEventCreate failure for data event\n",
				driverName, __func__);
		printf("%s: init FAIL!\n", __func__);
		return;
	}

	/* Create the thread that continuous logged data readout */
	status = (epicsThreadCreate("LTPR59Task",
			epicsThreadPriorityMedium,
			epicsThreadGetStackSize(epicsThreadStackMedium),
			(EPICSTHREADFUNC) taskC, this) == NULL);
	if (status) {
		printf("%s:%s: epicsThreadCreate failure for data task\n",
				driverName, __func__);
		printf("%s: init FAIL!\n", __func__);
		return;
	}

	printf("%s: init complete OK!\n", __func__);
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
