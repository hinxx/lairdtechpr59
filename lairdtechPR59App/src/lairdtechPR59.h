/*
 * lairdtechPR59.h
 *
 *  Created on: Dec 6, 2016
 *      Author: hinkokocevar
 */

#ifndef LAIRDTECHPR59APP_SRC_LAIRDTECHPR59_H_
#define LAIRDTECHPR59APP_SRC_LAIRDTECHPR59_H_

#include <asynPortDriver.h>
#include <asynOctetSyncIO.h>

#define LTPR59_MAX_MSG_SZ				512
#define LTPR59_REQ_TYPE_INVALID			0
#define LTPR59_REQ_TYPE_CMD				1
#define LTPR59_REQ_TYPE_READ			2
#define LTPR59_REQ_TYPE_WRITE			3

#define LTStatusMessageString			"LT_STATUS_MESSAGE"			/**< (asynOctet,			r/o) Status message */
#define LTIDString						"LT_ID"						/**< (asynOctet,			r/o) ID string */
#define LTStatusString					"LT_STATUS"					/**< (asynOctet,			r/o) Status flags string */
#define LTStatusAlarmString				"LT_STATUS_ALARM"			/**< (asynUInt32Digital,	r/o) Status alarm flags */
#define LTStatusErrorString				"LT_STATUS_ERROR"			/**< (asynUInt32Digital,	r/o) Status error flags */
#define LTRegulatorModeString			"LT_REGULATOR_MODE"			/**< (asynUInt32Digital,	r/w) Regulator mode of operation */
#define LTRegulatorModeFlagsString		"LT_REGULATOR_MODE_FLAGS"	/**< (asynUInt32Digital,	r/w) Regulator mode flags */
#define LTRegulatorFilterAString		"LT_REGULATOR_FILTER_A"		/**< (asynUInt32Digital,	r/w) Regulator filter A */
#define LTRegulatorFilterBString		"LT_REGULATOR_FILTER_B"		/**< (asynUInt32Digital,	r/w) Regulator filter B */
#define LTTemp1ModeString				"LT_TEMP_1_MODE"			/**< (asynUInt32Digital,	r/w) Temperature sensor 1 mode of operation */
#define LTSetPointTrString				"LT_SET_POINT_TR"			/**< (asynFloat64,			r/w) Temperature reference set point Tr */
#define LTLoggingModeString				"LT_LOGGING_MODE"			/**< (asynInt32,			r/w) Continuous logging mode */
#define LTTemp1String					"LT_TEMP_1"					/**< (asynFloat64,			r/o) Temperature 1 value */
#define LTTemp2String					"LT_TEMP_2"					/**< (asynFloat64,			r/o) Temperature 2 value */
#define LTTemp3String					"LT_TEMP_3"					/**< (asynFloat64,			r/o) Temperature 3 value */
#define LTTemp4String					"LT_TEMP_4"					/**< (asynFloat64,			r/o) Temperature 4 value */
#define LTOutputTcString				"LT_OUTPUT_TC"				/**< (asynFloat64,			r/o) Output Tc value */
#define LTPIDTaString					"LT_PID_TA"					/**< (asynFloat64,			r/o) PID Ta value */
#define LTPIDTpString					"LT_PID_TP"					/**< (asynFloat64,			r/o) PID Tp value */
#define LTPIDTiString					"LT_PID_TI"					/**< (asynFloat64,			r/o) PID Ti value */
#define LTPIDTdString					"LT_PID_TD"					/**< (asynFloat64,			r/o) PID Td value */
#define LTPIDTLPaString					"LT_PID_TLPA"				/**< (asynFloat64,			r/o) PID TLP a value */
#define LTPIDTLPbString					"LT_PID_TLPB"				/**< (asynFloat64,			r/o) PID TLP b value */
#define LTInputVoltageString			"LT_INPUT_VOLTAGE"			/**< (asynFloat64,			r/o) Input voltage */
#define LTInternal12VString				"LT_INTERNAL_VOLTAGE"		/**< (asynFloat64,			r/o) Internal 12V voltage */
#define LTMainCurrentString				"LT_MAIN_CURRENT"			/**< (asynFloat64,			r/o) Main current */
#define LTFan1CurrentString				"LT_FAN_1_CURRENT"			/**< (asynFloat64,			r/o) Fan 1 current */
#define LTFan2CurrentString				"LT_FAN_2_CURRENT"			/**< (asynFloat64,			r/o) Fan 2 current */

class LTPR59 : public asynPortDriver {
public:
	LTPR59(const char *portName, const char *serialPort);
	virtual ~LTPR59();

	/* These are the methods that we override from asynPortDriver */
	virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);
    virtual asynStatus readUInt32Digital(asynUser *pasynUser, epicsUInt32 *value, epicsUInt32 mask);
    virtual asynStatus writeUInt32Digital(asynUser *pasynUser, epicsUInt32 value, epicsUInt32 mask);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
    virtual asynStatus readFloat64(asynUser *pasynUser, epicsFloat64 *value);
    virtual asynStatus readOctet(asynUser *pasynUser, char *value, size_t maxChars,
                                        size_t *nActual, int *eomReason);
	void report(FILE *fp, int details);
	/* These are new methods */
	// Should be private, but are called from C so must be public
	void dataTask(void);

protected:
	virtual asynStatus serialPortWrite(double timeout);
	virtual asynStatus serialPortRead(double timeout);
	virtual asynStatus serialPortWriteRead(double timeout);
	/* Derived classes need access to these members */
	char mReq[LTPR59_MAX_MSG_SZ];
	size_t mReqSz;
	size_t mReqActSz;
	char mResp[LTPR59_MAX_MSG_SZ];
	size_t mRespSz;
	size_t mRespActSz;
	char mStatusMsg[LTPR59_MAX_MSG_SZ];

	/* Our parameter list */
	int LTStatusMessage;
	int LTID;
	int LTStatus;
	int LTStatusAlarm;
	int LTStatusError;
	int LTRegulatorMode;
	int LTRegulatorModeFlags;
	int LTRegulatorFilterA;
	int LTRegulatorFilterB;
	int LTTemp1Mode;
	int LTSetPointTr;
	int LTLoggingMode;

	/* monitoring analog values */
	int LTTemp1;
	int LTTemp2;
	int LTTemp3;
	int LTTemp4;
	int LTOutputTc;
	int LTPIDTa;
	int LTPIDTp;
	int LTPIDTi;
	int LTPIDTd;
	int LTPIDTLPa;
	int LTPIDTLPb;
	int LTInputVoltage;
	int LTInternal12V;
	int LTMainCurrent;
	int LTFan1Current;
	int LTFan2Current;
	int LTLast;
#define FIRST_LTPR59_PARAM LTStatusMessage
#define LAST_LTPR59_PARAM LTLast

private:
	/* These are the methods that are new to this class */
	void hexdump(void *mem, unsigned int len);
    void updateStatus(const char *msg);
    asynStatus trimResponse(char *buf, unsigned int *len);
    asynStatus convToString(epicsInt32 val, char *buf, unsigned int *len);
    asynStatus convToString(epicsFloat64 val, char *buf, unsigned int *len);
    asynStatus convFromString(epicsInt32 *val);
    asynStatus convFromString(epicsFloat64 *val);

    asynStatus xfer(unsigned int reqType, const char *cmd, unsigned int reg,
    		const char *data, bool readData = true, double timeout = 1.0);
    asynStatus writeDataFloat(unsigned int reg, const epicsFloat64 val);
    asynStatus writeDataInt(unsigned int reg, const epicsInt32 val);
    asynStatus readDataFloat(unsigned int reg, epicsFloat64 *val);
    asynStatus readDataInt(unsigned int reg, epicsInt32 *val);
    asynStatus readString(const char *cmd, char *val, unsigned int *len);

	char *mSerialPort;
	asynUser *mAsynUserCommand;
	epicsEventId mDataEvent;
	unsigned int mAcquiringData;
	unsigned int mFinish;
};

#define NUM_LTPR59_PARAMS ((int)(&LAST_LTPR59_PARAM - &FIRST_LTPR59_PARAM + 1))

#endif /* LAIRDTECHPR59APP_SRC_LAIRDTECHPR59_H_ */
