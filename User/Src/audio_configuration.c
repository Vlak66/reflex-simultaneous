/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2019, 2020
*
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/

#include "audio_configuration.h"
#include "usbd_audio.h"
#include "SAI.h"

// Переменные для конфигурации аудио
static uint8_t AudioConfiguration = 0; // Конфигурация аудио
static uint8_t SyncMode = 0; // Режим синхронизации
static uint8_t BclkFsRatioMode = BCLK_Fs_RES_DEPENDENT; // Соотношение BCLK к Fs
static uint8_t ChannelsPairs = 1; // Пары каналов

void SetAudioConfigDependedFuncs(AUDIO_SpeakerNode_t *speaker)
{
  // Установка функции воспроизведения
  speaker->SpeakerPlay = Play_SAIMaster;
  // Подготовка данных не требуется
  speaker->SpeakerPrepareData = 0;
}

//------------------------------------------------------------------------------
// Функция воспроизведения для мастера SAI
void Play_SAIMaster(uint16_t *Data, uint16_t Size, uint8_t ResByte)
{
  // выравниваем по границе слова, если необходимо
  if (ResByte == 3)
    ResByte = 4;

  uint16_t TxSize = Size / CONFIG_2_0_SAI_COUNT / ResByte; // Размер передачи

  SAI_MasterDMAPrepareTx(Data, TxSize); // Подготовка DMA для передачи
  SAI_MasterDMAEnable(); // Включение DMA
}

//------------------------------------------------------------------------------
// Изменение частоты аудио
void AudioChangeFrequency(uint32_t AudioFrequency)
{
  SAI_MasterDisable(); // Отключение мастера SAI

  // Если внутрення синхронизация
  if (SyncMode == MASTER_INT_SYNC)
  {
    // Настраиваем частоту синтезатора (тактовая)
    SAI_PLLSAIConfig(AudioFrequency); // Настройка PLL для SAI
    // Настраиваем частоту аудио (частота кадров)
    SAI_ChangeFrequency(AudioFrequency); // Изменение частоты SAI
  }

  // Выбор источника внешней синхронизации
  ExtSyncSelectSource(AudioFrequency);

  SAI_MasterEnable(); // Включение мастера SAI
}

//------------------------------------------------------------------------------
// Выбор источника внешней синхронизации
void ExtSyncSelectSource(uint32_t AudioFrequency)
{

  // Установка конфигурации GPIO для синхронизации внешнего аудио в зависимости от частоты

  switch(AudioFrequency)
  {
    case USB_AUDIO_CONFIG_FREQ_44_1_K:
      EXT_SYNC_SELECT_3_GPIO->ODR &= ~EXT_SYNC_SELECT_3_MASK;
      EXT_SYNC_SELECT_GPIO->ODR &= ~EXT_SYNC_SELECT_MASK;
    break;

    case USB_AUDIO_CONFIG_FREQ_48_K:
    default:
      EXT_SYNC_SELECT_3_GPIO->ODR &= ~EXT_SYNC_SELECT_3_MASK;
      EXT_SYNC_SELECT_GPIO->ODR |= EXT_SYNC_SELECT_1_MASK;
      EXT_SYNC_SELECT_GPIO->ODR &= ~EXT_SYNC_SELECT_2_MASK;
    break;

    case USB_AUDIO_CONFIG_FREQ_88_2_K:
      EXT_SYNC_SELECT_3_GPIO->ODR |= EXT_SYNC_SELECT_3_MASK;
      EXT_SYNC_SELECT_GPIO->ODR &= ~EXT_SYNC_SELECT_1_MASK;
      EXT_SYNC_SELECT_GPIO->ODR |= EXT_SYNC_SELECT_2_MASK;
    break;

    case USB_AUDIO_CONFIG_FREQ_96_K:
      EXT_SYNC_SELECT_3_GPIO->ODR &= ~EXT_SYNC_SELECT_3_MASK;
      EXT_SYNC_SELECT_GPIO->ODR &= ~EXT_SYNC_SELECT_1_MASK;
      EXT_SYNC_SELECT_GPIO->ODR |= EXT_SYNC_SELECT_2_MASK;
    break;

    case USB_AUDIO_CONFIG_FREQ_176_4_K:
      EXT_SYNC_SELECT_3_GPIO->ODR |= EXT_SYNC_SELECT_3_MASK;
      EXT_SYNC_SELECT_GPIO->ODR |= EXT_SYNC_SELECT_MASK;
    break;

    case USB_AUDIO_CONFIG_FREQ_192_K:
      EXT_SYNC_SELECT_3_GPIO->ODR &= ~EXT_SYNC_SELECT_3_MASK;
      EXT_SYNC_SELECT_GPIO->ODR |= EXT_SYNC_SELECT_MASK;
    break;
  }
}

//------------------------------------------------------------------------------
// Изменение разрешения аудио
void AudioChangeResolution(uint8_t AudioResolution)
{
  SAI_MasterDisable(); // Отключение мастера SAI

  // Инициализация I2S для мастера
  SAI_Init_I2S(SAI_MASTER, AudioResolution, BclkFsRatioMode);
  // Изменение размера данных для DMA
  SAI_MasterDMAChangeDataSize(AudioResolution);

  SAI_MasterEnable(); // Включение мастера SAI
}

//------------------------------------------------------------------------------
// Установка режима заглушения аудио
void AudioOutMute(uint8_t MuteFlag)
{
  SAI_MasterMute(MuteFlag); // Установка режима заглушения для мастера
}

//------------------------------------------------------------------------------
// Получение оставшегося размера передачи
uint16_t GetRemainingTxSize(void)
{
   // Получение оставшегося размера передачи
  uint16_t TxSize = SAI_GetRemainingTxSize();
  return TxSize; // Возврат оставшегося размера передачи
}

//------------------------------------------------------------------------------
// Получение последнего размера передачи
uint16_t GetLastTxSize(void)
{
   // Получение последнего размера передачи
  uint16_t TxSize = SAI_GetLastTransferSize();
  return TxSize; // Возврат последнего размера передачи
}

//------------------------------------------------------------------------------
// Инициализация аудиовыхода
void AudioOutInit(uint32_t AudioFrequency, uint8_t AudioResolution)
{
  if (SyncMode == MASTER_INT_SYNC)
    // Если внутрення синхронизация настраиваем синтезатор
    SAI_PLLSAIConfig(AudioFrequency); // Настройка PLL для SAI
  else if (SyncMode == MASTER_EXT_SYNC)
    // или выбираем источник внешней синхронизации
    SAI_ExternalSyncInit(); // Инициализация внешней синхронизации

  // Настраиваем порты для первого порта
  SAI_MasterGPIOInit(); // Инициализация GPIO для мастера
  SAI_MasterDMAInit(AudioResolution); // Инициализация DMA для мастера

  SAI_MasterInit_I2S(AudioResolution, BclkFsRatioMode);

  if (SyncMode == MASTER_INT_SYNC)
    // настриваем частоту
    SAI_ChangeFrequency(AudioFrequency); // Изменение частоты SAI
  else if (SyncMode == SLAVE_SYNC)
    // или внешнюю синхронизацию
    SAI_MasterSetSlaveSync(); // Установка синхронизации для мастера

  ExtSyncSelectSource(AudioFrequency); // Выбор источника внешней синхронизации
  SAI_MasterEnable(); // Включение мастера SAI
}

//------------------------------------------------------------------------------
// Инициализация описания воспроизведения
void PlayDescriptionInit(AUDIO_Description_t *Description)
{
  // Установка количества каналов
  Description->channels_count = CONFIG_2_0_STEREO_CHANNEL_COUNT;
  // Установка карты каналов
  Description->channels_map = CONFIG_2_0_STEREO_CHANNEL_MAP;

  // Установка разрешения по умолчанию
  Description->resolution = CONFIG_RES_BYTE_24;
  // Установка частоты по умолчанию
  Description->frequency = CONFIG_2_0_FREQUENCY_DEFAULT;
  // Установка типа аудио
  Description->audio_type = USBD_AUDIO_FORMAT_TYPE_PCM;
  // Установка громкости по умолчанию
  Description->audio_volume_db_256 = VOLUME_SPEAKER_DEFAULT_DB_256;
  // Установка флага заглушения
  Description->audio_mute = 0;
}
//------------------------------------------------------------------------------
// Получение текущей конфигурации аудио
uint8_t GetAudioConfiguration(void)
{
  return AudioConfiguration; // Возврат текущей конфигурации аудио
}

//------------------------------------------------------------------------------
// Генерация серийного номера
// В STM32F446 DEVICE_ID представляет собой уникальный идентификатор устройства,
// который хранится в памяти. Он состоит из трех 32-битных регистров:
// DEVICE_ID1, DEVICE_ID2 и DEVICE_ID3. Эти регистры содержат информацию о
// производителе, типе устройства и других характеристиках. Генерация серийного
// номера может использовать эти значения для создания уникального идентификатора
// для каждого устройства.
void MakeSerialNumber(uint32_t *Buffer)
{
  // Генерация первого слова серийного номера
  Buffer[0] = (*(uint32_t*)DEVICE_ID1) + (*(uint32_t*)DEVICE_ID3);
  // Генерация второго слова серийного номера
  Buffer[1] = (((*(uint32_t*)DEVICE_ID2) >> 16) + AudioConfiguration) << 16;
}

//------------------------------------------------------------------------------
// Инициализация конфигурации аудио
void AudioConfig_Init(void)
{
  // Установка конфигурации для 2.0 стерео
  AudioConfiguration = AUDIO_CONFIG_2_0_STEREO;
  ChannelsPairs = 1; // Установка количества пар каналов

  uint8_t SAISyncMode = SYNC_MODE_MASK; // Инициализация режима синхронизации
  if ((CONFIG_GPIO->IDR & (1 << SYNC_MODE_PIN_1)) != (1 << SYNC_MODE_PIN_1))
  {
    SAISyncMode &= ~SYNC_MODE_1_MASK; // Корректировка режима синхронизации
  }

  if ((CONFIG_GPIO->IDR & (1 << SYNC_MODE_PIN_2)) != (1 << SYNC_MODE_PIN_2))
    SAISyncMode &= ~SYNC_MODE_2_MASK; // Корректировка режима синхронизации

  switch (SAISyncMode)
  {
    case SYNC_MODE_MASK:
    default:
      SyncMode = MASTER_INT_SYNC; // Установка режима внутренней синхронизации
    break;

    case SYNC_MODE_2_MASK:
      SyncMode = MASTER_EXT_SYNC; // Установка режима внешней синхронизации
    break;

    case SYNC_MODE_1_MASK:
      SyncMode = SLAVE_SYNC; // Установка режима синхронизации слейва
    break;
  }

  if ((BCLK_Fs_RATIO_GPIO->IDR & (1 << BCLK_Fs_RATIO_PIN)) != (1 << BCLK_Fs_RATIO_PIN))
    // Установка фиксированного соотношения BCLK к Fs
    BclkFsRatioMode = BCLK_Fs_FIXED;
}

//------------------------------------------------------------------------------
// Инициализация выходного тактового сигнала
void OUTClk_Init(void)
{
   // Частота аудио по умолчанию 48 кГц
  uint32_t AudioFreq = CONFIG_2_0_FREQUENCY_DEFAULT;
  // Разрешение аудио по умолчанию
  uint8_t AudioRes = CONFIG_RES_BIT_24;

  // Для всех конфигураций синхронизации
  // кроме синхронизации от внешнего источника
  if (SyncMode != MASTER_EXT_SYNC)
  {
    switch(
      (OUT_CLK_CONFIG_1_GPIO->IDR & (1 << OUT_CLK_CONFIG_1_PIN)) |
      (OUT_CLK_CONFIG_2_GPIO->IDR & (1 << OUT_CLK_CONFIG_2_PIN))
    )
    {
      // PIN3 + PIN1
      case OUT_CLK_CONFIG_1:
        AudioFreq = USB_AUDIO_CONFIG_FREQ_44_1_K; // Установка частоты 44.1 кГц
      break;

      // PIN3
      case OUT_CLK_CONFIG_2:
      default:
        AudioFreq = USB_AUDIO_CONFIG_FREQ_48_K; // Установка частоты 48 кГц
      break;

      // PIN1
      case OUT_CLK_CONFIG_3:
        AudioFreq = USB_AUDIO_CONFIG_FREQ_96_K; // Установка частоты 96 кГц
      break;

      // = 0
      case OUT_CLK_CONFIG_4:
        AudioFreq = USB_AUDIO_CONFIG_FREQ_192_K; // Установка частоты 192 кГц
      break;
    }
  }

  AudioOutInit(AudioFreq, AudioRes); // Инициализация аудиовыхода
}

//------------------------------------------------------------------------------
// Отключение внешнего питания
void ExtPowerDisable(void)
{
  POWER_DISABLE_GPIO->ODR |= 1 << POWER_DISABLE_PIN; // Отключение внешнего питания
}

//------------------------------------------------------------------------------
// Инициализация GPIO
void ConfigGPIOs_Init(void)
{
  // Настройка подтягивающего резистора для CONFIG входов
  CONFIG_GPIO->PUPDR |=
    (1 << (2 * CONFIG_1_PIN)) |  // Подтяжка для CONFIG_1_PIN
    (1 << (2 * CONFIG_2_PIN)) |  // Подтяжка для CONFIG_2_PIN
    (1 << (2 * CONFIG_3_PIN)) |  // Подтяжка для CONFIG_3_PIN
    (1 << (2 * CONFIG_4_PIN)) |  // Подтяжка для CONFIG_4_PIN (TDM_MODE_PIN)
    (1 << (2 * CONFIG_5_PIN)) |  // Подтяжка для CONFIG_5_PIN (SYNC_MODE_PIN_1)
    (1 << (2 * CONFIG_6_PIN));   // Подтяжка для CONFIG_6_PIN (SYNC_MODE_PIN_2)


  // ожидание установки
  while (
    // Проверка CONFIG_1_PIN
    ((CONFIG_GPIO->PUPDR & (3 << (2 * CONFIG_1_PIN))) != (1 << (2 * CONFIG_1_PIN))) ||
    // Проверка CONFIG_2_PIN
    ((CONFIG_GPIO->PUPDR & (3 << (2 * CONFIG_2_PIN))) != (1 << (2 * CONFIG_2_PIN))) ||
    // Проверка CONFIG_3_PIN
    ((CONFIG_GPIO->PUPDR & (3 << (2 * CONFIG_3_PIN))) != (1 << (2 * CONFIG_3_PIN))) ||
    // Проверка CONFIG_4_PIN
    ((CONFIG_GPIO->PUPDR & (3 << (2 * CONFIG_4_PIN))) != (1 << (2 * CONFIG_4_PIN))) ||
    // Проверка CONFIG_5_PIN
    ((CONFIG_GPIO->PUPDR & (3 << (2 * CONFIG_4_PIN))) != (1 << (2 * CONFIG_4_PIN))) ||
    // Проверка CONFIG_6_PIN
    ((CONFIG_GPIO->PUPDR & (3 << (2 * CONFIG_6_PIN))) != (1 << (2 * CONFIG_6_PIN)))
  );

  // Настройка подтягивающих резисторов для OUT_CLK_CONFIG_1 и OUT_CLK_CONFIG_2
  OUT_CLK_CONFIG_1_GPIO->PUPDR |= (1 << (2 * OUT_CLK_CONFIG_1_PIN));
  OUT_CLK_CONFIG_2_GPIO->PUPDR |= (1 << (2 * OUT_CLK_CONFIG_2_PIN));

  // ожидание установки
  while (
    // Проверка CONFIG_1_PIN
    ((OUT_CLK_CONFIG_1_GPIO->PUPDR & (3 << (2 * OUT_CLK_CONFIG_1_PIN))) != (1 << (2 * OUT_CLK_CONFIG_1_PIN))) ||
    // Проверка CONFIG_2_PIN
    ((OUT_CLK_CONFIG_2_GPIO->PUPDR & (3 << (2 * OUT_CLK_CONFIG_2_PIN))) != (1 << (2 * OUT_CLK_CONFIG_2_PIN)))
  );


  // Установка режима для отключения питания
  // Настрока вывода как цифровой выход
  POWER_DISABLE_GPIO->MODER |= 1 << (2 * POWER_DISABLE_PIN);

  // Сброс режима для EXT_SYNC_SELECT
  EXT_SYNC_SELECT_3_GPIO->MODER &= ~(3 << (2 * EXT_SYNC_SELECT_3_PIN));
  EXT_SYNC_SELECT_GPIO->MODER &= ~((3 << (2 * EXT_SYNC_SELECT_2_PIN)) | (3 << (2 * EXT_SYNC_SELECT_1_PIN)));
  // Сброс подтягивающего резистора для EXT_SYNC_SELECT
  EXT_SYNC_SELECT_3_GPIO->PUPDR &= ~(3 << (2 * EXT_SYNC_SELECT_3_PIN));
  EXT_SYNC_SELECT_GPIO->PUPDR &= ~((3 << (2 * EXT_SYNC_SELECT_2_PIN)) | (3 << (2 * EXT_SYNC_SELECT_1_PIN)));

  // Установка режима для EXT_SYNC_SELECT
  EXT_SYNC_SELECT_3_GPIO->MODER |= (1 << (2 * EXT_SYNC_SELECT_3_PIN));
  EXT_SYNC_SELECT_GPIO->MODER |= (1 << (2 * EXT_SYNC_SELECT_2_PIN)) | (1 << (2 * EXT_SYNC_SELECT_1_PIN));

   // Настройка подтягивающего резистора для BCLK_Fs_RATIO
  BCLK_Fs_RATIO_GPIO->PUPDR |= 1 << (2 * BCLK_Fs_RATIO_PIN);

}

