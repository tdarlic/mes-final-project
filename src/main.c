/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include <stdlib.h>
#include <lv_widgets.h>
#include "stm32f4xx.h"
#include "hal_stm_lvgl/stm32f429i_discovery.h"

#include "lvgl/lvgl.h"
//#include "lvgl/examples/lv_examples.h"

#include "hal_stm_lvgl/tft/tft.h"
#include "hal_stm_lvgl/touchpad/touchpad.h"
#include "console.h"
#include "retarget.h"
#include "Drivers/barometer.h"
#include "Drivers/MMA8652/mma865x_driver.h"
#include "Drivers/MMA8652/mma865x_regdef.h"
#include "circular_buffer.h"
#include "main.h"

UART_HandleTypeDef huart1;
bdata_t bdata;
volatile uint16_t delayTime = 100;

// buffer containing the barometer values
uint16_t * buffer;
// handle for circular buffer
cbuf_handle_t me;

// screen rotation constants
volatile lv_disp_rot_t rotation;
volatile bool screen_rotated;
uint8_t orientation;

// Accelerometer I2C driver
mma865x_driver_t I2C;

bool warnShown = false;

static void SystemClock_Config(void);
static void MX_USART1_UART_Init(void);
static bool get_press_trend(void);
void Error_Handler(void);

int main(void)
{
	uint32_t minTick;
	uint32_t lastMov = 0;
	uint8_t eventVal;
	uint16_t bval;

	HAL_Init();

	buffer  = malloc(BAROMETER_BUFFER_SIZE * sizeof(uint8_t));
	me = circular_buf_init(buffer, BAROMETER_BUFFER_SIZE);

	I2C.pComHandle = (sensor_comm_handle_t*) &I2cHandle;

	/* Configure the system clock to 180 MHz */
	SystemClock_Config();

	MX_USART1_UART_Init();

	/*Start up indication*/
	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	uint32_t i;
	for (i = 0; i < 8; i++) {
		BSP_LED_Toggle(LED3);
		HAL_Delay(50);
	}

	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
	ACC_interrupt_init();

	lv_init();

	tft_init();
	touchpad_init();

	lv_widgets();

	RetargetInit(&huart1);
	ConsoleInit(&huart1);
	barometer_init();

	// immediately get barometer value
	minTick = 0;
	// switch off LED3 (green)
	BSP_LED_Off(LED3);

	// set the barometer value
	bdata = barometer_data();
	set_barometer_value(bdata.hpa);

	// initialize the accelerometer orientation detection mode
	mma865x_init(&I2C);
	mma865x_set_embedded_function(&I2C, MMA865x_ORIENT_DETECTION_MODE);
	screen_rotated = true;
	mma865x_read_event(&I2C, MMA865x_ORIENTATION, &eventVal);

	// Superloop
	while (1)
	{
		HAL_Delay(3);
		lv_task_handler();
		ConsoleProcess();

		// Sample barometer every minute
		if (minTick < HAL_GetTick()){
			bdata = barometer_data();
			set_barometer_value(bdata.hpa);
			minTick = HAL_GetTick() + (BAROMETER_LOG_INTERVAL * 1000);
			// convert float into uint16_t for storing variable into a buffer
			// value is stored as a uint16_t integer by subtracting 900 and multiplying
			bval = (uint16_t) ((bdata.hpa - 900)  * 100);
			circular_buf_put(me, bval);
			lv_add_baro_value((uint16_t) bdata.hpa);
			// calculate pressure trend and if needed send alarm
			get_press_trend();
		}

		// if no interrupt was detected but the pin is held low then reset the interrupt in accelerometer
		if ((HAL_GPIO_ReadPin(ACC_INT1_GPIO_Port, ACC_INT1_Pin) == 0) && (!screen_rotated)){
			mma865x_read_event(&I2C, MMA865x_ORIENTATION, &eventVal);
		}
		// now process the rotation of the screen if rotated
		if (screen_rotated){
			lastMov = HAL_GetTick() + 60 * 1000;
			// Disable orientation interrupt so that it does not interfere here
			HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
			mma865x_read_event(&I2C, MMA865x_ORIENTATION, &orientation);
			if (eventVal != 0){
				switch (eventVal){
				case MMA865x_PORTRAIT_UP:
					lv_rotate_screen(LV_DISP_ROT_90);
					break;
				case MMA865x_PORTRAIT_DOWN:
					lv_rotate_screen(LV_DISP_ROT_270);
					break;
				case MMA865x_LANDSCAPE_RIGHT:
					lv_rotate_screen(LV_DISP_ROT_NONE);
					break;
				case MMA865x_LANDSCAPE_LEFT:
					lv_rotate_screen(LV_DISP_ROT_180);
					break;
				default:
					break;
				}
			}
			// reset screen rotated variable
			screen_rotated = false;
			HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
		}

		if (warnShown){
			lv_ex_msgbox();
			warnShown = false;
		}
	}
}

/**
 * Calculates the pressure trend for this circular buffer
 * If trend drops more than 4 mb in last 4 hours storm is coming
 */
static bool get_press_trend(void){
	uint16_t i;
	float val;
	float min = 1200.0;
	float max = 0.0;
	uint16_t bdata;
	int rs;
	// walk the circular buffer and get max and minimum
	for (i = 1; i < circular_buf_size(me); ++i) {
		rs = circular_buf_peek(me, &bdata, i);
		if (rs == -1){
			break;
		} else {
			val = ((float)bdata/100) + 900;
			if (val < min){
				min = val;
			}
			if (val > max){
				max = val;
			}
		}
	}
	// if highest pressure was lower than storm limit
	if (max >= 1009.144){
		// if pressure was dropping more than 1 mb per hour storm is coming
		if ((max - min) >= 4){
			warnShown = true;
			return true;
		}
	}
	return false;
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 180000000
  *            HCLK(Hz)                       = 180000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 360
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  *         The LTDC Clock is configured as follow :
  *            PLLSAIN                        = 192
  *            PLLSAIR                        = 4
  *            PLLSAIDivR                     = 8
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /*##-1- System Clock Configuration #########################################*/
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Activate the Over-Drive mode */
  HAL_PWREx_EnableOverDrive();

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  /*##-2- LTDC Clock Configuration ###########################################*/
  /* LCD clock configuration */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 MHz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 MHz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 MHz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDIVR_8 = 48/8 = 6 MHz */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

