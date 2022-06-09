/**
  ******************************************************************************
  * @file    barometer.h
  * @author  Tomislav Darlić
  * @version V1
  * @date    31-May-2022
  * @brief   This header file contains the functions prototypes for the gyroscope driver.
  ******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BAROMETER_H
#define __BAROMETER_H

#ifdef __cplusplus
 extern "C" {
#endif

 typedef struct {
	 float hpa;
	 float deg_c;
 } bdata_t;

/* Includes ------------------------------------------------------------------*/
#include "LPS28DFW/lps28dfw_reg.h"

stmdev_ctx_t lps28dfw_init(void);
void barometer_init(void);
bdata_t barometer_data(void);

#ifdef __cplusplus
}
#endif

#endif /* __BAROMETER_H */
