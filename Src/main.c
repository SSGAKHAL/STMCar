/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "hcsr04.h"
#include "motor.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint8_t buff_recive[2];
HCSRO4t hc;
Motort mt;
extern TIM_OC_InitTypeDef sConfigOC_TIM3;  // sConfigOC structure used in the MX_TIM3_Init function;
extern TIM_OC_InitTypeDef sConfigOC_TIM4;  // sConfigOC structure used in the MX_TIM3_Init function;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
                                    
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                
                                
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
//INIT HCSR04 Structure
void HCRSR04_init(GPIO_TypeDef* ECHOPORT,uint16_t ECHOPIN,GPIO_TypeDef* TRIGGERPORT,uint16_t TRIGGERPIN){
	hc.ECHO_GPIOx=ECHOPORT;
	hc.ECHO_GPIO_Pin=ECHOPIN;
	hc.TRIGGER_GPIOx=TRIGGERPORT;
	hc.TRIGGER_GPIO_Pin=TRIGGERPIN;	
}
//INIT MOTOR INPUT
void Motor_init(GPIO_TypeDef* F1PORT,uint16_t F1PIN,GPIO_TypeDef* B1PORT,uint16_t B1PIN,GPIO_TypeDef* F2PORT,uint16_t F2PIN,GPIO_TypeDef* B2PORT,uint16_t B2PIN){
	mt.FORWARD1_GPIOx=F1PORT;
	mt.FORWARD1_GPIO_Pin=F1PIN;
	mt.BACK1_GPIOx=B1PORT;
	mt.BACK1_GPIO_Pin=B1PIN;
	
	mt.FORWARD2_GPIOx=F2PORT;
	mt.FORWARD2_GPIO_Pin=F2PIN;
	mt.BACK2_GPIOx=B2PORT;
	mt.BACK2_GPIO_Pin=B2PIN;	
}
//Function Ultrasonic read
float SR04read(HCSRO4t* hc) {
  __HAL_TIM_SetCounter(&htim2, 0);
  HAL_GPIO_WritePin(hc->TRIGGER_GPIOx,hc->TRIGGER_GPIO_Pin,1);
  while(__HAL_TIM_GetCounter(&htim2) < 15);
  HAL_GPIO_WritePin(hc->TRIGGER_GPIOx, hc->TRIGGER_GPIO_Pin,0);
  while(!HAL_GPIO_ReadPin(hc->ECHO_GPIOx,hc->ECHO_GPIO_Pin) && (__HAL_TIM_GetCounter(&htim2) < 50000));
  __HAL_TIM_SetCounter(&htim2, 0);
  while(HAL_GPIO_ReadPin(hc->ECHO_GPIOx,hc->ECHO_GPIO_Pin)&& (__HAL_TIM_GetCounter(&htim2) < 50000));
  return ((float)__HAL_TIM_GetCounter(&htim2) * 0.01715 + SR04_OFFSET);
}
//Function start PWM
void WritePulse(TIM_HandleTypeDef* htimer, TIM_OC_InitTypeDef sConfig, int pulse, uint32_t channel) {
  sConfig.Pulse = pulse; // Set the new PWM pulse;
  HAL_TIM_PWM_ConfigChannel(htimer, &sConfig, channel);  // Reconfiguration of the htimer structure;
  HAL_TIM_PWM_Start(htimer, channel);  // Start the htimer TIMER ;
}
//Forward
void Forward(Motort *m){
		HAL_GPIO_WritePin(m->FORWARD1_GPIOx,m->FORWARD1_GPIO_Pin,1);
		HAL_GPIO_WritePin(m->FORWARD2_GPIOx,m->FORWARD2_GPIO_Pin,1);
		HAL_GPIO_WritePin(m->BACK1_GPIOx,m->BACK2_GPIO_Pin,0);
		HAL_GPIO_WritePin(m->BACK2_GPIOx,m->BACK2_GPIO_Pin,0);
}
// Back
void Back(Motort *m){
		HAL_GPIO_WritePin(m->FORWARD1_GPIOx,m->FORWARD1_GPIO_Pin,0);
		HAL_GPIO_WritePin(m->FORWARD2_GPIOx,m->FORWARD2_GPIO_Pin,0);
		HAL_GPIO_WritePin(m->BACK1_GPIOx,m->BACK1_GPIO_Pin,1);
		HAL_GPIO_WritePin(m->BACK2_GPIOx,m->BACK2_GPIO_Pin,1);
}
//Stop
void Stop(Motort *m){
		HAL_GPIO_WritePin(m->FORWARD1_GPIOx,m->FORWARD1_GPIO_Pin,0);
		HAL_GPIO_WritePin(m->FORWARD2_GPIOx,m->FORWARD2_GPIO_Pin,0);
		HAL_GPIO_WritePin(m->BACK1_GPIOx,m->BACK1_GPIO_Pin,0);
		HAL_GPIO_WritePin(m->BACK2_GPIOx,m->BACK2_GPIO_Pin,0);
}
//Right
void Right(Motort *m){
		HAL_GPIO_WritePin(m->FORWARD1_GPIOx,m->FORWARD1_GPIO_Pin,1);
		HAL_GPIO_WritePin(m->FORWARD2_GPIOx,m->FORWARD2_GPIO_Pin,0);
		HAL_GPIO_WritePin(m->BACK1_GPIOx,m->BACK1_GPIO_Pin,0);
		HAL_GPIO_WritePin(m->BACK2_GPIOx,m->BACK2_GPIO_Pin,0);
}
//Left
void Left(Motort *m){
		HAL_GPIO_WritePin(m->FORWARD1_GPIOx,m->FORWARD1_GPIO_Pin,0);
		HAL_GPIO_WritePin(m->FORWARD2_GPIOx,m->FORWARD2_GPIO_Pin,1);
		HAL_GPIO_WritePin(m->BACK1_GPIOx,m->BACK1_GPIO_Pin,0);
		HAL_GPIO_WritePin(m->BACK2_GPIOx,m->BACK2_GPIO_Pin,0);
}
//Rotation
void Rotation(Motort *m){
		HAL_GPIO_WritePin(m->FORWARD1_GPIOx,m->FORWARD1_GPIO_Pin,1);
		HAL_GPIO_WritePin(m->FORWARD2_GPIOx,m->FORWARD2_GPIO_Pin,0);
		HAL_GPIO_WritePin(m->BACK1_GPIOx,m->BACK1_GPIO_Pin,1);
		HAL_GPIO_WritePin(m->BACK2_GPIOx,m->BACK2_GPIO_Pin,0);
}


/* USER CODE END 0 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();

  /* USER CODE BEGIN 2 */

	float distance=0;
	char a;
	HCRSR04_init(GPIOC,GPIO_PIN_7,GPIOC,GPIO_PIN_6);
	Motor_init(GPIOD,GPIO_PIN_12,GPIOD,GPIO_PIN_13,GPIOD,GPIO_PIN_14,GPIOD,GPIO_PIN_15);
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);
	//WritePulse(&htim3,sConfigOC_TIM3,500,TIM_CHANNEL_1);
  //WritePulse(&htim4,sConfigOC_TIM4,500,TIM_CHANNEL_1);
	/* USER CODE END 2 */
	HAL_TIM_Base_Start(&htim2);
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */
		distance=SR04read(&hc);
		if (distance>5 && distance<10){
				buff_recive[0]='s';
		}
		HAL_UART_Receive(&huart2,buff_recive,2,1);
		if(buff_recive[0]=='1'){
				Forward(&mt);		
		}
		else if (buff_recive[0]=='2'){
				Back(&mt);	
		}
		else if(buff_recive[0]=='s'){
				Stop(&mt);
		}
		else if(buff_recive[0]=='3'){
				Right(&mt);
		}
		else if(buff_recive[0]=='4'){
				Left(&mt);
		}
		else if(buff_recive[0]=='5'){
				Rotation(&mt);
		}

		HAL_Delay(500);
  /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = (uint16_t) (SystemCoreClock / 1000000) - 1;;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535 - 1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
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

}
TIM_OC_InitTypeDef sConfigOC_TIM3;
/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 10;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 799;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

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
  sConfigOC.Pulse = 700;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim3);

}
TIM_OC_InitTypeDef sConfigOC_TIM4;
/* TIM4 init function */
static void MX_TIM4_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 10;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 799;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 700;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim4);

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PC6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
