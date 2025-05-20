/**
  ******************************************************************************
  * @file    usb_audio_user_cfg.h
  * @author  MCD Application Team
  * @brief   Конфигурация приложения USB Audio
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Авторские права (c) 2019 STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован компанией ST под лицензией Ultimate Liberty license
  * SLA0044, "Лицензия"; Вы не можете использовать этот файл вне рамок соблюдения
  * условий Лицензии. Копию Лицензии можно получить по адресу:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#ifndef __USB_AUDIO_USER_CFG_H
#define __USB_AUDIO_USER_CFG_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include "usb_audio_constants.h"

/* Экспортируемые константы -----------------------------------------------*/
/* Настройки проекта */

#if USE_USB_AUDIO_PLAYBACK
/* Метод синхронизации */
#define USE_AUDIO_PLAYBACK_USB_FEEDBACK  1  // Включить обратную связь для синхронизации

/* Конфигурация каналов */
/* ВАЖНО: В текущей версии поддерживается только стерео (2 канала) */
#define USB_AUDIO_CONFIG_PLAY_CHANNEL_COUNT 0x02  // Стерео (2 канала)

/* Разрешение аудио */
/* Поддерживается 16/24 бита (другие разрешения требуют доработки) */
#define USB_AUDIO_CONFIG_PLAY_RES_BIT     24  // Разрешение: 24 бита на семпл
#define USB_AUDIO_CONFIG_PLAY_RES_BYTE    3   // Размер в байтах (24 бита = 3 байта)

/* Поддерживаемые частоты дискретизации (1 - включить, 0 - отключить) */
#define USB_AUDIO_CONFIG_PLAY_USE_FREQ_192_K   1  // 192 кГц
#define USB_AUDIO_CONFIG_PLAY_USE_FREQ_176_4_K 1  // 176.4 кГц
#define USB_AUDIO_CONFIG_PLAY_USE_FREQ_96_K    1  // 96 кГц
#define USB_AUDIO_CONFIG_PLAY_USE_FREQ_88_2_K  1  // 88.2 кГц
#define USB_AUDIO_CONFIG_PLAY_USE_FREQ_48_K    1  // 48 кГц
#define USB_AUDIO_CONFIG_PLAY_USE_FREQ_44_1_K  1  // 44.1 кГц
#define USB_AUDIO_CONFIG_PLAY_USE_FREQ_32_K    0  // 32 кГц
#define USB_AUDIO_CONFIG_PLAY_USE_FREQ_16_K    0  // 16 кГц
#define USB_AUDIO_CONFIG_PLAY_USE_FREQ_8_K     0  // 8 кГц

#define USE_AUDIO_TIMER_VOLUME_CTRL 0  // Управление громкостью через таймер (0 - отключено)
#endif /* USE_USB_AUDIO_PLAYBACK */

#ifdef __cplusplus
}
#endif

#endif /* __USB_AUDIO_USER_CFG_H */