/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include "cracker.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t is_cdc_initialized = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*Please please please don't use print functions for debugging. Use print functions only for
 *status update, that will be used once the program will be in a state to run standalone.*/

/* Retarget printf*/
int _write(int fd, char * ptr, int len)
{
	if(is_cdc_initialized){

		int no_of_retries = 0;
		// I'm pretty sure that CDC_Transmit_FS() doesn't return USBD_OK, if no USB connection is established.
		// To make this work under debug and to prevent blocking code, printf gives up after 9000 tries.
		while(CDC_Transmit_FS((uint8_t *)ptr, len) != USBD_OK){
			no_of_retries++;
			if(no_of_retries > 9000){
				// It's over 9000!
				return 0;
			}
		}
		return (len);
	}
	else{
		return 0;
	}
}


uint32_t send_1byte(uint8_t byte, uint8_t byte_pos)
{

	uint32_t timer_ticks;
	uint32_t returncode;
	// Reset target first
	target_reset(GPIOE, target_reset_Pin, target_mode_Pin);

	// Initiate the target communication
	returncode = init_target_connection(&huart1);
	if(returncode != CON_INIT_OK)
	{
		printf("Initiating target communication failed with error: %lu \n", returncode);
		return 0;
	}

	returncode = set_baudrate(&huart1, TEST_TARGET_SPEED);
	if(returncode != BAUDRATE_CHANGE_OK)
	{
		printf("Setting the baud rate failed with error: %lu \n", returncode);
		return 0;
	}

	send_one_key_byte(byte, byte_pos, &huart1, &htim2);
	timer_ticks = read_and_reset_timer(&htim2);
	return timer_ticks;
}


void set_min_clock()
{
	printf("Finding minimal clock speed");
	uint32_t clock_speed_khz;
	for(clock_speed_khz = 100; clock_speed_khz <= MAX_TARGET_CLKSPEED_KHZ; clock_speed_khz += 100){
		uint32_t init_returncode;
		uint32_t baudrate_returncode;
		printf("Testing clock of: %lu KHz\n", clock_speed_khz);
		set_target_clock_generator(&htim3, clock_speed_khz);
		// Reset target first
		target_reset(GPIOE, target_reset_Pin, target_mode_Pin);

		// Initiate the target communication
		init_returncode = init_target_connection(&huart1);
		// Test baud rate switch.
		baudrate_returncode = set_baudrate(&huart1, TEST_TARGET_SPEED);

		if((init_returncode == CON_INIT_OK) && (baudrate_returncode == BAUDRATE_CHANGE_OK)){
			printf("Clock speed %lu KHz seems fine. Testing clock stability.\n", clock_speed_khz);
			//Test communication a few more times to make sure it runs reliably. This is to avoid any instabilities during testing.
			int i;
			uint8_t no_of_fails = 0;
			for(i = 0; i < 10; i++)
			{
				target_reset(GPIOE, target_reset_Pin, target_mode_Pin);
				// Initiate the target communication
				init_returncode = init_target_connection(&huart1);
				// Test baud rate switch.
				baudrate_returncode = set_baudrate(&huart1, TEST_TARGET_SPEED);
				if((init_returncode != CON_INIT_OK) && (baudrate_returncode != BAUDRATE_CHANGE_OK)){
					no_of_fails++;
					break;
				}
			}
			if(no_of_fails == 0){
				printf("Clock speed %lu KHz is the lowest detected speed.\n", clock_speed_khz);
				return;
			}
			else{
				printf("Clock speed %lu KHz was unstable.\n", clock_speed_khz);
			}
		}
		else
		{
			printf("Clock speed to low.\n");
		}
	}
	printf("Target didn't respond at any of the selected speeds. Defaulting to %u KHz. Please check the connection.\n", TARGET_CLOCK_KHZ);
	set_target_clock_generator(&htim3, TARGET_CLOCK_KHZ);
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /*This is so that we don't need new line for every printf.*/
  setvbuf(stdout, NULL, _IONBF, 0);
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USB_DEVICE_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  is_cdc_initialized = 1;
  #ifdef GET_MIN_CLK_SPEED
  set_min_clock();
  #else
  set_target_clock_generator(&htim3, TARGET_CLOCK_KHZ);
  #endif

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	HAL_Delay(1000);



	printf("Press key0 to start cracker.\n");
	//Wait for start button press (key0 on board).
	while(HAL_GPIO_ReadPin(GPIOE, k0_Pin));

	printf("Cracker started.\n");
	HAL_Delay(500);

	uint8_t byte_no = 0;

	uint8_t detected_key[7];

	int i;
	for(byte_no = 0; byte_no < 7; byte_no++){
		uint32_t longest_tick = 0;
		uint32_t longest_tick_byte = 0;

		for(i = 0; i < 256; i++){
			uint32_t timer_ticks;
			timer_ticks = send_1byte(i, 0);

			if(!timer_ticks)
			{
				// No ticks detected handler
				printf("Timer ticks were 0 which indicates an error. Press key0 to continue or key1 to cancel this run.\n");
				while(HAL_GPIO_ReadPin(GPIOE, k0_Pin) == GPIO_PIN_SET)
				{
					if(HAL_GPIO_ReadPin(GPIOE, k1_Pin) == GPIO_PIN_RESET)
					{
						byte_no = 7;
						i = 256;
						break;
					}
				}
			}

			if(timer_ticks > longest_tick)
			{
				longest_tick = timer_ticks;
				longest_tick_byte = i;
			}

			printf("Number of ticks for Byte %d at Key Byte %d was : %lu \n", i, byte_no + 1, timer_ticks);
		}
		detected_key[byte_no] = longest_tick_byte;
	}

	printf("\nDetected key was:");
	for(i = 0; i < 7; i++)
	{
		printf(" 0x%x", detected_key[i]);
	}
	printf("\n\n");






    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 7;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

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
  huart1.Init.BaudRate = 9600;
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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(target_reset_GPIO_Port, target_reset_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(target_mode_GPIO_Port, target_mode_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : k1_Pin k0_Pin */
  GPIO_InitStruct.Pin = k1_Pin|k0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : target_reset_Pin */
  GPIO_InitStruct.Pin = target_reset_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(target_reset_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : target_mode_Pin */
  GPIO_InitStruct.Pin = target_mode_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(target_mode_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : tx_trigger_Pin rx_trigger_Pin */
  GPIO_InitStruct.Pin = tx_trigger_Pin|rx_trigger_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
