/**
  ******************************************************************************
  * @file    audio_speaker_node.c
  * @author  MCD Application Team
  * @brief   реализация узла динамика.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Авторские права (c) 2019  STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован ST по лицензии Ultimate Liberty
  * SLA0044, "Лицензия"; Вы не можете использовать этот файл, кроме как в соответствии с
  * Лицензией. Вы можете получить копию Лицензии по адресу:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/*
*******************************************************************************
*
* Modified by ChipDip to support several Alternate Settings
* to change 16 bit and 24 bit audio resolution on the fly;
* multichannel audio stream support added, 2019.
*
* Изменен ЗАО «ЧИП и ДИП» для одновременной поддержки нескольких Alternate Settings
* (16 бит и 24 бита);
* добавлена поддержка многоканального аудио потока 2019.
*
*******************************************************************************
*/




/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "usbd_audio.h"
#include "audio_speaker_node.h"
#include "audio_configuration.h"
#include "usb_audio.h"
#include "board.h"


/* Private defines -----------------------------------------------------------*/
#define SPEAKER_CMD_STOP                1
#define SPEAKER_CMD_EXIT                2
#define SPEAKER_CMD_CHANGE_FREQUENCE    4
#define SPEAKER_CMD_CHANGE_RESOLUTION   (1 << 3)
#define VOLUME_DB_256_TO_PERCENT(volume_db_256) \
    ((uint8_t)((((int)(volume_db_256) - VOLUME_SPEAKER_MIN_DB_256) * 100) / \
    (VOLUME_SPEAKER_MAX_DB_256 - VOLUME_SPEAKER_MIN_DB_256)))

/* alt buffer max size */
#define SPEAKER_ALT_BUFFER_SIZE ((USB_AUDIO_CONFIG_PLAY_FREQ_MAX+999)/1000)*2*4*2

#ifdef DEBUG_SPEAKER_NODE
#define SPEAKER_DEBUG_BUFFER_SIZE 1000
#endif /*DEBUG_SPEAKER_NODE*/

/* Private function prototypes -----------------------------------------------*/

static int8_t  AUDIO_SpeakerStart(AUDIO_CircularBuffer_t* buffer, uint32_t node_handle);
static int8_t  AUDIO_SpeakerStop( uint32_t node_handle);
static int8_t  AUDIO_SpeakerMute( uint16_t channel_number,  uint8_t mute , uint32_t node_handle);
static int8_t  AUDIO_SpeakerSetVolume( uint16_t channel_number,  int volume ,  uint32_t node_handle);
static void    AUDIO_SpeakerInitInjectionsParams( AUDIO_SpeakerNode_t* speaker);
static void AUDIO_DoPadding_24_32(AUDIO_CircularBuffer_t *buff_src,  uint8_t *data_dest ,  int size);
static int8_t  AUDIO_SpeakerStartReadCount( uint32_t node_handle);
static uint16_t AUDIO_SpeakerGetLastReadCount( uint32_t node_handle);

/* Приватные типы -----------------------------------------------------------*/
#ifdef DEBUG_SPEAKER_NODE
typedef struct
{
  uint32_t time;
  uint16_t injection_size;
  uint16_t read;
  uint16_t dump;
  uint8_t* data;
} AUDIO_SpeakerNodeBufferStats_t;
#endif /* DEBUG_SPEAKER_NODE*/

/* Приватные макросы ------------------------------------------------------------*/
/* Внешние переменные --------------------------------------------------------*/
//extern SAI_HandleTypeDef         haudio_out_sai_1;
#ifdef DEBUG_SPEAKER_NODE
extern __IO uint32_t uwTick;
#endif /* DEBUG_SPEAKER_NODE*/

/* Приватные переменные -----------------------------------------------------------*/
static AUDIO_SpeakerNode_t *AUDIO_SpeakerHandler = 0;
#ifdef DEBUG_SPEAKER_NODE
static AUDIO_SpeakerNodeBufferStats_t AUDIO_SpeakerDebugStats[SPEAKER_DEBUG_BUFFER_SIZE];
static  int AUDIO_SpeakerDebugStats_count =0;
#endif /* DEBUG_SPEAKER_NODE*/

/* Экспортируемые функции ---------------------------------------------------------*/

/**
  * @brief  AUDIO_SpeakerInit
  *         Инициализирует узел аудиодинамика, устанавливает обратные вызовы и запускает кодек. Поскольку данные не готовы,
  *         SAI заполняется из альтернативного буфера (заполненного нулями)
  * @param  audio_description(ВХ): информация об аудио
  * @param  session_handle(ВХ):   дескриптор сессии
  * @param  node_handle(ВХ):      дескриптор узла динамика должен быть выделен
  * @retval 0 в случае отсутствия ошибок
  */
 int8_t  AUDIO_SpeakerInit(AUDIO_Description_t* audio_description,  AUDIO_Session_t* session_handle,
                           uint32_t node_handle)
{
  AUDIO_SpeakerNode_t* speaker;

  speaker = (AUDIO_SpeakerNode_t*)node_handle;
  memset(speaker, 0, sizeof(AUDIO_SpeakerNode_t));
  speaker->node.type = AUDIO_OUTPUT;
  speaker->node.state = AUDIO_NODE_INITIALIZED;
  speaker->node.session_handle = session_handle;
  speaker->node.audio_description = audio_description;
  speaker->specific.alt_buffer = malloc(SPEAKER_ALT_BUFFER_SIZE);
  if(speaker->specific.alt_buffer == 0)
  {
    Error_Handler();
  }
  AUDIO_SpeakerInitInjectionsParams( speaker);

  /* установить обратные вызовы */
  //speaker->SpeakerDeInit = AUDIO_SpeakerDeInit;
  speaker->SpeakerStart = AUDIO_SpeakerStart;
  speaker->SpeakerStop = AUDIO_SpeakerStop;
  speaker->SpeakerChangeFrequency = AUDIO_SpeakerChangeFrequency;
  speaker->SpeakerMute = AUDIO_SpeakerMute;
  speaker->SpeakerSetVolume = AUDIO_SpeakerSetVolume;
  speaker->SpeakerStartReadCount = AUDIO_SpeakerStartReadCount;
  speaker->SpeakerGetReadCount = AUDIO_SpeakerGetLastReadCount;

  SetAudioConfigDependedFuncs(speaker);

  AudioOutInit(speaker->node.audio_description->frequency, audio_description->resolution<<3);
  ExtPowerDisable();

/*
  BSP_AUDIO_OUT_Init_Ext(OUTPUT_DEVICE_AUTO,
                     VOLUME_DB_256_TO_PERCENT(VOLUME_SPEAKER_DEFAULT_DB_256),
                     speaker->node.audio_description->frequency, audio_description->resolution<<3 );
*/

  speaker->SpeakerPlay((uint16_t *)speaker->specific.data,
                       speaker->specific.data_size,
                       speaker->node.audio_description->resolution);
  //BSP_AUDIO_OUT_Play((uint16_t *)speaker->specific.data ,speaker->specific.data_size );

  //начиная с версии 1.6
  //если USB-устройство инициализировано - программный сброс
  //нужно для корректного функционирования при перезагрузке ОС
  //и при включении ПК
  if (AUDIO_SpeakerHandler != 0)
  {
    //Soft reset
    __DSB();

    SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos)      |
                   (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
                   SCB_AIRCR_SYSRESETREQ_Msk);
    __DSB();
    while(1);
  }

  AUDIO_SpeakerHandler = speaker;
  return 0;
}


/**
  * @brief  BSP_AUDIO_OUT_Error_CallBack
  *         Manages the DMA error event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_Error_CallBack(void)
{
  Error_Handler();
}

/**
  * @brief  BSP_AUDIO_OUT_TransferComplete_CallBack
  *         Управляет событием полного завершения передачи DMA.
  * @param  Нет
  * @retval Нет
  */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
  uint16_t wr_distance, read_length;

  if ((AUDIO_SpeakerHandler) &&
      (AUDIO_SpeakerHandler->node.state != AUDIO_NODE_OFF))
  {
    // выполняется, если получена команда остановки
    if (AUDIO_SpeakerHandler->specific.cmd & SPEAKER_CMD_EXIT)
    {
      AUDIO_SpeakerHandler->specific.cmd = 0;
      return;
    }

    if (AUDIO_SpeakerHandler->specific.cmd &
        SPEAKER_CMD_CHANGE_FREQUENCE)
    {
      AUDIO_SpeakerHandler->node.state = AUDIO_NODE_STOPPED;
      AUDIO_SpeakerInitInjectionsParams(AUDIO_SpeakerHandler);
      AUDIO_SpeakerHandler->injection_44_count = 0;
      AudioChangeFrequency(
        AUDIO_SpeakerHandler->node.audio_description->frequency);
      AUDIO_SpeakerHandler->specific.cmd &=
        ~SPEAKER_CMD_CHANGE_FREQUENCE;
    }

    if (AUDIO_SpeakerHandler->specific.cmd & SPEAKER_CMD_STOP)
    {
      AUDIO_SpeakerHandler->specific.data =
        AUDIO_SpeakerHandler->specific.alt_buffer;
      AUDIO_SpeakerHandler->specific.data_size =
        AUDIO_SpeakerHandler->specific.injection_size;
      AUDIO_SpeakerHandler->specific.offset = 0;
      memset(AUDIO_SpeakerHandler->specific.data, 0,
        AUDIO_SpeakerHandler->specific.data_size);
      AUDIO_SpeakerHandler->node.state = AUDIO_NODE_STOPPED;
      AUDIO_SpeakerHandler->specific.cmd ^= SPEAKER_CMD_STOP;
    }

    if (AUDIO_SpeakerHandler->specific.cmd &
        SPEAKER_CMD_CHANGE_RESOLUTION)
    {
      AUDIO_SpeakerHandler->node.state = AUDIO_NODE_STOPPED;
      AUDIO_SpeakerInitInjectionsParams(AUDIO_SpeakerHandler);
      AudioChangeResolution(
        AUDIO_SpeakerHandler->node.audio_description->resolution
        << 3);
      AUDIO_SpeakerHandler->specific.cmd &=
        ~SPEAKER_CMD_CHANGE_RESOLUTION;
    }

    AUDIO_SpeakerHandler->SpeakerPlay(
      (uint16_t *)AUDIO_SpeakerHandler->specific.data,
      (uint16_t)AUDIO_SpeakerHandler->specific.data_size,
      AUDIO_SpeakerHandler->node.audio_description->resolution);

    /* если динамик был запущен, подготовьте следующие данные */
    if (AUDIO_SpeakerHandler->node.state == AUDIO_NODE_STARTED)
    {
#ifdef DEBUG_SPEAKER_NODE
      AUDIO_SpeakerDebugStats[AUDIO_SpeakerDebugStats_count].time =
        uwTick;
#endif /* DEBUG_SPEAKER_NODE */

      /* уведомить сессию о том, что пакет воспроизведен */
      AUDIO_SpeakerHandler->node.session_handle->SessionCallback(
        AUDIO_PACKET_PLAYED,
        (AUDIO_Node_t *)AUDIO_SpeakerHandler,
        AUDIO_SpeakerHandler->node.session_handle);

      /* подготовить следующий размер для инъекции */
      if (AUDIO_SpeakerHandler->node.audio_description->resolution
          == CONFIG_RES_BYTE_24)
      {
        AUDIO_SpeakerHandler->specific.data =
          (AUDIO_SpeakerHandler->specific.offset) ?
          AUDIO_SpeakerHandler->specific.alt_buffer :
          AUDIO_SpeakerHandler->specific.alt_buffer +
          AUDIO_SpeakerHandler->specific.data_size;
        AUDIO_SpeakerHandler->specific.offset ^= 1;
      }

      AUDIO_SpeakerHandler->specific.data_size =
        AUDIO_SpeakerHandler->specific.injection_size;
      read_length = AUDIO_SpeakerHandler->packet_length;

#if USB_AUDIO_CONFIG_PLAY_USE_FREQ_44_1_K
      if ((AUDIO_SpeakerHandler->node.audio_description->frequency
           == USB_AUDIO_CONFIG_FREQ_44_1_K) ||
          (AUDIO_SpeakerHandler->node.audio_description->frequency
           == USB_AUDIO_CONFIG_FREQ_88_2_K) ||
          (AUDIO_SpeakerHandler->node.audio_description->frequency
           == USB_AUDIO_CONFIG_FREQ_176_4_K))
      {
        if (((AUDIO_SpeakerHandler->node.audio_description->frequency
              == USB_AUDIO_CONFIG_FREQ_44_1_K) &&
             (AUDIO_SpeakerHandler->injection_44_count < 9)) ||
            ((AUDIO_SpeakerHandler->node.audio_description->frequency
              == USB_AUDIO_CONFIG_FREQ_88_2_K) &&
             (AUDIO_SpeakerHandler->injection_44_count < 4)) ||
            ((AUDIO_SpeakerHandler->node.audio_description->frequency
              == USB_AUDIO_CONFIG_FREQ_176_4_K) &&
             (AUDIO_SpeakerHandler->injection_44_count < 4)))
        {
          AUDIO_SpeakerHandler->injection_44_count++;
        }
        else
        {
          AUDIO_SpeakerHandler->injection_44_count = 0;
          AUDIO_SpeakerHandler->specific.data_size =
            AUDIO_SpeakerHandler->specific.alt_buf_half_size;
          read_length =
            AUDIO_SpeakerHandler->packet_length_max_44_1;
        }
      }
#endif /* USB_AUDIO_CONFIG_PLAY_USE_FREQ_44_1_K */

      wr_distance =
        AUDIO_BUFFER_FILLED_SIZE(AUDIO_SpeakerHandler->buf);
      if (wr_distance <
          AUDIO_SpeakerHandler->specific.injection_size)
      {
        /** информировать сессию о том, что произошел недозапись */
        AUDIO_SpeakerHandler->node.session_handle->
          SessionCallback(AUDIO_UNDERRUN,
          (AUDIO_Node_t *)AUDIO_SpeakerHandler,
          AUDIO_SpeakerHandler->node.session_handle);
      }
      else
      {
        /* буфер уже подготовлен в половинной передаче */
        if (AUDIO_SpeakerHandler->node.audio_description->
            resolution == CONFIG_RES_BYTE_24)
        {
          AUDIO_DoPadding_24_32(AUDIO_SpeakerHandler->buf,
            AUDIO_SpeakerHandler->specific.data, read_length);
        }
        else
        {
          AUDIO_SpeakerHandler->specific.data =
            AUDIO_SpeakerHandler->buf->data +
            AUDIO_SpeakerHandler->buf->rd_ptr;
#if USB_AUDIO_CONFIG_PLAY_USE_FREQ_44_1_K
          if ((AUDIO_SpeakerHandler->node.audio_description->
               frequency == USB_AUDIO_CONFIG_FREQ_44_1_K) ||
              (AUDIO_SpeakerHandler->node.audio_description->
               frequency == USB_AUDIO_CONFIG_FREQ_88_2_K) ||
              (AUDIO_SpeakerHandler->node.audio_description->
               frequency == USB_AUDIO_CONFIG_FREQ_176_4_K))
          {
            uint16_t d = AUDIO_SpeakerHandler->buf->size -
                         AUDIO_SpeakerHandler->buf->rd_ptr;
            if (d < AUDIO_SpeakerHandler->specific.data_size)
            {
              memcpy(AUDIO_SpeakerHandler->specific.alt_buffer,
                AUDIO_SpeakerHandler->buf->data +
                AUDIO_SpeakerHandler->buf->rd_ptr, d);
              memcpy(AUDIO_SpeakerHandler->specific.alt_buffer + d,
                AUDIO_SpeakerHandler->buf->data,
                AUDIO_SpeakerHandler->specific.data_size - d);
              AUDIO_SpeakerHandler->specific.data =
                AUDIO_SpeakerHandler->specific.alt_buffer;
            }
          }
#endif /* USB_AUDIO_CONFIG_PLAY_USE_FREQ_44_1_K */
        }

        if (AUDIO_SpeakerHandler->SpeakerPrepareData != 0)
        {
          AUDIO_SpeakerHandler->SpeakerPrepareData(
            AUDIO_SpeakerHandler->specific.data,
            (uint16_t)AUDIO_SpeakerHandler->specific.data_size,
            AUDIO_SpeakerHandler->node.audio_description->
            resolution);
        }

#ifdef DEBUG_SPEAKER_NODE
        AUDIO_SpeakerDebugStats[AUDIO_SpeakerDebugStats_count].
          data = AUDIO_SpeakerHandler->specific.data;
        AUDIO_SpeakerDebugStats[AUDIO_SpeakerDebugStats_count].
          injection_size =
          AUDIO_SpeakerHandler->specific.data_size;
#endif /* DEBUG_SPEAKER_NODE */

        /* обновить указатель чтения */
        AUDIO_SpeakerHandler->buf->rd_ptr += read_length;
        if (AUDIO_SpeakerHandler->buf->rd_ptr >=
            AUDIO_SpeakerHandler->buf->size)
        {
          AUDIO_SpeakerHandler->buf->rd_ptr -=
            AUDIO_SpeakerHandler->buf->size;
        }

#ifdef DEBUG_SPEAKER_NODE
        AUDIO_SpeakerDebugStats[AUDIO_SpeakerDebugStats_count].
          read = AUDIO_SpeakerHandler->buf->rd_ptr;
#endif /* DEBUG_SPEAKER_NODE */
      }

#ifdef DEBUG_SPEAKER_NODE
      if (++AUDIO_SpeakerDebugStats_count ==
          SPEAKER_DEBUG_BUFFER_SIZE)
      {
        AUDIO_SpeakerDebugStats_count = 0;
      }
#endif /* DEBUG_SPEAKER_NODE */
    } /* AUDIO_NODE_STARTED */
  }
}
/* Приватные функции ---------------------------------------------------------*/
/**
  * @brief  AUDIO_SpeakerStart
  *         Запустить узел динамика
  * @param  buffer(IN):     буфер, который будет использоваться во время запуска узла
  * @param  node_handle(IN): дескриптор узла динамика должен быть инициализирован
  * @retval 0 в случае отсутствия ошибок
  */
// Функция AUDIO_SpeakerStart инициализирует узел динамика
static int8_t  AUDIO_SpeakerStart(AUDIO_CircularBuffer_t* buffer,  uint32_t node_handle)
{
  AUDIO_SpeakerNode_t* speaker;

  // Привязываем дескриптор узла динамика к переменной speaker
  speaker = (AUDIO_SpeakerNode_t*)node_handle;
  // Устанавливаем буфер для динамика
  speaker->buf = buffer;
  // Сбрасываем команду динамика
  speaker->specific.cmd = 0;
  // Устанавливаем состояние mute
  AUDIO_SpeakerMute( 0,  speaker->node.audio_description->audio_mute , node_handle);
  // Устанавливаем громкость динамика
  AUDIO_SpeakerSetVolume( 0,  speaker->node.audio_description->audio_volume_db_256 , node_handle);
  // Устанавливаем состояние узла динамика в "запущен"
  speaker->node.state = AUDIO_NODE_STARTED;
  return 0; // Возвращаем 0, если ошибок нет
}

 /**
  * @brief  AUDIO_SpeakerStop
  *         Остановить узел динамика. Динамик будет остановлен после
  *         завершения передачи текущего пакета.
  * @param  node_handle(IN): дескриптор узла динамика
  * @retval 0 в случае отсутствия ошибок
  */
static int8_t  AUDIO_SpeakerStop(uint32_t node_handle)
{
  AUDIO_SpeakerNode_t* speaker;

  speaker = (AUDIO_SpeakerNode_t*)node_handle;
  // Устанавливаем команду остановки динамика
  speaker->specific.cmd |= SPEAKER_CMD_STOP;

  return 0; // Возвращаем 0, если ошибок нет
}

 /**
  * @brief  AUDIO_SpeakerChangeFrequency
  *         изменить частоту, затем остановить узел динамика
  * @param  node_handle: дескриптор узла динамика должен быть запущен
  * @retval 0 в случае отсутствия ошибок
  */
int8_t  AUDIO_SpeakerChangeFrequency( uint32_t node_handle)
{
  AUDIO_SpeakerNode_t* speaker;

  speaker = (AUDIO_SpeakerNode_t*)node_handle;
  // Устанавливаем команду изменения частоты
  speaker->specific.cmd |= SPEAKER_CMD_CHANGE_FREQUENCE;
  return 0; // Возвращаем 0, если ошибок нет
}

 /**
  * @brief  AUDIO_SpeakerInitInjectionsParams
  *         Инициализация параметров инъекций динамика
  * @param  speaker(IN): дескриптор узла динамика должен быть запущен
  * @retval 0 в случае отсутствия ошибок
  */
static void  AUDIO_SpeakerInitInjectionsParams( AUDIO_SpeakerNode_t* speaker)
{
  // Устанавливаем длину пакета на основе аудио описания
  speaker->packet_length = AUDIO_MS_PACKET_SIZE_FROM_AUD_DESC(
    speaker->node.audio_description);

  // Вычисляем размер инъекции на основе частоты, количества каналов и разрешения
  speaker->specific.injection_size = AUDIO_MS_PACKET_SIZE( \
    speaker->node.audio_description->frequency, \
    speaker->node.audio_description->channels_count, \
    speaker->node.audio_description->resolution);

  // Инициализация параметров двойного буфера и смещения
  speaker->specific.double_buff = 0;
  speaker->specific.offset = 0;

  // Проверяем, если разрешение 24 бита
  if (speaker->node.audio_description->resolution == CONFIG_RES_BYTE_24)
  {
    // Устанавливаем размер инъекции для 24-битного разрешения
    speaker->specific.injection_size = AUDIO_MS_PACKET_SIZE( \
        speaker->node.audio_description->frequency, \
        speaker->node.audio_description->channels_count, \
        4);
    speaker->specific.double_buff = 1; // Включаем двойной буфер
    // Устанавливаем размер половины альтернативного буфера
    speaker->specific.alt_buf_half_size = speaker->specific.injection_size;
  }

#if USB_AUDIO_CONFIG_PLAY_USE_FREQ_44_1_K
  if((speaker->node.audio_description->frequency == USB_AUDIO_CONFIG_FREQ_44_1_K)
  || (speaker->node.audio_description->frequency == USB_AUDIO_CONFIG_FREQ_88_2_K)
  || (speaker->node.audio_description->frequency == USB_AUDIO_CONFIG_FREQ_176_4_K))
  {
    speaker->specific.double_buff = 1; // Включаем двойной буфер
    // Устанавливаем максимальную длину пакета для 44.1 кГц
    if (speaker->node.audio_description->frequency != USB_AUDIO_CONFIG_FREQ_176_4_K)
      speaker->packet_length_max_44_1 = speaker->packet_length + \
        AUDIO_SAMPLE_LENGTH(speaker->node.audio_description);
    else
      speaker->packet_length_max_44_1 = 712;

    // Устанавливаем размер половины альтернативного буфера
    if (speaker->node.audio_description->frequency !=
        USB_AUDIO_CONFIG_FREQ_176_4_K) {
      speaker->specific.alt_buf_half_size =
        speaker->specific.injection_size +
        (speaker->node.audio_description->resolution *
        speaker->node.audio_description->channels_count);
    } else {
      speaker->specific.alt_buf_half_size = 712;
    }

    // Если разрешение 24 бита, обновляем размер половины альтернативного буфера
    if (speaker->node.audio_description->resolution == CONFIG_RES_BYTE_24) {
      speaker->specific.alt_buf_half_size =
        speaker->specific.injection_size +
        (4 * speaker->node.audio_description->channels_count);
    }
  }
#endif /* USB_AUDIO_CONFIG_PLAY_USE_FREQ_44_1_K*/

  // Обнуляем альтернативный буфер
  memset(speaker->specific.alt_buffer, 0, speaker->specific.injection_size);
  speaker->specific.data = speaker->specific.alt_buffer; // Начинаем инъекцию данных
  speaker->specific.data_size = speaker->specific.injection_size; // Устанавливаем размер данных
}

 /**
  * @brief  AUDIO_SpeakerMute
  *         Устанавливает значение заглушки для динамика
  * @param  channel_number(IN): номер канала
  * @param  mute(IN): значение заглушки (0 : заглушить, 1 : включить)
  * @param  node_handle(IN): дескриптор узла динамика
  * @retval  : 0 в случае отсутствия ошибок
  */
static int8_t  AUDIO_SpeakerMute( uint16_t channel_number,  uint8_t mute , uint32_t node_handle)
{
  AudioOutMute(mute);
  return 0;
}

 /**
  * @brief  AUDIO_SpeakerSetVolume
  *         Устанавливает значение громкости для динамика
  * @param  channel_number(IN): номер канала
  * @param  volume_db_256(IN):  значение громкости в дБ
  * @param  node_handle(IN):    дескриптор узла динамика
  * @retval 0 в случае отсутствия ошибок
  */
static int8_t  AUDIO_SpeakerSetVolume( uint16_t channel_number,  int volume_db_256 ,  uint32_t node_handle)
{

  return 0;
}

/**
  * @brief  AUDIO_DoPadding_24_32
  *         дополнение 24-битного образца до 32-битного, добавляя нули.
  * @param  buff_src(IN):
  * @param  data_dest(OUT):
  * @param  size(IN):
  * @retval None
  */
 /**
  * @brief  AUDIO_DoPadding_24_32
  *         Функция для дополнения 24-битного образца до 32-битного, добавляя нули.
  * @param  buff_src(IN): указатель на источник данных (круговой буфер).
  * @param  data_dest(OUT): указатель на массив, куда будут записаны дополненные данные.
  * @param  size(IN): количество байт, которые нужно дополнить.
  * @retval None
  */
 static void AUDIO_DoPadding_24_32(AUDIO_CircularBuffer_t *buff_src,  uint8_t *data_dest ,  int size)
 {
   int k = 0, j = buff_src->rd_ptr; // Инициализация индексов для записи и чтения
   for(int i = 0; i < size; i += 3) // Проход по каждому 24-битному образцу
   {
     data_dest[k++] = 0; // Добавление нуля для дополнения до 32 бит
     for(int p = 0; p < 3; p++) // Копирование 3 байт из источника
     {
       if(j == buff_src->size) // Проверка на выход за пределы буфера
       {
         j = 0; // Сброс индекса, если достигнут конец буфера
       }
       data_dest[k++] = buff_src->data[j++]; // Копирование байта из буфера
     }
   }
 }

 /**
  * @brief  AUDIO_SpeakerStartReadCount
  *         Запускает счетчик того, сколько байт было прочитано из буфера (передано в SAI)
  * @param  node_handle: дескриптор узла микрофона, который должен быть запущен
  * @retval  : 0 в случае отсутствия ошибок
  */
static int8_t  AUDIO_SpeakerStartReadCount( uint32_t node_handle)
{
     AUDIO_SpeakerNode_t* speaker;

    speaker = (AUDIO_SpeakerNode_t*)node_handle;
    speaker->specific.dma_remaining = GetRemainingTxSize();//SAI_GetRemainingTxSize();//SAI_MASTER_DMA_STREAM->NDTR;//__HAL_DMA_GET_COUNTER(haudio_out_sai_1.hdmatx);
    return 0;
}


 /**
  * @brief  AUDIO_SpeakerGetLastReadCount
  *         возвращает количество байт, которые были прочитаны, и сбрасывает счетчик
  * @param  node_handle: дескриптор узла динамика, который должен быть запущен
  * @retval  :  количество прочитанных байт, 0 в случае ошибки
  */

static uint16_t  AUDIO_SpeakerGetLastReadCount( uint32_t node_handle)
{
  AUDIO_SpeakerNode_t* speaker;
  int cur_waiting_bytes, read_bytes, last_packet_size;

   speaker = (AUDIO_SpeakerNode_t*)node_handle;
   /* считываем оставшееся значение в dma буфере */
    cur_waiting_bytes = GetRemainingTxSize();//SAI_GetRemainingTxSize();//SAI_MASTER_DMA_STREAM->NDTR; //__HAL_DMA_GET_COUNTER(haudio_out_sai_1.hdmatx);
    last_packet_size = GetLastTxSize();//SAI_GetLastTransferSize();//haudio_out_sai_1.XferSize;
    read_bytes = (speaker->specific.dma_remaining >= cur_waiting_bytes) ?
                 (speaker->specific.dma_remaining - cur_waiting_bytes) :
                 (last_packet_size - cur_waiting_bytes) + speaker->specific.dma_remaining;
    if(read_bytes<(last_packet_size>>1))
    {
      read_bytes+=last_packet_size;
    }
   speaker->specific.dma_remaining = cur_waiting_bytes;

    return read_bytes;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

//-------------------------------------------------------------------
/**
  * @brief  AUDIO_SpeakerChangeResolution
  *         Функция для изменения разрешения динамика.
  * @param  node_handle: дескриптор узла динамика, который необходимо изменить
  * @retval None
  */
void AUDIO_SpeakerChangeResolution( uint32_t node_handle)
{
  AUDIO_SpeakerNode_t* speaker;

  speaker = (AUDIO_SpeakerNode_t*)node_handle;
  // Устанавливаем команду изменения разрешения
  speaker->specific.cmd |= SPEAKER_CMD_CHANGE_RESOLUTION;
}
