/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file sensor_comm.c
 * @brief The sensor_comm.c file implements the sensor communication virtual interface. 
   User application needs to implement/call underlying SDK communication interfaces such i2c/SPI. 
   This is the SDK agnostic layer for the sensor for communication.
 */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "sensor_comm.h"
#include "../hal_stm_lvgl/stm32f429i_discovery.h"
#include "../mma865x_regdef.h"

//-----------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------
uint8_t sensor_comm_init(sensor_comm_handle_t *pComHandle)
{
 
}

uint8_t sensor_comm_write(sensor_comm_handle_t *pComHandle, uint16_t offset, uint16_t size, uint8_t *pWritebuffer)
{
	HAL_I2C_Mem_Write(pComHandle, MMA865x_I2C_ADDRESS_WRITE, offset, I2C_MEMADD_SIZE_8BIT, pWritebuffer, size, 1000);
	return 0;
}

uint8_t sensor_comm_read(sensor_comm_handle_t *pComHandle, uint16_t offset, uint16_t size, uint8_t *pReadbuffer)
{
	HAL_I2C_Mem_Read(pComHandle, MMA865x_I2C_ADDRESS_READ, offset, I2C_MEMADD_SIZE_8BIT, pReadbuffer, size, 1000);
	return 0;
}
