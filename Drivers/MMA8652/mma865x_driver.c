/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file  mma865x_driver.c
 * @brief This file implements sensor interface module for MMA865x, the 6-axis sensor
 *        with integrated linear accelerometer and magnetometer.
*/

/*******************************************************************************
 * Includes
 ******************************************************************************/
/* Component Lib Includes */
#include "mma865x_driver.h"
#include "mma865x_config.h"
#include "common/sensor_common.h"
#include "common/sensor_comm.h"
#include <stddef.h>

/*******************************************************************************
 * Local Functions Prototypes
 ******************************************************************************/
/*! @brief       The interface function to set MMA865x sensor mode.
 *  @details     This function set required MMA865x sensor mode.
 *  @param[in]   mma865x_driver_t *pDriver, the pointer to the mma865x comm handle.
 *  @param[in]   mma865x_mode_type_t sensorMode, MMA865x sensor mode that user want to set to.
 *  @return      returns the status of the operation.
 */
static uint8_t mma865x_set_mode(mma865x_driver_t *pDriver, mma865x_mode_type_t sensorMode);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*! @brief  The function to Initialize MMA865x sensor communication interface.
 */
uint8_t mma865x_init(mma865x_driver_t *pDriver)
{
  /* Initialize the sensor driver handler and interfaces */
	if(NULL == pDriver){
        return SENSOR_INVALIDPARAM_ERR;
    }
    sensor_comm_init(pDriver->pComHandle);
	return SENSOR_SUCCESS;
}


/*! @brief  The local function to set operating mode for the MMA865x sensor.
*/
static uint8_t mma865x_set_mode(mma865x_driver_t *pDriver, mma865x_mode_type_t sensorMode)
{
	uint8_t status = SENSOR_SUCCESS;

	/* Check for bad address. */
	if (NULL == pDriver)
	{
	    return SENSOR_BAD_ADDRESS;
	}

	switch (sensorMode)
	{
		case MMA865x_STANDBY_MODE:

            /*! Apply Register Configuration to set MMA865x into Standby mode. */
			status = sensor_burst_write(pDriver->pComHandle, gMma865xStandbyModeConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			break;
		case MMA865x_ACTIVE_MODE:

            /*! Apply Register Configuration to set MMA865x into Active mode. */
			status = sensor_burst_write(pDriver->pComHandle, gMma865xActiveModeConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			break;
        default:
            status = SENSOR_INVALIDPARAM_ERR;
            break;
	}
	return status;
}

/*! @brief mma865x_read_reg
 */
uint8_t mma865x_read_reg(mma865x_driver_t *pDriver, uint16_t address, uint16_t nByteToRead, uint8_t *pReadBuffer)
{
	if((NULL == pDriver) || (NULL == pReadBuffer))
	{
		return SENSOR_INVALIDPARAM_ERR;
	}
	sensor_comm_read(pDriver->pComHandle, address, nByteToRead, pReadBuffer);
	return SENSOR_SUCCESS;
}

/*! @brief mma865x_write_reg
 */
uint8_t mma865x_write_reg(mma865x_driver_t *pDriver, uint16_t address, uint16_t nByteToWrite, uint8_t *pWriteBuffer)
{

	if((NULL == pDriver) || (NULL == pWriteBuffer))
	{
		return SENSOR_INVALIDPARAM_ERR;
	}
	sensor_comm_write(pDriver->pComHandle, address, nByteToWrite, pWriteBuffer);
	return SENSOR_SUCCESS;
}

/*! @brief  The interface function to read MMA865x sensor data.
 */
uint8_t mma865x_read_data(mma865x_driver_t *pDriver, mma865x_data_type_t dataType, mma865x_data_t* pDataBuffer)
{
	uint8_t status;
	uint8_t dr_status = 0;
	uint8_t data[MMA865x_ACCEL_DATA_SIZE * FIFO_SIZE];

	/* Check for bad address and invalid params. */
	if ((NULL == pDataBuffer) || (NULL == pDriver))
	{
	    return SENSOR_BAD_ADDRESS;
	}

	switch (dataType)
	{
		case MMA865x_ACCEL_14BIT_DATAREAD:

            /* Read the MMA865x status register and wait till new data is ready*/
			status = mma865x_read_reg(pDriver, MMA865x_STATUS, 1, &dr_status);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }
			while (0 == (dr_status & MMA865x_STATUS_ZYXDR_MASK))
			{
				status = mma865x_read_reg(pDriver, MMA865x_STATUS, 1, &dr_status);
                if (SENSOR_SUCCESS != status)
                {
                    return status;
                }
			}

            /* Received DataReady event, Read the MMA865x Accel samples*/
			status = sensor_burst_read(pDriver->pComHandle, gMma865xReadAccel, data);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Convert the raw sensor data to signed 16-bit container. */
			pDataBuffer->accel[0] = ((int16_t)data[0] << 8) | data[1];
			pDataBuffer->accel[0] /= 16;
			pDataBuffer->accel[1] = ((int16_t)data[2] << 8) | data[3];
			pDataBuffer->accel[1] /= 16;
			pDataBuffer->accel[2] = ((int16_t)data[4] << 8) | data[5];
			pDataBuffer->accel[2] /= 16;

			break;
		case MMA865x_ACCEL_14BIT_FIFO_DATAREAD:

            /* Read the FXOS8700 status register and wait till new data is ready*/
			status = mma865x_read_reg(pDriver, MMA865x_F_STATUS, 1, &dr_status);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }
			while (0 == (dr_status & MMA865x_F_STATUS_F_WMKF_MASK))
			{
				status = mma865x_read_reg(pDriver, MMA865x_F_STATUS, 1, &dr_status);
                if (SENSOR_SUCCESS != status)
                {
                    return status;
                }
			}

            /* Received DataReady event, Read the FXOS8700 Accel samples*/
			status = sensor_burst_read(pDriver->pComHandle, gMma865xReadAccelFifo, data);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

	        for (uint8_t i = 0; i < FIFO_SIZE; i++)
	        {
				/*! Convert the raw sensor data to signed 16-bit container. */
				pDataBuffer->accel[i*3 + 0] = ((int16_t)data[i * MMA865x_ACCEL_DATA_SIZE + 0] << 8) | data[i * MMA865x_ACCEL_DATA_SIZE + 1];
				pDataBuffer->accel[i*3 + 0] /= 16;
				pDataBuffer->accel[i*3 + 1] = ((int16_t)data[i * MMA865x_ACCEL_DATA_SIZE + 2] << 8) | data[i * MMA865x_ACCEL_DATA_SIZE + 3];
				pDataBuffer->accel[i*3 + 1] /= 16;
				pDataBuffer->accel[i*3 + 2] = ((int16_t)data[i * MMA865x_ACCEL_DATA_SIZE + 4] << 8) | data[i * MMA865x_ACCEL_DATA_SIZE + 5];
				pDataBuffer->accel[i*3 + 2] /= 16;
	        }

			break;
		case MMA865x_ACCEL_8BIT_DATAREAD:

            /* Read the MMA865x status register and wait till new data is ready*/
			status = mma865x_read_reg(pDriver, MMA865x_STATUS, 1, &dr_status);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }
			while (0 == (dr_status & MMA865x_STATUS_ZYXDR_MASK))
			{
				status = mma865x_read_reg(pDriver, MMA865x_STATUS, 1, &dr_status);
                if (SENSOR_SUCCESS != status)
                {
                    return status;
                }
			}

            /* Received DataReady event, Read the MMA865x Accel samples*/
			status = sensor_burst_read(pDriver->pComHandle, gMma865xReadAccel8bit, data);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Convert the raw sensor data to signed 16-bit container. */
			pDataBuffer->accel[0] = ((int16_t)data[0]);
			pDataBuffer->accel[1] = ((int16_t)data[1]);
			pDataBuffer->accel[2] = ((int16_t)data[2]);

			break;
        default:
            status = SENSOR_INVALIDPARAM_ERR;

            break;
	}
	return status;
}

/*! @brief  The interface function to read MMA865x sensor events.
 */
uint8_t mma865x_read_event(mma865x_driver_t *pDriver, mma865x_event_type_t eventType, uint8_t* eventVal)
{
	uint8_t status;
    uint8_t eventStatus;

	(* eventVal) = MMA865x_NO_EVENT_DETECTED;

	switch (eventType)
	{
		case MMA865x_FREEFALL:

			status = sensor_burst_read(pDriver->pComHandle, gMma865xReadFFMTSrc, &eventStatus);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            if (0 == (eventStatus & MMA865x_FF_MT_SRC_EA_MASK))
            { /* Return, if new event is not detected. */
              return SENSOR_INVALIDPARAM_ERR;
            }

            (* eventVal) = MMA865x_FREEFALL_DETECTED;

			break;
		case MMA865x_MOTION:

			status = sensor_burst_read(pDriver->pComHandle, gMma865xReadFFMTSrc, &eventStatus);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            if (0x80 == (eventStatus & MMA865x_FF_MT_SRC_EA_MASK))
            { /*! Motion event has been detected. */
            	(* eventVal) = MMA865x_MOTION_DETECTED;
            }

			break;
		case MMA865x_DOUBLETAP:

			status = sensor_burst_read(pDriver->pComHandle, gMma865xReadPulseSrc, &eventStatus);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            if (0x01 == (eventStatus & MMA865x_PULSE_SRC_DPE_MASK))
            { /*! Double-Tap event has been detected. */
            	(* eventVal) = MMA865x_DOUBLETAP_DETECTED;
            }

			break;
		case MMA865x_ORIENTATION:

			status = sensor_burst_read(pDriver->pComHandle, gMma865xReadPLStatus, &eventStatus);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			if (((eventStatus & MMA865x_PL_STATUS_NEWLP_MASK) == 0x80) &&
				((eventStatus & MMA865x_PL_STATUS_LO_MASK) == 0x00))
			{
				uint8_t lp_orient = eventStatus & MMA865x_PL_STATUS_LAPO_MASK;
				switch(lp_orient)
				{
					case 0x00:
						(* eventVal) = MMA865x_PORTRAIT_UP;
						break;
					case 0x02:
						(* eventVal) = MMA865x_PORTRAIT_DOWN;
						break;
					case 0x04:
						(* eventVal) = MMA865x_LANDSCAPE_RIGHT;
						break;
					case 0x06:
						(* eventVal) = MMA865x_LANDSCAPE_LEFT;
						break;
					default:
					    break;
				}
			}

			if (((eventStatus & MMA865x_PL_STATUS_NEWLP_MASK) == 0x80) &&
				((eventStatus & MMA865x_PL_STATUS_LO_MASK) == 0x40))
			{
				uint8_t bf_orient = eventStatus & MMA865x_PL_STATUS_BAFRO_MASK;
				switch(bf_orient)
				{
					case 0x00:
						(* eventVal) = MMA865x_FRONT_SIDE;
						break;
					case 0x01:
						(* eventVal) = MMA865x_BACK_SIDE;
						break;
					default:
					    break;
				}
			}

			break;
        default:
            status = SENSOR_INVALIDPARAM_ERR;
            break;
	}
	return status;
}

/*! @brief  The interface function to apply MMA865x Accel configuration.
 */
uint8_t mma865x_configure(mma865x_driver_t *pDriver, mma865x_odr_t odr, mma865x_power_mode_t powerMode, mma865x_config_type_t pConfig)
{
	uint8_t status = SENSOR_SUCCESS;

	/* Check for bad address. */
	if (NULL == pDriver)
	{
	    return SENSOR_BAD_ADDRESS;
	}

	/*! Prepare the register write list to configure MMA865x for required ODR and power mode. */
	registerwritelist_t mma865xOdrSmodConfig[] = {
		/*! Configure MMA865x CTRL_REG1 Register "dr[2:0]" bit-fields to set ODR value. */
		{MMA865x_CTRL_REG1, odr, MMA865x_CTRL_REG1_DR_MASK},
		/*! Configure MMA865x CTRL REG2 Register "smods[1:0]" bit-fields to set power mode. */
		{MMA865x_CTRL_REG2, powerMode, MMA865x_CTRL_REG2_SMODS_MASK},
		__END_WRITE_DATA__};

	switch (pConfig)
	{
		case MMA865x_ACCEL_8BIT_READ_POLL_MODE:

		    /*! Set MMA865x into standby mode so that configuration can be applied.*/
			status = mma865x_set_mode(pDriver, MMA865x_STANDBY_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            /*! Apply Register Configuration to configure MMA865x for required ODR and SMOD */
            status = sensor_burst_write(pDriver->pComHandle, mma865xOdrSmodConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure MMA865x for reading Accel 8-bit samples in Polling mode */
			status = sensor_burst_write(pDriver->pComHandle, gMma865x8bitAccelPollConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

		    /*! Set MMA865x into Active mode.*/
			status = mma865x_set_mode(pDriver, MMA865x_ACTIVE_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Successfully applied sensor configuration. */

			break;
		case MMA865x_ACCEL_14BIT_READ_POLL_MODE:

		    /*! Set MMA865x into standby mode so that configuration can be applied.*/
			status = mma865x_set_mode(pDriver, MMA865x_STANDBY_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            /*! Apply Register Configuration to configure MMA865x for required ODR and SMOD */
            status = sensor_burst_write(pDriver->pComHandle, mma865xOdrSmodConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure MMA865x for reading Accel samples in Polling mode */
			status = sensor_burst_write(pDriver->pComHandle, gMma865xAccelPollConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

		    /*! Set MMA865x into Active mode.*/
			status = mma865x_set_mode(pDriver, MMA865x_ACTIVE_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Successfully applied sensor configuration. */

			break;
		case MMA865x_ACCEL_14BIT_READ_FIFO_MODE:

		    /*! Set MMA865x into standby mode so that configuration can be applied.*/
			status = mma865x_set_mode(pDriver, MMA865x_STANDBY_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            /*! Apply Register Configuration to configure MMA865x for required ODR and SMOD */
            status = sensor_burst_write(pDriver->pComHandle, mma865xOdrSmodConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure MMA865x for reading Accel samples in FIFO mode */
			status = sensor_burst_write(pDriver->pComHandle, gMma865xAccelFifoConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

		    /*! Set MMA865x into Active mode.*/
			status = mma865x_set_mode(pDriver, MMA865x_ACTIVE_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Successfully applied sensor configuration. */

			break;
		case MMA865x_ACCEL_14BIT_READ_INT_MODE:

		    /*! Set MMA865x into standby mode so that configuration can be applied.*/
			status = mma865x_set_mode(pDriver, MMA865x_STANDBY_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            /*! Apply Register Configuration to configure MMA865x for required ODR and SMOD */
            status = sensor_burst_write(pDriver->pComHandle, mma865xOdrSmodConfig);
            if (SENSOR_SUCCESS != status)
            {
                return status;
            }

            /*! Apply Register Configuration to configure MMA865x for reading Accel samples in INT mode */
			status = sensor_burst_write(pDriver->pComHandle, gMma865xAccelInterruptConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

		    /*! Set MMA865x into Active mode.*/
			status = mma865x_set_mode(pDriver, MMA865x_ACTIVE_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Successfully applied sensor configuration. */

			break;
        default:
            status = SENSOR_INVALIDPARAM_ERR;

            break;
	}
    return status;
}

/*! @brief  The interface function to apply embedded functionality configuration for MMA865x sensor.
 */
uint8_t mma865x_set_embedded_function(mma865x_driver_t *pDriver, mma865x_embedded_func_config_type_t configMode)
{
	uint8_t status = SENSOR_SUCCESS;

	/* Check for bad address. */
	if (NULL == pDriver)
	{
	    return SENSOR_BAD_ADDRESS;
	}

	switch (configMode)
	{
		case MMA865x_FREEFALL_DETECTION_MODE:

		    /*! Set MMA865x into standby mode so that configuration can be applied.*/
			status = mma865x_set_mode(pDriver, MMA865x_STANDBY_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            /*! Apply Register Configuration to configure MMA865x for Freefall detection mode */
			status = sensor_burst_write(pDriver->pComHandle, gMma865xFreefallDetectConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

		    /*! Set MMA865x into Active mode.*/
			status = mma865x_set_mode(pDriver, MMA865x_ACTIVE_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Successfully applied sensor configuration. */

			break;
		case MMA865x_MOTION_DETECTION_MODE:

		    /*! Set MMA865x into standby mode so that configuration can be applied.*/
			status = mma865x_set_mode(pDriver, MMA865x_STANDBY_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            /*! Apply Register Configuration to configure MMA865x for Motion detection mode */
			status = sensor_burst_write(pDriver->pComHandle, gMma865xMotiontDetectConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

		    /*! Set MMA865x into Active mode.*/
			status = mma865x_set_mode(pDriver, MMA865x_ACTIVE_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Successfully applied sensor configuration. */

			break;
		case MMA865x_ORIENT_DETECTION_MODE:

		    /*! Set MMA865x into standby mode so that configuration can be applied.*/
			status = mma865x_set_mode(pDriver, MMA865x_STANDBY_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

            /*! Apply Register Configuration to configure MMA865x for Orientation detection mode */
			status = sensor_burst_write(pDriver->pComHandle, gMma865xOrientDetectConfig);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

		    /*! Set MMA865x into Active mode.*/
			status = mma865x_set_mode(pDriver, MMA865x_ACTIVE_MODE);
			if (SENSOR_SUCCESS != status)
			{
				return status;
			}

			/*! Successfully applied sensor configuration. */

			break;
        default:
            status = SENSOR_INVALIDPARAM_ERR;

            break;
	}
	return status;
}

/*! @brief  The interface function to apply interrupt configuration for MMA865x sensor.
 */
uint8_t mma865x_config_interrupt(mma865x_driver_t *pDriver, mma865x_interrupt_config_t *pConfig)
{
    if(NULL == pDriver)
	{
		return SENSOR_INVALIDPARAM_ERR;
	}
	uint8_t ctrlReg[3];
	uint8_t status = SENSOR_SUCCESS;


	/* Read the CTRL_REG3 and preserve the existing configuration bits of the control registers other than interrupt configuration bits. */
	status = sensor_comm_read(pDriver->pComHandle, MMA865x_CTRL_REG3, 1, ctrlReg);
	if(status != SENSOR_SUCCESS)
	{
	    return status;
	}
	/* Update the Ctrl reg with polarity and open drain/push pull. */
	//ctrlReg[0] |= (*pConfig)& (MMA865x_CTRL_REG3_PP_OD_MASK | MMA865x_CTRL_REG3_IPOL_MASK));

	/* Enable the desired interrupt sources. */
	//ctrlReg[1] = pConfig->intSources;

	/* configure the interrupt routing */
	//ctrlReg[2]  = pConfig->int1_2 ;


	/* configure the interrupts sources with desired pin configuration setting for mma865x */
	status = sensor_comm_write(pDriver->pComHandle, MMA865x_CTRL_REG3, 3, ctrlReg);
	if(status != SENSOR_SUCCESS)
	{
	    return status;
	}

    return status;
}

/*! @brief  The interface function to disable interrupt MMA865x sensor.
 */
uint8_t mma865x_disable_interrupt(mma865x_driver_t *pDriver, mma865x_interrupt_source_t intSource)
{
	uint8_t status;

    if(NULL == pDriver)
	{
		return SENSOR_INVALIDPARAM_ERR;
	}
	uint8_t ctrlReg;
	status = sensor_comm_read(pDriver->pComHandle, MMA865x_CTRL_REG4, 1, &ctrlReg);
	if(status != SENSOR_SUCCESS)
	{
	    return status;
	}
	ctrlReg &= ~intSource;

	/* Disable the interrupt sources configured */
	status = sensor_comm_write(pDriver->pComHandle, MMA865x_CTRL_REG4, 1, &ctrlReg);
	if(status != SENSOR_SUCCESS)
	{
	    return status;
	}
    return status;
}
