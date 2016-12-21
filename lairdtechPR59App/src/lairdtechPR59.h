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
#define LTVersionString					"LT_VERSION"				/**< (asynOctet,			r/o) Software version */
#define LTStatusString					"LT_STATUS"					/**< (asynOctet,			r/o) Status flags string */
#define LTStatusAlarmString				"LT_STATUS_ALARM"			/**< (asynUInt32Digital,	r/o) Status alarm flags */
#define LTStatusErrorString				"LT_STATUS_ERROR"			/**< (asynUInt32Digital,	r/o) Status error flags */
#define LTLoggingModeString				"LT_LOGGING_MODE"			/**< (asynInt32,			r/w) Continuous logging mode */
#define LTStatusClearString				"LT_STATUS_CLEAR"			/**< (asynInt32,			w/o) Clear status flags */
#define LTStartStopString				"LT_START_STOP"				/**< (asynInt32,			r/w) Start / stop regulator */
#define LTWriteEEPROMString				"LT_WRITE_EEPROM"			/**< (asynInt32,			w/o) Write settings to EEPROM */
#define LTClearEEPROMString				"LT_CLEAR_EEPROM"			/**< (asynInt32,			w/o) Clear EEPROM */
#define LTSendString					"LT_SEND"					/**< (asynInt32,			w/o) Send parameters */
#define LTRetrieveString				"LT_RETRIEVE"				/**< (asynInt32,			w/o) Retrieve parameters */

#define LTModeString					"LT_MODE"					/**< (asynUInt32Digital,	r/w) Regulator mode of operation */
#define LTModeFlagsString				"LT_MODE_FLAGS"				/**< (asynUInt32Digital,	r/w) Regulator mode flags */
#define LTFilterAString					"LT_FILTER_A"				/**< (asynUInt32Digital,	r/w) Regulator filter A */
#define LTFilterBString					"LT_FILTER_B"				/**< (asynUInt32Digital,	r/w) Regulator filter B */
#define LTEventCounterString			"LT_EVENT_COUNTER"			/**< (asynInt32,			r/o) Regulator event counter */
#define LTTrSetPointString				"LT_TR_SET_POINT"			/**< (asynFloat64,			r/w) Temperature reference set point Tr */
#define LTTcLimitString					"LT_TC_LIMIT"				/**< (asynFloat64,			r/w) Limit of the Tc signal */
#define LTTcDeadBandString				"LT_TC_DEADBAND"			/**< (asynFloat64,			r/w) Dead band of the Tc signal */
#define LTSampleRateString				"LT_SAMPLE_RATE"			/**< (asynInt32,			r/o) Regulator sample rate */
#define LTTcCoolGainString				"LT_TC_COOL_GAIN"			/**< (asynFloat64,			r/w) Cool gain of the Tc signal */
#define LTTcHeatGainString				"LT_TC_HEAT_GAIN"			/**< (asynFloat64,			r/w) Heat gain of the Tc signal */
#define LTAlgoDeadBandString			"LT_ALGO_DEAD_BAND"			/**< (asynFloat64,			r/w) ON/OFF dead band */
#define LTAlgoHysteresisString			"LT_ALGO_HYSTERESIS"		/**< (asynFloat64,			r/w) ON/OFF hysteresis */
#define LTTRefString					"LT_TREF"					/**< (asynFloat64,			r/o) TRef value */
#define LTTcOutputString				"LT_TC_OUTPUT"				/**< (asynFloat64,			r/o) Output Tc value */
#define LTInputVoltageString			"LT_INPUT_VOLTAGE"			/**< (asynFloat64,			r/o) Input voltage */
#define LTInternal12VString				"LT_INTERNAL_VOLTAGE"		/**< (asynFloat64,			r/o) Internal 12V voltage */
#define LTMainCurrentString				"LT_MAIN_CURRENT"			/**< (asynFloat64,			r/o) Main current */

#define LTTemp1String					"LT_TEMP_1"					/**< (asynFloat64,			r/o) Temperature sensor 1 value */
#define LTTemp1ModeString				"LT_TEMP_1_MODE"			/**< (asynUInt32Digital,	r/w) Temperature sensor 1 mode of operation */
#define LTTemp1GainString				"LT_TEMP_1_GAIN"			/**< (asynFloat64,			r/w) Temperature sensor 1 gain */
#define LTTemp1OffsetString				"LT_TEMP_1_OFFSET"			/**< (asynFloat64,			r/w) Temperature sensor 1 offset */
#define LTTemp1CoeffAString				"LT_TEMP_1_COEFF_A"			/**< (asynFloat64,			r/w) Temperature sensor 1 Steinhart coefficient A */
#define LTTemp1CoeffBString				"LT_TEMP_1_COEFF_B"			/**< (asynFloat64,			r/w) Temperature sensor 1 Steinhart coefficient B */
#define LTTemp1CoeffCString				"LT_TEMP_1_COEFF_C"			/**< (asynFloat64,			r/w) Temperature sensor 1 Steinhart coefficient C */
#define LTTemp1ResHighString			"LT_TEMP_1_RES_HIGH"		/**< (asynFloat64,			r/w) Temperature sensor 1 resistance high */
#define LTTemp1ResMedString				"LT_TEMP_1_RES_MED"			/**< (asynFloat64,			r/w) Temperature sensor 1 resistance medium */
#define LTTemp1ResLowString				"LT_TEMP_1_RES_LOW"			/**< (asynFloat64,			r/w) Temperature sensor 1 resistance low */
#define LTTemp1TempHighString			"LT_TEMP_1_TEMP_HIGH"		/**< (asynFloat64,			r/w) Temperature sensor 1 temperature high */
#define LTTemp1TempMedString			"LT_TEMP_1_TEMP_MED"		/**< (asynFloat64,			r/w) Temperature sensor 1 temperature medium */
#define LTTemp1TempLowString			"LT_TEMP_1_TEMP_LOW"		/**< (asynFloat64,			r/w) Temperature sensor 1 temperature low */

#define LTTemp4String					"LT_TEMP_4"					/**< (asynFloat64,			r/o) Temperature sensor 4 value */
#define LTTemp4ModeString				"LT_TEMP_4_MODE"			/**< (asynUInt32Digital,	r/w) Temperature sensor 4 mode of operation */
#define LTTemp4GainString				"LT_TEMP_4_GAIN"			/**< (asynFloat64,			r/w) Temperature sensor 4 gain */
#define LTTemp4OffsetString				"LT_TEMP_4_OFFSET"			/**< (asynFloat64,			r/w) Temperature sensor 4 offset */
#define LTTemp4CoeffAString				"LT_TEMP_4_COEFF_A"			/**< (asynFloat64,			r/w) Temperature sensor 4 Steinhart coefficient A */
#define LTTemp4CoeffBString				"LT_TEMP_4_COEFF_B"			/**< (asynFloat64,			r/w) Temperature sensor 4 Steinhart coefficient B */
#define LTTemp4CoeffCString				"LT_TEMP_4_COEFF_C"			/**< (asynFloat64,			r/w) Temperature sensor 4 Steinhart coefficient C */
#define LTTemp4ResHighString			"LT_TEMP_4_RES_HIGH"		/**< (asynFloat64,			r/w) Temperature sensor 4 resistance high */
#define LTTemp4ResMedString				"LT_TEMP_4_RES_MED"			/**< (asynFloat64,			r/w) Temperature sensor 4 resistance medium */
#define LTTemp4ResLowString				"LT_TEMP_4_RES_LOW"			/**< (asynFloat64,			r/w) Temperature sensor 4 resistance low */
#define LTTemp4TempHighString			"LT_TEMP_4_TEMP_HIGH"		/**< (asynFloat64,			r/w) Temperature sensor 4 temperature high */
#define LTTemp4TempMedString			"LT_TEMP_4_TEMP_MED"		/**< (asynFloat64,			r/w) Temperature sensor 4 temperature medium */
#define LTTemp4TempLowString			"LT_TEMP_4_TEMP_LOW"		/**< (asynFloat64,			r/w) Temperature sensor 4 temperature low */

#define LTPIDKpString					"LT_PID_KP"					/**< (asynFloat64,			r/w) PID Kp value */
#define LTPIDKiString					"LT_PID_KI"					/**< (asynFloat64,			r/w) PID Ki value */
#define LTPIDKdString					"LT_PID_KD"					/**< (asynFloat64,			r/w) PID Kd value */
#define LTPIDKLPaString					"LT_PID_KLPA"				/**< (asynFloat64,			r/w) PID KLP a value */
#define LTPIDKLPbString					"LT_PID_KLPB"				/**< (asynFloat64,			r/w) PID KLP b value */
#define LTPIDILimitString				"LT_PID_ILIMIT"				/**< (asynFloat64,			r/w) PID limit I value */
#define LTPIDDecayString				"LT_PID_DECAY"				/**< (asynFloat64,			r/w) PID decay value */
#define LTPIDTaString					"LT_PID_TA"					/**< (asynFloat64,			r/o) PID Ta value */
#define LTPIDTeString					"LT_PID_TE"					/**< (asynFloat64,			r/o) PID Te value */
#define LTPIDTpString					"LT_PID_TP"					/**< (asynFloat64,			r/o) PID Tp value */
#define LTPIDTiString					"LT_PID_TI"					/**< (asynFloat64,			r/o) PID Ti value */
#define LTPIDTdString					"LT_PID_TD"					/**< (asynFloat64,			r/o) PID Td value */
#define LTPIDTLPaString					"LT_PID_TLPA"				/**< (asynFloat64,			r/o) PID TLP a value */
#define LTPIDTLPbString					"LT_PID_TLPB"				/**< (asynFloat64,			r/o) PID TLP b value */

struct LTPR59Register {
	int num;
	int mask;
	int write;
	asynParamType type;
	int param;
};
#define LTPR59_MAX_REGISTERS	256

class LTPR59 : public asynPortDriver {
public:
	LTPR59(const char *portName, const char *serialPort);
	virtual ~LTPR59();

	/* These are the methods that we override from asynPortDriver */
	virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
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
	int LTVersion;
	int LTStatus;
	int LTStatusAlarm;
	int LTStatusError;
	int LTLoggingMode;
	int LTStatusClear;
	int LTStartStop;
	int LTWriteEEPROM;
	int LTClearEEPROM;
	int LTSend;
	int LTRetrieve;

	int LTMode;
	int LTModeFlags;
	int LTFilterA;
	int LTFilterB;
	int LTEventCounter;
	int LTTrSetPoint;
	int LTTcLimit;
	int LTTcDeadBand;
	int LTSampleRate;
	int LTTcCoolGain;
	int LTTcHeatGain;
	int LTAlgoDeadBand;
	int LTAlgoHysteresis;
	int LTTRef;
	int LTTcOutput;
	int LTInputVoltage;
	int LTInternal12V;
	int LTMainCurrent;

	int LTTemp1;
	int LTTemp1Mode;
	int LTTemp1Gain;
	int LTTemp1Offset;
	int LTTemp1CoeffA;
	int LTTemp1CoeffB;
	int LTTemp1CoeffC;
	int LTTemp1ResHigh;
	int LTTemp1ResMed;
	int LTTemp1ResLow;
	int LTTemp1TempHigh;
	int LTTemp1TempMed;
	int LTTemp1TempLow;

	int LTTemp4;
	int LTTemp4Mode;
	int LTTemp4Gain;
	int LTTemp4Offset;
	int LTTemp4CoeffA;
	int LTTemp4CoeffB;
	int LTTemp4CoeffC;
	int LTTemp4ResHigh;
	int LTTemp4ResMed;
	int LTTemp4ResLow;
	int LTTemp4TempHigh;
	int LTTemp4TempMed;
	int LTTemp4TempLow;

	int LTPIDKp;
	int LTPIDKi;
	int LTPIDKd;
	int LTPIDKLPa;
	int LTPIDKLPb;
	int LTPIDILimit;
	int LTPIDDecay;
	int LTPIDTa;
	int LTPIDTe;
	int LTPIDTp;
	int LTPIDTi;
	int LTPIDTd;
	int LTPIDTLPa;
	int LTPIDTLPb;
	int LTLast;
#define FIRST_LTPR59_PARAM LTStatusMessage
#define LAST_LTPR59_PARAM LTLast

private:
	/* These are the methods that are new to this class */
	void hexdump(void *mem, unsigned int len);
    void updateStatus(const char *msg);
    asynStatus convToString(epicsInt32 val, char *buf, unsigned int *len);
    asynStatus convToString(epicsFloat64 val, char *buf, unsigned int *len);
    asynStatus convFromString(epicsInt32 *val);
    asynStatus convFromString(epicsFloat64 *val);

    asynStatus xfer(unsigned int reqType, const char *cmd, unsigned int reg,
    		const char *data, bool readData = true, double timeout = 0.3);
    asynStatus writeDataFloat(unsigned int reg, const epicsFloat64 val);
    asynStatus writeDataInt(unsigned int reg, const epicsInt32 val);
    asynStatus readDataFloat(unsigned int reg, epicsFloat64 *val);
    asynStatus readDataInt(unsigned int reg, epicsInt32 *val);
    asynStatus readString(const char *cmd, char *val, unsigned int *len);
    asynStatus controlLogging(const bool active);
    asynStatus createRegisterParam(const int num, const int mask, const char *name, asynParamType type, int *index);
    asynStatus findRegister(const int num, struct LTPR59Register **reg);
    asynStatus readAllRegisterParams(void);
    asynStatus writeAllRegisterParams(void);

	asynUser *mAsynUserCommand;
	epicsEventId mDataEvent;
	unsigned int mAcquiringData;
	unsigned int mFinish;
    struct LTPR59Register mRegs[LTPR59_MAX_REGISTERS];
    int mRegsIndex;
};

#define NUM_LTPR59_PARAMS ((int)(&LAST_LTPR59_PARAM - &FIRST_LTPR59_PARAM + 1))

#endif /* LAIRDTECHPR59APP_SRC_LAIRDTECHPR59_H_ */
