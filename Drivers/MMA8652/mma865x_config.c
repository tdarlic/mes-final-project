/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file mma865x_config.c
 * @brief The mma865x_driver_config.c file contains definitions for MMA865x sensor configurations.
 * @see   https://cache.freescale.com/files/sensors/doc/data_sheet/MMA8652FC.pdf
          https://cache.freescale.com/files/sensors/doc/data_sheet/MMA8653FC.pdf
 */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "mma865x_driver.h"
#include "mma865x_config.h"
#include "common/sensor_common.h"

/*-------------------------------------------------------------------
//Macros
-------------------------------------------------------------------*/

/*! @def    MMA865x FF_MT freefall & Pulse detection counter register threshold values
 *  @brief  MMA865x FF_MT freefall & Pulse detection counter register threshold values.
 *          These values have been derived based on the Application Note AN4070(similar
 *          threshold values are applied for MMA865x).
 *  @see    http://cache.freescale.com/files/sensors/doc/app_note/AN4070.pdf
 */
#define FF_FF_MT_THS  0x18  /* FreeFall Threshold Value. */
#define MT_FF_MT_THS  0x15  /* Motion Threshold Value. */
#define FF_MT_COUNT   0x08  /* Freefall/motion debounce count value. */
#define PL_COUNT      0x15  /* Debounce count value. */
#define ASLP_COUNTER  0x07  /* Auto Sleep after ~5s. */

#define PULSE_THX     0x28  /* X-axis pulse threshold value. */
#define PULSE_THY     0x28  /* Y-axis pulse threshold value. */
#define PULSE_THZ     0x28  /* Z-axis pulse threshold value. */
#define PULSE_TL      0x30  /* Time limit value for pulse detection. */
#define PULSE_LT      0x50  /* Latency time for second pulse detection. */
#define PULSE_WT      0x78  /* Window time for second pulse detection. */

//-----------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------
/*! --------------------------------------------------------------------------*/
/*! MMA865x Register Write List for various modes                            */
/*! --------------------------------------------------------------------------*/

/*! @brief Register Configuration to put MMA865x sensor in operating mode as Active */
const registerwritelist_t gMma865xActiveModeConfig[] = {
    /*! Set MMA865x CTRL_REG1 Register "active" bit-field to put sensor in Active mode. */
    {MMA865x_CTRL_REG1, MMA865x_CTRL_REG1_ACTIVE_ACTIVATED, MMA865x_CTRL_REG1_ACTIVE_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to put MMA865x sensor in operating mode as STANDBY */
const registerwritelist_t gMma865xStandbyModeConfig[] = {
    /*! Reset MMA865x CTRL_REG1 Register "active" bit-field to put sensor in StandBy mode. */
    {MMA865x_CTRL_REG1, MMA865x_CTRL_REG1_ACTIVE_STANDBY, MMA865x_CTRL_REG1_ACTIVE_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure MMA865x for reading Accel samples in Interrupt mode */
const registerwritelist_t gMma865xAccelInterruptConfig[] = {
    /*! Configure the MMA865x to set FS Range as 2g. */
    {MMA865x_XYZ_DATA_CFG, MMA865x_XYZ_DATA_CFG_FS_2G, MMA865x_XYZ_DATA_CFG_FS_MASK},
    /*! Configure the MMA865x to set ODR to 6.25Hz. */
    {MMA865x_CTRL_REG1, MMA865x_CTRL_REG1_DR_6_25HZ, MMA865x_CTRL_REG1_DR_MASK},
    /*! Configure the MMA865x to set High Resolution mode. */
    {MMA865x_CTRL_REG2, MMA865x_CTRL_REG2_MODS_HR, MMA865x_CTRL_REG2_MODS_MASK},
    /*! Configure the MMA865x to set interrupt polarity as Active High. */
    {MMA865x_CTRL_REG3, MMA865x_CTRL_REG3_IPOL_ACTIVE_HIGH, MMA865x_CTRL_REG3_IPOL_MASK},
    /*! Configure the MMA865x to enable Interrupts for Data Ready. */
    {MMA865x_CTRL_REG4, MMA865x_CTRL_REG4_INT_EN_DRDY_EN, MMA865x_CTRL_REG4_INT_EN_DRDY_MASK},
    /*! Configure the MMA865x to route Data Ready Interrupts to INT1. */
    {MMA865x_CTRL_REG5, MMA865x_CTRL_REG5_INT_CFG_DRDY_INT1, MMA865x_CTRL_REG5_INT_CFG_DRDY_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure MMA865x for reading Accel 8-bit samples in Polling mode */
const registerwritelist_t gMma865x8bitAccelPollConfig[] = {
    /*! Configure the MMA865x to set FS Range as 2g. */
    {MMA865x_XYZ_DATA_CFG, MMA865x_XYZ_DATA_CFG_FS_2G, MMA865x_XYZ_DATA_CFG_FS_MASK},
    /*! Configure the MMA865x to set ODR to 6.25Hz. */
    {MMA865x_CTRL_REG1, MMA865x_CTRL_REG1_DR_6_25HZ, MMA865x_CTRL_REG1_DR_MASK},
    /*! Configure the MMA865x to set High Resolution mode. */
    {MMA865x_CTRL_REG2, MMA865x_CTRL_REG2_MODS_HR, MMA865x_CTRL_REG2_MODS_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure MMA865x for reading Accel samples in Polling mode */
const registerwritelist_t gMma865xAccelPollConfig[] = {
    /*! Configure the MMA865x to set FS Range as 2g. */
    {MMA865x_XYZ_DATA_CFG, MMA865x_XYZ_DATA_CFG_FS_2G, MMA865x_XYZ_DATA_CFG_FS_MASK},
    /*! Configure the MMA865x to set ODR to 6.25Hz. */
    {MMA865x_CTRL_REG1, MMA865x_CTRL_REG1_DR_6_25HZ, MMA865x_CTRL_REG1_DR_MASK},
    /*! Configure the MMA865x to set High Resolution mode. */
    {MMA865x_CTRL_REG2, MMA865x_CTRL_REG2_MODS_HR, MMA865x_CTRL_REG2_MODS_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure MMA865x for Accel samples in Fifo mode */
const registerwritelist_t gMma865xAccelFifoConfig[] = {
	/*! Configure the MMA865x to enable FIFO Mode and set Watermark. */
	{MMA865x_F_SETUP, MMA865x_F_SETUP_F_MODE_STOP_MODE | FIFO_SIZE,
	 MMA865x_F_SETUP_F_MODE_MASK | MMA865x_F_SETUP_F_WMRK_MASK},
	/*! Configure the MMA865x to set FS Range as 2g. */
	{MMA865x_XYZ_DATA_CFG, MMA865x_XYZ_DATA_CFG_FS_2G, MMA865x_XYZ_DATA_CFG_FS_MASK},
	/*! Configure the MMA865x to set ODR to 100Hz. */
	{MMA865x_CTRL_REG1, MMA865x_CTRL_REG1_DR_100HZ, MMA865x_CTRL_REG1_DR_MASK},
	/*! Configure the MMA865x to set High Resolution mode. */
	{MMA865x_CTRL_REG2, MMA865x_CTRL_REG2_MODS_HR, MMA865x_CTRL_REG2_MODS_MASK},
	__END_WRITE_DATA__};

/*! @brief Register Configuration to configure MMA865x for Orientation-detect Mode. */
const registerwritelist_t gMma865xOrientDetectConfig[] = {
    {MMA865x_PL_CFG, MMA865x_PL_CFG_DBCNTM_CLEAR | MMA865x_PL_CFG_PL_EN_EN,
     MMA865x_PL_CFG_PL_EN_MASK | MMA865x_PL_CFG_DBCNTM_MASK},
    {MMA865x_CTRL_REG4, MMA865x_CTRL_REG4_INT_EN_LNDPRT_EN | MMA865x_CTRL_REG4_INT_EN_ASLP_EN,
     MMA865x_CTRL_REG4_INT_EN_LNDPRT_MASK | MMA865x_CTRL_REG4_INT_EN_ASLP_MASK},
    {MMA865x_CTRL_REG3, MMA865x_CTRL_REG3_WAKE_LNDPRT_EN, MMA865x_CTRL_REG3_WAKE_LNDPRT_MASK},
    {MMA865x_CTRL_REG2, MMA865x_CTRL_REG2_SLPE_EN, MMA865x_CTRL_REG2_SLPE_MASK},
    {MMA865x_CTRL_REG1, MMA865x_CTRL_REG1_DR_6_25HZ | MMA865x_CTRL_REG1_ACTIVE_ACTIVATED,
     MMA865x_CTRL_REG1_DR_MASK | MMA865x_CTRL_REG1_ACTIVE_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure MMA865x for Motion-detect Mode. */
const registerwritelist_t gMma865xMotiontDetectConfig[] = {
    {MMA865x_FF_MT_THS, MT_FF_MT_THS, MMA865x_FF_MT_THS_THS_MASK}, /* Threshold */
    {MMA865x_FF_MT_CFG, MMA865x_FF_MT_CFG_OAE_MOTION | MMA865x_FF_MT_CFG_ZEFE_EN |
        MMA865x_FF_MT_CFG_YEFE_EN | MMA865x_FF_MT_CFG_XEFE_EN,
     MMA865x_FF_MT_CFG_OAE_MASK | MMA865x_FF_MT_CFG_ZEFE_MASK | MMA865x_FF_MT_CFG_YEFE_MASK |
        MMA865x_FF_MT_CFG_XEFE_MASK},
    {MMA865x_CTRL_REG4, MMA865x_CTRL_REG4_INT_EN_FF_MT_EN, MMA865x_CTRL_REG4_INT_EN_FF_MT_MASK},
    {MMA865x_CTRL_REG1, MMA865x_CTRL_REG1_DR_6_25HZ | MMA865x_CTRL_REG1_ACTIVE_ACTIVATED,
     MMA865x_CTRL_REG1_DR_MASK | MMA865x_CTRL_REG1_ACTIVE_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure MMA865x for Free-fall Mode. */
const registerwritelist_t gMma865xFreefallDetectConfig[] = {
    {MMA865x_FF_MT_COUNT, FF_MT_COUNT, 0}, /* Debounce Counter */
    {MMA865x_FF_MT_THS, FF_FF_MT_THS | MMA865x_FF_MT_THS_DBCNTM_MASK,
     MMA865x_FF_MT_THS_THS_MASK | MMA865x_FF_MT_THS_DBCNTM_MASK}, /* Threshold */
    {MMA865x_FF_MT_CFG,
     MMA865x_FF_MT_CFG_OAE_FREEFALL | MMA865x_FF_MT_CFG_ELE_EN | MMA865x_FF_MT_CFG_ZEFE_EN |
	 MMA865x_FF_MT_CFG_YEFE_EN | MMA865x_FF_MT_CFG_XEFE_EN,
     MMA865x_FF_MT_CFG_OAE_MASK | MMA865x_FF_MT_CFG_ELE_MASK | MMA865x_FF_MT_CFG_ZEFE_MASK |
         MMA865x_FF_MT_CFG_YEFE_MASK | MMA865x_FF_MT_CFG_XEFE_MASK},
    {MMA865x_CTRL_REG4, MMA865x_CTRL_REG4_INT_EN_FF_MT_EN, MMA865x_CTRL_REG4_INT_EN_FF_MT_MASK},
    {MMA865x_CTRL_REG1, MMA865x_CTRL_REG1_DR_100HZ, MMA865x_CTRL_REG1_DR_MASK},
    __END_WRITE_DATA__};

/*! @brief Register Configuration to configure MMA865x for Double-tap pulse detection Mode. */
const registerwritelist_t gMma865xDoubleTapDetectConfig[] = {
    /*! HP_FILTER_CUTOFF to be set to 0x00 */
    {MMA865x_HP_FILTER_CUTOFF, MMA865x_HP_FILTER_CUTOFF_PULSE_HPF_BYP_ENABLED, MMA865x_HP_FILTER_CUTOFF_PULSE_HPF_BYP_MASK},
    /*! Enable double-pulse event on X, Y & Z axis */
    {MMA865x_PULSE_CFG, MMA865x_PULSE_CFG_XDPEFE_EN | MMA865x_PULSE_CFG_YDPEFE_EN | MMA865x_PULSE_CFG_ZDPEFE_EN,
    		MMA865x_PULSE_CFG_XDPEFE_MASK | MMA865x_PULSE_CFG_YDPEFE_MASK | MMA865x_PULSE_CFG_ZDPEFE_MASK},
     /*! Set thresholds to be used by the system to start the pulse-event detection procedure */
    {MMA865x_PULSE_THSX, PULSE_THX, MMA865x_PULSE_THSX_THSX_MASK},
    {MMA865x_PULSE_THSY, PULSE_THY, MMA865x_PULSE_THSY_THSY_MASK},
    {MMA865x_PULSE_THSZ, PULSE_THZ, MMA865x_PULSE_THSZ_THSZ_MASK},
     /*! Set Pulse time limit threshold to PULSE_TL */
    {MMA865x_PULSE_TMLT, PULSE_TL, 0},
     /*! Set Pulse latency time threshold to PULSE_LT */
    {MMA865x_PULSE_LTCY, PULSE_LT, 0},
     /*! Set Pulse window time threshold to PULSE_WT */
    {MMA865x_PULSE_WIND, PULSE_WT, 0},
    /*! Configure MMA865x in standby mode, normal read and noise mode, 800Hz ODR and auto-wake frequency of 50Hz */
    {MMA865x_CTRL_REG1, MMA865x_CTRL_REG1_ACTIVE_STANDBY | MMA865x_CTRL_REG1_F_READ_NORMAL | MMA865x_CTRL_REG1_DR_800HZ | MMA865x_CTRL_REG1_ASLP_RATE_50HZ,
     MMA865x_CTRL_REG1_ACTIVE_MASK | MMA865x_CTRL_REG1_F_READ_MASK | MMA865x_CTRL_REG1_DR_MASK | MMA865x_CTRL_REG1_ASLP_RATE_MASK},
    /*! Configure MMA865x in low power wake mode */
    {MMA865x_CTRL_REG2, MMA865x_CTRL_REG2_MODS_LP, MMA865x_CTRL_REG2_MODS_MASK},
    __END_WRITE_DATA__};

/*! --------------------------------------------------------------------------*/
/*! MMA865x Register Read List                                               */
/*! --------------------------------------------------------------------------*/

/*! @brief Read MMA865x STATUS Register */
const registerreadlist_t gMma865xReadStatus[] = {
	{.readFrom = MMA865x_STATUS, .numBytes = 1},
	__END_READ_DATA__};

/*! @brief Read register list for MMA865x to read Accel 14bit samples */
const registerreadlist_t gMma865xReadAccel[] = {
	{.readFrom = MMA865x_OUT_X_MSB, .numBytes = MMA865x_ACCEL_DATA_SIZE},
    __END_READ_DATA__};

/*! @brief Read register list for FXOS8700 to read Accel 14bit samples */
const registerreadlist_t gMma865xReadAccelFifo[] = {
	{.readFrom = MMA865x_OUT_X_MSB, .numBytes = MMA865x_ACCEL_DATA_SIZE * FIFO_SIZE},
    __END_READ_DATA__};

/*! Prepare the register read list to read the FIFO Status from MMA865x. */
const registerreadlist_t gMma865xFifoStatus[] = {
	{.readFrom = MMA865x_F_STATUS, .numBytes = 1},
	__END_READ_DATA__};

/*! @brief Read register list for MMA865x to read Accel 8bit samples */
const registerreadlist_t gMma865xReadAccel8bit[] = {
	{.readFrom = MMA865x_OUT_X_MSB, .numBytes = MMA865x_ACCEL_8BITDATA_SIZE},
    __END_READ_DATA__};

/*! @brief Read register list for MMA865x to read Free-Fall/Motion Status Register. */
const registerreadlist_t gMma865xReadFFMTSrc[] = {
	{.readFrom = MMA865x_FF_MT_SRC, .numBytes = 1},
	__END_READ_DATA__};

/*! @brief Read register list for MMA865x to read Interrupt Source Register. */
const registerreadlist_t gMma865xReadINTSrc[] = {
	{.readFrom = MMA865x_INT_SOURCE, .numBytes = 1},
	__END_READ_DATA__};

/*! @brief Read register list for MMA865x to read Portrait/Landscape Status Register. */
const registerreadlist_t gMma865xReadPLStatus[] = {
	{.readFrom = MMA865x_PL_STATUS, .numBytes = 1},
	__END_READ_DATA__};

/*! @brief Read register list for MMA865x to read status bits for the pulse detection function. */
const registerreadlist_t gMma865xReadPulseSrc[] = {
	{.readFrom = MMA865x_PULSE_SRC, .numBytes = 1},
	__END_READ_DATA__};
