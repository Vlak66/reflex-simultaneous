/********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2019
*
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/

#ifndef __SAI_H
#define __SAI_H

#include "board.h" // Конфигурация аппаратной платформы

// Приоритет прерывания SAI (0 - наивысший, 15 - наименьший)
#define SAI_IRQ_PRIORITY ((uint32_t)2)

/* Настройка тактирования */
void SAI_PLLSAIConfig(uint32_t AudioFrequency); // Конфигурация PLLSAI для заданной частоты
void SAI_ChangeFrequency(uint32_t AudioFrequency); // Изменение частоты на лету

/* Синхронизация */
void SAI_ExternalSyncInit(void); // Инициализация внешней синхронизации

/* Статус передачи */
uint16_t SAI_GetLastTransferSize(void); // Размер последней передачи через DMA
uint16_t SAI_GetRemainingTxSize(void); // Оставшиеся данные для передачи в DMA

/* DMA: Подготовка передачи */
void SAI_MasterDMAPrepareTx(uint16_t *Data, uint16_t Size); // Для мастера
void SAI_DMAPrepareTx(DMA_Stream_TypeDef *DMAStream, uint16_t *Data, uint16_t Size); // Универсальная

/* Управление интерфейсом */
void SAI_MasterEnable(void); // Включить мастер-режим
void SAI_Enable(SAI_Block_TypeDef *SAIBlock); // Универсальное включение
void SAI_MasterDisable(void); // Отключить мастер
void SAI_Disable(SAI_Block_TypeDef *SAIBlock); // Универсальное отключение

/* Управление звуком */
void SAI_MasterMute(uint8_t MuteFlag); // Заглушить мастер
void SAI_Mute(SAI_Block_TypeDef *SAIBlock, uint8_t MuteFlag); // Универсальное управление

/* Инициализация режимов */
void SAI_MasterInit_I2S(uint8_t AudioResolution, uint8_t BCLKMode); // I2S мастер
void SAI_Init_I2S(SAI_Block_TypeDef *SAIBlock, uint8_t AudioResolution, uint8_t BCLKMode); // Базовая настройка I2S

/* Синхронизация мастер/слейв */
void SAI_MasterSetSlaveSync(void); // Синхронизация мастера с внешним слейвом

/* DMA: Настройка */
void SAI_MasterDMAEnable(void); // Включить DMA для мастера
void SAI_DMAEnable(SAI_Block_TypeDef *SAIBlock); // Универсальное включение DMA
void SAI_MasterDMAInit(uint8_t AudioResolution); // Инициализация DMA мастера
void SAI_DMAInit(DMA_Stream_TypeDef *DMAStream, uint8_t AudioResolution); // Универсальная настройка DMA
void SAI_MasterDMAChangeDataSize(uint8_t Resolution); // Изменить разрешение данных в DMA
void SAI_DMAChangeDataSize(DMA_Stream_TypeDef *DMAStream, uint8_t Resolution); // Универсальное изменение

/* Настройка GPIO */
void SAI_MasterGPIOInit(void); // Настройка выводов для мастера

#endif // __SAI_H