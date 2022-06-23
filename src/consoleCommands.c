// ConsoleCommands.c
// This is where you add commands:
//		1. Add a protoype
//			static eCommandResult_T ConsoleCommandVer(const char buffer[]);
//		2. Add the command to mConsoleCommandTable
//		    {"ver", &ConsoleCommandVer, HELP("Get the version string")},
//		3. Implement the function, using ConsoleReceiveParam<Type> to get the parameters from the buffer.
#include <stdio.h>
#include <string.h>
#include "consoleCommands.h"
#include "console.h"
#include "consoleIo.h"
#include "version.h"
#include "../Drivers/i3g4250d.h"
#include "../Drivers/stm32f429i_discovery_gyroscope.h"
#include "../Drivers/barometer.h"
#ifdef MMA8452
#	include "../Drivers/accelerometer.h"
#endif
#include "../Drivers/MMA8652/mma865x_driver.h"
#include "../Drivers/MMA8652/mma865x_regdef.h"
#include "main.h"
#include "circular_buffer.h"

#define IGNORE_UNUSED_VARIABLE(x)  if ( &x == &x ) {}
#define ABS(x)  (x < 0) ? (-x) : x

#define GBAR_LEN 8
#define GDISP_LEN ((GBAR_LEN * 2) + 2)
//Set the full scale for the gyro
#define GYRO_SCALE I3G4250D_FULLSCALE_245

typedef struct _ACC_DATA {
	float x, y, z;
} ACC_DATA;

static eCommandResult_T ConsoleCommandVer(const char buffer[]);
static eCommandResult_T ConsoleCommandHelp(const char buffer[]);
static eCommandResult_T ConsoleCommandGyroPresent(const char buffer[]);
static eCommandResult_T ConsoleCommandGyroTest(const char buffer[]);
static eCommandResult_T ConsoleCommandComment(const char buffer[]);
static eCommandResult_T ConsoleCommandBaroPresent(const char buffer[]);
static eCommandResult_T ConsoleCommandBaroData(const char buffer[]);
static eCommandResult_T ConsoleCommandBaroReset(const char buffer[]);
static eCommandResult_T ConsoleCommandAccPresent(const char buffer[]);
static eCommandResult_T ConsoleCommandAccData(const char buffer[]);
static eCommandResult_T ConsoleCommandAccOrient(const char buffer[]);
static eCommandResult_T ConsoleCommandSimWarn(const char buffer[]);
static eCommandResult_T ConsoleCommandCircBuf(const char buffer[]);

static const sConsoleCommandTable_T mConsoleCommandTable[] =
{
    {";", &ConsoleCommandComment, HELP("Comment! You do need a space after the semicolon. ")},
    {"help", &ConsoleCommandHelp, HELP("Lists the commands available")},
    {"ver", &ConsoleCommandVer, HELP("Get the version string")},
	{"gp", &ConsoleCommandGyroPresent, HELP("Check is gyro present and responding")},
	{"gt", &ConsoleCommandGyroTest, HELP("Test gyro: params 10 - number of seconds to test")},
	{"bp", &ConsoleCommandBaroPresent, HELP("Check is barometer present and responding")},
	{"bd", &ConsoleCommandBaroData, HELP("Get barometer data: params 10 - number of seconds to test")},
	{"br", &ConsoleCommandBaroReset, HELP("Reset barometer")},
	{"ap", &ConsoleCommandAccPresent, HELP("Check is accelerometer present and responding")},
	{"ad", &ConsoleCommandAccData, HELP("Get accelerometer data: params 10 - number of seconds to test")},
	{"ao", &ConsoleCommandAccOrient, HELP("Get accelerometer orientation: params 10 - number of seconds to test")},
	{"sw", &ConsoleCommandSimWarn, HELP("Simulate barometer warning")},
	{"cb", &ConsoleCommandCircBuf, HELP("Output circular buffer")},

	CONSOLE_COMMAND_TABLE_END // must be LAST
};

/**
 * @brief Draws gyro bars in ASCII
 * @param buffer *char Pointer to string which will be output to console
 * @param val float Value to be displayed
 * @param blen uint16_t Length of buffer
 */
static void getAxisBar(char * buffer, float val, uint16_t blen){
    uint8_t lenang = 0;
    uint16_t gyro_scale_divider;
    // calculate the individual display strings
    memset(buffer, 0x00, blen);
    //Xval = -100.00f; // debug
    switch (GYRO_SCALE){
        case I3G4250D_FULLSCALE_245:
            gyro_scale_divider = 245;
            break;
        case I3G4250D_FULLSCALE_500:
            gyro_scale_divider = 500;
            break;
        case I3G4250D_FULLSCALE_2000:
            gyro_scale_divider = 2000;
            break;
        default:
            gyro_scale_divider = 245;
    }

    lenang = (ABS((val/gyro_scale_divider) * GBAR_LEN));
    if (lenang > GBAR_LEN) {
        lenang = GBAR_LEN;
    }
    if (val < 0){
        memset(buffer, ' ', GBAR_LEN - lenang);
        memset(buffer + GBAR_LEN - lenang, '*', lenang);
        memset(buffer + GBAR_LEN, '-', 1);
        memset(buffer + GBAR_LEN + 1, ' ', GBAR_LEN);
    } else {
        if (0 == val){
            memset(buffer, ' ', GBAR_LEN);
            memset(buffer + GBAR_LEN, '0', 1);
            memset(buffer + GBAR_LEN + 1, ' ', GBAR_LEN);
        } else {
            memset(buffer, ' ', GBAR_LEN);
            memset(buffer + GBAR_LEN, '+', 1);
            memset(buffer + GBAR_LEN + 1, '*', lenang);
            memset(buffer + GBAR_LEN + 1 + lenang, ' ', GBAR_LEN - lenang);
        }
    }
}

static eCommandResult_T ConsoleCommandGyroTest(const char buffer[]){
	float Buffer[3];
	float Xval, Yval, Zval = 0x00;
	int16_t tsec;
    char strbuf[100];
    eCommandResult_T result;
    uint32_t endTick = 0;
    char xbuf[GDISP_LEN], ybuf[GDISP_LEN], zbuf[GDISP_LEN];

    result = ConsoleReceiveParamInt16(buffer, 1, &tsec);

    if (COMMAND_SUCCESS == result ){
		if (BSP_GYRO_Init(GYRO_SCALE) == GYRO_OK){
			ConsoleIoSendString("Starting test:\n");
			if (tsec < 1){
				ConsoleIoSendString("Error in duration of test: < 1");
				return COMMAND_PARAMETER_ERROR;
			}
            //function will exit after tsec
			endTick = HAL_GetTick() + (tsec * 1000);

			while(HAL_GetTick() < endTick){

				/* Read Gyro Angular data */
				BSP_GYRO_GetXYZ(Buffer);

				// device is outputting mdps (millidegrees per second)
				// to get DPS we need to divide by 1000
				Xval = (Buffer[0]/1000);
				Yval = (Buffer[1]/1000);
				Zval = (Buffer[2]/1000);
				//Reset the string buffer
				memset(strbuf, 0x00, 100);


				//sprintf(strbuf, "%+06.2f | %+06.2f | %+06.2f\n", Xval, Yval, Zval);
				//ConsoleIoSendString(strbuf);

				getAxisBar(xbuf, Xval, GDISP_LEN);
				getAxisBar(ybuf, Yval, GDISP_LEN);
				getAxisBar(zbuf, Zval, GDISP_LEN);
				sprintf(strbuf, "%s|%s|%s\n", xbuf, ybuf, zbuf);

				ConsoleIoSendString(strbuf);

				// Delay for 10 ms
				HAL_Delay(10);
			}
			ConsoleIoSendString("Test completed\n");
		}
    } else {
    	return COMMAND_ERROR;
    }
	return COMMAND_SUCCESS;
}


static eCommandResult_T ConsoleCommandCircBuf(const char buffer[]){
	uint16_t i;
	uint16_t bdata;
	char strbuf[100];
	int rs;

	ConsoleIoSendString("\r\n************\r\nCircular Buffer:\r\n");
	sprintf(strbuf, "Circular buffer size/capacity %i / %i\n", circular_buf_size(me), circular_buf_capacity(me));
	ConsoleIoSendString(strbuf);

	memset(strbuf, 0x00, 100);


	for (i = 1; i < BAROMETER_BUFFER_SIZE; ++i) {
		rs = circular_buf_peek(me, &bdata, i);
		if (rs == -1){
			break;
		} else {
			//Reset the string buffer
			memset(strbuf, 0x00, 100);
			sprintf(strbuf, "%i - %i\n", i, bdata);
			ConsoleIoSendString(strbuf);
		}
	}
	ConsoleIoSendString("\r\nDone\r\n");

	return COMMAND_SUCCESS;
}

static eCommandResult_T ConsoleCommandSimWarn(const char buffer[]){
	warnShown = true;
	return COMMAND_SUCCESS;
}

/**
 * Testing is the gyro present or not
 * In case that the gyro is present the device will return Gyro OK else Gyro error
 */
static eCommandResult_T ConsoleCommandGyroPresent(const char buffer[]){
	if (BSP_GYRO_Init(GYRO_SCALE) == GYRO_OK){
		ConsoleIoSendString("Gyro OK\n");
	} else {
		ConsoleIoSendString("Gyro Error\n");
	}
	return COMMAND_SUCCESS;
}

static eCommandResult_T ConsoleCommandBaroReset(const char buffer[]){

	stmdev_ctx_t dev_ctx;
	lps28dfw_stat_t status;
	uint32_t endTick = 0;

	dev_ctx = lps28dfw_init();
	ConsoleIoSendString("Resetting Barometer\n");
	lps28dfw_init_set(&dev_ctx, LPS28DFW_BOOT);
	lps28dfw_init_set(&dev_ctx, LPS28DFW_RESET);
	//function will exit after tsec
	endTick = HAL_GetTick() + (3 * 1000);
	/* Read samples in polling mode (no int) */
	do {
		lps28dfw_status_get(&dev_ctx, &status);
		if (HAL_GetTick() > endTick){
			ConsoleIoSendString("Timeout resetting\n");
			break;
		}
	} while (status.sw_reset);

	return COMMAND_SUCCESS;
}

static eCommandResult_T ConsoleCommandAccPresent(const char buffer[]){
	uint8_t buf;
	char strbuf[100];
	mma865x_driver_t I2C;

	I2C.pComHandle = (sensor_comm_handle_t*) &I2cHandle;

	mma865x_init(&I2C);
	mma865x_read_reg(&I2C, MMA865x_WHO_AM_I, 1, &buf);
	if (MMA8652_WHOAMI_VALUE == buf){
		sprintf(strbuf, "Accelerometer initialized\r\n");
	} else {
		sprintf(strbuf, "Accelerometer failed\r\n");
	}
	ConsoleIoSendString(strbuf);

	return COMMAND_SUCCESS;
}

static eCommandResult_T ConsoleCommandAccOrient(const char buffer[]){
	uint8_t buf;
	char strbuf[100];
	mma865x_driver_t I2C;
	int16_t tsec;
	uint32_t endTick = 0;
	uint8_t eventVal;
	eCommandResult_T result;

	I2C.pComHandle = (sensor_comm_handle_t*) &I2cHandle;

	result = ConsoleReceiveParamInt16(buffer, 1, &tsec);
	if (COMMAND_SUCCESS == result ){
		mma865x_init(&I2C);
		mma865x_read_reg(&I2C, MMA865x_WHO_AM_I, 1, &buf);
		mma865x_set_embedded_function(&I2C, MMA865x_ORIENT_DETECTION_MODE);

		endTick = HAL_GetTick() + (tsec * 1000);

		ConsoleIoSendString("\r\n**********\r\nDetecting orientation\r\n");
		while(HAL_GetTick() < endTick)
		{
			memset(strbuf, 0x00, 100);
			if (screen_rotated){
				ConsoleIoSendString("Orientation interrupt\r\n");
				screen_rotated = false;
			}
			mma865x_read_event(&I2C, MMA865x_ORIENTATION, &eventVal);
			switch (eventVal){
			case MMA865x_PORTRAIT_UP:
				sprintf(strbuf, "Portrait up: Real landscape left\r\n");
				ConsoleIoSendString(strbuf);
				break;
			case MMA865x_PORTRAIT_DOWN:
				sprintf(strbuf, "Portrait down: Real landscape right\r\n");
				ConsoleIoSendString(strbuf);
				break;
			case MMA865x_LANDSCAPE_RIGHT:
				sprintf(strbuf, "Landscape right: Real PORTRAIT UP\r\n");
				ConsoleIoSendString(strbuf);
				break;
			case MMA865x_LANDSCAPE_LEFT:
				sprintf(strbuf, "Landscape left: Real PORTRAIT DOWN\r\n");
				ConsoleIoSendString(strbuf);
				break;
			default:
				break;
			}
			HAL_Delay(100);
		}
	}


	ConsoleIoSendString("\r\nDone\r\n**********\r\n");

	return COMMAND_SUCCESS;
}

static void convertAccData(mma865x_data_t data, float *x, float *y, float *z, uint8_t m_scale){
	*x = (float)data.accel[0] / (float)(1 << 11) * (float)(m_scale);
	*y = (float)data.accel[1] / (float)(1 << 11) * (float)(m_scale);
	*z = (float)data.accel[2] / (float)(1 << 11) * (float)(m_scale);
}

/**
 *Testing output of accelerometer
 */
static eCommandResult_T ConsoleCommandAccData(const char buffer[]){
	uint8_t buf;
	char strbuf[100];
	char linebuf[120];
	mma865x_driver_t I2C;
	eCommandResult_T result;
	mma865x_data_t accBuf;
	int16_t tsec;
	uint32_t endTick = 0;
	float x, y, z = 0.0;

	I2C.pComHandle = (sensor_comm_handle_t*) &I2cHandle;

	result = ConsoleReceiveParamInt16(buffer, 1, &tsec);
	if (COMMAND_SUCCESS == result ){

		mma865x_init(&I2C);
		mma865x_read_reg(&I2C, MMA865x_WHO_AM_I, 1, &buf);
		if (MMA8652_WHOAMI_VALUE == buf){
			sprintf(strbuf, "Accelerometer initialized\r\n");
		} else {
			sprintf(strbuf, "Accelerometer failed\r\n");
		}
		ConsoleIoSendString(strbuf);

		mma865x_configure(&I2C, MMA865x_ODR_6P25_HZ, MMA865x_ACCEL_NORMAL, MMA865x_ACCEL_14BIT_READ_POLL_MODE);
		mma865x_write_reg(&I2C, MMA865x_XYZ_DATA_CFG, MMA865x_XYZ_DATA_CFG_FS_2G, (uint8_t *) MMA865x_XYZ_DATA_CFG_FS_MASK);
		endTick = HAL_GetTick() + (tsec * 1000);
		/* Read samples in polling mode (no int) */
		while(HAL_GetTick() < endTick)
		{
			memset(linebuf, 0x00, 120);
			mma865x_read_data(&I2C, MMA865x_ACCEL_14BIT_DATAREAD, &accBuf);
			convertAccData(accBuf, &x, &y, &z, 2);
			sprintf(linebuf, "X:%08X Y:%08X Z:%08X - X:%09.6f Y:%09.6f Z:%09.6f\r\n", accBuf.accel[0], accBuf.accel[1], accBuf.accel[2], x, y, z);
			ConsoleIoSendString(linebuf);
			HAL_Delay(100);
		}
	}
	return COMMAND_SUCCESS;
}

#ifdef MMA8452
/**
 * Testing is accelerometer present
 */
static eCommandResult_T ConsoleCommandAccPresent(const char buffer[]){
	char strbuf[100];
	memset(strbuf, 0, 100);
	stmdevacc_ctx_t dev_ctx;

	dev_ctx = mma8452q_init();

	if (MMA8452Q_init_set(&dev_ctx, SCALE_2G, ODR_12)){
		sprintf(strbuf, "Accelerometer initialized\r\n");
	} else {
		sprintf(strbuf, "Accelerometer failed\r\n");
	}
	ConsoleIoSendString(strbuf);

	return COMMAND_SUCCESS;
}

/**
 *Testing output of accelerometer
 */
static eCommandResult_T ConsoleCommandAccData(const char buffer[]){
	ACC_DATA acdt;
	ACC_RAW_DATA acrwdt;
	int16_t tsec;
	char linebuf[120];
	eCommandResult_T result;
	stmdevacc_ctx_t dev_ctx;
	uint32_t endTick = 0;

	result = ConsoleReceiveParamInt16(buffer, 1, &tsec);
	if (COMMAND_SUCCESS == result ){

		if (tsec < 1){
			ConsoleIoSendString("Error in duration of test: < 1");
			return COMMAND_PARAMETER_ERROR;
		}

		dev_ctx = mma8452q_init();

		if (MMA8452Q_init_set(&dev_ctx, SCALE_4G, ODR_12)){
			sprintf(linebuf, "Accelerometer initialized\r\n");
		} else {
			sprintf(linebuf, "Accelerometer failed\r\n");
			return COMMAND_ERROR;
		}
		ConsoleIoSendString(linebuf);
		//function will exit after tsec
		endTick = HAL_GetTick() + (tsec * 1000);
		/* Read samples in polling mode (no int) */
		while(HAL_GetTick() < endTick)
		{
			memset(linebuf, 0x00, 120);
			if (MMA8452Q_available(&dev_ctx)){
				MMA8452Q_read(&dev_ctx, &acdt, &acrwdt);
				sprintf(linebuf, "X:%05X Y:%05X Z:%05X - X:%09.6f Y:%09.6f Z:%09.6f\r\n", acrwdt.x, acrwdt.y, acrwdt.z, acdt.x, acdt.y, acdt.z);
			}
			ConsoleIoSendString(linebuf);
			HAL_Delay(100);
		}
	}
	return COMMAND_SUCCESS;
}

#endif

/**
 * Testing is the barometer present or not
 * In case that the barometer is present the device will return Barometer OK else Barometer error
 */
static eCommandResult_T ConsoleCommandBaroPresent(const char buffer[]){

	lps28dfw_id_t id;
	stmdev_ctx_t dev_ctx;
	lps28dfw_stat_t status;
	lps28dfw_all_sources_t all_sources;
	char strbuf[100];
	static lps28dfw_data_t data;
	lps28dfw_md_t md;

	id.whoami = 0;

	dev_ctx = lps28dfw_init();
	/* Check device ID */
	lps28dfw_id_get(&dev_ctx, &id);

	if (id.whoami == LPS28DFW_ID){
		ConsoleIoSendString("\r\n**********************************\r\nBarometer OK\n");

//		lps28dfw_init_set(&dev_ctx, LPS28DFW_BOOT);
//		lps28dfw_init_set(&dev_ctx, LPS28DFW_RESET);
		HAL_Delay(100);

		lps28dfw_status_get(&dev_ctx, &status);

		ConsoleIoSendString("Barometer Status\n");
		memset(strbuf, 0, 100);
        sprintf(strbuf, "Restoring configuration registers (reset): %i\r\n", status.sw_reset);
        ConsoleIoSendString(strbuf);
        memset(strbuf, 0, 100);
        sprintf(strbuf, "Restoring calibration parameters (boot): %i\r\n", status.boot);
        ConsoleIoSendString(strbuf);
        memset(strbuf, 0, 100);
        sprintf(strbuf, "Pressure data ready: %i\r\n", status.drdy_pres);
        ConsoleIoSendString(strbuf);
        memset(strbuf, 0, 100);
        sprintf(strbuf, "Pressure data overrun: %i\r\n", status.ovr_pres);
        ConsoleIoSendString(strbuf);
        memset(strbuf, 0, 100);
        sprintf(strbuf, "Temperature data ready: %i\r\n", status.drdy_temp);
        ConsoleIoSendString(strbuf);
        memset(strbuf, 0, 100);
        sprintf(strbuf, "Temperature data overrun: %i\r\n", status.ovr_temp);
        ConsoleIoSendString(strbuf);
        memset(strbuf, 0, 100);
        sprintf(strbuf, "Single measurement is finished: %i\r\n", status.end_meas);
        ConsoleIoSendString(strbuf);
        memset(strbuf, 0, 100);
        sprintf(strbuf, "Auto-Zero value is set: %i\r\n", status.ref_done);
        ConsoleIoSendString(strbuf);

        lps28dfw_all_sources_get(&dev_ctx, &all_sources);

        ConsoleIoSendString("Barometer All Sources Data:\n");
        memset(strbuf, 0, 100);
        sprintf(strbuf, "Pressure data ready: %i\r\n", all_sources.drdy_pres);
        ConsoleIoSendString(strbuf);
        memset(strbuf, 0, 100);
        sprintf(strbuf, "Temperature data ready: %i\r\n", all_sources.drdy_temp);
        ConsoleIoSendString(strbuf);
        memset(strbuf, 0, 100);
        sprintf(strbuf, "Over pressure event: %i\r\n", all_sources.over_pres);
        ConsoleIoSendString(strbuf);
        memset(strbuf, 0, 100);
        sprintf(strbuf, "Under pressure event: %i\r\n", all_sources.under_pres);
        ConsoleIoSendString(strbuf);
        memset(strbuf, 0, 100);
        sprintf(strbuf, "FIFO Full: %i\r\n", all_sources.fifo_full);
        ConsoleIoSendString(strbuf);
        memset(strbuf, 0, 100);
        sprintf(strbuf, "FIFO overrun: %i\r\n", all_sources.fifo_ovr);
        ConsoleIoSendString(strbuf);
        memset(strbuf, 0, 100);
        sprintf(strbuf, "FIFO threshold reached: %i\r\n", all_sources.fifo_th);
        ConsoleIoSendString(strbuf);

        memset(strbuf, 0, 100);
        lps28dfw_data_get(&dev_ctx, &md, &data);
        sprintf(strbuf, "pressure [hPa]:%6.2f temperature [degC]:%6.2f\r\n", data.pressure.hpa, data.heat.deg_c);
        ConsoleIoSendString(strbuf);

	} else {
		ConsoleIoSendString("Barometer Error\n");
		memset(strbuf, 0, 100);
		sprintf(strbuf, "Barometer returning: 0x%X\r\n", id.whoami);
		ConsoleIoSendString(strbuf);

	}
	return COMMAND_SUCCESS;
}

/**
 * Get data from barometer
 */
static eCommandResult_T ConsoleCommandBaroData(const char buffer[]){
	stmdev_ctx_t dev_ctx;
	eCommandResult_T result;
	uint32_t endTick = 0;
	lps28dfw_all_sources_t all_sources;
	lps28dfw_bus_mode_t bus_mode;
	lps28dfw_stat_t status;
	lps28dfw_pin_int_route_t int_route;
	lps28dfw_md_t md;
	char strbuf[100];
	int16_t tsec;
	static lps28dfw_data_t data;

	dev_ctx = lps28dfw_init();

	result = ConsoleReceiveParamInt16(buffer, 1, &tsec);
	if (COMMAND_SUCCESS == result ){

		if (tsec < 1){
			ConsoleIoSendString("Error in duration of test: < 1");
			return COMMAND_PARAMETER_ERROR;
		}

		/* Restore default configuration */
		lps28dfw_init_set(&dev_ctx, LPS28DFW_BOOT);
		lps28dfw_init_set(&dev_ctx, LPS28DFW_RESET);
		do {
			lps28dfw_status_get(&dev_ctx, &status);
		} while (status.sw_reset);


		/* Set bdu and if_inc recommended for driver usage */
		lps28dfw_init_set(&dev_ctx, LPS28DFW_DRV_RDY);

		lps28dfw_fifo_mode_set(&dev_ctx, (lps28dfw_fifo_md_t *) LPS28DFW_STREAM);

		/* Select bus interface */
		bus_mode.filter = LPS28DFW_AUTO;
		bus_mode.interface = LPS28DFW_SEL_BY_HW;
		lps28dfw_bus_mode_set(&dev_ctx, &bus_mode);

		/* Set Output Data Rate */
		md.odr = LPS28DFW_4Hz;
		md.avg = LPS28DFW_4_AVG;
		md.lpf = LPS28DFW_LPF_ODR_DIV_4;
		md.fs = LPS28DFW_1260hPa;
		lps28dfw_mode_set(&dev_ctx, &md);

		/* Configure inerrupt pins */
		lps28dfw_pin_int_route_get(&dev_ctx, &int_route);
		int_route.drdy_pres   = PROPERTY_DISABLE;
		lps28dfw_pin_int_route_set(&dev_ctx, &int_route);

		//function will exit after tsec
		endTick = HAL_GetTick() + (tsec * 1000);
		/* Read samples in polling mode (no int) */
		while(HAL_GetTick() < endTick)
		{
			lps28dfw_all_sources_get(&dev_ctx, &all_sources);
			    if (all_sources.drdy_pres | all_sources.drdy_temp ) {
					lps28dfw_data_get(&dev_ctx, &md, &data);
					sprintf(strbuf, "pressure [hPa]:%6.2f temperature [degC]:%6.2f\r\n", data.pressure.hpa, data.heat.deg_c);
					ConsoleIoSendString(strbuf);
			    } else {
			    	sprintf(strbuf, "pressure data:%i | temperature data:%i\r\n", all_sources.drdy_pres, all_sources.drdy_temp);
			    	ConsoleIoSendString(strbuf);
			    }
			// Delay for 500 ms
			HAL_Delay(500);
		}

		return COMMAND_SUCCESS;
	}

	return COMMAND_ERROR;
}

static eCommandResult_T ConsoleCommandComment(const char buffer[])
{
	// do nothing
	IGNORE_UNUSED_VARIABLE(buffer);
	return COMMAND_SUCCESS;
}

static eCommandResult_T ConsoleCommandHelp(const char buffer[])
{
	uint32_t i;
	uint32_t tableLength;
	eCommandResult_T result = COMMAND_SUCCESS;

    IGNORE_UNUSED_VARIABLE(buffer);

	tableLength = sizeof(mConsoleCommandTable) / sizeof(mConsoleCommandTable[0]);
	for ( i = 0u ; i < tableLength - 1u ; i++ )
	{
		ConsoleIoSendString(mConsoleCommandTable[i].name);
#if CONSOLE_COMMAND_MAX_HELP_LENGTH > 0
		ConsoleIoSendString(" : ");
		ConsoleIoSendString(mConsoleCommandTable[i].help);
#endif // CONSOLE_COMMAND_MAX_HELP_LENGTH > 0
		ConsoleIoSendString(STR_ENDLINE);
	}
	return result;
}

static eCommandResult_T ConsoleCommandVer(const char buffer[])
{
	eCommandResult_T result = COMMAND_SUCCESS;

    IGNORE_UNUSED_VARIABLE(buffer);

	ConsoleIoSendString(VERSION_STRING);
	ConsoleIoSendString(STR_ENDLINE);
	return result;
}


const sConsoleCommandTable_T* ConsoleCommandsGetTable(void)
{
	return (mConsoleCommandTable);
}


