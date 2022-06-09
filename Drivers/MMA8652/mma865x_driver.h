/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file  mma865x_driver.h
 * @brief This header contains definitions and metadata required for sensor interface module
 *        for MMA865x
*/
#ifndef MMA865x_DRIVER_H_
#define MMA865x_DRIVER_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "mma865x_regdef.h"
#include "common/sensor_common.h"
#include "common/sensor_comm.h"
#include "mma865x_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define NUM_AXES                      (3U)
#define MMA865x_ACCEL_DATA_SIZE       (6U)   /* 2 byte X,Y,Z ACCEL */
#define MMA865x_ACCEL_8BITDATA_SIZE   (3U)   /* 1 byte (MSB) X,Y,Z ACCEL */

/*******************************************************************************
 * Typedefs
 ******************************************************************************/

/*!
 * @brief MMA865x Sensor Embedded Functionality Configurations
 * @see   Section#5: https://cache.freescale.com/files/sensors/doc/data_sheet/MMA8652FC.pdf
 */
typedef enum mma865x_embedded_func_config_type
{
    /*!< Auto-wake/sleep mode */
	MMA865x_AUTOWAKE_SLEEP             = 1U,  /*!< MMA865x Register Configuration to configure sensor for Auto-wake/sleep mode. */

    /*!< Accelerometer freefall and motion event detection */
    MMA865x_FREEFALL_DETECTION_MODE    = 2U, /*!< MMA865x Register Configuration to configure sensor for Freefall detection mode. */
    MMA865x_MOTION_DETECTION_MODE      = 3U, /*!< MMA865x Register Configuration to configure sensor for Motion detectionmode. */

    /*!< Transient detection */
    MMA865x_TRANSIENT_DETECTION_MODE   = 4U, /*!< MMA865x Register Configuration to configure sensor for Transient detection mode. */

    /*!< Pulse/Tap detection */
    MMA865x_PULSE_DETECTION_MODE       = 5U, /*!< MMA865x Register Configuration to configure sensor for Pulse detection mode. */

    /*!< Orientation detection */
    MMA865x_ORIENT_DETECTION_MODE      = 6U, /*!< MMA865x Register Configuration to configure sensor for detecting change in orientation. */

    /*!< Acceleration vector-magnitude detection */
    MMA865x_VM_DETECTION_MODE          = 7U, /*!< MMA865x Register Configuration to configure sensor for acceleration vector-magnitude detection mode. */

	MMA865x_EMBEDDED_FUNCT_CONFIG_END

} mma865x_embedded_func_config_type_t;

/*!
 * @brief MMA865x Accel Configurations
 */
typedef enum mma865x_config_type
{
	/*!< 8-bit or 14-bit accelerometer data. */
	MMA865x_ACCEL_8BIT_READ_POLL_MODE  = 0U,  /*!< MMA865x Register Configuration to configure sensor for reading Accel 8-bit samples in polling mode. */
	MMA865x_ACCEL_14BIT_READ_POLL_MODE = 1U,  /*!< MMA865x Register Configuration to configure sensor for reading Accel 14-bit samples in polling mode. */
    MMA865x_ACCEL_14BIT_READ_FIFO_MODE = 2U,  /*!< MMA865x Register Configuration to configure sensor for reading Accel 14-bit samples in FIFO mode. */
    MMA865x_ACCEL_14BIT_READ_INT_MODE  = 3U,  /*!< MMA865x Register Configuration to configure sensor for reading Accel 14-bit samples in Interrupt mode. */
	MMA865x_ACCEL_CONFIG_END

} mma865x_config_type_t;

/*!
 * @brief MMA865x Sensor Data Type
 */
typedef enum mma865x_data_type
{
    MMA865x_ACCEL_14BIT_DATAREAD       = 0U, /*!< Accelerometer data read in 14-bit mode. */
    MMA865x_ACCEL_14BIT_FIFO_DATAREAD  = 1U, /*!< Accelerometer data read in 14-bit FIFO mode. */
    MMA865x_ACCEL_8BIT_DATAREAD        = 2U, /*!< Accelerometer data read in 8-bit mode. */
} mma865x_data_type_t;

/*!
 * @brief MMA865x Sensor Event Type
 */
typedef enum mma865x_event_type
{
    MMA865x_FREEFALL                   = 0U, /*!< Freefall detection. */
    MMA865x_MOTION                     = 1U, /*!< Motion detection. */
    MMA865x_TRANSIENT                  = 2U, /*!< Transient detection. */
    MMA865x_DOUBLETAP                  = 3U, /*!< Double Tap Pulse detection. */
    MMA865x_ORIENTATION                = 4U, /*!< Orientation change detection. */
    MMA865x_VECTOR_MAGNITUDE           = 5U, /*!< Acceleration vector-magnitude detection.*/
} mma865x_event_type_t;

/*!
 * @brief MMA865x Sensor Event Status Type
 */
typedef enum mma865x_event_status_type
{
    MMA865x_NO_EVENT_DETECTED          = 0U,  /*!< No event detected. */
    MMA865x_FREEFALL_DETECTED          = 1U,  /*!< Freefall event detected. */
    MMA865x_MOTION_DETECTED            = 2U,  /*!< Motion event detected.*/
    MMA865x_TRANSIENT_DETECTED         = 3U,  /*!< Transient event detected.*/
    MMA865x_DOUBLETAP_DETECTED         = 4U,  /*!< Double-Tap Pulse event detected. */
	MMA865x_PORTRAIT_UP                = 5U,  /*!< Orientation: Portrait UP detected*/
	MMA865x_PORTRAIT_DOWN              = 6U,  /*!< Orientation: Portrait Down detected*/
	MMA865x_LANDSCAPE_RIGHT            = 7U,  /*!< Orientation: Landscape Right detected*/
	MMA865x_LANDSCAPE_LEFT             = 8U,  /*!< Orientation: Landscape Left detected*/
	MMA865x_FRONT_SIDE                 = 9U,  /*!< Orientation: Front Side detected*/
	MMA865x_BACK_SIDE                  = 10U, /*!< Orientation: Back Side detected*/
    MMA865x_ACCEL_VM_DETECTED          = 11U, /*!< Accel vector-magnitude event detected. */
    MMA865x_FIFO_WTRMRK_DETECTED       = 12U, /*!< FIFO watermark event detected. */
} mma865x_event_status_type_t;

/*!
 * @brief MMA865x Sensor Mode Type
 */
typedef enum mma865x_mode_type
{
    MMA865x_STANDBY_MODE               = 0U, /*!< Standby Mode. */
    MMA865x_ACTIVE_MODE                = 1U, /*!< Active Mode. */
	MMA865x_MODE_END
} mma865x_mode_type_t;

/*!
 * @brief This structure defines the mma865x raw accel + mag data buffer.
 */
typedef struct
{
    int16_t  accel[NUM_AXES * FIFO_SIZE];     /*!< The accel data */
} mma865x_data_t;

/*!
 * @brief This structure defines the mma865x comm handle.
 */
typedef struct mma865x_driver
{
    sensor_comm_handle_t *pComHandle;
} mma865x_driver_t;

/*!
 * @brief mma865x interrupt configuration parameters
 */
typedef struct mma865x_interrupt_config
{
	uint8_t                 pp_od : 1;   /*!<  - Push-Pull/Open Drain selection on interrupt pad for INT1/INT2
                                               0: Push-pull (default)
											   1: Open-drain.	*/
	uint8_t                 ipol  : 1;   /*!<  - Interrupt polarity ACTIVE high, or ACTIVE low for INT1/INT2.
	                                           0: Active low (default)
											   1: Active high. */
	uint8_t                 reserved: 5;
	MMA865x_CTRL_REG4_t    intSources;  /*!<  Sources to be configured.
	                                           0: to a specific source field bit -disable the interrupt
											   1: to a specific source field bit -Enable the interrupt
											   eg. int_en_ff_mt bit to zero disable the interrupt, int_en_ff_mt bit to 1 enable the interrupt. */
	MMA865x_CTRL_REG5_t    int1_2;      /*!< INT1 or INT2 Routing configuration for specified source
	                                           0: to a bit configures interrupt for specified source to INT2 pin
											   1: to a bit configures interrupt for specified source to INT1 pin
											   eg. int_cfg_pulse bit to '0' configures pulse interrupt to INT2, int_cfg_pulse bit to '1' configures pulse interrupt to INT1 */

} mma865x_interrupt_config_t;

/*!
 * @brief mma865x interrupt sources
 */
typedef enum mma865x_interrupt_source
{
    MMA865x_DRDY                       = 0x01,
	MMA865x_SRC_ASLP                   = 0x02,
	MMA865x_SRC_FFMT                   = 0x04,
	MMA865x_SRC_PULSE                  = 0x08,
	MMA865x_SRC_LNDPRT                 = 0x10,
	MMA865x_SRC_TRANS                  = 0x20,
	MMA865x_FIFO                       = 0x40,
	MMA865x_ASLP                       = 0x80,
} mma865x_interrupt_source_t;

/*!
 * @brief mma865x power mode
 */
typedef enum mma865x_power_mode
{
    MMA865x_ACCEL_NORMAL               = 0x00,  /*!< Normal Power Mode.*/
	MMA865x_ACCEL_LOWNOISE_LOWPOWER    = 0x08,  /*!< Low Noise and Low Power Mode.*/
	MMA865x_ACCEL_HIGHRESOLUTION       = 0x10,  /*!< High Resolution via OSR.*/
	MMA865x_ACCEL_LOWPOWER             = 0x18,  /*!< Low Power Mode .*/
} mma865x_power_mode_t;

/*!
 * @brief mma865x Output Data Rate
 */
typedef enum mma865x_odr
{
    MMA865x_ODR_800_HZ                 = 0x00,
    MMA865x_ODR_400_HZ                 = 0x08,
    MMA865x_ODR_200_HZ                 = 0x10,
    MMA865x_ODR_100_HZ                 = 0x18,
    MMA865x_ODR_50_HZ                  = 0x20,
    MMA865x_ODR_12P5_HZ                = 0x28,
    MMA865x_ODR_6P25_HZ                = 0x30,
    MMA865x_ODR_1P5625_HZ              = 0x38,
} mma865x_odr_t;

/*******************************************************************************
 * Constants
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * APIs Prototype
 ******************************************************************************/

/*! @brief       The interface function to initialize the MMA865x sensor comm.
 *  @details     This function initialize the MMA865x sensor communication interface.
 *  @param[in]   mma865x_driver_t *pComHandle, the pointer to the MMA865x driver handle.
 *  @return      returns the status of the operation.
 */
uint8_t mma865x_init(mma865x_driver_t *pDriver);

/*! @brief       The interface function to generically read a mma865x sensor register.
 *  @details     This function read a mma865x sensor register.
 *  @param[in]   mma865x_driver_t *pDriver, the pointer to the MMA865x driver handle.
 *  @param[in]   address - Address from the register to read.
 *  @param[in]   nByteToRead - number of byte to read.
 *  @param[out]  pReadBuffer - a pointer to read buffer to to store the requested data read.
 *  @return      returns the status of the operation.
 */
uint8_t mma865x_read_reg(mma865x_driver_t *pDriver, uint16_t address, uint16_t nByteToRead, uint8_t *pReadBuffer);

/*! @brief       The interface function to generically write to a mma865x sensor register.
 *  @details     This function write to a mma865x sensor registers.
 *  @param[in]   mma865x_driver_t *pDriver, the pointer to the MMA865x driver handle.
 *  @param[in]   pWriteAddress - Address from the register to write.
 *  @param[out]  pWriteBuffer - a pointer to write buffer having value to write.
 *  @param[in]   nByteToWrite - number of byte to write.
 *  @return      returns the status of the operation.
 */
uint8_t mma865x_write_reg(mma865x_driver_t *pDriver, uint16_t address, uint16_t nByteToWrite, uint8_t *pWriteBuffer);

/*! @brief       The interface function to set and configure mma865x sensor embedded functions.
 *  @details     This function configures the mma865x sensor with the required embedded configuration.
                 User can configure multiple embedded function using the single call.
 *  @param[in]   mma865x_driver_t *pDriver - the pointer to the MMA865x driver handle.
 *  @param[in]   configType - types of embedded function to be configured.
 *  @return      returns the status of the operation.
 */

uint8_t mma865x_set_embedded_function(mma865x_driver_t *pDriver, mma865x_embedded_func_config_type_t configType);

/*! @brief       The interface function to configure mma865x accel
 *  @details     This function configure the accel with desired configuration.
 *  @param[in]   mma865x_driver_t *pDriver - the pointer to the MMA865x driver handle.
 *  @param[in]   ODR - ODR to be configured
 *  @param[in]   pConfig, the pointer to the acceleration configuration.
 *  @return      returns the status of the operation.
 */
uint8_t mma865x_configure(mma865x_driver_t *pDriver, mma865x_odr_t odr, mma865x_power_mode_t powerMode, mma865x_config_type_t pConfig );

/*! @brief       The interface function to configure mma865x interrupt controller for desired sources.
 *  @details     This function configure the mma865x interrupts for desired sources.It is possible that multiple source can be configured in same INT1 or INT2 pin
                 thus one or more functional blocks can assert an interrupt pin simultaneously; therefore a host application responding to an interrupt should read the INT_SOURCE register to determine the source(s) of the interrupt(s)
                 this function allows to configure single or multiple interrupt sources using single call.
				 IMPORTANT NOTE:
				 It is important to understand that application developers should handle the ISR handle at MCU level. this function just configure sensor interrupt mode only.
 *  @param[in]   mma865x_driver_t *pDriver - the pointer to the MMA865x driver handle.
 *  @param[in]   pConfig  - Configuration data for the interrupt mode
 *  @return      returns the status of the operation.
 */
uint8_t mma865x_config_interrupt(mma865x_driver_t *pDriver, mma865x_interrupt_config_t *pConfig);

/*! @brief       The interface function to disable specified interrupt source/sources
 *  @details     This function allow the disable the multiple source using single call.
 *  @param[in]   mma865x_driver_t *pDriver - the pointer to the MMA865x driver handle.
 *  @param[in]   ODR - ODR to be configured
 *  @param[in]   pConfig, the pointer to the acceleration configuration.
 *  @return      returns the status of the operation.
 */
uint8_t mma865x_disable_interrupt(mma865x_driver_t *pDriver, mma865x_interrupt_source_t intSource);

/*! @brief       The interface function to read MMA865x sensor data.
 *  @details     This function reads the MMA865x sensor output data.
 *  @param[in]   mma865x_driver_t *pDriver, the pointer to the MMA865x driver handle.
 *  @param[in]   dataType - The MMA865x sensor data type to be read.
 *  @param[in]   mma865x_data_t* pDataBuffer, the pointer to the data buffer to store MMA865x sensor data output.
 *  @return      returns the status of the operation.
 */
uint8_t mma865x_read_data(mma865x_driver_t *pDriver, mma865x_data_type_t dataType, mma865x_data_t* pDataBuffer);

/*! @brief       The interface function to read MMA865x sensor events.
 *  @details     This function reads the MMA865x sensor events.
 *  @param[in]   mma865x_driver_t *pDriver, the pointer to the MMA865x driver handle.
 *  @param[in]   eventType - The MMA865x sensor event type to be read.
 *  @param[in]   eventVal, the pointer to the event value/status storage.
 *  @return      returns the status of the operation.
 */
uint8_t mma865x_read_event(mma865x_driver_t *pDriver, mma865x_event_type_t eventType, uint8_t* eventVal);

/*! @brief       The interface function to de-initialize the MMA865x sensor.
 *  @details     This function de-initialize the MMA865x sensor.
 *  @param[in]   mma865x_driver_t *pDriver, the pointer to the MMA865x driver handle.
 *  @return      returns the status of the operation.
 */
uint8_t mma865x_deinit(mma865x_driver_t *pDriver);

#endif /* MMA865x_DRIVER_H_ */
