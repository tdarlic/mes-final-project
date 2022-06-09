/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file mma865x_config.h
 * @brief The mma865x_driver_config.h file contains definitions for mma865x sensor configurations.
 */

#ifndef MMA865x_CONFIG_H_
#define MMA865x_CONFIG_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "common/sensor_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @def    FIFO_SIZE
 *  @brief  The watermark value configured for MMA865x FIFO Buffer.
 */
#define FIFO_SIZE 16

/*******************************************************************************
 * Typedefs
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

extern const registerwritelist_t gMma865xActiveModeConfig[];
extern const registerwritelist_t gMma865xStandbyModeConfig[];
extern const registerwritelist_t gMma865xAccelInterruptConfig[];
extern const registerwritelist_t gMma865x8bitAccelPollConfig[];
extern const registerwritelist_t gMma865xAccelPollConfig[];
extern const registerwritelist_t gMma865xAccelFifoConfig[];
extern const registerwritelist_t gMma865xOrientDetectConfig[];
extern const registerwritelist_t gMma865xMotiontDetectConfig[];
extern const registerwritelist_t gMma865xFreefallDetectConfig[];
extern const registerwritelist_t gMma865xDoubleTapDetectConfig[];

extern const registerreadlist_t gMma865xReadStatus[];
extern const registerreadlist_t gMma865xReadAccel[];
extern const registerreadlist_t gMma865xReadAccelFifo[];
extern const registerreadlist_t gMma865xFifoStatus[];
extern const registerreadlist_t gMma865xReadAccel8bit[];
extern const registerreadlist_t gMma865xReadFFMTSrc[];
extern const registerreadlist_t gMma865xReadINTSrc[];
extern const registerreadlist_t gMma865xReadPLStatus[];
extern const registerreadlist_t gMma865xReadPulseSrc[];

/*******************************************************************************
 * APIs Prototype
 ******************************************************************************/

#endif /* MMA865x_CONFIG_H_ */
