/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2019, 2020
*
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/



#ifndef __BOARD_H
#define __BOARD_H


#include "stm32f446xx.h"

//USB
#define   USB_FS_GPIO                      GPIOA
#define   USB_FS_DM_PIN                    11
#define   USB_FS_DP_PIN                    12
#define   USB_FS_PINS_AF                   10
//SAI1 - master (порт 1)
#define   SAI_MASTER                       SAI1_Block_B
#define   SAI_MASTER_MCLK_GPIO             GPIOC
#define   SAI_MASTER_MCK_PIN               0
#define   SAI_MASTER_SCK_FS_GPIO           GPIOB
#define   SAI_MASTER_SCK_PIN               12
#define   SAI_MASTER_FS_PIN                9
#define   SAI_MASTER_SD_GPIO               GPIOA
#define   SAI_MASTER_SD_PIN                9
#define   SAI_MASTER_MCLK_SCK_SD_FS_AF     6
#define   SAI_MASTER_DMA_STREAM            DMA2_Stream5
#define   SAI_MASTER_DMA_CHANNEL           0
#define   SAI_MASTER_DMA_IRQ               DMA2_Stream5_IRQn


// конфигурационные выводы
#define   CONFIG_GPIO                      GPIOC
#define   CONFIG_1_PIN                     3  // Обозначение на плате 6
#define   CONFIG_2_PIN                     2  // Обозначение на плате 7
#define   CONFIG_3_PIN                     1  // Обозначение на плате 8
#define   CONFIG_4_PIN                     15 // Обозначение на плате 9
#define   CONFIG_5_PIN                     14 // Обозначение на плате 10
#define   CONFIG_6_PIN                     13 // Обозначение на плате 11

// выводы отображающие частоту аудио
#define   EXT_SYNC_SELECT_GPIO             GPIOB
#define   EXT_SYNC_SELECT_1_PIN            0 // Обозначение на плате 26
#define   EXT_SYNC_SELECT_2_PIN            1 // Обозначение на плате 25
#define   EXT_SYNC_SELECT_3_GPIO           GPIOC
#define   EXT_SYNC_SELECT_3_PIN            5 // Обозначение на плате 27
#define   EXT_SYNC_SELECT_MASK             ((1 << EXT_SYNC_SELECT_1_PIN) | (1 << EXT_SYNC_SELECT_2_PIN))
#define   EXT_SYNC_SELECT_3_MASK           (1 << EXT_SYNC_SELECT_3_PIN)
#define   EXT_SYNC_SELECT_2_MASK           (1 << EXT_SYNC_SELECT_2_PIN)
#define   EXT_SYNC_SELECT_1_MASK           (1 << EXT_SYNC_SELECT_1_PIN)

// порт для внешней синхронизации
#define   EXT_SYNC_GPIO                    GPIOC
#define   EXT_SYNC_PIN                     9 // Обозначение на плате EXT CLK
#define   EXT_SYNC_AF                      5

#define   OUT_CLK_CONFIG_1_GPIO            GPIOC
#define   OUT_CLK_CONFIG_1_PIN             4  // Обозначение на плате 36
#define   OUT_CLK_CONFIG_2_GPIO            GPIOA
#define   OUT_CLK_CONFIG_2_PIN             15 // Обозначение на плате 35

#define   POWER_DISABLE_GPIO               GPIOC
#define   POWER_DISABLE_PIN                12 // Обозначение на плате 14

#define   SYNC_MODE_GPIO                   GPIOC
#define   SYNC_MODE_PIN_1                  CONFIG_5_PIN
#define   SYNC_MODE_PIN_2                  CONFIG_6_PIN

#define   BCLK_Fs_RATIO_GPIO               GPIOA
#define   BCLK_Fs_RATIO_PIN                8 // Обозначение на плате 13

#define   OUT_CLK_CONFIG_1                 ((1 << OUT_CLK_CONFIG_1_PIN) | (1 << OUT_CLK_CONFIG_2_PIN))
#define   OUT_CLK_CONFIG_2                 (1 << OUT_CLK_CONFIG_2_PIN)
#define   OUT_CLK_CONFIG_3                 (1 << OUT_CLK_CONFIG_1_PIN)
#define   OUT_CLK_CONFIG_4                 0

#define   SYNC_MODE_1_MASK                 (1 << 1)
#define   SYNC_MODE_2_MASK                 (1 << 0)
#define   SYNC_MODE_MASK                   (SYNC_MODE_1_MASK | SYNC_MODE_2_MASK)

// UPSAMPLING
#define UPSAMPLING_ENABLE_PIN              CONFIG_2_PIN    // Pin to enable/disable upsampling (LOW for enabled)
#define UPSAMPLING_ALGORITHM_SELECT_PIN    CONFIG_3_PIN    // Pin to select upsampling algorithm (LOW for Algo0, HIGH for Algo1)

enum BCLK_FsRatioModes
{
  BCLK_Fs_RES_DEPENDENT = 0,
  BCLK_Fs_FIXED,
};



void USB_I2S_Init(void);


#endif //__BOARD_H


