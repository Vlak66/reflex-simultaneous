/**
 ******************************************************************************
  * @file    audio_usb_nodes.c
  * @author  MCD Application Team
  * @brief   Реализация модулей ввода/вывода USB и функциональных блоков (Feature Unit).
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019  STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован компанией ST по лицензии Ultimate Liberty,
  * SLA0044, "License"; вы не можете использовать этот файл, кроме как в соответствии
  * с условиями лицензии. Вы можете получить копию лицензии по адресу:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usb_audio.h"
#include "audio_usb_nodes.h"
#include "usb_audio_descriptors.h"
#include "audio_configuration.h"

/* Внешние переменные --------------------------------------------------------*/
/* Приватные макросы ---------------------------------------------------------*/
/* Приватные переменные ------------------------------------------------------*/
/* Приватные определения -----------------------------------------------------*/
#if USE_USB_AUDIO_CLASS_10
#if (defined USE_AUDIO_USB_PLAY_MULTI_FREQUENCIES)||(defined USE_AUDIO_USB_RECORD_MULTI_FREQUENCIES)
#if USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES
#define USE_AUDIO_USB_MULTI_FREQUENCIES 1
#else /* USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES */
#error "Для поддержки нескольких частот необходимо определить USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES"
#endif /* USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES */
#endif /*(defined USE_AUDIO_USB_PLAY_MULTI_FREQUENCIES)||(defined USE_AUDIO_USB_RECORD_MULTI_FREQUENCIES) */
#endif /* USE_USB_AUDIO_CLASS_10 */

#define DEBUG_USB_NODES  0  /* установите 1 для отладки USB-ввода при воспроизведении */
#if DEBUG_USB_NODES
#define USB_INPUT_NODE_DEBUG_BUFFER_SIZE 1000
/* Приватные типы данных -----------------------------------------------------------*/
typedef struct
{
  uint32_t time;
  uint16_t write;
  uint16_t read;
  uint16_t error;
} AUDIO_USBInputBufferDebugStats_t;
#endif /* DEBUG_USB_NODES */

/* Прототипы приватных функций -----------------------------------------------*/
static int8_t     USB_AudioStreamingInputOutputDeInit(uint32_t node_handle);
static int8_t     USB_AudioStreamingInputOutputStart( AUDIO_CircularBuffer_t* buffer, uint16_t threshold ,uint32_t node_handle);
static int8_t     USB_AudioStreamingInputOutputStop( uint32_t node_handle);
static uint16_t   USB_AudioStreamingInputOutputGetMaxPacketLength(uint32_t node_handle);
#if USE_USB_AUDIO_CLASS_10
static int8_t     USB_AudioStreamingInputOutputGetState(uint32_t node_handle);
#endif /*USE_USB_AUDIO_CLASS_10*/
static int8_t     USB_AudioStreamingInputOutputRestart( uint32_t node_handle);
#if USE_USB_AUDIO_PLAYBACK
static int8_t     USB_AudioStreamingInputDataReceived( uint16_t data_len,uint32_t node_handle);
static uint8_t*   USB_AudioStreamingInputGetBuffer(uint32_t node_handle, uint16_t* max_packet_length);
#endif /* USE_USB_AUDIO_PLAYBACK*/

static int8_t USB_AudioStreamingFeatureUnitDInit(uint32_t node_handle);
static int8_t USB_AudioStreamingFeatureUnitStart(AUDIO_USBFeatureUnitCommands_t* commands, uint32_t node_handle);
static int8_t USB_AudioStreamingFeatureUnitStop( uint32_t node_handle);
static int8_t USB_AudioStreamingFeatureUnitGetMute(uint16_t channel,uint8_t* mute, uint32_t node_handle);
static int8_t USB_AudioStreamingFeatureUnitSetMute(uint16_t channel,uint8_t mute, uint32_t node_handle);
#if USE_USB_AUDIO_CLASS_10
static int8_t USB_AudioStreamingFeatureUnitGetStatus(uint32_t node_handle);
#endif /*USE_USB_AUDIO_CLASS_10*/
#if USE_USB_AUDIO_CLASS_10
#ifdef USE_AUDIO_USB_MULTI_FREQUENCIES
static int8_t  USB_AudioStreamingInputOutputGetCurFrequency(uint32_t* freq, uint32_t node_handle);
static int8_t  USB_AudioStreamingInputOutputSetCurFrequency(uint32_t freq,uint8_t*  usb_ep_restart_is_required , uint32_t node_handle);
static uint32_t  USB_AudioStreamingGetNearestFrequency(uint32_t freq,  uint32_t* freq_table, int freq_count);
#endif /*USE_AUDIO_USB_MULTI_FREQUENCIES*/
/* Приватные переменные --------------------------------------------------------*/
#ifdef USE_AUDIO_USB_PLAY_MULTI_FREQUENCIES
/* Объявляет таблицу всех поддерживаемых частот, используется при получении команды изменения частоты */
 uint32_t USB_AUDIO_CONFIG_PLAY_FREQENCIES[USB_AUDIO_CONFIG_PLAY_FREQ_COUNT]=
{
#if USB_AUDIO_CONFIG_PLAY_USE_FREQ_192_K
USB_AUDIO_CONFIG_FREQ_192_K,
#endif /* USB_AUDIO_CONFIG_PLAY_USE_FREQ_192_K */

USB_AUDIO_CONFIG_FREQ_176_4_K,

#if USB_AUDIO_CONFIG_PLAY_USE_FREQ_96_K
USB_AUDIO_CONFIG_FREQ_96_K,
#endif /* USB_AUDIO_CONFIG_PLAY_USE_FREQ_96_K */

USB_AUDIO_CONFIG_FREQ_88_2_K,

#if USB_AUDIO_CONFIG_PLAY_USE_FREQ_48_K
USB_AUDIO_CONFIG_FREQ_48_K,
#endif /*USB_AUDIO_CONFIG_PLAY_USE_FREQ_48_K*/
#if USB_AUDIO_CONFIG_PLAY_USE_FREQ_44_1_K
USB_AUDIO_CONFIG_FREQ_44_1_K,
#endif /*USB_AUDIO_CONFIG_PLAY_USE_FREQ_44_1_K*/
};
#endif /* USE_AUDIO_USB_PLAY_MULTI_FREQUENCIES */
#endif /* USE_USB_AUDIO_CLASS_10 */

#if DEBUG_USB_NODES
static AUDIO_USBInputBufferDebugStats_t stats_buffer [USB_INPUT_NODE_DEBUG_BUFFER_SIZE];
static int stats_count=0;
extern __IO uint32_t uwTick;
#endif /* DEBUG_USB_NODES */

/* Функции ---------------------------------------------------------*/
#if USE_USB_AUDIO_PLAYBACK
/**
  * @brief  USB_AudioStreamingInputInit — инициализирует узел USB Audio Input.
  *         Представляет собой терминал USB-ввода.
  *
  * @param  data_ep (OUT):            Список информации (например, номер эндпоинта, максимальный размер пакета и поддерживаемые элементы управления),
  *                                   а также обратные вызовы для взаимодействия с модулем класса USB Audio.
  * @param  audio_desc(IN):         Поддерживаемые аудиохарактеристики.
  * @param  session_handle(IN):     Дескриптор сессии, из которой создан этот узел.
  * @param  node_handle(IN):        Дескриптор узла, узел должен быть уже выделен.
  * @retval 0 — успех, другое значение — ошибка.
  */
 int8_t  USB_AudioStreamingInputInit(USBD_AUDIO_EP_DataTypeDef* data_ep,
                                              AUDIO_Description_t* audio_desc,
                                              AUDIO_Session_t* session_handle,  uint32_t node_handle)
{
  AUDIO_USBInputOutputNode_t * input_node;

  input_node = (AUDIO_USBInputOutputNode_t *)node_handle;
  input_node->node.audio_description = audio_desc;
  input_node->node.session_handle = session_handle;
  input_node->flags = 0;
  input_node->node.state = AUDIO_NODE_INITIALIZED;
  input_node->node.type = AUDIO_INPUT;
  /* Установка callback'ов узла, которые будут вызываться сессией */
  input_node->IODeInit = USB_AudioStreamingInputOutputDeInit;
  input_node->IOStart = USB_AudioStreamingInputOutputStart;
  input_node->IORestart = USB_AudioStreamingInputOutputRestart;
  input_node->IOStop = USB_AudioStreamingInputOutputStop;
  input_node->packet_length = AUDIO_USB_PACKET_SIZE_FROM_AUD_DESC(audio_desc);
  /* Вычисление максимальной длины пакета */
  input_node->max_packet_length = AUDIO_MAX_PACKET_WITH_FEEDBACK_LENGTH(audio_desc);
  /* Настройка обратных вызовов эндпоинта данных для вызова из класса USB */
  data_ep->ep_num = CONFIG_AUDIO_EP_OUT;
  data_ep->control_name_map = 0;
  data_ep->control_selector_map = 0;
  data_ep->private_data = node_handle;
  data_ep->DataReceived = USB_AudioStreamingInputDataReceived;
  data_ep->GetBuffer = USB_AudioStreamingInputGetBuffer;
  data_ep->GetMaxPacketLength = USB_AudioStreamingInputOutputGetMaxPacketLength;
#if USE_USB_AUDIO_CLASS_10
  data_ep->GetState = USB_AudioStreamingInputOutputGetState;
#ifdef USE_AUDIO_USB_PLAY_MULTI_FREQUENCIES
  data_ep->control_selector_map = USBD_AUDIO_CONTROL_EP_SAMPL_FREQ;
  data_ep->control_cbk.GetCurFrequency = USB_AudioStreamingInputOutputGetCurFrequency;
  data_ep->control_cbk.SetCurFrequency = USB_AudioStreamingInputOutputSetCurFrequency;
  data_ep->control_cbk.MaxFrequency = USB_AUDIO_CONFIG_PLAY_FREQENCIES[0];
  data_ep->control_cbk.MinFrequency = USB_AUDIO_CONFIG_PLAY_FREQENCIES[USB_AUDIO_CONFIG_PLAY_FREQ_COUNT-1];
  data_ep->control_cbk.ResFrequency = 1;
#endif /* USE_AUDIO_USB_PLAY_MULTI_FREQUENCIES */
#endif /* USE_USB_AUDIO_CLASS_10 */
  return 0;
}
#endif /* USE_USB_AUDIO_PLAYBACK*/

/**
  * @brief  USB_AudioStreamingInputOutputDeInit
  *         Деинициализирует узел аудио-ввода по USB
  * @param  node_handle(IN): дескриптор узла, узел должен быть выделен
  * @retval  0 — нет ошибок
  */
 static int8_t  USB_AudioStreamingInputOutputDeInit(uint32_t node_handle)
{
  ((AUDIO_USBInputOutputNode_t *)node_handle)->node.state = AUDIO_NODE_OFF;
  return 0;
}


/**
  * @brief  USB_AudioStreamingInputOutputStart
  *         Запускает узел ввода/вывода по USB: после этого узел готов принимать/отправлять пакеты от/к хосту.
  *         Должна вызываться сразу же после получения запроса на установку альтернативного режима (alternate setting = 1).
  * @param  buffer(IN):             основной кольцевой буфер.
  * @param  threshold(IN):          пороговое значение для кольцевого буфера. После запуска потока запись в буфер разрешена, чтение заблокировано.
  *                             Когда порог достигнут, генерируется событие для сессии и чтение из буфера разблокируется.
  * @param  node_handle(IN):        дескриптор узла, узел должен быть уже инициализирован
  * @retval 0 — нет ошибок
  */
static int8_t  USB_AudioStreamingInputOutputStart( AUDIO_CircularBuffer_t* buffer, uint16_t threshold ,uint32_t node_handle)
{
  AUDIO_USBInputOutputNode_t * io_node;

  io_node = (AUDIO_USBInputOutputNode_t *)node_handle;
  if((io_node->node.state == AUDIO_NODE_INITIALIZED ) ||(io_node->node.state == AUDIO_NODE_STOPPED))
  {
     io_node->node.state = AUDIO_NODE_STARTED;
     io_node->buf = buffer;
     io_node->buf->rd_ptr = io_node->buf->wr_ptr=0;
     io_node->flags = 0;
     if(io_node->node.type == AUDIO_INPUT)
     {
       io_node->specific.input.threshold = threshold;
     }
     else
     {
#if USB_AUDIO_CONFIG_RECORD_USE_FREQ_44_1_K
       if(io_node->node.audio_description->frequency == USB_AUDIO_CONFIG_FREQ_44_1_K)
       {
         io_node->specific.output.packet_44_counter = 0;
       }
#endif /* USB_AUDIO_CONFIG_RECORD_USE_FREQ_44_1_K */
     }
  }
  return 0;
}

/**
  * @brief  USB_AudioStreamingInputOutputStop
  *         Останавливает узел ввода или вывода по USB
  * @param  node_handle: дескриптор узла, узел должен быть инициализирован
  * @retval  0 — нет ошибок
  */
static int8_t  USB_AudioStreamingInputOutputStop( uint32_t node_handle)
{
  AUDIO_USBInputOutputNode_t * io_node;
  io_node = (AUDIO_USBInputOutputNode_t *)node_handle;
  io_node->node.state = AUDIO_NODE_STOPPED;
  return 0;
}
/**
  * @brief  USB_AudioStreamingInputOutputRestart
  *         Вызывается, когда требуется перезапуск узла, например, после изменения частоты
  * @param  node_handle(IN):
  * @retval  0 — нет ошибок
  */
static int8_t  USB_AudioStreamingInputOutputRestart( uint32_t node_handle)
{
  AUDIO_USBInputOutputNode_t * io_node;
  io_node = (AUDIO_USBInputOutputNode_t *)node_handle;
  if(io_node->node.state == AUDIO_NODE_STARTED)
  {
    io_node->flags = AUDIO_IO_RESTART_REQUIRED;   /* этот флаг указывает, что узел должен быть остановлен при следующем вызове через callback */
    return 0;
  }
  return 0;
}

#if USE_USB_AUDIO_PLAYBACK
/**
  * @brief  USB_AudioStreamingInputDataReceived
  *         Callback-функция, вызываемая классом USB при получении нового пакета
  * @param  data_len(IN):           длина пакета
  * @param  node_handle(IN):        дескриптор входного узла, узел должен быть инициализирован и запущен
  * @retval  0 — если ошибок нет
  */
static int8_t  USB_AudioStreamingInputDataReceived( uint16_t data_len, uint32_t node_handle)
 {
   AUDIO_USBInputOutputNode_t * input_node;
   AUDIO_CircularBuffer_t *buf;
   uint16_t buffer_data_count;

   input_node = (AUDIO_USBInputOutputNode_t *)node_handle;
   if(input_node->node.state == AUDIO_NODE_STARTED)
   {
     /* @TODO добавить обнаружение переполнения буфера (overrun) */
     if(input_node->flags&AUDIO_IO_RESTART_REQUIRED)
     {
     /* При необходимости перезапуска игнорируем пакет и сбрасываем буфер */
       input_node->flags = 0;
       input_node->buf->rd_ptr=input_node->buf->wr_ptr = 0;
       return 0;
     }

     buf=input_node->buf;
     buf->wr_ptr += data_len;/* увеличиваем указатель записи */

     if((input_node->flags&AUDIO_IO_BEGIN_OF_STREAM) == 0)
     { /* это первый пакет */
       input_node->node.session_handle->SessionCallback(AUDIO_BEGIN_OF_STREAM,(AUDIO_Node_t*)input_node,
                                                        input_node->node.session_handle);   /* отправляем событие сессии */
       input_node->flags |= AUDIO_IO_BEGIN_OF_STREAM;
     }
     else
     {   /* если данные вышли за пределы буфера, то копируем их в начало */
        if(buf->wr_ptr > buf->size)
        {
          buf->wr_ptr -= buf->size;
          memcpy(buf->data, buf->data+buf->size, buf->wr_ptr);
        }
      /* считаем количество доступных данных в буфере */
      buffer_data_count = AUDIO_BUFFER_FILLED_SIZE(buf);
      if(buf->wr_ptr == buf->size)
      {
        buf->wr_ptr = 0;
      }
      if(((input_node->flags&AUDIO_IO_THRESHOLD_REACHED) == 0)&&
          (buffer_data_count >= input_node->specific.input.threshold))
      {
         input_node->node.session_handle->SessionCallback(AUDIO_THRESHOLD_REACHED, (AUDIO_Node_t*)input_node,
                                                         input_node->node.session_handle);   /* информируем сессию, что порог заполнения буфера достигнут */
          input_node->flags |= AUDIO_IO_THRESHOLD_REACHED ;
       }
       else
       {
        input_node->node.session_handle->SessionCallback(AUDIO_PACKET_RECEIVED, (AUDIO_Node_t*)input_node,
                                                         input_node->node.session_handle); /* информируем сессию, что пакет получен */
       }
     }
    }
   else
   {
     Error_Handler();
   }
   return 0;
 }
/**
  * @brief  USB_AudioStreamingInputGetBuffer
  *         Callback-функция, вызываемая классом аудио по USB для получения буфера, в который будет записан следующий пакет
  * @param  node_handle(IN):        дескриптор входного узла, узел должен быть инициализирован и запущен
  * @param  max_packet_length(OUT): максимальная длина принимаемого пакета
  * @retval указатель на буфер для приёма данных
  */
 static uint8_t* USB_AudioStreamingInputGetBuffer(uint32_t node_handle, uint16_t* max_packet_length)
 {
   AUDIO_USBInputOutputNode_t* input_node;
   uint16_t buffer_free_size;
 
   input_node = (AUDIO_USBInputOutputNode_t *)node_handle;
 #if DEBUG_USB_NODES
   stats_buffer[stats_count].read = input_node->buf->rd_ptr;
   stats_buffer[stats_count].write = input_node->buf->wr_ptr;
   stats_buffer[stats_count].time = uwTick;
 
   stats_count++;
   if(stats_count == USB_INPUT_NODE_DEBUG_BUFFER_SIZE)
   {
     stats_count=0;
   }
 #endif /*DEBUG_USB_NODES*/
   *max_packet_length = input_node->max_packet_length;
   if( input_node->node.state == AUDIO_NODE_STARTED)
   {
     /* Проверка на возможное переполнение буфера */
     buffer_free_size  = AUDIO_BUFFER_FREE_SIZE(input_node->buf);
 
     if(buffer_free_size < input_node->max_packet_length)
     {
       input_node->node.session_handle->SessionCallback(AUDIO_OVERRUN, (AUDIO_Node_t*)input_node,
                                                        input_node->node.session_handle);
     }
 
     if(input_node->flags&AUDIO_IO_RESTART_REQUIRED)
     {
      input_node->flags = 0;
      input_node->buf->rd_ptr = input_node->buf->wr_ptr = 0;
     }
     return input_node->buf->data+input_node->buf->wr_ptr;
   }
   else
   {
     Error_Handler();
     return 0; /* эта строка недостижима */
   }
 }
 #endif /* USE_USB_AUDIO_PLAYBACK */
 
/**
  * @brief  USB_AudioStreamingInputOutputGetMaxPacketLength
  *         Возвращает максимальную длину пакета. Вызывается классом USB Audio.
  * @param  node_handle(IN): дескриптор входного узла, узел должен быть инициализирован
  * @retval Максимальная длина пакета
*/
static uint16_t  USB_AudioStreamingInputOutputGetMaxPacketLength(uint32_t node_handle)
{

  return ((AUDIO_USBInputOutputNode_t *)node_handle)->max_packet_length;
}

#if USE_USB_AUDIO_CLASS_10
#ifdef USE_AUDIO_USB_MULTI_FREQUENCIES
/**
  * @brief  USB_AudioStreamingInputOutputGetCurFrequency
  *         Возвращает текущую частоту дискретизации. Вызывается классом USB Audio.
  * @param  freq(OUT): текущая частота дискретизации
  * @param  node_handle: дескриптор узла ввода/вывода USB, узел должен быть инициализирован
  * @retval 0, если ошибок нет
*/
static int8_t  USB_AudioStreamingInputOutputGetCurFrequency(uint32_t* freq, uint32_t node_handle)
{
 *freq = ((AUDIO_USBInputOutputNode_t *)node_handle) ->node.audio_description->frequency;
 return 0;
}

/**
  * @brief  USB_AudioStreamingInputOutputSetCurFrequency
  *         Устанавливает текущую частоту дискретизации. Если заданная частота не поддерживается,
  *         устанавливается ближайшая допустимая частота. Вызывается классом USB Audio.
  * @param  freq(IN): частота для установки
  * @param  usb_ep_restart_is_required: указатель на флаг, который будет установлен,
  *         если требуется перезапуск USB-эндпоинта
  * @param  node_handle: дескриптор узла ввода/вывода USB, узел должен быть инициализирован
  * @retval 0, если ошибок нет
*/
static int8_t  USB_AudioStreamingInputOutputSetCurFrequency(uint32_t freq, uint8_t* usb_ep_restart_is_required, uint32_t node_handle)
{
  AUDIO_USBInputOutputNode_t *usb_io_node=(AUDIO_USBInputOutputNode_t *)node_handle;
  AUDIO_Description_t* aud;
  uint32_t best_matched_freq;
 /* Поиск ближайшей поддерживаемой частоты */
 aud = usb_io_node->node.audio_description;
 if(aud->frequency != freq)
 {
 #ifdef USE_AUDIO_USB_PLAY_MULTI_FREQUENCIES
 if(usb_io_node->node.type == AUDIO_INPUT)
 {

    best_matched_freq = USB_AudioStreamingGetNearestFrequency(freq,USB_AUDIO_CONFIG_PLAY_FREQENCIES,USB_AUDIO_CONFIG_PLAY_FREQ_COUNT);
  if(aud->frequency == best_matched_freq)
  {/* Частота не изменилась, перезапуск эндпоинта не требуется */
    *usb_ep_restart_is_required = 0;
    return 0;
  }
  else
  {
    aud->frequency = best_matched_freq;
  }
  /* Обновить максимальную длину пакета */
  usb_io_node->max_packet_length = AUDIO_MAX_PACKET_WITH_FEEDBACK_LENGTH(aud);
 }
 #endif /* USE_AUDIO_USB_PLAY_MULTI_FREQUENCIES*/

  usb_io_node->packet_length = AUDIO_USB_PACKET_SIZE_FROM_AUD_DESC(aud);
  usb_io_node->node.session_handle->SessionCallback(AUDIO_FREQUENCY_CHANGED,(AUDIO_Node_t*)usb_io_node,
                                                        usb_io_node->node.session_handle);
 *usb_ep_restart_is_required = 1;
 }
 else
 {
   *usb_ep_restart_is_required = 0;
 }
 return 0;
}
#endif /*USE_AUDIO_USB_MULTI_FREQUENCIES*/
#endif /* USE_USB_AUDIO_CLASS_10 */
#if (defined USE_AUDIO_USB_PLAY_MULTI_FREQUENCIES)||(defined USE_AUDIO_USB_RECORD_MULTI_FREQUENCIES)
/**
  * @brief  USB_AudioStreamingGetNearestFrequency
  *        ищет в таблице частот ближайшее значение к частоте, переданной в параметре
  * @param  freq(IN)): целевая частота
  * @param  freq_table(IN): таблица частот (должна быть отсортирована)
  * @param  freq_count(IN): размер таблицы
  * @retval  ближайшее значение частоты
*/
static uint32_t  USB_AudioStreamingGetNearestFrequency(uint32_t freq,  uint32_t* freq_table,  int freq_count)
{
  if(freq >= freq_table[0])
 {
   return freq_table[0];
 }
 else
 {
    if(freq <= freq_table[freq_count-1])
   {
     return freq_table[freq_count-1];
   }
   else
   {
     for(int i = 1; i<freq_count; i++)
     {
       if(freq >= freq_table[i])
       {
         return ((freq_table[i-1] - freq )<= ( freq - freq_table[i]))?
           freq_table[i-1] : freq_table[i];
       }
     }
   }
 }
return 0;
}
#endif /* (defined USE_AUDIO_USB_PLAY_MULTI_FREQUENCIES)||(defined USE_AUDIO_USB_RECORD_MULTI_FREQUENCIES) */
#if USE_USB_AUDIO_CLASS_10
/**
  * @brief  USB_AudioStreamingInputOutputGetState
  *         возвращает состояние конечной точки данных
  * @param  node_handle: дескриптор входного узла, узел должен быть инициализирован
  * @retval  0
*/
static int8_t  USB_AudioStreamingInputOutputGetState(uint32_t node_handle)
{
  return 0;
}
#endif /* USE_USB_AUDIO_CLASS_10 */
/**
  * @brief  USB_AudioStreamingFeatureUnitInit
  *         Инициализирует управляющий узел "Feature Unit"
  * @param  usb_control_feature(OUT): структура для связи с классом USB Audio, содержит информацию,
  *                                   элементы управления и обратные вызовы для обработки управлений, относящихся к Feature Unit
  * @param  audio_defaults(IN):             настройки по умолчанию для аудио
  * @param  unit_id(IN):                    идентификатор USB-устройства
  * @param  node_handle(IN):                дескриптор узла, узел должен быть выделен
  * @retval  0 при отсутствии ошибок
  */
 int8_t USB_AudioStreamingFeatureUnitInit(USBD_AUDIO_ControlTypeDef* usb_control_feature,
                                   AUDIO_USBFeatureUnitDefaults_t* audio_defaults, uint8_t unit_id,
                                   uint32_t node_handle)
{
  AUDIO_USB_CF_NodeTypeDef * cf;
  cf = (AUDIO_USB_CF_NodeTypeDef*)node_handle;
  memset(cf,0,sizeof(AUDIO_USB_CF_NodeTypeDef));
  cf->node.state = AUDIO_NODE_INITIALIZED;
  cf->node.type = AUDIO_CONTROL;
  cf->unit_id = unit_id;
  cf->CFInit = USB_AudioStreamingFeatureUnitInit;
  cf->CFDeInit = USB_AudioStreamingFeatureUnitDInit;
  cf->CFStart = USB_AudioStreamingFeatureUnitStart;
  cf->CFStop = USB_AudioStreamingFeatureUnitStop;
  cf->CFSetMute = USB_AudioStreamingFeatureUnitSetMute;
#if USE_USB_AUDIO_CLASS_10
  cf->usb_control_callbacks.GetStatus = USB_AudioStreamingFeatureUnitGetStatus;
#endif /*USE_USB_AUDIO_CLASS_10*/
  cf->usb_control_callbacks.GetMute = USB_AudioStreamingFeatureUnitGetMute;
  cf->usb_control_callbacks.SetMute = USB_AudioStreamingFeatureUnitSetMute;
  cf->node.audio_description=audio_defaults->audio_description;
  /* заполнение структуры, используемой модулем USB Audio Class */
  usb_control_feature->id = unit_id;
  usb_control_feature->control_req_map = 0;
  usb_control_feature->control_selector_map = USBD_AUDIO_FU_MUTE_CONTROL;
  usb_control_feature->type = USBD_AUDIO_CS_AC_SUBTYPE_FEATURE_UNIT;
  usb_control_feature->Callbacks.feature_control = &cf->usb_control_callbacks;
  usb_control_feature->private_data = node_handle;
  return 0;
}
/**
  * @brief  USB_AudioStreamingFeatureUnitDInit
  *         Деинициализирует узел "Feature Unit"
  * @param  node_handle: дескриптор узла, узел должен быть инициализирован
  * @retval  0 при отсутствии ошибок
  */
static int8_t USB_AudioStreamingFeatureUnitDInit(uint32_t node_handle)
{
  ((AUDIO_USB_CF_NodeTypeDef*)node_handle)->node.state = AUDIO_NODE_OFF;
  return 0;
}
/**
  * @brief  USB_AudioStreamingFeatureUnitStart
  *         Запускает узел "Feature Unit". После вызова start узел выполняет команды управления, такие как установка громкости и Mute.
  *         Если есть уже полученные и ожидающие команды, они будут выполнены в этой функции.
  * @param  commands(IN): список обратных вызовов для выполнения команд управления, таких как SetVolume и Mute. 
  *                      Эта функция зависит от кодека и микрофона.
  * @param  node_handle(IN): дескриптор узла, узел должен быть выделен
  * @retval  0 при отсутствии ошибок
  */
static int8_t USB_AudioStreamingFeatureUnitStart(AUDIO_USBFeatureUnitCommands_t* commands, uint32_t node_handle)
{
  AUDIO_USB_CF_NodeTypeDef *cf;
  cf = (AUDIO_USB_CF_NodeTypeDef*)node_handle;
  cf->control_cbks = *commands;
  cf->node.state = AUDIO_NODE_STARTED;
  return 0;
}
/**
  * @brief  USB_AudioStreamingFeatureUnitStop
  *         Останавливает узел "Feature Unit"
  * @param  node_handle: дескриптор узла, узел должен быть запущен
  * @retval  0 при отсутствии ошибок
  */
static int8_t USB_AudioStreamingFeatureUnitStop( uint32_t node_handle)
{
  /* @TODO разработать реализацию для feature */
  AUDIO_USB_CF_NodeTypeDef * cf;
  cf = (AUDIO_USB_CF_NodeTypeDef*)node_handle;
  cf->node.state = AUDIO_NODE_STOPPED;
  return 0;
}
/**
  * @brief  USB_AudioStreamingFeatureUnitGetMute
  *         Получает текущее значение Mute
  * @param  channel: номер канала, 0 для главного канала (в данный момент поддерживается только этот вариант)
  * @param  mute: возвращаемое значение Mute
  * @param  node_handle: дескриптор узла Feature, узел должен быть инициализирован
  * @retval  0 при отсутствии ошибок
  */
static int8_t USB_AudioStreamingFeatureUnitGetMute(uint16_t channel, uint8_t* mute, uint32_t node_handle)
{
  /**@TODO добавить поддержку нескольких каналов */
  *mute = ((AUDIO_USB_CF_NodeTypeDef*)node_handle)->node.audio_description->audio_mute;
  return 0;
}
/**
  * @brief  USB_AudioStreamingFeatureUnitSetMute
  *         Устанавливает значение Mute
  * @param  channel: номер канала, 0 для главного канала (в данный момент поддерживается только этот вариант)
  * @param  mute: новое значение Mute
  * @param  node_handle: дескриптор узла Feature, узел должен быть инициализирован
  * @retval  0 при отсутствии ошибок
  */
static int8_t USB_AudioStreamingFeatureUnitSetMute(uint16_t channel, uint8_t mute, uint32_t node_handle)
{
  AUDIO_USB_CF_NodeTypeDef * cf;
  cf = (AUDIO_USB_CF_NodeTypeDef*)node_handle;
  /**@TODO добавить поддержку нескольких каналов */
  cf->node.audio_description->audio_mute = mute;
  if((cf->node.state == AUDIO_NODE_STARTED)&&(cf->control_cbks.SetMute))
  {
      cf->control_cbks.SetMute(channel, mute, cf->control_cbks.private_data);
  }
  return 0;
}


#if USE_USB_AUDIO_CLASS_10
/**
  * @brief  USB_AudioStreamingFeatureUnitGetStatus
  *         Получает статус узла Feature Unit
  * @param  node_handle:        дескриптор узла Feature, узел должен быть инициализирован
  * @retval 0 при отсутствии ошибок
  */
static int8_t  USB_AudioStreamingFeatureUnitGetStatus( uint32_t node_handle )
{
  return 0;
}
#endif /* USE_USB_AUDIO_CLASS_10 */
/**
  * @brief  USB_AudioStreamingInitializeDataBuffer
  *         Кольцевой буфер имеет общий размер buffer_size. Этот размер делится на две части: основной размер и защитный участок (margin).
  *         Margin находится в конце кольцевого буфера. Он используется потому, что некоторые пакеты могут иметь размер равный regular size +/- 1 семпл.
  * @param  buf:  основной кольцевой буфер
  * @param  buffer_size: общий размер буфера при выделении
  * @param  packet_size: размер пакета USB Audio
  * @param  margin: размер защитного участка
  * @retval 0 при отсутствии ошибок
  */
  void USB_AudioStreamingInitializeDataBuffer(AUDIO_CircularBuffer_t* buf,
                                       uint32_t buffer_size,
                                       uint16_t packet_size, uint16_t margin)
 {
    buf->size = ((int)((buffer_size - margin )
                       / packet_size)) * packet_size;
    buf->rd_ptr = buf->wr_ptr = 0;
 }
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/