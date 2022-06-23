/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @author  Ac6
  * @version V1.0
  * @date    02-Feb-2015
  * @brief   Default Interrupt Service Routines.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#ifdef USE_RTOS_SYSTICK
#include <cmsis_os.h>
#endif
#include "stm32f4xx_it.h"
#include "main.h"
#include "hal_stm_lvgl/stm32f429i_discovery.h"
#include "lvgl/lvgl.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;

/******************************************************************************/
/*            	  	    Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles SysTick Handler, but only if no RTOS defines it.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();

	lv_tick_inc(1);

#ifdef USE_RTOS_SYSTICK
	osSystickHandler();
#endif
}


/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
 HAL_UART_IRQHandler(&huart1);
}

/**
  * @brief This function handles EXTI line0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */

  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(KEY_BUTTON_PIN);
  /* USER CODE BEGIN EXTI0_IRQn 1 */

  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles EXTI line0 interrupt.
  */
void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */

  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(ACC_INT1_Pin);
  /* USER CODE BEGIN EXTI0_IRQn 1 */

  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles EXTI line0 interrupt.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */

  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(ACC_INT2_Pin);
  /* USER CODE BEGIN EXTI0_IRQn 1 */

  /* USER CODE END EXTI0_IRQn 1 */
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	static uint32_t lastButtonTime = 0;

		if (GPIO_Pin == KEY_BUTTON_PIN){
			if ((HAL_GetTick() - lastButtonTime) > delayTime){
				BSP_LED_Toggle(LED3);
				lastButtonTime = HAL_GetTick();
			}
		}

		if (GPIO_Pin == ACC_INT1_Pin){
			// not used
			screen_rotated = true;

			BSP_LED_Toggle(LED4);
		}

		if (GPIO_Pin == ACC_INT2_Pin){

		}


}
