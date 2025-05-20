/**
 ******************************************************************************
  * @file    audio_sessions_usb.h
  * @author  MCD Application Team
  * @brief   Заголовочный файл для audio_session_usb.c
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019  STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован компанией ST по лицензии Ultimate Liberty
  * SLA0044, "License"; Вы не можете использовать этот файл, кроме как в соответствии с
  * Лицензией. Вы можете получить копию Лицензии по адресу:
  *                             www.st.com/SLA0044
  *
 ******************************************************************************
  */

/* Защита от повторного включения -------------------------------------*/
#ifndef __AUDIO_SESSIONS_USB_H
#define __AUDIO_SESSIONS_USB_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_node.h"
#include "audio_usb_nodes.h"

/* Экспортированные типы ------------------------------------------------------------*/


/**
  * @brief Структура USB-сессии: может использоваться как сессия воспроизведения или записи
  */
typedef struct    AUDIO_USB_StreamingSession
{
  AUDIO_Session_t session; /* Основная структура сессии */
  int8_t               (*SessionDeInit)       (uint32_t /*дескриптор сессии*/);
  uint8_t              interface_num; /* Номер USB-интерфейса */
  uint8_t              alternate; /* Текущее альтернативное состояние интерфейса */
  AUDIO_CircularBuffer_t  buffer; /* Кольцевой буфер для аудио */
}
AUDIO_USBSession_t;


#if USE_USB_AUDIO_PLAYBACK
 /**
   * @brief Инициализация сессии воспроизведения аудио через USB
   */
 int8_t  AUDIO_PlaybackSessionInit(USBD_AUDIO_AS_InterfaceTypeDef* as_desc,
                                    USBD_AUDIO_ControlTypeDef* controls_desc,
                                    uint8_t* control_count, uint32_t session_handle);
#endif /* USE_USB_AUDIO_PLAYBACK*/


#ifdef __cplusplus
}
#endif
#endif  /* __AUDIO_SESSIONS_USB_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/