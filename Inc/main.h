/**
 ******************************************************************************
 * @file    FatFs/FatFs_uSD/Inc/main.h
 * @author  MCD Application Team
 * @version V1.4.0
 * @date    13-November-2015
 * @brief   Header for main.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"

/* EVAL includes component */
#include "stm32303e_eval.h"

/* FatFs includes component */
#include "ff_gen_drv.h"
#include "sd_diskio.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor ADCx instance used and associated
 resources */

/* User can use this section to tailor DACx instance used and associated
 resources */

/* ## Definition of ADC related resources ################################### */
/* Definition of ADCx clock resources */
#define ADCx                            ADC1
#define ADCx_CLK_ENABLE()               __HAL_RCC_ADC12_CLK_ENABLE()

#define ADCx_FORCE_RESET()              __HAL_RCC_ADC12_FORCE_RESET()
#define ADCx_RELEASE_RESET()            __HAL_RCC_ADC12_RELEASE_RESET()

/* Definition of ADCx channels */
#define ADCx_CHANNELa                   ADC_CHANNEL_8

/* Definition of ADCx channels pins */
#define ADCx_CHANNELa_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()
#define ADCx_CHANNELa_GPIO_PORT         GPIOC
#define ADCx_CHANNELa_PIN               GPIO_PIN_2

/* Definition of ADCx DMA resources */
#define ADCx_DMA_CLK_ENABLE()            __HAL_RCC_DMA1_CLK_ENABLE()
#define ADCx_DMA_STREAM                  DMA1_Channel1

#define ADCx_DMA_IRQn                    DMA1_Channel1_IRQn
#define ADCx_DMA_IRQHandler              DMA1_Channel1_IRQHandler

/* Definition of ADCx NVIC resources */
#define ADCx_IRQn                        ADC1_2_IRQn
#define ADCx_IRQHandler                  ADC1_2_IRQHandler

/* Definition of ADCy clock resources */
#define ADCy                             ADC2
#define ADCy_CLK_ENABLE()                __HAL_RCC_ADC12_CLK_ENABLE()
#define ADCy_CHANNEL_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOC_CLK_ENABLE()

#define ADCy_FORCE_RESET()               __HAL_RCC_ADC12_FORCE_RESET()
#define ADCy_RELEASE_RESET()             __HAL_RCC_ADC12_RELEASE_RESET()

/* Definition for DACx clock resources */
#define DACx                            DAC1
#define DACx_CHANNEL_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()
#define DMAx_CLK_ENABLE()               __HAL_RCC_DMA1_CLK_ENABLE()

#define DACx_CLK_ENABLE()               __HAL_RCC_DAC1_CLK_ENABLE()
#define DACx_FORCE_RESET()              __HAL_RCC_DAC1_FORCE_RESET()
#define DACx_RELEASE_RESET()            __HAL_RCC_DAC1_RELEASE_RESET()

/* Definition for DACx Channel Pin */
#define DACx_CHANNEL_PIN                GPIO_PIN_4
#define DACx_CHANNEL_GPIO_PORT          GPIOA

/* Definition for DACx's Channel */
#define DACx_CHANNEL                    DAC_CHANNEL_1

/* Definition for DACx's DMA1_CHANNEL3 */
#define DACx_DMA_INSTANCE               DMA1_Channel3

/* Definition for DACx's NVIC */
#define DACx_DMA_IRQn                   DMA1_Channel3_IRQn
#define DACx_DMA_IRQHandler             DMA1_Channel3_IRQHandler

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
