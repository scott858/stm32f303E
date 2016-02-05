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
#include "arm_math.h"
#include "arm_common_tables.h"

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

#define dataBufferSize 18
uint8_t dataBuffer[dataBufferSize] = { 0 };

uint32_t SDWriteFinished = 1;

/* ADC handler declaration */
ADC_HandleTypeDef AdcHandle_master;
ADC_HandleTypeDef AdcHandle_slave;

/* Variable containing ADC conversions results */
#define convertedValueBufferSize 1024
#define sinBufferSize 1024
__IO uint16_t uhDACxConvertedValue = 0;
__IO uint16_t sinBuffer[sinBufferSize] = {0};
__IO uint32_t aADCDualConvertedValue[convertedValueBufferSize];
__IO uint16_t parsedDataBuffer[convertedValueBufferSize];
__IO uint16_t parsedDataHalfBuffer[convertedValueBufferSize / 2];
__IO uint8_t aADCDualConversionDone = 0;
__IO uint8_t aADCDualConversionValue = 0;

DAC_HandleTypeDef DacHandle;
static DAC_ChannelConfTypeDef sConfig;
const uint8_t aEscalator8bit[6] = { 0x0, 0x33, 0x66, 0x99, 0xCC, 0xFF };
__IO uint8_t ubSelectedWavesForm = 1;
__IO uint8_t ubKeyPressed = SET;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void Error_Handler(void);
static void ADC_Config(void);
void writeData(int start, int end);
void writeAsciiData(int start, int end);

static void DAC_Ch1_TriangleConfig(void);
static void DAC_Ch1_EscalatorConfig(void);
static void TIM6_Config(void);

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle);
static void DAC_Ch1_SinConfig(void);

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

	/*##-2- Configure the ADC peripheral ######################################*/
	ADC_Config();

	/* Run the ADC calibration in single-ended mode */
	if (HAL_ADCEx_Calibration_Start(&AdcHandle_master, ADC_SINGLE_ENDED)
			!= HAL_OK) {
		/* Calibration Error */
		Error_Handler();
	}

	if (HAL_ADCEx_Calibration_Start(&AdcHandle_slave, ADC_SINGLE_ENDED)
			!= HAL_OK) {
		/* Calibration Error */
		Error_Handler();
	}

	/*## Enable peripherals ####################################################*/

	/* Enable ADC slave */
	if (HAL_ADC_Start(&AdcHandle_slave) != HAL_OK) {
		/* Start Error */
		Error_Handler();
	}

	/*## Start ADC conversions #################################################*/

	/* Start ADCx and ADCy multimode conversion with interruption */
	if (HAL_ADCEx_MultiModeStart_DMA(&AdcHandle_master,
			(uint32_t *) aADCDualConvertedValue, convertedValueBufferSize)
			!= HAL_OK) {
		/* Start Error */
		Error_Handler();
	}

	/*##-1- Configure the DAC peripheral #######################################*/
	DacHandle.Instance = DACx;

	/*##-2- Configure the TIM peripheral #######################################*/
	TIM6_Config();

	HAL_DAC_DeInit(&DacHandle);
	DAC_Ch1_SinConfig();
//	DAC_Ch1_TriangleConfig();
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
 * @note   This function configures the ADC peripheral
 1) Configuration of ADCx peripheral (ADC master)
 2) Configuration of ADCy peripheral (ADC slave)
 3) Configuration of channel on ADCx regular group on rank 1
 4) Configuration of channel on ADCy regular group on rank 1
 5) Configuration of multimode
 * @param  None
 * @retval None
 */
static void ADC_Config(void) {
	ADC_ChannelConfTypeDef sConfig;
	ADC_MultiModeTypeDef mode;

	/*##-1- Configuration of ADCx peripheral (ADC master) ######################*/
	/* Configuration of ADCx init structure: ADC parameters and regular group */
	AdcHandle_master.Instance = ADCx;

	AdcHandle_master.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1; /* ADC clock to AHB without prescaler to have maximum frequency 72MHz */
	AdcHandle_master.Init.Resolution = ADC_RESOLUTION_12B; /* ADC resolution 6 bits to have conversion time = 6.5 ADC clock cycles */
	AdcHandle_master.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	AdcHandle_master.Init.ScanConvMode = DISABLE; /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
	AdcHandle_master.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	AdcHandle_master.Init.LowPowerAutoWait = DISABLE;
	AdcHandle_master.Init.ContinuousConvMode = ENABLE; /* Continuous mode to have maximum conversion speed (no delay between conversions) */
	AdcHandle_master.Init.NbrOfConversion = 1; /* Parameter discarded because sequencer is disabled */
	AdcHandle_master.Init.DiscontinuousConvMode = DISABLE; /* Parameter discarded because sequencer is disabled */
	AdcHandle_master.Init.NbrOfDiscConversion = 1; /* Parameter discarded because sequencer is disabled */
	AdcHandle_master.Init.ExternalTrigConv = ADC_SOFTWARE_START; /* Software start to trig the 1st conversion manually, without external event */
	AdcHandle_master.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	AdcHandle_master.Init.DMAContinuousRequests = ENABLE; /* ADC-DMA continuous requests to match with DMA in circular mode */
	AdcHandle_master.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;

	if (HAL_ADC_Init(&AdcHandle_master) != HAL_OK) {
		/* Initialization Error */
		Error_Handler();
	}

	/*##-2- Configuration of ADCy peripheral (ADC slave) #######################*/
	AdcHandle_slave.Instance = ADCy;

	/* Configuration of ADCy init structure: ADC parameters and regular group */
	/* Same configuration as ADCx */
	AdcHandle_slave.Init = AdcHandle_master.Init;

	if (HAL_ADC_Init(&AdcHandle_slave) != HAL_OK) {
		/* Initialization Error */
		Error_Handler();
	}

	/*##-3- Configuration of channel on ADCx regular group on rank 1 ###########*/
	sConfig.Channel = ADCx_CHANNELa;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_4CYCLES_5;
	sConfig.Offset = 0;

	if (HAL_ADC_ConfigChannel(&AdcHandle_master, &sConfig) != HAL_OK) {
		/* Channel Configuration Error */
		Error_Handler();
	}

	/*##-4- Configuration of channel on ADCy regular group on rank 1 ###########*/
	/* Same channel as ADCx for dual mode interleaved: both ADC are converting  */
	/* the same channel.                                                        */
	sConfig.Channel = ADCy_CHANNELa;

	if (HAL_ADC_ConfigChannel(&AdcHandle_slave, &sConfig) != HAL_OK) {
		/* Channel Configuration Error */
		Error_Handler();
	}

	/*##-5- Configuration of multimode #########################################*/
	/* Multimode parameters settings and set ADCy (slave) under control of      */
	/* ADCx (master).                                                           */
	mode.Mode = ADC_DUALMODE_REGSIMULT;
	mode.DMAAccessMode = ADC_DMAACCESSMODE_12_10_BITS;
//	mode.TwoSamplingDelay = ADC_TWOSAMPLINGDELAY_12CYCLES;
	if (HAL_ADCEx_MultiModeConfigChannel(&AdcHandle_master, &mode) != HAL_OK) {
		/* Channel Configuration Error */
		Error_Handler();
	}
}

void writeAsciiData(int start, int end){
	uint32_t masterConvertedValue = 0;
	uint32_t slaveConvertedValue = 0;
	uint32_t compoundConvertedValue = 0;
	uint32_t sysTime = HAL_GetTick();
	aADCDualConversionDone = 1;
	adcTick += 1;
	if (!SDWriteFinished) {

		int i, j;
		for (j = start; j < end; j++) {
			compoundConvertedValue = aADCDualConvertedValue[j];
			masterConvertedValue += compoundConvertedValue & 0xFFFF;
			slaveConvertedValue += compoundConvertedValue >> 16;
		}
		masterConvertedValue = 2 * masterConvertedValue / convertedValueBufferSize;
		slaveConvertedValue = 2 * slaveConvertedValue / convertedValueBufferSize;
		for (i = 0; i < dataBufferSize; i++) {
			dataBuffer[i] = 32;
		}
		sprintf(dataBuffer, "%u, %u, %u", sysTime, slaveConvertedValue,
				masterConvertedValue);
		for (i = 0; i < dataBufferSize; i++) {
			if (dataBuffer[i] == 0) {
				dataBuffer[i] = 32;
			}
		}
		dataBuffer[dataBufferSize - 1] = 13;
		res = f_write(&MyFile, dataBuffer, sizeof(dataBuffer),
				(void *) &byteswritten);
		if (adcTick >= 80000) {
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

void writeData(int start, int end){
	uint32_t masterConvertedValue = 0;
//	uint32_t slaveConvertedValue = 0;
//	uint32_t compoundConvertedValue = 0;
//	uint32_t sysTime = HAL_GetTick();
//	uint16_t averageValue = 0;
	aADCDualConversionDone = 1;
	if (!SDWriteFinished) {
		adcTick += 1;

		int j;
		for (j = start; j < end; j++) {
//			parsedDataHalfBuffer[j - start] = aADCDualConvertedValue[j] & 0xFFFF;
//			compoundConvertedValue = aADCDualConvertedValue[j];
			masterConvertedValue += aADCDualConvertedValue[j] & 0xFFFF;
//			slaveConvertedValue += compoundConvertedValue >> 16;
		}
//		masterConvertedValue = 2 * masterConvertedValue / convertedValueBufferSize;
//		slaveConvertedValue = 2 * slaveConvertedValue / convertedValueBufferSize;

//		compoundConvertedValue = 0;
//		compoundConvertedValue += masterConvertedValue & 0xFFFF;
//		compoundConvertedValue += slaveConvertedValue >> 16;
//		averageValue = masterConvertedValue;

		res = f_write(&MyFile, (uint32_t*)&masterConvertedValue, sizeof(masterConvertedValue),
				(void *) &byteswritten);
		if (adcTick >= 5000) {
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

/**
 * @brief  Conversion complete callback in non blocking mode
 * @param  AdcHandle : AdcHandle handle
 * @note   This example shows a simple way to report end of conversion, and
 *         you can add your own implementation.
 * @retval None
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	/* Get the converted value of regular channel */
	int start = convertedValueBufferSize / 2;
	int end = convertedValueBufferSize;
//	writeAsciiData(start, end);
	writeData(start, end);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
	/* Get the converted value of regular channel */
	int start = 0;
	int end = convertedValueBufferSize / 2;
//	writeAsciiData(start, end);
	writeData(start, end);
}

void ConvCpltCallbackCSV(ADC_HandleTypeDef *hadc) {
	/* Get the converted value of regular channel */
	aADCDualConversionDone = 1;
	adcTick += 1;
	if (!SDWriteFinished) {
		int i;
		for (i = 0; i < dataBufferSize; i++) {
			dataBuffer[i] = 32;
		}
		sprintf(dataBuffer, "%u, %u", HAL_GetTick(), aADCDualConversionValue);
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

void ConvCpltCallbackBinary(ADC_HandleTypeDef *hadc) {
	/* Get the converted value of regular channel */
	aADCDualConversionDone = 1;
	adcTick += 1;
	if (!SDWriteFinished) {
		res = f_write(&MyFile, aADCDualConvertedValue,
				sizeof(aADCDualConvertedValue), (void *) &byteswritten);
		if (adcTick >= 10000) {
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

#define PERIOD 2520
static void DAC_Ch1_SinConfig(void) {
	float32_t phase, sin, cos;

	uint16_t waveformValue;

	int i;
	for(i=0; i<sinBufferSize; i++){
		phase = ((float32_t) i) * 2. * 3.14 / ((float32_t) sinBufferSize);
		sin = arm_sin_f32(phase * 10.0);
		cos = arm_cos_f32(phase);

		waveformValue = 4095. * (2. + .3 * sin + cos) / 4.;
		sinBuffer[i] = waveformValue;
	}

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
	if (HAL_DAC_Start_DMA(&DacHandle, DACx_CHANNEL, (uint32_t *)sinBuffer,
			(size_t)sinBufferSize, DAC_ALIGN_12B_R) != HAL_OK) {
		/* Start DMA Error */
		Error_Handler();
	}
}

/**
 * @brief  Conversion complete callback in non blocking mode for Channel1
 * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
 *         the configuration information for the specified DAC.
 * @retval None
 */
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac) {
//	SET_DAC();
}

void SET_DAC() {
	float32_t phase = ((float32_t) HAL_GetTick() * (convertedValueBufferSize / 2)) * 2. * 3.14 / ((float32_t) PERIOD);
	float32_t sin = arm_sin_f32(phase * 10.0);
	float32_t cos = arm_cos_f32(phase);

	uint16_t waveformValue = 4095. * (2. + .3 * sin + cos) / 4.;

	HAL_DAC_SetValue(&DacHandle, DACx_CHANNEL, DAC_ALIGN_12B_R, waveformValue);
	HAL_DAC_Start(&DacHandle, DACx_CHANNEL);
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

	htim.Init.Period = 0xFFF;
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
