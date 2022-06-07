/*
  ******************************************************************************
  * @file    mma8452q.h
  * @author  Tomislav Darlic
  * @brief   Adapted from Sparkfun MMA8452Q Arduino Library
  * 		 https://github.com/sparkfun/SparkFun_MMA8452Q_Arduino_Library
  ******************************************************************************/

#include "mma8452q.h"

static uint8_t m_deviceAddress;

MMA8452Q_Scale m_scale;
static uint16_t x, y, z;
static float cx, cy, cz;


/**
  * @defgroup    LPS28DFW
  * @brief       This file provides a set of functions needed to drive the
  *              lps28dfw nano pressure sensor.
  * @{
  *
  */

/**
  * @defgroup    Interfaces_Functions
  * @brief       This section provide a set of functions used to read and
  *              write a generic register of the device.
  *              MANDATORY: return 0 -> no Error.
  * @{
  *
  */

/**
  * @brief  Read generic device register
  *
  * @param  ctx   read / write interface definitions(ptr)
  * @param  reg   register to read
  * @param  data  pointer to buffer that store the data read(ptr)
  * @param  len   number of consecutive register to read
  * @retval       interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t MMA8452Q_read_reg(stmdevacc_ctx_t *ctx, uint8_t reg, uint8_t *data, uint16_t len)
{
  int32_t ret;
  ret = ctx->read_reg(ctx->handle, reg, data, len);
  return ret;
}

/**
  * @brief  Write generic device register
  *
  * @param  ctx   read / write interface definitions(ptr)
  * @param  reg   register to write
  * @param  data  pointer to data to write in register reg(ptr)
  * @param  len   number of consecutive register to write
  * @retval       interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t MMA8452Q_write_reg(stmdevacc_ctx_t *ctx, uint8_t reg, uint8_t *data, uint16_t len)
{
  int32_t ret;
  ret = ctx->write_reg(ctx->handle, reg, data, len);
  return ret;
}

/**
  * @}
  *
  */



// BEGIN INITIALIZATION (New Implementation of Init)
// 	This will be used instead of init in future sketches
// 	to match Arudino guidelines. We will maintain init
// 	for backwards compatability purposes.
bool MMA8452Q_init(stmdevacc_ctx_t *ctx, uint8_t deviceAddress, MMA8452Q_Scale fsr, MMA8452Q_ODR odr)
{
    m_deviceAddress = deviceAddress;
    m_scale = fsr;

    uint8_t tmp;
    if (!MMA8452Q_readRegister(ctx, WHO_AM_I, &tmp) || tmp != 0x2A)
        return false;

    MMA8452Q_standby(); // Must be in standby to change registers

    MMA8452Q_setScale(m_scale);  // Set up accelerometer scale
    MMA8452Q_setDataRate(odr); // Set up output data rate
    MMA8452Q_setupPL();		  // Set up portrait/landscape detection

    // Multiply parameter by 0.0625g to calculate threshold.
    MMA8452Q_setupTap(0x80, 0x80, 0x08); // Disable x, y, set z to 0.5g

    MMA8452Q_setActive(); // Set to active to start reading
    return true;
}

// GET FUNCTIONS FOR RAW ACCELERATION DATA
// Returns raw X acceleration data
uint16_t MMA8452Q_getX()
{
    uint8_t rawData[2];
    MMA8452Q_readRegisters(OUT_X_MSB, rawData, 2); // Read the X data into a data array
    return (((uint16_t)(rawData[0] << 8 | rawData[1])) >> 4);
}

// Returns raw Y acceleration data
uint16_t MMA8452Q_getY()
{
    uint8_t rawData[2];
    MMA8452Q_readRegisters(OUT_Y_MSB, rawData, 2); // Read the Y data into a data array
    return (((uint16_t)(rawData[0] << 8 | rawData[1])) >> 4);
}

// Returns raw Z acceleration data
uint16_t MMA8452Q_getZ()
{
    uint8_t rawData[2];
    MMA8452Q_readRegisters(OUT_Z_MSB, rawData, 2); // Read the Z data into a data array
    return (((uint16_t)(rawData[0] << 8 | rawData[1])) >> 4);
}

// GET FUNCTIONS FOR CALCULATED ACCELERATION DATA
// Returns calculated X acceleration data
float MMA8452Q_getCalculatedX()
{
    x = MMA8452Q_getX();
    return (float)x / (float)(1 << 11) * (float)(m_scale);
}

// Returns calculated Y acceleration data
float MMA8452Q_getCalculatedY()
{
    y = MMA8452Q_getY();
    return ((float)y / (float)(1 << 11) * (float)(m_scale));
}

// Returns calculated Z acceleration data
float MMA8452Q_getCalculatedZ()
{
    z = MMA8452Q_getZ();
    return ((float)z / (float)(1 << 11) * (float)(m_scale));
}

// READ ACCELERATION DATA
//  This function will read the acceleration values from the MMA8452Q. After
//	reading, it will update two triplets of variables:
//		* int's x, y, and z will store the signed 12-bit values read out
//		  of the acceleromter.
//		* floats cx, cy, and cz will store the calculated acceleration from
//		  those 12-bit values. These variables are in units of g's.
void MMA8452Q_read()
{
    uint8_t rawData[6]; // x/y/z accel register data stored here
    MMA8452Q_readRegisters(OUT_X_MSB, rawData, 6); // Read the six raw data registers into data array

    x = ((uint16_t)(rawData[0] << 8 | rawData[1])) >> 4;
    y = ((uint16_t)(rawData[2] << 8 | rawData[3])) >> 4;
    z = ((uint16_t)(rawData[4] << 8 | rawData[5])) >> 4;
    cx = (float)x / (float)(1 << 11) * (float)(m_scale);
    cy = (float)y / (float)(1 << 11) * (float)(m_scale);
    cz = (float)z / (float)(1 << 11) * (float)(m_scale);
}

// CHECK IF NEW DATA IS AVAILABLE
//	This function checks the status of the MMA8452Q to see if new data is availble.
//	returns 0 if no new data is present, or a 1 if new data is available.
uint8_t MMA8452Q_available()
{
    uint8_t tmp;
    MMA8452Q_readRegister(STATUS_MMA8452Q, &tmp);
    return ((tmp & 0x08) >> 3);
}

// SET FULL-SCALE RANGE
//	This function sets the full-scale range of the x, y, and z axis accelerometers.
//	Possible values for the fsr variable are SCALE_2G, SCALE_4G, or SCALE_8G.
void MMA8452Q_setScale(MMA8452Q_Scale fsr)
{
    // Must be in standby mode to make changes!!!
    // Change to standby if currently in active state
    if (MMA8452Q_isActive() == true)
        MMA8452Q_standby();

    uint8_t cfg;
    if (!MMA8452Q_readRegister(XYZ_DATA_CFG, &cfg))
        return;
    cfg &= 0xFC;	   // Mask out scale bits
    cfg |= (fsr >> 2); // Neat trick, see page 22. 00 = 2G, 01 = 4A, 10 = 8G
    MMA8452Q_writeRegister(XYZ_DATA_CFG, cfg);

    // Return to active state when done
    // Must be in active state to read data
    MMA8452Q_setActive();

    m_scale = fsr;
}

// SET THE OUTPUT DATA RATE
//	This function sets the output data rate of the MMA8452Q.
//	Possible values for the odr parameter are: ODR_800, ODR_400, ODR_200,
//	ODR_100, ODR_50, ODR_12, ODR_6, or ODR_1
void MMA8452Q_setDataRate(MMA8452Q_ODR odr)
{
    // Must be in standby mode to make changes!!!
    // Change to standby if currently in active state
    if (MMA8452Q_isActive() == true)
        MMA8452Q_standby();

    uint8_t ctrl;
    MMA8452Q_readRegister(CTRL_REG1, &ctrl);
    ctrl &= 0xC7; // Mask out data rate bits
    ctrl |= (odr << 3);
    MMA8452Q_writeRegister(CTRL_REG1, ctrl);

    // Return to active state when done
    // Must be in active state to read data
    MMA8452Q_setActive();
}

// SET UP TAP DETECTION
//	This function can set up tap detection on the x, y, and/or z axes.
//	The xThs, yThs, and zThs parameters serve two functions:
//		1. Enable tap detection on an axis. If the 7th bit is SET (0x80)
//			tap detection on that axis will be DISABLED.
//		2. Set tap g's threshold. The lower 7 bits will set the tap threshold
//			on that axis.
void MMA8452Q_setupTap(uint8_t xThs, uint8_t yThs, uint8_t zThs)
{
    // Must be in standby mode to make changes!!!
    // Change to standby if currently in active state
    if (MMA8452Q_isActive() == true)
        MMA8452Q_standby();

    // Set up single and double tap - 5 steps:
    // for more info check out this app note:
    // http://cache.freescale.com/files/sensors/doc/app_note/AN4072.pdf
    // Set the threshold - minimum required acceleration to cause a tap.
    uint8_t temp = 0;
    // If top bit ISN'T set
    if (!(xThs & 0x80)) {
        temp |= 0x3;					 // Enable taps on x
        MMA8452Q_writeRegister(PULSE_THSX, xThs); // x thresh
    }

    if (!(yThs & 0x80)) {
        temp |= 0xC;					 // Enable taps on y
        MMA8452Q_writeRegister(PULSE_THSY, yThs); // y thresh
    }

    if (!(zThs & 0x80)) {
        temp |= 0x30;					 // Enable taps on z
        MMA8452Q_writeRegister(PULSE_THSZ, zThs); // z thresh
    }
    // Set up single and/or double tap detection on each axis individually.
    MMA8452Q_writeRegister(PULSE_CFG, temp | 0x40);
    // Set the time limit - the maximum time that a tap can be above the thresh
    MMA8452Q_writeRegister(PULSE_TMLT, 0x30); // 30ms time limit at 800Hz odr
    // Set the pulse latency - the minimum required time between pulses
    MMA8452Q_writeRegister(PULSE_LTCY, 0xA0); // 200ms (at 800Hz odr) between taps min
    // Set the second pulse window - maximum allowed time between end of
    //	latency and start of second pulse
    MMA8452Q_writeRegister(PULSE_WIND, 0xFF); // 5. 318ms (max value) between taps max

    // Return to active state when done
    // Must be in active state to read data
    MMA8452Q_setActive();
}

// READ TAP STATUS
//	This function returns any taps read by the MMA8452Q. If the function
//	returns no new taps were detected. Otherwise the function will return the
//	lower 7 bits of the PULSE_SRC register.
uint8_t MMA8452Q_readTap()
{
    uint8_t tapStat;
    if (MMA8452Q_readRegister(PULSE_SRC, &tapStat)) {
        if (tapStat & 0x80) // Read EA bit to check if a interrupt was generated
            return tapStat & 0x7F;
        else
            return 0;
    }
    return 0;
}

// SET UP PORTRAIT/LANDSCAPE DETECTION
//	This function sets up portrait and landscape detection.
void MMA8452Q_setupPL()
{
    // Must be in standby mode to make changes!!!
    // Change to standby if currently in active state
    if (MMA8452Q_isActive())
        MMA8452Q_standby();

    // For more info check out this app note:
    //	http://cache.freescale.com/files/sensors/doc/app_note/AN4068.pdf
    // 1. Enable P/L
    uint8_t pl;
    MMA8452Q_readRegister(PL_CFG, &pl);
    MMA8452Q_writeRegister(PL_CFG, pl | 0x40); // Set PL_EN (enable)
    // 2. Set the debounce rate
    MMA8452Q_writeRegister(PL_COUNT, 0x50); // Debounce counter at 100ms (at 800 hz)

    // Return to active state when done
    // Must be in active state to read data
    MMA8452Q_setActive();
}

// READ PORTRAIT/LANDSCAPE STATUS
//	This function reads the portrait/landscape status register of the MMA8452Q.
//	It will return either PORTRAIT_U, PORTRAIT_D, LANDSCAPE_R, LANDSCAPE_L,
//	or LOCKOUT. LOCKOUT indicates that the sensor is in neither p or ls.
uint8_t MMA8452Q_readPL()
{
    uint8_t plStat;
    if (MMA8452Q_readRegister(PL_STATUS, &plStat)) {
        if (plStat & 0x40) // Z-tilt lockout
            return LOCKOUT;
        else // Otherwise return LAPO status
            return (plStat & 0x6) >> 1;
    }
    return 0;
}

// CHECK FOR ORIENTATION
bool MMA8452Q_isRight()
{
    return (MMA8452Q_readPL() == LANDSCAPE_R);
}

bool MMA8452Q_isLeft()
{
    return (MMA8452Q_readPL() == LANDSCAPE_L);
}

bool MMA8452Q_isUp()
{
    return (MMA8452Q_readPL() == PORTRAIT_U);
}

bool MMA8452Q_isDown()
{
    return (MMA8452Q_readPL() == PORTRAIT_D);
}

bool MMA8452Q_isFlat()
{
    if (MMA8452Q_readPL() == LOCKOUT)
        return true;
    return false;
}

// SET STANDBY MODE
//	Sets the MMA8452 to standby mode. It must be in standby to change most register settings
void MMA8452Q_standby()
{
    uint8_t c;
    if (MMA8452Q_readRegister(CTRL_REG1, &c))
        MMA8452Q_writeRegister(CTRL_REG1, c & ~(0x01)); //Clear the active bit to go into standby
}

// SET ACTIVE MODE
//	Sets the MMA8452 to active mode. Needs to be in this mode to output data
void MMA8452Q_setActive()
{
    uint8_t c;
    if (MMA8452Q_readRegister(CTRL_REG1, &c))
        MMA8452Q_writeRegister(CTRL_REG1, c | 0x01); //Set the active bit to begin detection
}

// CHECK STATE (ACTIVE or STANDBY)
//	Returns true if in Active State, otherwise return false
bool MMA8452Q_isActive()
{
    uint8_t currentState;
    if (MMA8452Q_readRegister(SYSMOD, &currentState)) {
        return ((currentState & 0b11) == SYSMOD_STANDBY);
    }
    return false;
}

// WRITE A SINGLE REGISTER
// 	Write a single uint8_t of data to a register in the MMA8452Q.
bool MMA8452Q_writeRegister(stmdevacc_ctx_t * ctx, MMA8452Q_Register regAddress, uint8_t data)
{
    int32_t ret;
    ret = ctx->write_reg(ctx->handle, regAddress, data, 1);
    return true;
}

// READ A SINGLE REGISTER
//	Read a uint8_t from the MMA8452Q register "reg".
bool MMA8452Q_readRegister(stmdevacc_ctx_t * ctx, MMA8452Q_Register regAddress, uint8_t *dest)
{
	int32_t ret;
	ret = ctx->read_reg(ctx->handle, regAddress, dest, 1);
	return true;
}

// READ MULTIPLE REGISTERS
//	Read "len" uint8_ts from the MMA8452Q, starting at register "reg". uint8_ts are stored
//	in "buffer" on exit.
bool MMA8452Q_readRegisters(stmdevacc_ctx_t * ctx, MMA8452Q_Register regAddress, uint8_t *buffer, uint8_t len)
{
	int32_t ret;
	ret = ctx->read_reg(ctx->handle, regAddress, buffer, len);
	return true;
}
