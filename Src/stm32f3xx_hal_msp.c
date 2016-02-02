/**
 ******************************************************************************
 * File Name          : stm32f3xx_hal_msp.c
 * Description        : This file provides code for the MSP Initialization
 *                      and de-Initialization codes.
 ******************************************************************************
 *
 * COPYRIGHT(c) 2016 STMicroelectronics
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
#include "stm32f3xx_hal.h"

/* USER CODE BEGIN 0 */
#include "main.h"

/* USER CODE END 0 */

/**
 * Initializes the Global MSP.
 */
void HAL_MspInit(void) {
	/* USER CODE BEGIN MspInit 0 */

	/* USER CODE END MspInit 0 */

	__SYSCFG_CLK_ENABLE()
	;

	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	/* System interrupt init*/
	/* MemoryManagement_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
	/* DebugMonitor_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);
	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

	/* USER CODE BEGIN MspInit 1 */

	/* USER CODE END MspInit 1 */
}

/**
 * @brief ADC MSP Initialization
 *        This function configures the hardware resources used in this example:
 *          - Enable peripheral's clock
 *          - Configure the GPIO associated to the peripheral channels
 *          - Configure the DMA associated to the peripheral
 *          - Configure the NVIC associated to the peripheral interruptions
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc) {
	GPIO_InitTypeDef GPIO_InitStruct;
	static DMA_HandleTypeDef DmaHandle;

	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* Enable clock of GPIO associated to the peripheral channels */
	ADCx_CHANNELa_GPIO_CLK_ENABLE();
	/* Enable clock of ADCx peripheral */
	ADCx_CLK_ENABLE()
	;
	/* Enable clock of DMA associated to the peripheral */
	ADCx_DMA_CLK_ENABLE();

	/* Note: ADC slave does not need additional configuration, since it shares  */
	/*       the same clock domain, same GPIO pins (interleaved on the same     */
	/*       channel) and same DMA as ADC master.                               */

	/*##-2- Configure peripheral GPIO ##########################################*/
	/* ADCx Channel GPIO pin configuration */
	GPIO_InitStruct.Pin = ADCx_CHANNELa_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(ADCx_CHANNELa_GPIO_PORT, &GPIO_InitStruct);

	/*##-3- Configure the DMA ##################################################*/
	/* Configure DMA parameters */
	DmaHandle.Instance = ADCx_DMA_STREAM; /* DMA stream of ADC master */

	DmaHandle.Init.Direction = DMA_PERIPH_TO_MEMORY;
	DmaHandle.Init.PeriphInc = DMA_PINC_DISABLE;
	DmaHandle.Init.MemInc = DMA_MINC_ENABLE;
	DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; /* Transfer from ADC by half-word to match with ADC resolution 6 or 8 bits */
	DmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD; /* Transfer to memory by half-word to match with buffer variable type: half-word */
	DmaHandle.Init.Mode = DMA_CIRCULAR; /* DMA in circular mode to match with ADC-DMA continuous requests */
	DmaHandle.Init.Priority = DMA_PRIORITY_HIGH;

	/* Deinitialize  & Initialize the DMA for new transfer */
	HAL_DMA_DeInit(&DmaHandle);
	HAL_DMA_Init(&DmaHandle);

	/* Associate the initialized DMA handle to the ADC handle */
	__HAL_LINKDMA(hadc, DMA_Handle, DmaHandle);

	/*##-4- Configure the NVIC #################################################*/
	/* NVIC configuration for DMA transfer complete interrupt */
	HAL_NVIC_SetPriority(ADCx_DMA_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(ADCx_DMA_IRQn);
}

/**
 * @brief ADC MSP De-Initialization
 *        This function frees the hardware resources used in this example:
 *          - Disable peripheral's clock
 *          - Revert GPIO associated to the peripheral channels to their default state
 *          - Revert DMA associated to the peripheral to its default state
 *          - Revert NVIC associated to the peripheral interruptions to its default state
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc) {

	/*##-1- Reset peripherals ##################################################*/
	ADCx_FORCE_RESET();
	ADCx_RELEASE_RESET();

	/*##-2- Disable peripherals and GPIO Clocks ################################*/
	/* De-initialize the ADC Channel GPIO pin */
	HAL_GPIO_DeInit(ADCx_CHANNELa_GPIO_PORT, ADCx_CHANNELa_PIN);

	/*##-3- Disable the DMA Streams ############################################*/
	/* De-Initialize the DMA Stream associate to transmission process */
	HAL_DMA_DeInit(hadc->DMA_Handle);

	/*##-4- Disable the NVIC for DMA ###########################################*/
	HAL_NVIC_DisableIRQ(ADCx_DMA_IRQn);
}

/**
 * @brief TIM MSP Initialization
 *        This function configures the hardware resources used in this example:
 *           - Peripheral's clock enable
 *           - Peripheral's GPIO Configuration
 * @param htim: TIM handle pointer
 * @retval None
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) {
	/* TIM6 Periph clock enable */
	__HAL_RCC_TIM6_CLK_ENABLE()
	;
}

/**
 * @brief TIM MSP De-Initialization
 *        This function frees the hardware resources used in this example:
 *          - Disable the Peripheral's clock
 *          - Revert GPIO to their default state
 * @param htim: TIM handle pointer
 * @retval None
 */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim) {
	/*##-1- Reset peripherals ##################################################*/
	__HAL_RCC_TIM6_FORCE_RESET();
	__HAL_RCC_TIM6_RELEASE_RESET();
}

void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c) {

	GPIO_InitTypeDef GPIO_InitStruct;
	if (hi2c->Instance == I2C1) {
		/* USER CODE BEGIN I2C1_MspInit 0 */

		/* USER CODE END I2C1_MspInit 0 */

		/**I2C1 GPIO Configuration
		 PA14     ------> I2C1_SDA
		 PA15     ------> I2C1_SCL
		 */
		GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* Peripheral clock enable */
		__I2C1_CLK_ENABLE()
		;
		/* USER CODE BEGIN I2C1_MspInit 1 */

		/* USER CODE END I2C1_MspInit 1 */
	}

}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c) {

	if (hi2c->Instance == I2C1) {
		/* USER CODE BEGIN I2C1_MspDeInit 0 */

		/* USER CODE END I2C1_MspDeInit 0 */
		/* Peripheral clock disable */
		__I2C1_CLK_DISABLE();

		/**I2C1 GPIO Configuration
		 PA14     ------> I2C1_SDA
		 PA15     ------> I2C1_SCL
		 */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_14 | GPIO_PIN_15);

	}
	/* USER CODE BEGIN I2C1_MspDeInit 1 */

	/* USER CODE END I2C1_MspDeInit 1 */

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {

	GPIO_InitTypeDef GPIO_InitStruct;
	if (hspi->Instance == SPI1) {
		/* USER CODE BEGIN SPI1_MspInit 0 */

		/* USER CODE END SPI1_MspInit 0 */
		/* Peripheral clock enable */
		__SPI1_CLK_ENABLE()
		;

		/**SPI1 GPIO Configuration
		 PA5     ------> SPI1_SCK
		 PA6     ------> SPI1_MISO
		 PA7     ------> SPI1_MOSI
		 */
		GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* USER CODE BEGIN SPI1_MspInit 1 */

		/* USER CODE END SPI1_MspInit 1 */
	}

}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {

	if (hspi->Instance == SPI1) {
		/* USER CODE BEGIN SPI1_MspDeInit 0 */

		/* USER CODE END SPI1_MspDeInit 0 */
		/* Peripheral clock disable */
		__SPI1_CLK_DISABLE();

		/**SPI1 GPIO Configuration
		 PA5     ------> SPI1_SCK
		 PA6     ------> SPI1_MISO
		 PA7     ------> SPI1_MOSI
		 */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

	}
	/* USER CODE BEGIN SPI1_MspDeInit 1 */

	/* USER CODE END SPI1_MspDeInit 1 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* huart) {

	GPIO_InitTypeDef GPIO_InitStruct;
	if (huart->Instance == UART4) {
		/* USER CODE BEGIN UART4_MspInit 0 */

		/* USER CODE END UART4_MspInit 0 */
		/* Peripheral clock enable */
		__UART4_CLK_ENABLE()
		;

		/**UART4 GPIO Configuration
		 PC10     ------> UART4_TX
		 PC11     ------> UART4_RX
		 */
		GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF5_UART4;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		/* USER CODE BEGIN UART4_MspInit 1 */

		/* USER CODE END UART4_MspInit 1 */
	}

}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart) {

	if (huart->Instance == UART4) {
		/* USER CODE BEGIN UART4_MspDeInit 0 */

		/* USER CODE END UART4_MspDeInit 0 */
		/* Peripheral clock disable */
		__UART4_CLK_DISABLE();

		/**UART4 GPIO Configuration
		 PC10     ------> UART4_TX
		 PC11     ------> UART4_RX
		 */
		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10 | GPIO_PIN_11);

	}
	/* USER CODE BEGIN UART4_MspDeInit 1 */

	/* USER CODE END UART4_MspDeInit 1 */

}

/**
 * @brief DAC MSP Initialization
 *        This function configures the hardware resources used in this example:
 *           - Peripheral's clock enable
 *           - Peripheral's GPIO Configuration
 * @param hdac: DAC handle pointer
 * @retval None
 */
void HAL_DAC_MspInit(DAC_HandleTypeDef *hdac) {
	GPIO_InitTypeDef GPIO_InitStruct;
	static DMA_HandleTypeDef hdma_dac1;

	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* Enable GPIO clock ****************************************/
	DACx_CHANNEL_GPIO_CLK_ENABLE()
	;
	/* DAC Periph clock enable */
	DACx_CLK_ENABLE()
	;
	/* DMA1 clock enable */
	DMAx_CLK_ENABLE()
	;
	/* SYSCFG clock enable for DMA remapping */
	__HAL_RCC_SYSCFG_CLK_ENABLE()
	;

	/*##-2- Configure peripheral GPIO ##########################################*/
	/* DAC Channel1 GPIO pin configuration */
	GPIO_InitStruct.Pin = DACx_CHANNEL_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(DACx_CHANNEL_GPIO_PORT, &GPIO_InitStruct);

	/*##-3- Configure the DMA ##########################################*/
	/* Set the parameters to be configured for DACx_DMA1_CHANNEL3 */
	hdma_dac1.Instance = DACx_DMA_INSTANCE;

	hdma_dac1.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_dac1.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_dac1.Init.MemInc = DMA_MINC_ENABLE;
	hdma_dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_dac1.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_dac1.Init.Mode = DMA_CIRCULAR;
	hdma_dac1.Init.Priority = DMA_PRIORITY_HIGH;

	HAL_DMA_Init(&hdma_dac1);

	/* Associate the initialized DMA handle to the the DAC handle */
	__HAL_LINKDMA(hdac, DMA_Handle1, hdma_dac1);

	/*##-4- Configure the NVIC for DMA #########################################*/
	/* Enable the DMA1_Channel3 IRQ Channel */
	HAL_NVIC_SetPriority(DACx_DMA_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(DACx_DMA_IRQn);

	/*##-5- Configure the SYSCFG for DMA  remapping#############################*/
	__HAL_DMA_REMAP_CHANNEL_ENABLE(HAL_REMAPDMA_TIM6_DAC1_CH1_DMA1_CH3);
}

/**
 * @brief  DeInitializes the DAC MSP.
 * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
 *         the configuration information for the specified DAC.
 * @retval None
 */
void HAL_DAC_MspDeInit(DAC_HandleTypeDef *hdac) {
	/*##-1- Reset peripherals ##################################################*/
	DACx_FORCE_RESET();
	DACx_RELEASE_RESET();

	/*##-2- Disable peripherals and GPIO Clocks ################################*/
	/* De-initialize the DAC Channel1 GPIO pin */
	HAL_GPIO_DeInit(DACx_CHANNEL_GPIO_PORT, DACx_CHANNEL_PIN);

	/*##-3- Disable the DMA Channel ############################################*/
	/* De-Initialize the DMA Channel associate to DAC_Channel1 */
	HAL_DMA_DeInit(hdac->DMA_Handle1);

	/*##-4- Disable the NVIC for DMA ###########################################*/
	HAL_NVIC_DisableIRQ(DACx_DMA_IRQn);
}

/**
 /* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
