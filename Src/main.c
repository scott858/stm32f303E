/**
 ******************************************************************************
 * @file    FatFs/FatFs_uSD/Src/main.c
 * @author  MCD Application Team
 * @version V1.4.0
 * @date    13-November-2015
 * @brief   Main program body
 *          This sample code shows how to use FatFs with uSD card drive.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
FATFS SDFatFs; /* File system object for SD card logical drive */
FIL MyFile; /* File object */
char SDPath[4]; /* SD card logical drive path */

FRESULT res; /* FatFs function common result code */
uint32_t byteswritten, bytesread, adcTick = 0; /* File write/read counts */
uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
uint8_t rtext[100]; /* File read buffer */

#define dataBufferSize 13
uint8_t dataBuffer[dataBufferSize] = { 0 };

uint32_t SDWriteFinished = 1;

/* ADC handler declaration */
ADC_HandleTypeDef AdcHandle;

DAC_HandleTypeDef DacHandle;
static DAC_ChannelConfTypeDef sConfig;
const uint8_t aEscalator8bit[6] = { 0x0, 0x33, 0x66, 0x99, 0xCC, 0xFF };
__IO uint8_t ubSelectedWavesForm = 1;
__IO uint8_t ubKeyPressed = SET;

/* TIM handler declaration */
static TIM_HandleTypeDef htim;

/* Variable used to get converted value */
__IO uint16_t uhADCxConvertedValue = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void Error_Handler(void);
static void ADC_Config(void);
static void TIM_Config(void);

static void DAC_Ch1_TriangleConfig(void);
static void DAC_Ch1_EscalatorConfig(void);
static void TIM6_Config(void);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void) {
	/* STM32F3xx HAL library initialization:
	 - Configure the Flash prefetch
	 - Systick timer is configured by default as source of time base, but user
	 can eventually implement his proper time base source (a general purpose
	 timer for example or other time source), keeping in mind that Time base
	 duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
	 handled in milliseconds basis.
	 - Set NVIC Group Priority to 4
	 - Low Level Initialization
	 */
	HAL_Init();

	/* Configure LED1 and LED3 */
	BSP_LED_Init(LED1);
	BSP_LED_Init(LED3);

	/* Configure the system clock to 72 MHz */
	SystemClock_Config();

	/*##-1- TIM Peripheral Configuration ######################################*/
	TIM_Config();

	/*##-2- Configure the ADC peripheral ######################################*/
	ADC_Config();

	/*##-4- Start the conversion process and enable interrupt ##################*/
	if (HAL_ADC_Start_IT(&AdcHandle) != HAL_OK) {
		/* Start Conversation Error */
		Error_Handler();
	}

	/*##-3- TIM counter enable ################################################*/
	if (HAL_TIM_Base_Start(&htim) != HAL_OK) {
		/* Counter Enable Error */
		Error_Handler();
	}

	/*##-1- Configure the DAC peripheral #######################################*/
	DacHandle.Instance = DACx;

	/*##-2- Configure the TIM peripheral #######################################*/
	TIM6_Config();

	HAL_DAC_DeInit(&DacHandle);
	DAC_Ch1_TriangleConfig();
//	DAC_Ch1_EscalatorConfig();

	/*##-1- Link the micro SD disk I/O driver ##################################*/
	if (FATFS_LinkDriver(&SD_Driver, SDPath) == 0) {
		/*##-2- Register the file system object to the FatFs module ##############*/
		if (f_mount(&SDFatFs, (TCHAR const*) SDPath, 0) != FR_OK) {
			/* FatFs Initialization Error */
			Error_Handler();
		} else {
			/*##-3- Create a FAT file system (format) on the logical drive #########*/
			/* WARNING: Formatting the uSD card will delete all content on the device */
			if (f_mkfs((TCHAR const*) SDPath, 0, 0) != FR_OK) {
				/* FatFs Format Error */
				Error_Handler();
			} else {
				/*##-4- Create and Open a new text file object with write access #####*/
				if (f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE)
						!= FR_OK) {
					/* 'STM32.TXT' file Open for write Error */
					Error_Handler();
				} else {
					/*##-5- Write data to the text file ################################*/
					res = f_write(&MyFile, wtext, sizeof(wtext),
							(void *) &byteswritten);

					/*##-6- Close the open text file #################################*/
					if (f_close(&MyFile) != FR_OK) {
						Error_Handler();
					}

					if ((byteswritten == 0) || (res != FR_OK)) {
						/* 'STM32.TXT' file Write or EOF Error */
						Error_Handler();
					} else {
						/*##-7- Open the text file object with read access ###############*/
						if (f_open(&MyFile, "STM32.TXT", FA_READ) != FR_OK) {
							/* 'STM32.TXT' file Open for read Error */
							Error_Handler();
						} else {
							/*##-8- Read data from the text file ###########################*/
							res = f_read(&MyFile, rtext, sizeof(rtext),
									(UINT*) &bytesread);

							if ((bytesread == 0) || (res != FR_OK)) {
								/* 'STM32.TXT' file Read or EOF Error */
								Error_Handler();
							} else {
								/*##-9- Close the open text file #############################*/
								f_close(&MyFile);

								/*##-10- Compare read data with the expected data ############*/
								if ((bytesread != byteswritten)) {
									/* Read data is different from the expected data */
									Error_Handler();
								} else {
									/* Success of the demo: no error occurrence */
									BSP_LED_On(LED2);
								}
							}
						}
					}
				}
			}
		}
	}

	if (f_open(&MyFile, "DATA.TXT", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
		/* 'STM32.TXT' file Open for write Error */
		Error_Handler();
	}

	/*##-11- Unlink the RAM disk I/O driver ####################################*/
//	FATFS_UnLinkDriver(SDPath);
	SDWriteFinished = 0;
	/* Infinite loop */
	while (1) {
	}
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 72000000
 *            HCLK(Hz)                       = 72000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 2
 *            APB2 Prescaler                 = 1
 *            HSE Frequency(Hz)              = 8000000
 *            HSE PREDIV                     = RCC_PREDIV_DIV1 (1)
 *            PLLMUL                         = RCC_PLL_MUL9 (9)
 *            Flash Latency(WS)              = 2
 * @param  None
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/**
 *
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
static void Error_Handler(void) {
	while (1) {
		/* Toggle LED3 fast */
		BSP_LED_Toggle(LED3);
		HAL_Delay(40);
	}
}

/**
 * @brief  EXTI line detection callbacks.
 * @param  GPIO_Pin: Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == SD_DETECT_PIN) {
		/* Check SD card detect pin */
		BSP_SD_IsDetected();
	}
}

/**
 * @brief  ADC configuration
 * @param  None
 * @retval None
 */
static void ADC_Config(void) {
	ADC_ChannelConfTypeDef sConfig;

	/* ADC Initialization */
	AdcHandle.Instance = ADCx;

	AdcHandle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	AdcHandle.Init.Resolution = ADC_RESOLUTION_12B;
	AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	AdcHandle.Init.ScanConvMode = DISABLE; /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
	AdcHandle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	AdcHandle.Init.LowPowerAutoWait = DISABLE;
	AdcHandle.Init.ContinuousConvMode = DISABLE; /* Continuous mode disabled to have only 1 conversion at each conversion trig */
	AdcHandle.Init.NbrOfConversion = 1; /* Parameter discarded because sequencer is disabled */
	AdcHandle.Init.DiscontinuousConvMode = DISABLE; /* Parameter discarded because sequencer is disabled */
	AdcHandle.Init.NbrOfDiscConversion = 1; /* Parameter discarded because sequencer is disabled */
	AdcHandle.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO; /* Conversion start trigged at each external event */
	AdcHandle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
	AdcHandle.Init.DMAContinuousRequests = ENABLE;
	AdcHandle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;

	if (HAL_ADC_Init(&AdcHandle) != HAL_OK) {
		/* ADC initialization Error */
		Error_Handler();
	}

	/* Configure ADC regular channel */
	sConfig.Channel = ADCx_CHANNEL;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_19CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;

	if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK) {
		/* Channel Configuration Error */
		Error_Handler();
	}
}

/**
 * @brief  TIM configuration
 * @param  None
 * @retval None
 */
static void TIM_Config(void) {
	TIM_MasterConfigTypeDef sMasterConfig;

	/* Time Base configuration */
	htim.Instance = TIMx;

	htim.Init.Period = 7200;
	htim.Init.Prescaler = 0;
	htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim.Init.RepetitionCounter = 0x0;

	if (HAL_TIM_Base_Init(&htim) != HAL_OK) {
		/* Timer initialization Error */
		Error_Handler();
	}

	/* Timer TRGO selection */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

	if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig)
			!= HAL_OK) {
		/* Timer TRGO selection Error */
		Error_Handler();
	}
}

/**
 * @brief  Conversion complete callback in non blocking mode
 * @param  AdcHandle : AdcHandle handle
 * @note   This example shows a simple way to report end of conversion, and
 *         you can add your own implementation.
 * @retval None
 */void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *AdcHandle) {
	/* Get the converted value of regular channel */
	uhADCxConvertedValue = HAL_ADC_GetValue(AdcHandle);
	adcTick += 1;
	if (!SDWriteFinished) {
		int i;
		for (i = 0; i < dataBufferSize; i++) {
			dataBuffer[i] = 32;
		}
		sprintf(dataBuffer, "%d, %d", HAL_GetTick(), uhADCxConvertedValue);
		for (i = 0; i < dataBufferSize; i++) {
			if (dataBuffer[i] == 0) {
				dataBuffer[i] = 32;
			}
		}
		dataBuffer[dataBufferSize - 1] = 13;
		res = f_write(&MyFile, dataBuffer, sizeof(dataBuffer),
				(void *) &byteswritten);
		if (adcTick >= 100000) {
			SDWriteFinished = 1;
			if (f_close(&MyFile) != FR_OK) {
				Error_Handler();
			} else {
				/*##-11- Unlink the RAM disk I/O driver ####################################*/
				FATFS_UnLinkDriver(SDPath);
				BSP_LED_On(LED1);
			}
		}
	}
}

static void DAC_Ch1_EscalatorConfig(void) {
	/*##-1- Initialize the DAC peripheral ######################################*/
	if (HAL_DAC_Init(&DacHandle) != HAL_OK) {
		/* Initialization Error */
		Error_Handler();
	}

	/*##-1- DAC channel1 Configuration #########################################*/
	sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

	if (HAL_DAC_ConfigChannel(&DacHandle, &sConfig, DACx_CHANNEL) != HAL_OK) {
		/* Channel configuration Error */
		Error_Handler();
	}

	/*##-2- Enable DAC selected channel and associated DMA #############################*/
	if (HAL_DAC_Start_DMA(&DacHandle, DACx_CHANNEL, (uint32_t *) aEscalator8bit,
			6, DAC_ALIGN_8B_R) != HAL_OK) {
		/* Start DMA Error */
		Error_Handler();
	}
}

/**
 * @brief  DAC Channel1 Triangle Configuration
 * @param  None
 * @retval None
 */
static void DAC_Ch1_TriangleConfig(void) {
	/*##-1- Initialize the DAC peripheral ######################################*/
	if (HAL_DAC_Init(&DacHandle) != HAL_OK) {
		/* DAC initialization Error */
		Error_Handler();
	}

	/*##-2- DAC channel2 Configuration #########################################*/
	sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

	if (HAL_DAC_ConfigChannel(&DacHandle, &sConfig, DACx_CHANNEL) != HAL_OK) {
		/* Channel configuartion Error */
		Error_Handler();
	}

	/*##-3- DAC channel2 Triangle Wave generation configuration ################*/
	if (HAL_DACEx_TriangleWaveGenerate(&DacHandle, DACx_CHANNEL,
	DAC_TRIANGLEAMPLITUDE_1023) != HAL_OK) {
		/* Triangle wave generation Error */
		Error_Handler();
	}

	/*##-4- Enable DAC Channel1 ################################################*/
	if (HAL_DAC_Start(&DacHandle, DACx_CHANNEL) != HAL_OK) {
		/* Start Error */
		Error_Handler();
	}

	/*##-5- Set DAC channel1 DHR12RD register ################################################*/
	if (HAL_DAC_SetValue(&DacHandle, DACx_CHANNEL, DAC_ALIGN_12B_R, 0x100)
			!= HAL_OK) {
		/* Setting value Error */
		Error_Handler();
	}
}

/**
 * @brief  TIM6 Configuration
 * @note   TIM6 configuration is based on APB1 frequency
 * @note   TIM6 Update event occurs each TIM6CLK/256
 * @param  None
 * @retval None
 */
void TIM6_Config(void) {
	static TIM_HandleTypeDef htim;
	TIM_MasterConfigTypeDef sMasterConfig;

	/*##-1- Configure the TIM peripheral #######################################*/
	/* Time base configuration */
	htim.Instance = TIM6;

	htim.Init.Period = 0x7FF;
	htim.Init.Prescaler = 0;
	htim.Init.ClockDivision = 0;
	htim.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&htim);

	/* TIM6 TRGO selection */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

	HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig);

	/*##-2- Enable TIM peripheral counter ######################################*/
	HAL_TIM_Base_Start(&htim);
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{}
}

#endif

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
