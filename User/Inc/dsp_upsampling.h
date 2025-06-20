/*
********************************************************************************
* COPYRIGHT(c) HAND-AUDIO 2025
*
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/

#ifndef __DSP_UPSAMPLING_H
#define __DSP_UPSAMPLING_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h> // Для int16_t, uint32_t и других стандартных типов

/* Exported types ------------------------------------------------------------*/
/**
  * @brief Перечисление коэффициентов апсемплинга
  */
typedef enum
{
  UP_FACTOR_X1 = 1,  /*!< Коэффициент апсемплинга x1 (без изменений) */
  UP_FACTOR_X2 = 2,  /*!< Коэффициент апсемплинга x2 */
  UP_FACTOR_X4 = 4   /*!< Коэффициент апсемплинга x4 */
} UpFactor_t;


/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Инициализирует модуль апсемплинга DSP.
  * @param  factor: Коэффициент апсемплинга (UP_FACTOR_X1, UP_FACTOR_X2, UP_FACTOR_X4).
  * @param  algo_select: Выбор алгоритма апсемплинга (0 для Algo0, 1 для Algo1).
  * @retval None
  */
void DSP_UpsampleInit(UpFactor_t factor, uint8_t algo_select);
/**
  * @brief  Выполняет апсемплинг блока аудиоданных.
  *         Примечание: размер выходного буфера должен быть достаточно большим
  *         для размещения inLen * factor сэмплов (стерео сэмплов = 2 int16_t).
  * @param  in_buffer: Указатель на входной буфер 16-битных стерео сэмплов (Q15).
  * @param  in_len_samples: Количество стерео сэмплов во входном буфере.
  * @param  out_buffer: Указатель на выходной буфер для 16-битных стерео сэмплов после апсемплинга (Q15).
  * @retval None
  */
void DSP_UpsampleBlock(const int16_t *in_buffer, uint32_t in_len_samples,
                       int16_t *out_buffer);

#ifdef __cplusplus
}
#endif

#endif /* __DSP_UPSAMPLING_H */
