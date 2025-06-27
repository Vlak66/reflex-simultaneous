/**
 ******************************************************************************
  * @file    usbd_audio.c
  * @author  MCD Application Team
  * @brief   Этот файл содержит основные функции для работы с аудио-классом.
  *
  * @verbatim
  *
  *          ===================================================================
  *                                Описание класса AUDIO
  *          ===================================================================
  *           Данный драйвер реализует Audio Class 1.0 в соответствии со спецификацией
  *           "USB Device Class Definition for Audio Devices V1.0 от 18 марта 1998".
  *           Это новая реализация USB-класса аудио, поддерживающая дополнительные функции.
  *           Драйвер поддерживает следующие аспекты спецификации:
  *             - Управление стандартным AC (Audio Control) интерфейсным дескриптором
  *             - 2 потоковых аудиоинтерфейса (с одним каналом, PCM, стереорежим)
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован ST по лицензии Ultimate Liberty,
  * SLA0044, "Лицензия"; вы не можете использовать этот файл, кроме как в соответствии
  * с Лицензией. Копию Лицензии можно получить по адресу:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio.h"
#include "usbd_ctlreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_AUDIO
  * @brief Модуль ядра USB-устройства для работы с аудио
  * @{
  */

/** @defgroup USBD_AUDIO_Private_TypesDefinitions
  * @{
  */
typedef enum
{
  USBD_AUDIO_DATA_EP,         /**< Конечная точка данных */
  USBD_AUDIO_FEEDBACK_EP,     /**< Конечная точка обратной связи */
  USBD_AUDIO_INTERRUPT_EP     /**< Прерывательная конечная точка */
}USBD_AUDIO_EpUsageTypeDef;

/* Структура описания конечной точки: описание и состояние */
typedef struct
{
  union
  {
    USBD_AUDIO_EP_DataTypeDef* data_ep;
#if USBD_SUPPORT_AUDIO_OUT_FEEDBACK
    USBD_AUDIO_EP_SynchTypeDef* sync_ep;
#endif /* USBD_SUPPORT_AUDIO_OUT_FEEDBACK */
  }ep_description;
  USBD_AUDIO_EpUsageTypeDef ep_type;      /**< Тип конечной точки */
  uint8_t open;                           /**< Флаг открытия: 0 — закрыто, 1 — открыто */
  uint16_t max_packet_length;             /**< Максимальная длина пакета */
  uint16_t tx_rx_soffn;                   /**< Номер SOF для передачи/приёма */
}USBD_AUDIO_EPTypeDef;

/* Структура данных аудио-класса */
typedef struct
{
  USBD_AUDIO_FunctionDescriptionfTypeDef aud_function; /**< Описание функции аудио */
  USBD_AUDIO_EPTypeDef ep_in[USBD_AUDIO_MAX_IN_EP];   /**< Список IN-конечных точек */
  USBD_AUDIO_EPTypeDef ep_out[USBD_AUDIO_MAX_OUT_EP]; /**< Список OUT-конечных точек */

  /* Структура для обработки управляющих запросов */
  struct
  {
    union
    {
      USBD_AUDIO_ControlTypeDef *controller; /**< Указатель на соответствующий контроллер */
#if USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES
      USBD_AUDIO_EP_DataTypeDef* data_ep;    /**< Связанная конечная точка данных */
#endif /* USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES */
    } entity;
    uint8_t request_target;                 /**< Цель запроса */
    uint8_t  data[USB_MAX_EP0_SIZE];        /**< Буфер для получения значения запроса или отправки ответа */
    uint32_t len;                            /**< Используемая длина буфера данных */
    uint16_t wValue;                         /**< Поле wValue из запроса, специфичное для каждого управления */
    uint8_t  req;                            /**< Тип запроса, специфичный для каждой единицы */
  }last_control;
}USBD_AUDIO_HandleTypeDef;

/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_Defines
  * @{
  */
#define AUDIO_UNIT_CONTROL_REQUEST 0x01       /**< Запрос к управляющей единице аудио */
#define AUDIO_EP_REQUEST 0x02               /**< Запрос к конечной точке аудио */
#if USBD_SUPPORT_AUDIO_OUT_FEEDBACK
#define USBD_AUDIO_SOF_COUNT_FEEDBACK_BITS 7
#define USBD_AUDIO_SOF_COUNT_FEEDBACK (1 << USBD_AUDIO_SOF_COUNT_FEEDBACK_BITS)
#endif /*USBD_SUPPORT_AUDIO_OUT_FEEDBACK */
/**
  * @}
  */

/** @defgroup USBD_AUDIO_Private_Macros
  * @{
  */


/**
  * @}
  */



/** @defgroup USBD_AUDIO_Private_FunctionPrototypes
  * @{
  */

uint8_t  USBD_AUDIO_Init (USBD_HandleTypeDef *pdev,
                               uint8_t cfgidx);

uint8_t  USBD_AUDIO_DeInit (USBD_HandleTypeDef *pdev,
                                 uint8_t cfgidx);


uint8_t  USBD_AUDIO_Setup (USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req);

uint8_t  *USBD_AUDIO_GetCfgDesc (uint16_t *length);

uint8_t  *USBD_AUDIO_GetDeviceQualifierDesc (uint16_t *length);

uint8_t  USBD_AUDIO_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);

uint8_t  USBD_AUDIO_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);

uint8_t  USBD_AUDIO_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_AUDIO_EP0_TxReady (USBD_HandleTypeDef *pdev);

uint8_t  USBD_AUDIO_SOF (USBD_HandleTypeDef *pdev);

uint8_t  USBD_AUDIO_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_AUDIO_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t AUDIO_REQ(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);



#if USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES
static uint8_t AUDIO_EP_REQ(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
#endif /* USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES*/
#if USBD_SUPPORT_AUDIO_OUT_FEEDBACK
static  unsigned get_usb_full_speed_rate(unsigned int rate, unsigned char * buf);
#endif /*USBD_SUPPORT_AUDIO_OUT_FEEDBACK */

static uint8_t  USBD_AUDIO_SetInterfaceAlternate(USBD_HandleTypeDef *pdev,uint8_t as_interface_num,uint8_t new_alt);

/**
  * @}
  */

/** @defgroup USBD_AUDIO_Private_Variables
  * @{
  */

USBD_ClassTypeDef  USBD_AUDIO =
{
  USBD_AUDIO_Init,
  USBD_AUDIO_DeInit,
  USBD_AUDIO_Setup,
  USBD_AUDIO_EP0_TxReady,
  USBD_AUDIO_EP0_RxReady,
  USBD_AUDIO_DataIn,
  USBD_AUDIO_DataOut,
  USBD_AUDIO_SOF,
  USBD_AUDIO_IsoINIncomplete,
  USBD_AUDIO_IsoOutIncomplete,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetDeviceQualifierDesc,
};
/* Стандартный дескриптор устройства USB */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END=
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

static uint8_t *USBD_AUDIO_CfgDesc=0;
static uint16_t USBD_AUDIO_CfgDescSize=0;
/**
  * @}
  */

/** @defgroup USBD_AUDIO_Private_Functions
  * @{
  */

/**
  * @brief  USBD_AUDIO_Init
  *         Инициализирует интерфейс AUDIO
  * @param  pdev: указатель на экземпляр устройства
  * @param  cfgidx: индекс конфигурации, не используется
  * @retval статус операции
  */
uint8_t  USBD_AUDIO_Init (USBD_HandleTypeDef *pdev,
                               uint8_t cfgidx)
{
  /* Выделение памяти под структуру аудио */
  USBD_AUDIO_HandleTypeDef   *haudio;
  USBD_AUDIO_InterfaceCallbacksfTypeDef * aud_if_cbks;

  haudio = USBD_malloc(sizeof (USBD_AUDIO_HandleTypeDef));
  if(haudio == NULL)
  {
    return USBD_FAIL;
  }
  else
  {
    memset(haudio, 0, sizeof(USBD_AUDIO_HandleTypeDef));
    aud_if_cbks = (USBD_AUDIO_InterfaceCallbacksfTypeDef *)pdev->pUserData;
    /* Инициализация аппаратного уровня вывода звука */
    if (aud_if_cbks->Init(&haudio->aud_function,aud_if_cbks->private_data)!= USBD_OK)
    {
      USBD_free(pdev->pClassData);
      pdev->pClassData = 0;
      return USBD_FAIL;
    }
  }
  pdev->pClassData = haudio;
  return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_DeInit
  *         Деинициализирует уровень AUDIO
  * @param  pdev: указатель на экземпляр устройства
  * @param  cfgidx: индекс конфигурации, не используется
  * @retval статус операции
  */
uint8_t  USBD_AUDIO_DeInit (USBD_HandleTypeDef *pdev,
                                 uint8_t cfgidx)
{
    USBD_AUDIO_HandleTypeDef   *haudio;

    haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;

    /* Закрываем все открытые конечные точки */
    for(int i=1;i < USBD_AUDIO_MAX_IN_EP; i++)
    {
      if(haudio->ep_in[i].open)
      {
        USBD_LL_CloseEP(pdev, i|0x80);
        haudio->ep_in[i].open = 0;
      }
    }
    for(int i=1;i < USBD_AUDIO_MAX_OUT_EP; i++)
    {
      if(haudio->ep_out[i].open)
      {
        USBD_LL_CloseEP(pdev, i);
        haudio->ep_out[i].open = 0;
      }
    }

  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_SetInterfaceAlternate
  *         Устанавливает альтернативный интерфейс потокового аудиоинтерфейса
  * @param  pdev: экземпляр устройства
  * @param  as_interface_num: номер аудиопотокового интерфейса
  * @param  new_alt: новое значение альтернативного интерфейса
  * @retval статус операции
  */
 static uint8_t  USBD_AUDIO_SetInterfaceAlternate(USBD_HandleTypeDef *pdev,uint8_t as_interface_num,uint8_t new_alt)
 {
   USBD_AUDIO_HandleTypeDef   *haudio;
   USBD_AUDIO_AS_InterfaceTypeDef* pas_interface;
   USBD_AUDIO_EPTypeDef * ep;
 
   haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
   pas_interface = &haudio->aud_function.as_interfaces[as_interface_num];
   ep = (pas_interface->data_ep.ep_num&0x80)?&haudio->ep_in[pas_interface->data_ep.ep_num&0x0F]:
                                             &haudio->ep_out[pas_interface->data_ep.ep_num];
 
 
   /* Закрываем старый альтернативный интерфейс */
   if(new_alt == 0)
   {
     /* Закрываем все открытые конечные точки */
     if (pas_interface->alternate != 0)
     {
         /* @TODO : Закрытие связанных конечных точек */
       if(ep->open)
       {
         USBD_LL_CloseEP(pdev, ep->ep_description.data_ep->ep_num);
         ep->open=0;
       }
 #if USBD_SUPPORT_AUDIO_OUT_FEEDBACK
       if(pas_interface->synch_enabled)
       {
         /* Закрытие синхронизационной конечной точки */
           ep=&haudio->ep_in[pas_interface->synch_ep.ep_num&0x0F];
           if(ep->open)
           {
             USBD_LL_CloseEP(pdev, ep->ep_description.sync_ep->ep_num);
             ep->open = 0;
           }
       }
 #endif /*USBD_SUPPORT_AUDIO_OUT_FEEDBACK */
     }
     pas_interface->SetAS_Alternate(new_alt,pas_interface->private_data);
     pas_interface->alternate=0;
   }
   /* Инициализируем новый альтернативный интерфейс */
   else
   {
     /* Подготавливаем конечную точку */
     ep->ep_description.data_ep = &pas_interface->data_ep;
 
     /* Открываем конечную точку данных */
     pas_interface->SetAS_Alternate(new_alt, pas_interface->private_data);
     pas_interface->alternate = new_alt;
     ep->max_packet_length = ep->ep_description.data_ep->GetMaxPacketLength(ep->ep_description.data_ep->private_data);
 
     /* Открытие изохронной конечной точки */
     USBD_LL_OpenEP(pdev,
                  ep->ep_description.data_ep->ep_num,
                  USBD_EP_TYPE_ISOC,
                  ep->max_packet_length);
      ep->open = 1;
 
      /* Получаем буфер для работы USB */
     ep->ep_description.data_ep->buf = ep->ep_description.data_ep->GetBuffer(ep->ep_description.data_ep->private_data,
                                                                            &ep->ep_description.data_ep->length);
 
     if(ep->ep_description.data_ep->ep_num & 0x80)  /* IN EP - передача от устройства */
     {
       USBD_LL_FlushEP(pdev, ep->ep_description.data_ep->ep_num);
       ep->tx_rx_soffn = USB_SOF_NUMBER();
       USBD_LL_Transmit(pdev,
                         ep->ep_description.data_ep->ep_num,
                         ep->ep_description.data_ep->buf,
                         ep->ep_description.data_ep->length);
     }
     else /* OUT EP - приём на устройство */
     {
 #if USBD_SUPPORT_AUDIO_OUT_FEEDBACK
         uint32_t rate;
 #endif /*USBD_SUPPORT_AUDIO_OUT_FEEDBACK */
 
         /* Подготовка OUT-конечной точки к приёму первого пакета */
         USBD_LL_PrepareReceive(pdev,
                                ep->ep_description.data_ep->ep_num,
                                ep->ep_description.data_ep->buf,
                                ep->max_packet_length);
 
 #if USBD_SUPPORT_AUDIO_OUT_FEEDBACK
         if(pas_interface->synch_enabled)
         {
            USBD_AUDIO_EP_SynchTypeDef* sync_ep; /* Описание синхронизационной конечной точки */
            ep = &haudio->ep_in[pas_interface->synch_ep.ep_num & 0x0F];
            sync_ep = &pas_interface->synch_ep;
            ep->ep_description.sync_ep = sync_ep;
            ep->max_packet_length = AUDIO_FEEDBACK_EP_PACKET_SIZE;
            ep->ep_type = USBD_AUDIO_FEEDBACK_EP;
 
            /* Открытие синхронизационной конечной точки */
            USBD_LL_OpenEP(pdev, sync_ep->ep_num, USBD_EP_TYPE_ISOC, ep->max_packet_length);
            ep->open = 1;
 
            rate = sync_ep->GetFeedback(sync_ep->private_data);
            get_usb_full_speed_rate(rate, sync_ep->feedback_data);
            ep->tx_rx_soffn = USB_SOF_NUMBER();
            USBD_LL_Transmit(pdev, sync_ep->ep_num,
                              sync_ep->feedback_data, ep->max_packet_length);
         }
 #endif /*USBD_SUPPORT_AUDIO_OUT_FEEDBACK */
     }
   }
 
   return USBD_OK;
 }
 
 #if USBD_SUPPORT_AUDIO_OUT_FEEDBACK
 /**
   * @brief   get_usb_full_speed_rate
   *         Вычисляет значение обратной связи на основе заданной частоты
   * @param  rate: входная частота
   * @param  buf: буфер для хранения значения обратной связи
   * @retval всегда 0 — успешное завершение
   */
 static unsigned get_usb_full_speed_rate(unsigned int rate, unsigned char * buf)
 {
     /* Преобразование частоты в формат, используемый USB для обратной связи */
     uint32_t freq = ((rate << 13) + 62) / 125;
     buf[0] = freq >> 2;
     buf[1] = freq >> 10;
     buf[2] = freq >> 18;
 
     return 0;
 }
 #endif /*USBD_SUPPORT_AUDIO_OUT_FEEDBACK */
 
 
 /**
   * @brief  USBD_AUDIO_Setup
   *         Обрабатывает специфичные для класса AUDIO запросы
   * @param  pdev: указатель на экземпляр устройства
   * @param  req: указатель на структуру запроса
   * @retval статус выполнения запроса
   */
 uint8_t  USBD_AUDIO_Setup (USBD_HandleTypeDef *pdev,
                                 USBD_SetupReqTypedef *req)
 {
   USBD_AUDIO_HandleTypeDef   *haudio;
   uint16_t len;
   uint8_t *pbuf;
   uint8_t ret = USBD_OK;
 
   haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
 
   switch (req->bmRequest & USB_REQ_TYPE_MASK)
   {
   case USB_REQ_TYPE_CLASS :
     if((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_INTERFACE)
     {
       switch (req->bRequest)
       {
       case USBD_AUDIO_REQ_GET_CUR:
       case USBD_AUDIO_REQ_GET_MIN:
       case USBD_AUDIO_REQ_GET_MAX:
       case USBD_AUDIO_REQ_GET_RES:
       case USBD_AUDIO_REQ_SET_CUR:
            AUDIO_REQ(pdev, req); // Обработка стандартных управляющих запросов
         break;
 
       default:
         USBD_CtlError (pdev, req); // Неизвестный запрос
         ret = USBD_FAIL;
       }
     }
     else
 #if USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES
     {
       switch (req->bRequest)
       {
       case USBD_AUDIO_REQ_SET_CUR:
            AUDIO_EP_REQ(pdev, req); // Запросы, связанные с установкой параметров конечной точки
         break;
 
       default:
         USBD_CtlError (pdev, req); // Неизвестный запрос
         ret = USBD_FAIL;
       }
     }
 #else /* USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES*/
     {
      USBD_CtlError (pdev, req); // Поддержка множественных частот отключена
         ret = USBD_FAIL;
     }
 #endif /*USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES*/
     break;
 
   case USB_REQ_TYPE_STANDARD:
     switch (req->bRequest)
     {
     case USB_REQ_GET_DESCRIPTOR:
       if( (req->wValue >> 8) == USBD_AUDIO_DESC_TYPE_CS_DEVICE)
       {
         pbuf = USBD_AUDIO_CfgDesc + 18;
         len = MIN(USBD_AUDIO_DESC_SIZ , req->wLength);
 
         USBD_CtlSendData (pdev, pbuf, len); // Отправка дескриптора
       }
       break;
 
     case USB_REQ_GET_INTERFACE :
       {
         for(int i=0;i<haudio->aud_function.as_interfaces_count;i++)
         {
             if((uint8_t)(req->wIndex)==haudio->aud_function.as_interfaces[i].interface_num)
             {
               USBD_CtlSendData (pdev,
                         (uint8_t *)&(haudio->aud_function.as_interfaces[i].alternate),
                         1);
               return USBD_OK;
             }
         }
         USBD_CtlError (pdev, req);
         ret = USBD_FAIL;
       }
       break;
 
     case USB_REQ_SET_INTERFACE :
       {
         for(int i=0;i<haudio->aud_function.as_interfaces_count;i++)
         {
             if((uint8_t)(req->wIndex)==haudio->aud_function.as_interfaces[i].interface_num)
             {
               if((uint8_t)(req->wValue)==haudio->aud_function.as_interfaces[i].alternate)
               {
                 /* Интерфейс уже установлен в нужный альтернативный режим */
                 return USBD_OK;
               }
               else
               {
                 /* Альтернативный режим изменяется */
                 return USBD_AUDIO_SetInterfaceAlternate(pdev,i,(uint8_t)(req->wValue));
               }
             }
         }
 
         if(((uint8_t)(req->wIndex) ==0)&&((uint8_t)(req->wValue))==0)
         {
           /* Контрольный интерфейс аудио, принимается только альтернатива 0 */
                 return USBD_OK;
         }
           /* Неизвестный интерфейс — ошибка */
           USBD_CtlError (pdev, req);
           ret = USBD_FAIL;
       }
       break;
 
     default:
       USBD_CtlError (pdev, req); // Неизвестный запрос
       ret = USBD_FAIL;
     }
   }
   return ret;
 }
/**
  * @brief  USBD_AUDIO_GetCfgDesc
  *         Возвращает указатель на дескриптор конфигурации
  * @param  length: указатель, в который будет записана длина данных
  * @retval Указатель на буфер с дескриптором
  */
 uint8_t  *USBD_AUDIO_GetCfgDesc (uint16_t *length)
 {
   *length = USBD_AUDIO_CfgDescSize;
   return USBD_AUDIO_CfgDesc;
 }
 
 /**
   * @brief  USBD_AUDIO_DataIn
   *         Обрабатывает этап передачи данных IN
   * @param  pdev: экземпляр устройства
   * @param  epnum: номер конечной точки
   * @retval статус операции
   */
 uint8_t  USBD_AUDIO_DataIn (USBD_HandleTypeDef *pdev,
                               uint8_t epnum)
 {
   USBD_AUDIO_EPTypeDef * ep;
 
   // Получаем ссылку на IN-конечную точку
   ep = &((USBD_AUDIO_HandleTypeDef*) pdev->pClassData)->ep_in[epnum & 0x7F];
 
   if(ep->open)
   {
 #if USBD_SUPPORT_AUDIO_OUT_FEEDBACK
     if(ep->ep_type == USBD_AUDIO_DATA_EP)
     {
 #endif /*USBD_SUPPORT_AUDIO_OUT_FEEDBACK */
 
       // Получаем новый буфер и его длину
       ep->ep_description.data_ep->buf = ep->ep_description.data_ep->GetBuffer(
           ep->ep_description.data_ep->private_data,
           &ep->ep_description.data_ep->length);
 
       // Запоминаем номер SOF для синхронизации
       ep->tx_rx_soffn = USB_SOF_NUMBER();
 
       // Передаём данные через конечную точку
       USBD_LL_Transmit(pdev,
                        epnum | 0x80,
                        ep->ep_description.data_ep->buf,
                        ep->ep_description.data_ep->length);
 
 #if USBD_SUPPORT_AUDIO_OUT_FEEDBACK
     }
     else if(ep->ep_type == USBD_AUDIO_FEEDBACK_EP)
     {
       // Конечная точка обратной связи — обновляем частоту
       uint32_t rate;
       USBD_AUDIO_EP_SynchTypeDef* sync_ep = ep->ep_description.sync_ep;
 
       rate = sync_ep->GetFeedback(sync_ep->private_data);
       get_usb_full_speed_rate(rate, sync_ep->feedback_data);
 
       ep->tx_rx_soffn = USB_SOF_NUMBER();
       USBD_LL_Transmit(pdev,
                        epnum | 0x80,
                        sync_ep->feedback_data,
                        AUDIO_FEEDBACK_EP_PACKET_SIZE);
     }
 #endif /*USBD_SUPPORT_AUDIO_OUT_FEEDBACK */
 
   }
   else
   {
     // Ошибка: попытка работы с закрытой конечной точкой
     USBD_error_handler();
   }
 
   return USBD_OK;
 }
 
 /**
   * @brief  USBD_AUDIO_EP0_RxReady
   *         Обрабатывает событие завершения приёма на EP0
   * @param  pdev: экземпляр устройства
   * @retval статус операции
   */
 uint8_t  USBD_AUDIO_EP0_RxReady (USBD_HandleTypeDef *pdev)
 {
   USBD_AUDIO_HandleTypeDef   *haudio;
   uint16_t *tmpdata;
 
   haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
 
   if(haudio->last_control.req == 0x00)
   {
     /* TODO: Обработать эту ошибку */
     return USBD_OK;
   }
 
   if(haudio->last_control.request_target == AUDIO_UNIT_CONTROL_REQUEST)
   {
     USBD_AUDIO_ControlTypeDef *ctl;
     ctl = haudio->last_control.entity.controller;
 
     switch(ctl->type)
     {
       case USBD_AUDIO_CS_AC_SUBTYPE_FEATURE_UNIT:
       {
         uint16_t selector = HIBYTE(haudio->last_control.wValue);
         USBD_AUDIO_FeatureControlCallbacksTypeDef* feature_control = ctl->Callbacks.feature_control;
 
         switch(selector)
         {
           case USBD_AUDIO_CONTROL_FEATURE_UNIT_MUTE:
             {
               /* TODO: Обработать случай нескольких каналов и проверить тип запроса */
               if(feature_control->SetMute)
               {
                 feature_control->SetMute(LOBYTE(haudio->last_control.wValue),
                                          haudio->last_control.data[0],
                                          ctl->private_data);
               }
               break;
             }

           default :
             USBD_error_handler();
         }
         break;
       }
 
       default : /* switch(ctl->type) */
         USBD_error_handler();
     }
   }
 #if USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES
   else
   {
     USBD_AUDIO_EP_DataTypeDef* data_ep = haudio->last_control.entity.data_ep;
     uint16_t selector = HIBYTE(haudio->last_control.wValue);
 
     if(selector == USBD_AUDIO_CONTROL_EP_SAMPL_FREQ)
     {
       /* TODO: Проверить длину данных и корректность запроса */
       switch(haudio->last_control.req)
       {
         case USBD_AUDIO_REQ_SET_CUR:
           if(data_ep->control_cbk.SetCurFrequency)
           {
             uint8_t restart_interface = 0;
             data_ep->control_cbk.SetCurFrequency(AUDIO_FREQ_FROM_DATA(haudio->last_control.data),
                                                 &restart_interface,
                                                 data_ep->private_data);
 
             for(int i=0; i < haudio->aud_function.as_interfaces_count; i++)
             {
               if(data_ep == &haudio->aud_function.as_interfaces[i].data_ep)
               {
                 /* Обновляем частоту синхронизационной конечной точки */
 #if USBD_SUPPORT_AUDIO_OUT_FEEDBACK
                 if(haudio->aud_function.as_interfaces[i].synch_enabled)
                 {
                   uint32_t rate;
                   rate = haudio->aud_function.as_interfaces[i].synch_ep.GetFeedback(
                                  haudio->aud_function.as_interfaces[i].synch_ep.private_data);
                   get_usb_full_speed_rate(rate, haudio->aud_function.as_interfaces[i].synch_ep.feedback_data);
                 }
 #endif /*USBD_SUPPORT_AUDIO_OUT_FEEDBACK */
 
                 if(restart_interface)
                 {
                   if(haudio->aud_function.as_interfaces[i].alternate != 0)
                   {
                     int alt = haudio->aud_function.as_interfaces[i].alternate;
                     USBD_AUDIO_SetInterfaceAlternate(pdev, i, 0);
                     USBD_AUDIO_SetInterfaceAlternate(pdev, i, alt);
                   }
                 }
                 break;
               }
             }
 
           }
           break;
 
         default :
           USBD_error_handler();
       }
     }
     else
     {
       USBD_error_handler();
     }
   }
 #endif /* USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES */
 
   return USBD_OK;
 }
 
 /**
   * @brief  USBD_AUDIO_EP0_TxReady
   *         Обрабатывает событие завершения передачи на EP0
   * @param  pdev: экземпляр устройства
   * @retval статус операции
   */
 static uint8_t  USBD_AUDIO_EP0_TxReady (USBD_HandleTypeDef *pdev)
 {
   /* На данный момент обрабатываются только OUT-запросы */
   return USBD_OK;
 }
 
 /**
   * @brief  USBD_AUDIO_SOF
   *         Обрабатывает событие SOF (Start of Frame)
   * @param  pdev: экземпляр устройства
   * @retval статус операции
   */
 uint8_t  USBD_AUDIO_SOF (USBD_HandleTypeDef *pdev)
 {
   USBD_AUDIO_HandleTypeDef   *haudio;
 
   haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
 
   // Вызов обработчика SOF для всех активных аудиоинтерфейсов
   for(int i = 0; i < haudio->aud_function.as_interfaces_count; i++)
   {
     if(haudio->aud_function.as_interfaces[i].alternate != 0)
     {
       if(haudio->aud_function.as_interfaces[i].SofReceived)
       {
         haudio->aud_function.as_interfaces[i].SofReceived(haudio->aud_function.as_interfaces[i].private_data);
       }
     }
   }
 
   return USBD_OK;
 }
/**
  * @brief  USBD_AUDIO_IsoINIncomplete
  *         Обрабатывает событие незавершённого изохронного IN-пакета
  * @param  pdev: экземпляр устройства
  * @param  epnum: индекс конечной точки
  * @retval статус
  */
 uint8_t  USBD_AUDIO_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
 {
  USBD_AUDIO_EPTypeDef   *ep;
  USBD_AUDIO_HandleTypeDef   *haudio;
  uint16_t current_sof;
   haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  /* @TODO проверить, является ли обратная связь ответственной за это событие */
   for(int i = 1; i<USBD_AUDIO_MAX_IN_EP; i++)
   {
     ep = &haudio->ep_in[i];
     current_sof = USB_SOF_NUMBER();
     if((ep->open) && IS_ISO_IN_INCOMPLETE_EP(i,current_sof, ep->tx_rx_soffn))
     {
       epnum = i|0x80;
       USB_CLEAR_INCOMPLETE_IN_EP(epnum);
       USBD_LL_FlushEP(pdev, epnum);
       ep->tx_rx_soffn = USB_SOF_NUMBER();
 #if USBD_SUPPORT_AUDIO_OUT_FEEDBACK
      if(ep->ep_type==USBD_AUDIO_FEEDBACK_EP)
       {
         // Передача данных обратной связи через ISO IN
         USBD_LL_Transmit(pdev,
                          epnum,
                          ep->ep_description.sync_ep->feedback_data,
                          ep->max_packet_length);
         continue;
       }
      else
 #endif /*USBD_SUPPORT_AUDIO_OUT_FEEDBACK */
      if(ep->ep_type==USBD_AUDIO_DATA_EP)
       {
         // Передача аудиоданных через ISO IN
         USBD_LL_Transmit(pdev,
                       epnum,
                       ep->ep_description.data_ep->buf,
                       ep->ep_description.data_ep->length);
       }
      else
      {
        USBD_error_handler();
      }
 
     }
   }
   return 0;
 }
 /**
   * @brief  USBD_AUDIO_IsoOutIncomplete
   *         Обрабатывает событие незавершённого изохронного OUT-пакета
   * @param  pdev: экземпляр устройства
   * @param  epnum: индекс конечной точки
   * @retval статус
   */
 static uint8_t  USBD_AUDIO_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
 {
 
   return USBD_OK;
 }
 /**
   * @brief  USBD_AUDIO_DataOut
   *         Обрабатывает этап приёма данных OUT
   * @param  pdev: экземпляр устройства
   * @param  epnum: индекс конечной точки
   * @retval статус
   */
 
 uint8_t  USBD_AUDIO_DataOut (USBD_HandleTypeDef *pdev,
                               uint8_t epnum)
 {
 
   USBD_AUDIO_EPTypeDef * ep;
   uint8_t *pbuf ;
   uint16_t packet_length;
 
 
   ep=&((USBD_AUDIO_HandleTypeDef*) pdev->pClassData)->ep_out[epnum];
 
   if(ep->open)
   {
     /* Получаем длину принятых данных */
     packet_length = USBD_LL_GetRxDataSize(pdev, epnum);
     /* Уведомляем пользователя о приёме данных */
     ep->ep_description.data_ep->DataReceived(packet_length,ep->ep_description.data_ep->private_data);
 
     /* Получаем буфер для следующего пакета */
     pbuf=  ep->ep_description.data_ep->GetBuffer(ep->ep_description.data_ep->private_data,&packet_length);
     /* Подготавливаем OUT-точку к приёму следующего аудиопакета */
      USBD_LL_PrepareReceive(pdev,
                             epnum,
                             pbuf,
                             packet_length);
     }
     else
     {
       USBD_error_handler();
     }
 
 
     return USBD_OK;
 }
 
 /**
   * @brief  AUDIO_REQ
   *         Обрабатывает управляющие запросы класса AUDIO
   * @param  pdev: экземпляр устройства
   * @param  req: запрос setup класса
   * @retval статус
   */
 static uint8_t AUDIO_REQ(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
 {
   USBD_AUDIO_HandleTypeDef   *haudio;
   USBD_AUDIO_ControlTypeDef * ctl = 0;
   uint8_t unit_id,control_selector;
   uint16_t *tmpdata = NULL;
 
   haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
 
   /* Сбрасываем данные последней команды */
   haudio->last_control.req = 0x00;
 
   /* Извлекаем ID юнита из запроса */
   unit_id = HIBYTE(req->wIndex);
 
   for (int i = 0;i < haudio->aud_function.control_count; i++)
   {
     if(unit_id == haudio->aud_function.controls[i].id)
     {
       ctl = &haudio->aud_function.controls[i];
       break;
     }
   }
 
   if(!ctl)
   {
     /* Управление не поддерживается */
     USBD_CtlError (pdev, req);
     return  USBD_FAIL;
   }
 
   control_selector = HIBYTE(req->wValue);
 
   if((ctl->control_selector_map & control_selector) == 0)
   {
     /* Запрошенный параметр управления не поддерживается */
     USBD_CtlError (pdev, req);
       return  USBD_FAIL;
   }
 
   if(!(req->bRequest&0x80))
   {
     /* Это SET-запрос */
     /* @TODO проверить длину данных */
      haudio->last_control.wValue  = req->wValue;
      haudio->last_control.entity.controller= ctl;
      haudio->last_control.request_target = AUDIO_UNIT_CONTROL_REQUEST;
      haudio->last_control.len = req->wLength;
      haudio->last_control.req = req->bRequest;
      USBD_CtlPrepareRx (pdev,
                         haudio->last_control.data,
                        req->wLength);
       return USBD_OK;
   }
 
   switch(ctl->type)
   {
     case USBD_AUDIO_CS_AC_SUBTYPE_FEATURE_UNIT:
          {
            USBD_AUDIO_FeatureControlCallbacksTypeDef* feature_control = ctl->Callbacks.feature_control;
           switch(control_selector)
           {
                   case USBD_AUDIO_CONTROL_FEATURE_UNIT_MUTE:
                     {
                       /* @TODO обработать случай нескольких каналов и ошибки в GetCur */
 
                       haudio->last_control.data[0] = 0;
                       if(feature_control->GetMute)
                       {
                         feature_control->GetMute(LOBYTE(req->wValue),
                                                                 &haudio->last_control.data[0], ctl->private_data);
                       }
                       /* Отправляем текущее состояние Mute */
                       USBD_CtlSendData (pdev, haudio->last_control.data,1);
 
                       break;
                      }
                 default :
                           USBD_error_handler();
                 }
           break;
          }
 
   default : /* switch(ctl->type)*/
             USBD_error_handler();
     }
   return USBD_OK;
 }
 #if USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES
 /**
   * @brief  AUDIO_EP_REQ
   *         Обрабатывает управляющие запросы уровня конечной точки (EP)
   * @param  pdev: экземпляр устройства
   * @param  req: запрос setup класса
   * @retval статус
   */
 static uint8_t AUDIO_EP_REQ(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
 {
   USBD_AUDIO_HandleTypeDef   *haudio;
   USBD_AUDIO_EP_DataTypeDef* data_ep = 0;
   uint8_t ep_num, control_selector;
 
   /* Получаем указатель на основную структуру драйвера */
   haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
   /* Извлекаем номер конечной точки из запроса */
   ep_num = LOBYTE(req->wIndex);
 
   /* Поиск зарегистрированной аудио-точки данных */
   for (int i = 0;i < haudio->aud_function.as_interfaces_count; i++)
   {
     if(ep_num == haudio->aud_function.as_interfaces[i].data_ep.ep_num)
     {
         data_ep = &haudio->aud_function.as_interfaces[i].data_ep;
         break;
     }
   }
 
   if(!data_ep)
   {
     /* Конечная точка не найдена */
     USBD_CtlError (pdev, req);
     return  USBD_FAIL;
   }
 
   /* Извлекаем поле CS (Control Selector) из запроса */
   control_selector = HIBYTE(req->wValue);
 
   /* Проверяем, поддерживается ли запрошенный параметр управления */
   if((data_ep->control_selector_map & control_selector) == 0)
   {
     /* Управление не поддерживается */
     USBD_CtlError (pdev, req);
     return  USBD_FAIL;
   }
 
   /* Если это SET-запрос, готовим EP к приёму данных на этапе DATA */
   if(!(req->bRequest&0x80))
   {
     /* Это SET-запрос */
     /* @TODO проверить корректность длины передаваемых данных */
      haudio->last_control.wValue  = req->wValue;
      haudio->last_control.entity.data_ep = data_ep;
      haudio->last_control.request_target = AUDIO_EP_REQUEST;
      haudio->last_control.len = req->wLength;
      haudio->last_control.req = req->bRequest;
      USBD_CtlPrepareRx (pdev,
                         haudio->last_control.data,
                        req->wLength);
       return USBD_OK;
   }
 
   /* Текущая реализация поддерживает только управление частотой дискретизации */
   if(control_selector == USBD_AUDIO_CONTROL_EP_SAMPL_FREQ)
   {
     switch(req->bRequest)
     {
       case USBD_AUDIO_REQ_GET_CUR:
       {
         uint32_t freq=0;
         if(data_ep->control_cbk.GetCurFrequency)
         {
             // Получаем текущую частоту дискретизации
             data_ep->control_cbk.GetCurFrequency(&freq, data_ep->private_data);
         }
         AUDIO_FREQ_TO_DATA(freq , haudio->last_control.data)
         break;
       }
       case USBD_AUDIO_REQ_GET_MIN:
             AUDIO_FREQ_TO_DATA(data_ep->control_cbk.MinFrequency , haudio->last_control.data)
             break;
       case USBD_AUDIO_REQ_GET_MAX:
             AUDIO_FREQ_TO_DATA(data_ep->control_cbk.MaxFrequency , haudio->last_control.data)
           break;
 
       /* case USBD_AUDIO_REQ_GET_RES: - не реализовано */
       default :
              USBD_CtlError (pdev, req);
               return  USBD_FAIL;
       }
   }
   else
   {
     USBD_error_handler();
   }
 
   /* Отправляем данные в хост */
   USBD_CtlSendData (pdev, haudio->last_control.data,3);
   return 0;
 }
 #endif /*USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES*/
 
 
 /**
  * @brief  USBD_AUDIO_GetDeviceQualifierDesc
  *         Возвращает указатель на дескриптор Device Qualifier
  * @param  length : указатель на переменную для длины дескриптора
  * @retval указатель на буфер с дескриптором
  */
 uint8_t  *USBD_AUDIO_GetDeviceQualifierDesc (uint16_t *length)
 {
   *length = sizeof (USBD_AUDIO_DeviceQualifierDesc);
   return USBD_AUDIO_DeviceQualifierDesc;
 }
 
 /**
  * @brief  USBD_AUDIO_RegisterInterface
  *         Регистрирует пользовательский интерфейс аудиоустройства
  * @param  pdev: указатель на дескриптор устройства
  * @param  aifc: указатель на структуру с callback-функциями
  * @retval статус
  */
 uint8_t  USBD_AUDIO_RegisterInterface  (USBD_HandleTypeDef   *pdev,
                                         USBD_AUDIO_InterfaceCallbacksfTypeDef *aifc)
 {
   if(aifc != NULL)
   {
     pdev->pUserData= aifc;
     aifc->GetConfigDesc(&USBD_AUDIO_CfgDesc, &USBD_AUDIO_CfgDescSize, aifc->private_data);
   }
   return 0;
 }
 
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