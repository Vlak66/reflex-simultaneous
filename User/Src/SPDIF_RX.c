//***************************************************************************
// SPDIF_RX.c
//
// Имплементация приемника S/PDIF для микроконтроллера STM32
//***************************************************************************

#include "SPDIF_RX.h"
#include "SAI.h"
#include "usb_audio_constants.h"
#include "audio_configuration.h"

//****************************************************************************
// Локальные переменные
//****************************************************************************
static DMA_HandleTypeDef hdma_spdif_rx_dt; // Дескриптор DMA для передачи данных S/PDIF
static TIM_HandleTypeDef htim7;           // Дескриптор таймера TIM7 (используется для периодических операций)
static SPDIFRX_HandleTypeDef hspdif1;     // Дескриптор модуля S/PDIF RX
static SPDIF_RX mSPDIFRX;                 // Структура с данными модуля S/PDIF RX
static DeviceMode mode = USB_MODE;        // Режим работы устройства
static BufferState state = EmptyBuffer;   // Состояние буфера приема SPDIF сигнала

// Внутренние операции (обычно не вызываются напрямую)
static void SPDIF_RX_Init(uint32_t Freq_SPDiff_Clk);       // Инициализация модуля S/PDIF RX
static void SPDIF_RX_CalcSampleRate();                     // Вычисление частоты дискретизации входного потока
static void PeriodElapsedCallback(TIM_HandleTypeDef* phTIM); // Обработчик истечения периода таймера
static void OnReceiveComplete(SPDIFRX_HandleTypeDef *hspdif); // Обработчик завершения приема данных
static void OnReceiveHalfComplete(SPDIFRX_HandleTypeDef *hspdif); // Обработчик половины приема данных
static void DMA_Init(void);                                // Инициализация DMA
static void TIM7_Init(void);                               // Инициализация таймера TIM7
static void SPDIFRX_Init(void);                            // Инициализация модуля S/PDIF RX
static void CheckAndResetFlags(SPDIFRX_HandleTypeDef *hspdif); // Проверка статуса и сброс

static void LedInit(void);

// Внешний обработчик ошибок
extern void Error_Handler(void);

//****************************************************************************
// Определения функций
//****************************************************************************

/**
 * @brief  Инициализация модуля S/PDIF RX
 * @param  Freq_SPDiff_Clk: Частота системного тактового сигнала S/PDIF
 */
void SPDIF_RX_Init(uint32_t Freq_SPDiff_Clk) {
    // Инициализация полей структуры
    mSPDIFRX.phSPDIFRX = &hspdif1;
    mSPDIFRX.phTIM = &htim7;
    mSPDIFRX.Freq_SPDiff_Clk = Freq_SPDiff_Clk;
    
    // Инициализация переменных состояния
    mSPDIFRX.CtCallBack = 0;
    mSPDIFRX.EtatSPDif = SPDIF_INACTIVE;
    mSPDIFRX.SPDiff_SampleRate = 0;
    
    // Установка указателей на callback-функции
    hspdif1.RxHalfCpltCallback = OnReceiveHalfComplete;
    hspdif1.RxCpltCallback = OnReceiveComplete;
    
    // Регистрация callback-функции истечения периода таймера
    HAL_TIM_RegisterCallback(&htim7, HAL_TIM_PERIOD_ELAPSED_CB_ID, PeriodElapsedCallback);
    
    // Запуск таймера в режиме прерываний (периодические вызовы каждые 100 мс)
    HAL_TIM_Base_Start_IT(&htim7);
}

/**
 * @brief  Запуск приема данных через S/PDIF
 */
void SPDIF_RX_StartReceive() {
  mSPDIFRX.EtatSPDif = SPDIF_INIT;
}

/**
 * @brief  Остановка приема данных через S/PDIF
 */
void SPDIF_RX_StopReceive() {
  mSPDIFRX.EtatSPDif = SPDIF_STOP;
}

/**
 * @brief  Получение текущей частоты дискретизации
 * @retval Текущая частота дискретизации в Гц
 */
uint32_t SPDIF_RX_GetSampleRate() {
    return mSPDIFRX.SPDiff_SampleRate;
}

/**
 * @brief  Получение текущего состояния модуля S/PDIF
 * @retval Текущее состояние (типа eEtatSPDif)
 */
eEtatSPDif SPDIF_RX_GetState() {
    return mSPDIFRX.EtatSPDif;
}

/**
 * @brief  Вычисление частоты дискретизации входного S/PDIF потока
 */
static void SPDIF_RX_CalcSampleRate() {
    // Формула для вычисления частоты дискретизации:
    // SampleRate = (Частота тактового сигнала * 5) / (значение счетчика * 64)
    uint32_t SampleRate = (mSPDIFRX.Freq_SPDiff_Clk * 5) / 
                         ((((mSPDIFRX.phSPDIFRX->Instance->SR) >> 16) & 0xEFFF) * 64);
    
    // Определение ближайшей стандартной частоты дискретизации
    if (SampleRate > 190000) {
        mSPDIFRX.SPDiff_SampleRate = USB_AUDIO_CONFIG_FREQ_192_K;
    } else if (SampleRate > 170000) {
        mSPDIFRX.SPDiff_SampleRate = USB_AUDIO_CONFIG_FREQ_176_4_K;
    } else if (SampleRate > 90000) {
        mSPDIFRX.SPDiff_SampleRate = USB_AUDIO_CONFIG_FREQ_96_K;
    } else if (SampleRate > 46000) {
        mSPDIFRX.SPDiff_SampleRate = USB_AUDIO_CONFIG_FREQ_48_K;
    } else if (SampleRate > 4000) {
        mSPDIFRX.SPDiff_SampleRate = USB_AUDIO_CONFIG_FREQ_44_1_K;
    }
}

/**
 * @brief  Обработчик истечения периода таймера
 * @param  phTIM: Указатель на дескриптор таймера
 */
void PeriodElapsedCallback(TIM_HandleTypeDef* phTIM) {
    switch (mSPDIFRX.EtatSPDif) {
        case SPDIF_STOP:
            state = EmptyBuffer;
            SAI_MasterMute(1);
            // Прерывание DMA и переход в неактивное состояние
            HAL_DMA_Abort_IT(mSPDIFRX.phSPDIFRX->hdmaDrRx);
            __HAL_SPDIFRX_IDLE(mSPDIFRX.phSPDIFRX);
            mSPDIFRX.EtatSPDif = SPDIF_INACTIVE;
            break;
            
        case SPDIF_INACTIVE:
            break;
            
        case SPDIF_INIT:
            state = EmptyBuffer;
            SAI_MasterMute(1);
            // Сброс текущей операции и подготовка к синхронизации
            HAL_DMA_Abort_IT(mSPDIFRX.phSPDIFRX->hdmaDrRx);
            __HAL_SPDIFRX_IDLE(mSPDIFRX.phSPDIFRX);
            CheckAndResetFlags(mSPDIFRX.phSPDIFRX);
            mSPDIFRX.phSPDIFRX->State = HAL_SPDIFRX_STATE_READY;
            __HAL_SPDIFRX_SYNC(mSPDIFRX.phSPDIFRX);
            mSPDIFRX.EtatSPDif = SPDIF_SYNCHRO;
            break;
            
        case SPDIF_SYNCHRO:
            // Проверка успешной синхронизации
            if (__HAL_SPDIFRX_GET_FLAG(mSPDIFRX.phSPDIFRX, SPDIFRX_FLAG_SYNCD)) {
                // Запуск приема данных через DMA
                if (HAL_SPDIFRX_ReceiveDataFlow_DMA(
                    mSPDIFRX.phSPDIFRX, 
                    (uint32_t*)mSPDIFRX.Buffer, 
                    RX_BUFFER_SIZE * 2) != HAL_OK)
                    {
                      mSPDIFRX.EtatSPDif = SPDIF_INIT;
                      break;
                    }
                SPDIF_RX_CalcSampleRate(); // Определение частоты дискретизации
                // Меняем частоту дискретизации
                AudioChangeFrequency(mSPDIFRX.SPDiff_SampleRate);
                mSPDIFRX.EtatSPDif = SPDIF_RUN;
            }
             else {
                // Повторная попытка инициализации
                mSPDIFRX.EtatSPDif = SPDIF_INIT;
            }
            break;
            
        case SPDIF_RUN:
            {
                // Проверка наличия ошибок
                uint32_t Err = (mSPDIFRX.phSPDIFRX->Instance->SR) & 
                              (SPDIFRX_FLAG_TERR | SPDIFRX_FLAG_FERR | SPDIFRX_FLAG_SERR);
                if (Err != 0) {
                  // Обнаружена ошибка - переход к повторной инициализации
                  mSPDIFRX.EtatSPDif = SPDIF_INIT;
                }
                break;
            }
    }
}

// Проверяем флаги и сбрасываем их
static void CheckAndResetFlags(SPDIFRX_HandleTypeDef *hspdif) {
  if (__HAL_SPDIFRX_GET_FLAG(hspdif, SPDIFRX_FLAG_SYNCD)) {
    __HAL_SPDIFRX_CLEAR_IT(hspdif, SPDIFRX_SR_SYNCD);
  }
  HAL_SPDIFRX_IRQHandler(hspdif);
  if (__HAL_SPDIFRX_GET_FLAG(hspdif, SPDIFRX_FLAG_OVR)) {
    __HAL_SPDIFRX_CLEAR_IT(hspdif, SPDIFRX_IT_OVRIE);
  }
  if (__HAL_SPDIFRX_GET_FLAG(hspdif, SPDIFRX_FLAG_PERR)) {
    __HAL_SPDIFRX_CLEAR_IT(hspdif, SPDIFRX_IT_PERRIE);
  }
  if (__HAL_SPDIFRX_GET_FLAG(hspdif, SPDIFRX_FLAG_CSRNE)) {
    __HAL_SPDIFRX_CLEAR_IT(hspdif, SPDIFRX_IT_CSRNE);
  }
  if (__HAL_SPDIFRX_GET_FLAG(hspdif, SPDIFRX_FLAG_RXNE)) {
    __HAL_SPDIFRX_CLEAR_IT(hspdif, SPDIFRX_IT_RXNE);
  }
}

/**
 * @brief  Обработчик завершения приема данных через DMA
 * @param  hspdif: Указатель на дескриптор S/PDIF RX
 */
void OnReceiveComplete(SPDIFRX_HandleTypeDef *hspdif) {
    // Устанавливаем состояние, что второй буфер заполнен
    state = SecondFilled;
    mSPDIFRX.CtCallBack++;
}

/**
 * @brief  Обработчик завершения половины приема данных через DMA
 * @param  hspdif: Указатель на дескриптор S/PDIF RX
 */
void OnReceiveHalfComplete(SPDIFRX_HandleTypeDef *hspdif) {
    // Проверяем, что это первый запуск после синхронизации
    if (state == EmptyBuffer) {
      // Запускаем работы I2S
      SAI_MasterDMAPrepareTx((uint16_t *)mSPDIFRX.Buffer, RX_BUFFER_SIZE);
      SAI_MasterDMAEnable();
      SAI_MasterMute(0);
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
    }
    // Устанавливаем состояние, что первый буфер заполнен
    state = FirstFilled;
    mSPDIFRX.CtCallBack++;
}

/**
 * @brief  Функция инициализации всего модуля S/PDIF RX
 */
void InitSPDIF() {
  LedInit();
  DMA_Init();         // Инициализация DMA
  SPDIFRX_Init();     // Инициализация модуля S/PDIF R
  TIM7_Init();        // Инициализация таймера TIM7
  SPDIF_RX_Init(84000000); // Инициализация с тактовой частотой 84 МГц
  SPDIF_RX_StartReceive(); // Запуск приема данных
}

/**
 * @brief  Инициализация контроллера DMA
 */
static void DMA_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE(); // Включение тактирования DMA1
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0); // Настройка приоритета прерываний
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn); // Включение прерываний для DMA1 Stream1
}

/**
 * @brief  Инициализация таймера TIM7
 */
static void TIM7_Init(void)
{
  // Конфигурация для периода 100 мс при тактовой частоте 84 МГц
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 8400-1; // Предделитель (84 МГц / 8400 = 10 кГц)
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP; // Счет вверх
  htim7.Init.Period = 1000-1; // Период счета (10 кГц / 1000 = 10 Гц = 100 мс)
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; // Включение предзагрузки
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET; // Сброс выходного триггера
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE; // Отключение режима мастер-слейв
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief  Дополнительная инициализация таймера (вызывается из HAL)
 * @param  htim_base: Указатель на дескриптор таймера
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance == TIM7)
  {
    /* Включение тактирования TIM7 */
    __HAL_RCC_TIM7_CLK_ENABLE();
    /* Инициализация прерываний TIM7 */
    HAL_NVIC_SetPriority(TIM7_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);
  }
}

/**
 * @brief  Инициализация модуля S/PDIF RX
 */
static void SPDIFRX_Init(void)
{
  hspdif1.Instance = SPDIFRX;
  hspdif1.Init.InputSelection = SPDIFRX_INPUT_IN0; // Выбор входного канала IN0
  hspdif1.Init.Retries = SPDIFRX_MAXRETRIES_15;  // повторных попыток 15
  hspdif1.Init.WaitForActivity = SPDIFRX_WAITFORACTIVITY_ON; // Ждать активности
  hspdif1.Init.ChannelSelection = SPDIFRX_CHANNEL_A; // Выбор канала A (данные)
  hspdif1.Init.DataFormat = SPDIFRX_DATAFORMAT_MSB; // Формат данных MSB
  hspdif1.Init.StereoMode = SPDIFRX_STEREOMODE_ENABLE; // Включение стерео режима
  hspdif1.Init.PreambleTypeMask = SPDIFRX_PREAMBLETYPEMASK_ON; // Включение маски преамбул
  hspdif1.Init.ChannelStatusMask = SPDIFRX_CHANNELSTATUS_ON; // Включение маски статуса канала
  hspdif1.Init.ValidityBitMask = SPDIFRX_VALIDITYMASK_ON; // Включение маски бита достоверности
  hspdif1.Init.ParityErrorMask = SPDIFRX_PARITYERRORMASK_ON; // Включение маски ошибок четности
  if (HAL_SPDIFRX_Init(&hspdif1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief  Дополнительная инициализация S/PDIF RX (вызывается из HAL)
 * @param  hspdifrx: Указатель на дескриптор S/PDIF RX
 */
void HAL_SPDIFRX_MspInit(SPDIFRX_HandleTypeDef* hspdifrx)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(hspdifrx->Instance == SPDIFRX)
  {
    // Настройка тактирования периферийных устройств
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPDIFRX;
    PeriphClkInitStruct.SpdifClockSelection = RCC_SPDIFRXCLKSOURCE_PLLR; // Источник тактового сигнала
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Включение тактирования SPDIFRX */
    __HAL_RCC_SPDIFRX_CLK_ENABLE();

    /* Включение тактирования GPIOB */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /** Конфигурация GPIO для SPDIFRX1_IN0
    * PB7 ------> SPDIFRX1_IN0
    */
    GPIO_InitStruct.Pin = GPIO_PIN_7; //  контакт входа SPDIF 5 на плате PB7
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; // Альтернативная функция с push-pull
    GPIO_InitStruct.Pull = GPIO_NOPULL;     // Без подтяжки
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; // Высокая скорость
    GPIO_InitStruct.Alternate = GPIO_AF8_SPDIFRX; // Альтернативная функция 8
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); // Инициализация GPIOB

    /* Инициализация DMA для SPDIFRX */
    hdma_spdif_rx_dt.Instance = DMA1_Stream1; // Используется Stream1
    hdma_spdif_rx_dt.Init.Direction = DMA_PERIPH_TO_MEMORY; // Передача из периферии в память
    hdma_spdif_rx_dt.Init.Channel = DMA_CHANNEL_0; // Используется канал 0
    hdma_spdif_rx_dt.Init.PeriphInc = DMA_PINC_DISABLE; // Адрес периферии не инкрементируется
    hdma_spdif_rx_dt.Init.MemInc = DMA_MINC_ENABLE; // Адрес памяти инкрементируется
    hdma_spdif_rx_dt.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD; // Выравнивание данных 32 бита
    hdma_spdif_rx_dt.Init.MemDataAlignment = DMA_MDATAALIGN_WORD; // Выравнивание данных 32 бита
    hdma_spdif_rx_dt.Init.Mode = DMA_CIRCULAR; // Циклический режим
    hdma_spdif_rx_dt.Init.Priority = DMA_PRIORITY_HIGH; // Высокий приоритет
    hdma_spdif_rx_dt.Init.FIFOMode = DMA_FIFOMODE_DISABLE; // Отключение FIFO
    if (HAL_DMA_Init(&hdma_spdif_rx_dt) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hspdifrx, hdmaDrRx, hdma_spdif_rx_dt); // Связь с DMA
  }
}

/**
 * @brief  Обработчик прерываний DMA
 */
void DMA1_Stream1_IRQHandler(void)
{
   HAL_DMA_IRQHandler(&hdma_spdif_rx_dt);
}

/**
 * @brief  Обработчик прерываний таймера TIM7
 */
void TIM7_IRQHandler(void)
{
   HAL_TIM_IRQHandler(&htim7);
}

void SetMode(DeviceMode newMode) {
  mode = newMode;
}

DeviceMode getMode() {
  return mode;
}

void SAITransferCompleteHandler() {
  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
  switch (state) {
    case EmptyBuffer:
      break;
    case FirstFilled:
      // Первый буфер заполнен, отправляет его на I2S
      SAI_MasterDMAPrepareTx((uint16_t *)mSPDIFRX.Buffer, RX_BUFFER_SIZE);
      SAI_MasterDMAEnable();
      break;
    case SecondFilled:
      // Второй буфер заполнен, отправляет его на I2S
      SAI_MasterDMAPrepareTx((uint16_t *)&mSPDIFRX.Buffer[RX_BUFFER_SIZE], RX_BUFFER_SIZE);
      SAI_MasterDMAEnable();
      break;
  }
}

// Инициализация светодиода
// Используется для отладки, что сигнал отправляется на выход I2S
static void LedInit() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Альтернативная функция с push-pull
  GPIO_InitStruct.Pull = GPIO_NOPULL;     // Без подтяжки
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; // Высокая скорость
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); // Инициализация GPIOC
}
