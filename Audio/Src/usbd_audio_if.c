/**
 ******************************************************************************
  * @file    usbd_audio_if.c
  * @author  MCD Application Team
  * @brief   Файл интерфейса аудиоустройства USB.
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован компанией ST по лицензии Ultimate Liberty,
  * SLA0044, "License"; использовать этот файл можно только в соответствии с условиями
  * лицензии. Копию лицензии можно получить по адресу:
  *                             www.st.com/SLA0044
  *
 ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio.h"
#include "usb_audio.h"
#include "audio_sessions_usb.h"
#include "usbd_audio_if.h"

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int8_t  AUDIO_USB_Init(USBD_AUDIO_FunctionDescriptionfTypeDef* usb_audio_class_function, uint32_t private_data);
static int8_t  AUDIO_USB_DeInit(USBD_AUDIO_FunctionDescriptionfTypeDef* audio_function, uint32_t private_data);
static int8_t  AUDIO_USB_GetState(uint32_t private_data);
static int8_t  AUDIO_USB_GetConfigDesc (uint8_t ** pdata, uint16_t * psize, uint32_t private_data);

/* exported  variable ---------------------------------------------------------*/

/**
 * @brief Структура с указателями на функции обратного вызова для работы с аудиоинтерфейсом USB.
 */
 USBD_AUDIO_InterfaceCallbacksfTypeDef audio_class_interface =
 {
   .Init = AUDIO_USB_Init,
   .DeInit = AUDIO_USB_DeInit,
   .GetConfigDesc = AUDIO_USB_GetConfigDesc,
   .GetState = AUDIO_USB_GetState,
   .private_data = 0
 };

 /* exported  variables ---------------------------------------------------------*/
 /* список используемых сессий */

#if USE_USB_AUDIO_PLAYBACK
  /**
   * @brief Сессия воспроизведения аудио через USB.
   */
  AUDIO_USBSession_t USB_AudioPlabackSession;
#endif /* USE_USB_AUDIO_PLAYBACK */

 /* private  functions ---------------------------------------------------------*/

/**
  * @brief  AUDIO_USB_Init
  *         Инициализирует приложение потокового аудио по USB.
  *         Предоставляет информацию (номера конечных точек, поддерживаемые элементы управления и т.д.) 
  *         и функции обратного вызова классу аудиоустройства USB.
  * @param  usb_audio_class_function(OUT): описание и обратные вызовы для аудиофункции, 
  *         например, список конечных точек. Подробности см. в определении структуры.
  * @param  private_data: зарезервировано для будущего использования.
  * @retval Статус (0 — успех).
  */
static int8_t  AUDIO_USB_Init(USBD_AUDIO_FunctionDescriptionfTypeDef* usb_audio_class_function , uint32_t private_data)
{
  int interface_offset=0, total_control_count=0;
  uint8_t control_count = 0;

#if USE_USB_AUDIO_PLAYBACK
   /* Инициализация сессии воспроизведения через USB */
  AUDIO_PlaybackSessionInit(&usb_audio_class_function->as_interfaces[interface_offset], &(usb_audio_class_function->controls[interface_offset]), &control_count, (uint32_t) &USB_AudioPlabackSession);
  interface_offset++;
  total_control_count += control_count;
#endif /* USE_USB_AUDIO_PLAYBACK */
  usb_audio_class_function->as_interfaces_count = interface_offset;
  usb_audio_class_function->control_count = total_control_count;
  return 0;
}

/**
  * @brief  AUDIO_USB_DeInit
  *         Деинициализирует интерфейс.
  * @param  audio_function: описание аудиофункции.
  * @param  private_data: зарезервировано для будущего использования.
  * @retval Статус (0 — успех).
  */
static int8_t  AUDIO_USB_DeInit(USBD_AUDIO_FunctionDescriptionfTypeDef* audio_function, uint32_t private_data)
{
  return 0;
}

/**
  * @brief  AUDIO_USB_GetState
  *         Возвращает текущее состояние аудиоустройства USB.
  * @param  private_data: зарезервировано для будущего использования.
  * @retval Статус (0 — успех).
  */
static int8_t  AUDIO_USB_GetState(uint32_t private_data)
{
  return 0;
}

/**
  * @brief  AUDIO_USB_GetConfigDesc
  *         Возвращает дескриптор конфигурации устройства.
  * @param  pdata: указатель на буфер с дескриптором.
  * @param  psize: указатель на переменную, в которую будет записана длина дескриптора.
  * @param  private_data: зарезервировано для будущего использования.
  * @retval Статус (0 — успех).
  */
static int8_t  AUDIO_USB_GetConfigDesc (uint8_t ** pdata, uint16_t * psize, uint32_t private_data)
{
   *psize =  USB_AUDIO_GetConfigDescriptor(pdata);
    return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/