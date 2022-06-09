/*
 * barometer.c
 *
 *  Created on: May 30, 2022
 *      Author: tdarlic
 */

#include "../hal_stm_lvgl/stm32f429i_discovery.h"
#include "barometer.h"

stmdev_ctx_t baro_ctx;
lps28dfw_md_t md;


static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);

stmdev_ctx_t lps28dfw_init(void){
	  /* Initialize mems driver interface */
	baro_ctx.write_reg = platform_write;
	baro_ctx.read_reg = platform_read;
	baro_ctx.handle = &I2cHandle;

	return baro_ctx;
}

/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
  HAL_I2C_Mem_Write(handle, LPS28DFW_I2C_ADD_H, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
  return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
  HAL_I2C_Mem_Read(handle, LPS28DFW_I2C_ADD_H, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
  return 0;
}

void barometer_init(void){

	lps28dfw_bus_mode_t bus_mode;
	lps28dfw_stat_t status;
	lps28dfw_pin_int_route_t int_route;
	lps28dfw_md_t md;


	baro_ctx = lps28dfw_init();

	/* Restore default configuration */
	lps28dfw_init_set(&baro_ctx, LPS28DFW_BOOT);
	lps28dfw_init_set(&baro_ctx, LPS28DFW_RESET);
	do {
		lps28dfw_status_get(&baro_ctx, &status);
	} while (status.sw_reset);


	/* Set bdu and if_inc recommended for driver usage */
	lps28dfw_init_set(&baro_ctx, LPS28DFW_DRV_RDY);

	lps28dfw_fifo_mode_set(&baro_ctx, (lps28dfw_fifo_md_t *) LPS28DFW_STREAM);

	/* Select bus interface */
	bus_mode.filter = LPS28DFW_AUTO;
	bus_mode.interface = LPS28DFW_SEL_BY_HW;
	lps28dfw_bus_mode_set(&baro_ctx, &bus_mode);

	/* Set Output Data Rate */
	md.odr = LPS28DFW_4Hz;
	md.avg = LPS28DFW_4_AVG;
	md.lpf = LPS28DFW_LPF_ODR_DIV_4;
	md.fs = LPS28DFW_1260hPa;
	lps28dfw_mode_set(&baro_ctx, &md);

	/* Configure inerrupt pins */
	lps28dfw_pin_int_route_get(&baro_ctx, &int_route);
	int_route.drdy_pres   = PROPERTY_DISABLE;
	lps28dfw_pin_int_route_set(&baro_ctx, &int_route);
}

bdata_t barometer_data(void){
	lps28dfw_data_t data;
	bdata_t ret;
	lps28dfw_data_get(&baro_ctx, &md, &data);
	ret.deg_c = data.heat.deg_c;
	ret.hpa = data.pressure.hpa;
	return ret;
}




