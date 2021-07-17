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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "L506.h"
#include "trace.h"
#include "string.h"
#include "time.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
uint8_t data = 0;
uint8_t indexBuffer = 0;

volatile uint8_t flag_timer_10s = 0;
volatile uint8_t flag_timer_1000ms = 0;
volatile uint8_t flag_timer_500ms = 0;
volatile uint8_t flag_timer_2000ms = 0;

uint32_t vr_count_10s = 0;
uint32_t vr_count_1000ms = 0;
uint32_t vr_count_500ms = 0;
uint32_t vr_count_2000ms = 0;
uint32_t time_sendServer = 29500;
uint32_t time_conv = 0;

uint8_t send_count = 0;
uint8_t buffer[256];
uint8_t bufferTest[50];
uint8_t dataTest;
char revRtc[256];
char arrRevProcess[256]={0};

int result;

uint8_t value;
uint8_t test = 0;
uint8_t count = 0;
uint16_t num = 0;

uint8_t txTest[] = "quan\r\n";
uint8_t vr_test = 0;


// "\r\nOK\r\n"
// "\r\n+CPIN: READY\r\n\r\nOK\r\n"
// "\r\n+CSQ: "
// "\r\n+CGREG: 0,1\r\n\e\nOK\r\n"
// "\r\n+CREG: 0,1\r\n\r\nOK\r\n"
// "\r\n+CGATT: 1\r\n\r\nOK\r\n"
// "\r\nOK\r\n"
// "\r\nOK\r\n"
// ""
// ""
// ""
// ""


uint8_t arr[256];

int8_t gsm_state = -1;

RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef sDate = {0};

RTC_TimeTypeDef sTime2 = {0};
RTC_DateTypeDef sDate2 = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3);
		  vr_count_10s ++;
		  vr_count_1000ms ++;
		  vr_count_500ms ++;
		  vr_count_2000ms ++;

		  if(vr_count_10s == time_sendServer){
			  flag_timer_10s = 1; // timer du 10s thi gui du lieu len server
			  vr_count_10s = 0;
		  }
		  if(vr_count_1000ms == 1000){
			  flag_timer_1000ms = 1; // timer du 200ms
			  vr_count_1000ms = 0;
		  }
		  if(vr_count_500ms == 500){
			  flag_timer_500ms = 1; // timer du 500ms
			  vr_count_500ms = 0;
		  }
		  if(vr_count_2000ms == 2000){ // timer du 2s
			  flag_timer_2000ms = 1;
			  vr_count_2000ms = 0;
		  }
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART3)
	{
		if(data != 0){
		buffer[indexBuffer] = data;
		indexBuffer++;
		//HAL_UART_Transmit_IT(&huart1, (uint8_t *)&Data, 1);
		HAL_UART_Receive_IT(&huart3, &data, 1);
		}
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	t_uartAt arrInitialSim[]={{CMD_AT, 				{(uint8_t*)"AT\r\n",4},												fnParseOKPacket},
						  	  {CMD_CPIN, 			{(uint8_t*)"AT+CPIN?\r\n",10},										fnParseCPINPacket},
							  {CMD_CSQ,	 			{(uint8_t*)"AT+CSQ\r\n",8},											fnParseCSQPacket},
							  {CMD_CGREG, 			{(uint8_t*)"AT+CGREG?\r\n",11},										fnParseCGREGPacket},
							  {CMD_CREG,			{(uint8_t*)"AT+CREG?\r\n",10},										fnParseCREGPacket},
							  {CMD_CGATT, 			{(uint8_t*)"AT+CGATT?\r\n",11},										fnParseCGATTPacket},
							  {CMD_CIPTIMEOUT, 		{(uint8_t*)"AT+CIPTIMEOUT=30000,20000,40000,50000\r\n",39},			fnParseOKPacket},
							  {CMD_CIPMODE, 		{(uint8_t*)"AT+CIPMODE=0\r\n",14},									fnParseOKPacket},
							  {CMD_NETOPEN, 		{(uint8_t*)"AT+NETOPEN\r\n",12},									fnParseOKPacket},
							  {CMD_IPADDR, 			{(uint8_t*)"AT+IPADDR\r\n",11},										fnParseIPADDRPacket},
							  {CMD_CIPOPEN, 		{(uint8_t*)"AT+CIPOPEN=1,\"TCP\",\"113.190.240.47\",7580\r\n",42},	fnParseOKPacket},
							  {CMD_CIPOPQUERY, 		{(uint8_t*)"AT+CIPOPQUERY=1\r\n",17},								fnParseCIPOPQUERYPacket},
							  {CMD_CIPSEND, 		{(uint8_t*)"AT+CIPSEND=1,24\r\n",16},								fnParseSendSVPacket},
							  {CMD_CIPRXGET, 		{(uint8_t*)"\r\nAT+CIPRXGET=0,1\r\n",19},							fnParseReceiveSVPacket}};



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
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_USART3_UART_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart3, &data, 1);
  HAL_UART_Receive_IT(&huart1, &dataTest, 1);

  HAL_TIM_Base_Start_IT(&htim2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  switch (gsm_state){
  	 case -1:
  		GSM_Init();
  			gsm_state = CMD_AT;
  			break;
  	 case CMD_AT:
  		result = fnCheckPacket(arrInitialSim[CMD_AT].strSend.packetAt, arrInitialSim[CMD_AT].strSend.length, arrInitialSim[CMD_AT].fncType);
  		if(result != 0){
  			gsm_state = CMD_CPIN;
  			break;
  		}
  		else
  			wait_to_reinitial(10);
  		break;

  	 case CMD_CPIN:
  		result = fnCheckPacket(arrInitialSim[CMD_CPIN].strSend.packetAt, arrInitialSim[CMD_CPIN].strSend.length, arrInitialSim[CMD_CPIN].fncType);
  		if(result != 0){
  		  	gsm_state = CMD_CSQ;
  		  	break;
  		}
  		else
  			wait_to_reinitial(10);
  		break;

  	 case CMD_CSQ:
  		result = fnCheckPacket(arrInitialSim[CMD_CSQ].strSend.packetAt, arrInitialSim[CMD_CSQ].strSend.length, arrInitialSim[CMD_CSQ].fncType);
  		if(result != 0){
  			gsm_state = CMD_CGREG;
  			break;
  		}
  		else
  		  	wait_to_reinitial(10);
  		break;

  	 case CMD_CGREG:
  		result = fnCheckPacket(arrInitialSim[CMD_CGREG].strSend.packetAt, arrInitialSim[CMD_CGREG].strSend.length, arrInitialSim[CMD_CGREG].fncType);
   		if(result != 0){
   			gsm_state = CMD_CREG;
   			break;
   		}
   		else
   			wait_to_reinitial(10);
   		break;

  	 case CMD_CREG:
  		result = fnCheckPacket(arrInitialSim[CMD_CREG].strSend.packetAt, arrInitialSim[CMD_CREG].strSend.length, arrInitialSim[CMD_CREG].fncType);
    	if(result != 0){
    		gsm_state = CMD_CGATT;
    		break;
    	}
    	else
    		wait_to_reinitial(10);
    	break;

  	 case CMD_CGATT:
  		result = fnCheckPacket(arrInitialSim[CMD_CGATT].strSend.packetAt, arrInitialSim[CMD_CGATT].strSend.length, arrInitialSim[CMD_CGATT].fncType);
     	if(result != 0){
     		gsm_state = CMD_CIPTIMEOUT;
     		break;
     	}
     	else
     		wait_to_reinitial(10);
     	break;

  	 case CMD_CIPTIMEOUT:
  		result = fnCheckPacket(arrInitialSim[CMD_CIPTIMEOUT].strSend.packetAt, arrInitialSim[CMD_CIPTIMEOUT].strSend.length, arrInitialSim[CMD_CIPTIMEOUT].fncType);
      	if(result != 0){
      		gsm_state = CMD_CIPMODE;
      		break;
      	}
      	else
      		wait_to_reinitial(10);
      	break;

  	 case CMD_CIPMODE:
  		result = fnCheckPacket(arrInitialSim[CMD_CIPMODE].strSend.packetAt, arrInitialSim[CMD_CIPMODE].strSend.length, arrInitialSim[CMD_CIPMODE].fncType);
       	if(result != 0){
       		gsm_state = CMD_NETOPEN;
       		break;
       	}
       	else
       		wait_to_reinitial(10);
       	break;

 	 case CMD_NETOPEN:
 		result = fnCheckPacket(arrInitialSim[CMD_NETOPEN].strSend.packetAt, arrInitialSim[CMD_NETOPEN].strSend.length, arrInitialSim[CMD_NETOPEN].fncType);
        if(result != 0){
        	gsm_state = CMD_IPADDR;
        	break;
        }
        else
        	wait_to_reinitial(10);
        break;

 	 case CMD_IPADDR:
 		result = fnCheckPacket(arrInitialSim[CMD_IPADDR].strSend.packetAt, arrInitialSim[CMD_IPADDR].strSend.length, arrInitialSim[CMD_IPADDR].fncType);
         if(result != 0){
         	gsm_state = CMD_CIPOPEN;
         	break;
         }
         else
        	wait_to_reinitial(10);
         break;

 	 case CMD_CIPOPEN:
 		result = fnCheckPacket(arrInitialSim[CMD_CIPOPEN].strSend.packetAt, arrInitialSim[CMD_CIPOPEN].strSend.length, arrInitialSim[CMD_CIPOPEN].fncType);
         if(result != 0){
         	gsm_state = CMD_CIPOPQUERY;
         	break;
         }
         else
        	wait_to_reinitial(10);
         break;

 	 case CMD_CIPOPQUERY:
 		result = fnCheckPacket(arrInitialSim[CMD_CIPOPQUERY].strSend.packetAt, arrInitialSim[CMD_CIPOPQUERY].strSend.length, arrInitialSim[CMD_CIPOPQUERY].fncType);
         if(result != 0){
         	gsm_state = CMD_CIPSEND;
         	break;
         }
         else
        	 wait_to_reinitialTCP(10);
         break;

 	 case CMD_CIPSEND:
 		 if(flag_timer_10s == 1){
 			 result = fnCheckPacket(arrInitialSim[CMD_CIPSEND].strSend.packetAt,arrInitialSim[CMD_CIPSEND].strSend.length, arrInitialSim[CMD_CIPSEND].fncType);
 			 if(result != 0){
 				//GPRS_Ask(txTest, strlen(txTest)); // gui du lieu len server
 				num ++;
 				GPRS_Ask(arr, 24);
 			 }
 			 else{
 				 wait_to_reinitialTCP(3);
 				 break;
 			 }

 			flag_timer_10s = 0;
 		 }
 		 	 gsm_state = CMD_CIPRXGET;
 		 break;

 	 case CMD_CIPRXGET:
 		if(flag_timer_1000ms == 1){
 			 flag_timer_1000ms = 0;
 			 result = fnCheckPacket(arrInitialSim[CMD_CIPRXGET].strSend.packetAt,arrInitialSim[CMD_CIPRXGET].strSend.length, arrInitialSim[CMD_CIPRXGET].fncType);
 			 if(result == 1){
 				memcpy(revRtc, buffer, sizeof(buffer));
 				processChar(revRtc, '\n', arrRevProcess);
 				vr_test = strlen(arrRevProcess);
 				//HAL_UART_Transmit(&huart1, &vr_test, 1, 1000);
 				result = 0;
 				}
 			}
 			gsm_state = CMD_REVPROCESS;
 		break;

 	 case CMD_REVPROCESS:
 		 if(flag_timer_500ms == 1){
 			if(arrRevProcess[vr_test-1] == 1){
// 			 if(arrRevProcess[0] != 0){
 				takeTime((uint8_t*)arrRevProcess);
 				HAL_RTC_SetTime(&hrtc, &sTime2, RTC_FORMAT_BCD);
 				HAL_RTC_SetDate(&hrtc, &sDate2, RTC_FORMAT_BCD);
 				memset(arrRevProcess, '\0', 256);
 			}
 			if(arrRevProcess[vr_test-1] == 2){
 				time_conv = convertTimeSendSV(arrRevProcess[vr_test-2]);
 				time_sendServer = time_conv / 0.001;
 				memset(arrRevProcess, '\0', 256);
 			}
 				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
 				HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD);
 				convertTime(arr, num);
 				flag_timer_500ms = 0;
 		}
 		gsm_state = CMD_TRANSRTC;
 		break;

 	 case CMD_TRANSRTC:
 			 if(flag_timer_2000ms == 1){
 				  HAL_UART_Transmit(&huart1, arr, 17, 1000);
 				  flag_timer_2000ms = 0;
 			 }
 		gsm_state = CMD_CIPSEND;
 		break;
  	  }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_USART3;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

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
  htim2.Init.Prescaler = 16;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
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
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
