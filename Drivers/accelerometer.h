/**
  ******************************************************************************
  * @file    accelerometer.h
  * @author  Tomislav Darlić
  * @version V1
  * @date    31-May-2022
  * @brief   This header file contains the functions prototypes for the accelerometer driver.
  ******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ACCELEROMETER_H
#define __ACCELEROMETER_H

#ifdef __cplusplus
 extern "C" {
#endif



/* Includes ------------------------------------------------------------------*/
#include "MMA8452Q/mma8452q.h"

stmdev_ctx_t mma8452q_init(void);


#ifdef __cplusplus
}
#endif

#endif /* __ACCELEROMETER_H */
