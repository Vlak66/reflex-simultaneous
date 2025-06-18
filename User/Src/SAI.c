/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2019, 2020
*
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/



#include "SAI.h"
#include "stm32f4xx_hal_cortex.h"
#include "usb_audio_constants.h"

// Статическая переменная для хранения размера последней передачи
static uint16_t LastTransferSize = 0;

void SAI_PLLSAIConfig(uint32_t AudioFrequency)
{
  // Отключаем PLLSAI перед настройкой
  RCC->CR &= ~RCC_CR_PLLSAION; // Сброс бита PLLSAION (выключение PLLSAI)
  // Ожидаем, пока PLLSAI не включится (бит PLLSAIRDY сброшен)
  while((RCC->CR & RCC_CR_PLLSAIRDY) == RCC_CR_PLLSAIRDY);

  // Сбрасываем делитель PLLSAIDIVQ (используется для тактирования SAI)
  RCC->DCKCFGR &= ~RCC_DCKCFGR_PLLSAIDIVQ;

  // Настройка параметров PLLSAI в зависимости от требуемой аудиочастоты
  switch(AudioFrequency)
  {
    // По умолчанию: 48/96/192 кГц
    default:
      // PLLSAICFGR: настройка коэффициентов PLLSAI
      RCC->PLLSAICFGR =
        // PLLSAIQ[3:0] = 0b111 (делитель Q = 8) - выходная частота PLLSAI = VCO / Q
        RCC_PLLSAICFGR_PLLSAIQ_0 | RCC_PLLSAICFGR_PLLSAIQ_1 | RCC_PLLSAICFGR_PLLSAIQ_2 |
        // PLLSAIN = 344 (0x158) - множитель VCO (VCO = PLLVCO * N)
        (0x158 << RCC_PLLSAICFGR_PLLSAIN_Pos) |
        // PLLSAIM = 8 (0b1000) - делитель входной частоты (PLLVCO = HSE/M)
        RCC_PLLSAICFGR_PLLSAIM_3;
      break;

    // 44.1 кГц
    case USB_AUDIO_CONFIG_FREQ_44_1_K:
      RCC->PLLSAICFGR =
        // PLLSAIQ[3:0] = 0b0010 (Q = 2)
        RCC_PLLSAICFGR_PLLSAIQ_1 |
        // PLLSAIN = 429 (0x1AD) - VCO = PLLVCO * 429
        (0x1AD << RCC_PLLSAICFGR_PLLSAIN_Pos) |
        // PLLSAIM = 8 (0b1000)
        RCC_PLLSAICFGR_PLLSAIM_3;
      // Установка PLLSAIDIVQ = 18 (0x12) - дополнительный делитель для SAI (SAI_CLK = PLLSAI_Q / DIVQ)
      // Это делит выход PLLSAI_Q на 19  (18 + 1), чтобы получить частоту, совместимую с аудиостандартом 44.1 кГц.
      RCC->DCKCFGR |= 0x12 << RCC_DCKCFGR_PLLSAIDIVQ_Pos;
      break;

    // 88.2 кГц
    case USB_AUDIO_CONFIG_FREQ_88_2_K:
      RCC->PLLSAICFGR =
        // PLLSAIQ[3:0] = 0b1100 (Q = 12)
        RCC_PLLSAICFGR_PLLSAIQ_2 | RCC_PLLSAICFGR_PLLSAIQ_3 |
        // PLLSAIN = 237 (0xED) - VCO = PLLVCO * 237
        (0xED << RCC_PLLSAICFGR_PLLSAIN_Pos) |
        // PLLSAIM = 7 (0b0111)
        RCC_PLLSAICFGR_PLLSAIM_0 | RCC_PLLSAICFGR_PLLSAIM_1 | RCC_PLLSAICFGR_PLLSAIM_2;
      break;

    // 176.4 кГц
    case USB_AUDIO_CONFIG_FREQ_176_4_K:
      RCC->PLLSAICFGR =
        // PLLSAIQ[3:0] = 0b0110 (Q = 6)
        RCC_PLLSAICFGR_PLLSAIQ_1 | RCC_PLLSAICFGR_PLLSAIQ_2 |
        // PLLSAIN = 237 (0xED)
        (0xED << RCC_PLLSAICFGR_PLLSAIN_Pos) |
        // PLLSAIM = 7 (0b0111)
        RCC_PLLSAICFGR_PLLSAIM_0 | RCC_PLLSAICFGR_PLLSAIM_1 | RCC_PLLSAICFGR_PLLSAIM_2;
      break;
  }

  // Включаем PLLSAI
  RCC->CR |= RCC_CR_PLLSAION;
  // Ожидаем готовности PLLSAI
  while((RCC->CR & RCC_CR_PLLSAIRDY) != RCC_CR_PLLSAIRDY);
}

//------------------------------------------------------------------------------
void SAI_ChangeFrequency(uint32_t AudioFrequency)
{
  // Сбрасываем текущее значение делителя MCKDIV в регистре CR1 SAI
  // MCKDIV[3:0]  (биты 20–23): Делитель тактовой частоты SAI для вывода MCLK.
  SAI_MASTER->CR1 &= ~SAI_xCR1_MCKDIV;

  uint32_t NewMCKDIV = 0;

  // Проверяем, что частота не относится к семейству 44.1 кГц
  if ((AudioFrequency != USB_AUDIO_CONFIG_FREQ_44_1_K)
   && (AudioFrequency != USB_AUDIO_CONFIG_FREQ_88_2_K)
   && (AudioFrequency != USB_AUDIO_CONFIG_FREQ_176_4_K))
  {
    // Вычисляем делитель MCKDIV для частот 48/96/192 кГц
    // Формула: NewMCKDIV = (2 - (AudioFrequency / 48000 / 2)) << смещение MCKDIV
    NewMCKDIV = (2 - (AudioFrequency / 48000 / 2)) << SAI_xCR1_MCKDIV_Pos;
  }

  // Устанавливаем новый делитель MCKDIV
  SAI_MASTER->CR1 |= NewMCKDIV;
}

//------------------------------------------------------------------------------
void SAI_ExternalSyncInit(void)
{
  // 1. Настройка GPIO пина для альтернативной функции (внешняя синхронизация)
  EXT_SYNC_GPIO->MODER |= (2 << (2 * EXT_SYNC_PIN));
  // MODER: установка режима пина в Alternate Function (AF) (значение 2 = 10 в битах MODE)
  // Для пина EXT_SYNC_PIN: 2 бита на пин, смещение = 2 * номер пина

  // 2. Назначение альтернативной функции для пина
  EXT_SYNC_GPIO->AFR[1] |= (EXT_SYNC_AF << (4 * (EXT_SYNC_PIN - 8)));
  // AFR[1]: регистр альтернативных функций для пинов 8-15
  // Для пина >=8: вычисляем позицию (EXT_SYNC_PIN - 8), умножаем на 4 (4 бита на AF)
  // EXT_SYNC_AF: номер альтернативной функции (AF6 для SAI_SYNC)

  // 3. Выбор источника тактирования SAI1
  RCC->DCKCFGR |= RCC_DCKCFGR_SAI1SRC_0 | RCC_DCKCFGR_SAI1SRC_1;
  // DCKCFGR: установка битов SAI1SRC[1:0] = 0b11
  // Значение 0b11: источником тактирования SAI1 является внешний сигнал с GPIO
}
//------------------------------------------------------------------------------
// Возвращает размер последней передачи данных через DMA
uint16_t SAI_GetLastTransferSize(void)
{
  return LastTransferSize;
}
//------------------------------------------------------------------------------
// Возвращает количество данных, оставшихся в текущей DMA-передаче
uint16_t SAI_GetRemainingTxSize(void)
{
  return SAI_MASTER_DMA_STREAM->NDTR; // NDTR: Number of Data to Transfer
}
//------------------------------------------------------------------------------
// Подготовка DMA для передачи данных через SAI
void SAI_MasterDMAPrepareTx(uint16_t *Data, uint16_t Size)
{
  SAI_DMAPrepareTx(SAI_MASTER_DMA_STREAM, Data, Size); // Настройка DMA
  LastTransferSize = Size; // Сохраняем размер текущей передачи
}
//------------------------------------------------------------------------------
// Общая функция настройки DMA для передачи данных через SAI
void SAI_DMAPrepareTx(DMA_Stream_TypeDef *DMAStream, uint16_t *Data, uint16_t Size)
{
  // Установка количества данных для передачи (NDTR = Number of Data to Transfer)
  DMAStream->NDTR = Size;
  // Настройка адреса памяти (M0AR = Memory Address Register)
  DMAStream->M0AR = (uint32_t)(&Data[0]);
  // Включение DMA (бит EN в регистре CR)
  DMAStream->CR |= DMA_SxCR_EN;
}
//------------------------------------------------------------------------------
// Включение мастер-блока SAI
void SAI_MasterEnable()
{
  SAI_Enable(SAI_MASTER); // Вызов общей функции включения
}
//------------------------------------------------------------------------------
// Общая функция включения блока SAI
void SAI_Enable(SAI_Block_TypeDef *SAIBlock)
{
  // Установка бита SAIEN (SAI Enable) в регистре CR1
  SAIBlock->CR1 |= SAI_xCR1_SAIEN;
  // Ожидание подтверждения включения (проверка бита SAIEN)
  while((SAIBlock->CR1 & SAI_xCR1_SAIEN) != SAI_xCR1_SAIEN);
}
//------------------------------------------------------------------------------
// Включение/выключение режима MUTE для мастер-блока SAI
// MuteFlag: 1 - включить MUTE, 0 - выключить
void SAI_MasterMute(uint8_t MuteFlag)
{
  SAI_Mute(SAI_MASTER, MuteFlag); // Вызов общей функции для мастера
}

//------------------------------------------------------------------------------
// Общая функция управления MUTE для указанного блока SAI
// 1 - mute, 0 - unmute
void SAI_Mute(SAI_Block_TypeDef *SAIBlock, uint8_t MuteFlag)
{
  if (MuteFlag)
    SAIBlock->CR2 |= SAI_xCR2_MUTE;  // Установка бита MUTE в регистре CR2
  else
    SAIBlock->CR2 &= ~SAI_xCR2_MUTE; // Сброс бита MUTE
}
//------------------------------------------------------------------------------
// Отключение мастер-блока SAI
void SAI_MasterDisable()
{
  SAI_Disable(SAI_MASTER); // Вызов общей функции отключения для мастера
}

//------------------------------------------------------------------------------
// Общая функция отключения блока SAI
void SAI_Disable(SAI_Block_TypeDef *SAIBlock)
{
  // Сброс бита SAIEN (SAI Enable) в регистре CR1
  SAIBlock->CR1 &= ~SAI_xCR1_SAIEN;

  // Ожидание полного отключения SAI (проверка бита SAIEN)
  while((SAIBlock->CR1 & SAI_xCR1_SAIEN) == SAI_xCR1_SAIEN);
}
//------------------------------------------------------------------------------
// Инициализация мастер-блока SAI в режиме I2S
void SAI_MasterInit_I2S(uint8_t AudioResolution, uint8_t BCLKMode)
{
  // Включение тактирования SAI1
  RCC->APB2ENR |= RCC_APB2ENR_SAI1EN;
  // Ожидание подтверждения включения тактирования
  while((RCC->APB2ENR & RCC_APB2ENR_SAI1EN) != RCC_APB2ENR_SAI1EN);

  // Отключение SAI перед настройкой
  SAI_Disable(SAI_MASTER);
  // Настройка параметров I2S
  SAI_Init_I2S(SAI_MASTER, AudioResolution, BCLKMode);
}

//------------------------------------------------------------------------------
// Настройка мастер-блока SAI для синхронизации с ведомым устройством
void SAI_MasterSetSlaveSync(void)
{
  // Установка бита MODE_1 в регистре CR1 мастера
  // MODE[1:0] = 0b10: Мастер работает в режиме синхронизации с ведомым
  SAI_MASTER->CR1 |= SAI_xCR1_MODE_1;
}

//------------------------------------------------------------------------------
void SAI_Init_I2S(SAI_Block_TypeDef *SAIBlock, uint8_t AudioResolution, uint8_t BCLKMode)
{
  // 1. Настройка FIFO и кадровой синхронизации
  SAIBlock->CR2 = SAI_xCR2_FTH_0;  // Установка порога FIFO (FTH = 1 → прерывание при 1 слове в FIFO)

  SAIBlock->FRCR = SAI_xFRCR_FSOFF | SAI_xFRCR_FSDEF;
  // FSOFF: FS сдвигается на 1 такт BCLK (стандарт I2S)
  // FSDEF: FS активен на первом такте кадра

  // 2. Сброс битов разрешения данных (DS) перед настройкой
  SAIBlock->CR1 &= ~(SAI_xCR1_DS_2 | SAI_xCR1_DS_1 | SAI_xCR1_DS_0);

  // 3. Конфигурация в зависимости от разрешения и режима BCLK
  if ( AudioResolution == 16 )
  {

    SAIBlock->CR1 |= SAI_xCR1_DS_2; // DS = 0b100 → 16 бит данных

    if(BCLKMode == BCLK_Fs_RES_DEPENDENT){
      // Для 16 бит и режима "BCLK зависит от Fs и разрешения"
      SAIBlock->FRCR |= (0x0F << SAI_xFRCR_FSALL_Pos) | (0x1F << SAI_xFRCR_FRL_Pos);
      // FRL = 31 (0x1F) → 32 такта на кадр (I2S стандарт для 16 бит)
      // FSALL = 15 (0x0F) → 16 тактов активного FS (половина кадра)
      SAIBlock->SLOTR =
        (((1 << 0) | (1 << 1)) << SAI_xSLOTR_SLOTEN_Pos) | // Включены слоты 0 и 1 (стерео)
        (0x01 << SAI_xSLOTR_NBSLOT_Pos) |                  // 2 слота (0x01 = 1, но счет с нуля → 2 слота)
        SAI_xSLOTR_SLOTSZ_0;                               // Размер слота = 16 бит

    }else{
      SAIBlock->FRCR |= (0x1F << SAI_xFRCR_FSALL_Pos) | (0x3F << SAI_xFRCR_FRL_Pos);
      SAIBlock->SLOTR = (((1 << 0) | (1 << 2)) << SAI_xSLOTR_SLOTEN_Pos) | (0x03 << SAI_xSLOTR_NBSLOT_Pos) | SAI_xSLOTR_SLOTSZ_0;

    }

  }
  else
  {
    // Для 24/32 бит или других режимов BCLK
    SAIBlock->FRCR |= (0x1F << SAI_xFRCR_FSALL_Pos) | (0x3F << SAI_xFRCR_FRL_Pos);
    // FRL = 63 (0x3F) → 64 такта на кадр (для 32-битных данных)
    // FSALL = 31 (0x1F) → 32 такта активного FS

    SAIBlock->CR1 |= SAI_xCR1_DS_2 | SAI_xCR1_DS_1 | SAI_xCR1_DS_0; // DS = 0b111 → 32 бита данных

    SAIBlock->SLOTR =
      (((1 << 0) | (1 << 1)) << SAI_xSLOTR_SLOTEN_Pos) | // Включены слоты 0 и 1
      (0x01 << SAI_xSLOTR_NBSLOT_Pos) |                  // 2 слота
      SAI_xSLOTR_SLOTSZ_1;                               // Размер слота = 32 бита
  }

  // 4. Включение выходного драйвера (OUTDRIV)
  SAIBlock->CR1 |= SAI_xCR1_OUTDRIV;
}
//------------------------------------------------------------------------------
// Включение DMA для мастер-блока SAI
void SAI_MasterDMAEnable(void)
{
  SAI_DMAEnable(SAI_MASTER); // Вызов общей функции для мастера
}

//------------------------------------------------------------------------------
// Общая функция включения DMA для указанного блока SAI
void SAI_DMAEnable(SAI_Block_TypeDef *SAIBlock)
{
  // Установка бита DMAEN в регистре CR1
  SAIBlock->CR1 |= SAI_xCR1_DMAEN;
}
//------------------------------------------------------------------------------
// Инициализация DMA для мастер-блока SAI
void SAI_MasterDMAInit(uint8_t AudioResolution)
{
  // Включение тактирования DMA2 (мастер использует DMA2)
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
  // Ожидание подтверждения включения тактирования
  while((RCC->AHB1ENR & RCC_AHB1ENR_DMA2EN) != RCC_AHB1ENR_DMA2EN);

  // Общая настройка DMA
  SAI_DMAInit(SAI_MASTER_DMA_STREAM, AudioResolution);

  // Включение прерывания по завершению передачи (TCIE)
  SAI_MASTER_DMA_STREAM->CR |= DMA_SxCR_TCIE;
  // Установка физического адреса регистра данных SAI (DR)
  SAI_MASTER_DMA_STREAM->PAR = (uint32_t)(&(SAI_MASTER->DR));

  // Настройка NVIC для обработки прерываний DMA
  HAL_NVIC_SetPriority(SAI_MASTER_DMA_IRQ, SAI_IRQ_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(SAI_MASTER_DMA_IRQ);
}

//------------------------------------------------------------------------------
// Общая функция инициализации DMA
void SAI_DMAInit(DMA_Stream_TypeDef *DMAStream, uint8_t AudioResolution)
{
  // Настройка основных параметров DMA:
  DMAStream->CR =
    DMA_SxCR_PL_1 |   // Приоритет: средний (PL = 0b10)
    DMA_SxCR_MINC |   // Инкремент адреса памяти (пакетная передача)
    DMA_SxCR_DIR_0;   // Направление: память → периферия (DIR = 0b01)

  // Настройка размера данных в зависимости от разрешения:
  if (AudioResolution == 16)
  {
    // 16 бит: размер данных = полуслово (16 бит)
    DMAStream->CR |= DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0;
  }
  else
  {
    // 32 бита: размер данных = слово (32 бита)
    DMAStream->CR |= DMA_SxCR_MSIZE_1 | DMA_SxCR_PSIZE_1;
  }

  // Настройка FIFO:
  DMAStream->FCR =
    DMA_SxFCR_DMDIS | // Включение прямого доступа к памяти (FIFO отключен)
    DMA_SxFCR_FTH;     // Порог FIFO (по умолчанию 1/4)
}
//------------------------------------------------------------------------------
// Изменение размера данных для DMA мастер-блока SAI
void SAI_MasterDMAChangeDataSize(uint8_t Resolution)
{
  // Вызов общей функции для мастера
  SAI_DMAChangeDataSize(SAI_MASTER_DMA_STREAM, Resolution);
}

//------------------------------------------------------------------------------
// Общая функция изменения размера данных DMA
void SAI_DMAChangeDataSize(DMA_Stream_TypeDef *DMAStream, uint8_t Resolution)
{
  // Функция вызывается в прерывании по завершению передачи (TC),
  // когда DMA неактивен. Это позволяет безопасно изменять регистр CR.

  // Сброс битов MSIZE (Memory Size) и PSIZE (Peripheral Size)
  DMAStream->CR &= ~(DMA_SxCR_MSIZE_Msk | DMA_SxCR_PSIZE_Msk);

  if (Resolution == 16)
  {
    // Установка размера данных: 16 бит (полуслово)
    DMAStream->CR |= DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0;
  }
  else
  {
    // Установка размера данных: 32 бита (слово)
    DMAStream->CR |= DMA_SxCR_MSIZE_1 | DMA_SxCR_PSIZE_1;
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Инициализация GPIO для мастер-блока SAI
void SAI_MasterGPIOInit(void)
{
  // **** Настройка MCLK (Master Clock) ****
  // Режим порта = Alternate Function
  SAI_MASTER_MCLK_GPIO->MODER |= (2 << (2 * SAI_MASTER_MCK_PIN));
  // Настройка альтернативной функции (AF)
  SAI_MASTER_MCLK_GPIO->AFR[0] |= (SAI_MASTER_MCLK_SCK_SD_FS_AF << (4 * SAI_MASTER_MCK_PIN));
  // Максимальная скорость (High Speed)
  SAI_MASTER_MCLK_GPIO->OSPEEDR |= (3 << (2 * SAI_MASTER_MCK_PIN));
  // ***************************************

  // **** Настройка SCK (Serial Clock) и FS (Frame Sync) ****
  // Режим порта = Alternate Function
  SAI_MASTER_SCK_FS_GPIO->MODER |=
    (2 << (2 * SAI_MASTER_SCK_PIN)) | // SCK
    (2 << (2 * SAI_MASTER_FS_PIN));   // FS

  // Настройка альтернативной функции (AF)
  // Для пинов >=8 используется AFR[1] (4 бита на пин)
  SAI_MASTER_SCK_FS_GPIO->AFR[1] |=
    (SAI_MASTER_MCLK_SCK_SD_FS_AF << (4 * (SAI_MASTER_SCK_PIN - 8))) | // SCK
    (SAI_MASTER_MCLK_SCK_SD_FS_AF << (4 * (SAI_MASTER_FS_PIN - 8)));   // FS

  // Максимальная скорость (High Speed)
  SAI_MASTER_SCK_FS_GPIO->OSPEEDR |=
    (3 << (2 * SAI_MASTER_SCK_PIN)) | // SCK
    (3 << (2 * SAI_MASTER_FS_PIN));   // FS
  // ***************************************

  // **** Настройка SD (Serial Data) ****
  // Режим порта = Alternate Function
  SAI_MASTER_SD_GPIO->MODER |= (2 << (2 * SAI_MASTER_SD_PIN));
  // Настройка альтернативной функции (AF)
  SAI_MASTER_SD_GPIO->AFR[1] |= (SAI_MASTER_MCLK_SCK_SD_FS_AF << (4 * (SAI_MASTER_SD_PIN - 8)));
  // Максимальная скорость (High Speed)
  SAI_MASTER_SD_GPIO->OSPEEDR |= (3 << (2 * SAI_MASTER_SD_PIN));
  // ***************************************
}

