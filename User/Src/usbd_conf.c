/**
  ******************************************************************************
  * @file    usbd_conf.c
  * @author  MCD Application Team
  * @brief   Этот файл реализует обратные вызовы библиотеки USB Device и MSP
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019  STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован ST под лицензией Ultimate Liberty
  * SLA0044, "Лицензией"; Вы не можете использовать этот файл иначе как в соответствии с
  * Лицензией. Вы можете получить копию Лицензии по адресу:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Включаемые файлы ----------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "usb_audio.h"
#include "board.h"
#include "usb_audio_descriptors.h"
#include "usbd_core.h"


/* Приватные определения типов -----------------------------------------------------------*/
/* Приватные определения ------------------------------------------------------------*/
/* Приватные макросы -------------------------------------------------------------*/
/* Приватные переменные ---------------------------------------------------------*/
PCD_HandleTypeDef hpcd;

/* Прототипы приватных функций -----------------------------------------------*/
static USBD_StatusTypeDef USBD_LL_Setup_Fifo(void);
/* Приватные функции ---------------------------------------------------------*/

/*******************************************************************************
                       Подпрограммы PCD BSP
*******************************************************************************/

/**
  * @brief  Инициализация MSP для PCD.
  * @param  hpcd: Дескриптор PCD
  * @retval Нет
  */
void HAL_PCD_MspInit(PCD_HandleTypeDef *hpcd)
{
  (void)hpcd;
  /* USB_FS_GPIO имя порта для USB - PORTA - определенов в board.h */
  /* USB_FS_DM_PIN номер порта для D- USB - PA11 - определенов в board.h */
  /* USB_FS_DP_PIN номер порта для D+ USB - PA12 - определенов в board.h */
  /* Настройка режима альтернативной функции для пинов DM и DP */
  USB_FS_GPIO->MODER |= (2 << (2 * USB_FS_DM_PIN)) | (2 << (2 * USB_FS_DP_PIN));
  /* Установка функции USB для пинов DM и DP */
  USB_FS_GPIO->AFR[1] |= (USB_FS_PINS_AF << (4 * (USB_FS_DM_PIN - 8))) | (USB_FS_PINS_AF << (4 * (USB_FS_DP_PIN - 8)));
  /* Установка максимальной скорости для пинов DM и DP */
  /* 0 - Низкая скорость 2 Мгц */
  /* 1 - Средняя скорость 25 Мгц */
  /* 2 - Высокая скорость 50 Мгц */
  /* 2 - Очень высокая скорость 100 Мгц */
  USB_FS_GPIO->OSPEEDR |= (3 << (2 * USB_FS_DM_PIN)) | (3 << (2 * USB_FS_DP_PIN));

  /* Включение тактирования USB OTG FS */
  RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
  // ??? RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; ???

  /* Установка высокого (3) приоритета прерывания USB OTG FS */
  /* 0 самый высокий приоритет - 15 - самый низкий */
  /* для USB используется достаточно высокий приоритет */
  HAL_NVIC_SetPriority(OTG_FS_IRQn, USB_IRQ_PREPRIO, 0);
  /* Разрешение прерывания USB OTG FS */
  NVIC_EnableIRQ(OTG_FS_IRQn);
}

/**
  * @brief  Деинициализация MSP для PCD.
  * @param  hpcd: Дескриптор PCD
  * @retval Нет
  */
void HAL_PCD_MspDeInit(PCD_HandleTypeDef *hpcd)
{
  if(hpcd->Instance == USB_OTG_FS)
  {
    /* Выключение тактирования USB OTG FS */
    RCC->AHB2ENR &= ~RCC_AHB2ENR_OTGFSEN;
  }
}

/*******************************************************************************
   Интерфейс взаимодействия между протокольным драйвером PCD и USB Device библиотекой

   Данный раздел реализует функции обратного вызова (callback) для обработки событий USB:
   - Настройка устройства (Setup)
   - Передача и прием данных через конечные точки (Endpoints)
   - События шины USB (Сброс, Приостановка, Возобновление)
   - Управление состоянием подключения

   Эти функции обеспечивают связующее звено между низкоуровневыми аппаратными событиями
   и высокоуровневой логикой USB Device библиотеки.
*******************************************************************************/

/**
  * @brief  Обработчик этапа настройки (Setup Stage) USB транзакции.
  *         Вызывается автоматически при получении SETUP пакета от хоста.
  *         Передает управление на уровень протокола USB Device Library
  *         для дальнейшей обработки запроса.
  * @param  hpcd: Дескриптор PCD
  * @retval Нет
  * @note   Основные характеристики:
  *         - Выполняется в контексте прерывания, поэтому должен быть максимально быстрым
  *         - Используется для обработки стандартных и специфических запросов USB
  *         - Может вызываться для различных типов запросов:
  *           * Стандартные запросы USB (Standard Requests)
  *           * Запросы класса (Class Requests)
  *           * Запросы производителя (Vendor Requests)
  *
  *         Типичные действия:
  *         - Анализ содержимого SETUP пакета
  *         - Подготовка данных для ответа
  *         - Настройка конечных точек для последующих этапов транзакции
  */
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
  /* передача SETUP пакета для обработки */
  USBD_LL_SetupStage(hpcd->pData, (uint8_t *)hpcd->Setup);
}

/**
  * @brief  Обработчик этапа приема данных.
  *         Вызывается автоматически при получении данных от хоста.
  *         Передает управление на уровень протокола USB для дальнейшей обработки.
  * @param  hpcd: Дескриптор PCD
  * @param  epnum: Номер конечной точки
  * @retval Нет
  * @note   Критичная к скорости выполнения функция, так как работает в контексте прерывания
  *         Не должен выполнять длительных операций
  */
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_DataOutStage(hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

/**
  * @brief  Обработчик этапа передачи данных.
  *         Вызывается автоматически после завершения передачи данных хосту.
  *         Используется для оповещения верхнего уровня об окончании передачи.
  * @param  hpcd: Дескриптор PCD
  * @param  epnum: Номер конечной точки
  * @retval Нет
  * @note   Может использоваться для реализации потоковой передачи данных
  */
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_DataInStage(hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

/**
  * @brief  Обработчик события Start of Frame.
  *         Вызывается каждую миллисекунду (при полной скорости USB).
  *         Может использоваться для задач синхронизации и таймеров.
  * @param  hpcd: Дескриптор PCD
  * @retval Нет
  * @note   Полезен для синхронизации потоковых данных (например, аудио)
  */
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_SOF(hpcd->pData);
}

/**
  * @brief  Обработчик события сброса USB шины.
  *         Вызывается при получении сигнала RESET от хоста.
  *         Используется для полной переинициализации USB стека.
  * @param  hpcd: Дескриптор PCD
  * @retval Нет
  * @note   Выполняет следующие действия:
  *         - Сброс всех конечных точек
  *         - Установка скорости соединения
  *         - Переинициализация внутренних структур данных
  *         - Подготовку устройства к новой инициализации
  */
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_SpeedTypeDef speed = USBD_SPEED_FULL;

  /* Устанавливаем текущую скорость USB */
  switch(hpcd->Init.speed)
  {
  case PCD_SPEED_HIGH:
    speed = USBD_SPEED_HIGH;
    break;

  case PCD_SPEED_FULL:
    speed = USBD_SPEED_FULL;
    break;

  default:
    speed = USBD_SPEED_FULL;
    break;
  }

  /* Сброс устройства */
  USBD_LL_Reset(hpcd->pData);

  USBD_LL_SetSpeed(hpcd->pData, speed);
}

/**
  * @brief  Обработчик перехода в режим приостановки (Suspend).
  *         Вызывается при получении команды Suspend от хоста.
  *         Используется для перевода устройства в режим пониженного энергопотребления.
  * @param  hpcd: Дескриптор PCD
  * @retval Нет
  * @note   Рекомендуется использовать для:
  *         - Остановки активных процессов
  *         - Перевода периферии в спящий режим
  *         - Сохранения важного состояния
  */
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_Suspend(hpcd->pData);
}

/**
  * @brief  Обработчик события возобновления работы после приостановки.
  *         Вызывается при выходе устройства из состояния приостановки (Suspend).
  *         Используется для восстановления нормальной работы устройства после режима пониженного энергопотребления.
  * @param  hpcd: Дескриптор PCD
  * @retval Нет
  * @note   Рекомендуется использовать для:
  *         - Восстановления активных процессов
  *         - Реинициализации периферийных устройств, переведенных в спящий режим
  *         - Восстановления состояния перед приостановкой
  *         - Проверки целостности данных и состояния устройства
  */
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_Resume(hpcd->pData);
}

/**
  * @brief  Обработчик ошибок ISO OUT передачи.
  *         Вызывается при возникновении ошибки во время потоковой передачи данных.
  * @param  hpcd: Дескриптор PCD
  * @param  epnum: Номер конечной точки
  * @retval Нет
  * @note   Используется для обработки ситуаций потери синхронизации или переполнения буферов
  */
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_IsoOUTIncomplete(hpcd->pData, epnum);
}

/**
  * @brief  Обработчик ошибок ISO IN передачи.
  *         Вызывается при возникновении ошибки во время потокового приема данных.
  * @param  hpcd: Дескриптор PCD
  * @param  epnum: Номер конечной точки
  * @retval Нет
  * @note   Может использоваться для повторной отправки потерянных пакетов
  */
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_IsoINIncomplete(hpcd->pData, epnum);
}

/**
  * @brief  Обработчик события подключения USB устройства.
  *         Вызывается при успешном подключении устройства к хосту.
  *         Используется для инициализации ресурсов после установления физического соединения.
  * @param  hpcd: Дескриптор PCD
  * @retval Нет
  * @note   Может использоваться для:
  *         - Включения индикации подключения
  *         - Инициализации зависимых периферийных устройств
  *         - Настройки начального состояния устройства
  */
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_DevConnected(hpcd->pData);
}

/**
  * @brief  Обработчик события отключения USB устройства.
  *         Вызывается при физическом отсоединении устройства от хоста.
  *         Используется для освобождения ресурсов и завершения работы.
  * @param  hpcd: Дескриптор PCD
  * @retval Нет
  * @note   Может использоваться для:
  *         - Выключения индикации подключения
  *         - Деинициализации зависимых периферийных устройств
  *         - Сохранения состояния перед отключением
  */
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_DevDisconnected(hpcd->pData);
}

/*******************************************************************************
* Интерфейс взаимодействия между USB Device библиотекой и низкоуровневым
* драйвером PCD (Peripheral Control Driver).
*
* Данный раздел реализует функции, обеспечивающие связь высокоуровневого
* USB стека с аппаратно-зависимым уровнем управления USB периферией.
*
* Основные задачи:
* - Инициализация и настройка USB контроллера
* - Управление конечными точками (Endpoints)
* - Передача и прием данных
* - Обработка состояний USB устройства
*******************************************************************************/

/**
  * @brief  Инициализация низкоуровневого драйвера USB.
  *         Выполняет настройку параметров USB контроллера и связывает его со стеком USB Device Library.
  * @param  pdev: Дескриптор устройства
  * @retval USBD Статус
  * @note   Основные действия:
  *         - Настройка параметров USB OTG FS контроллера.
  *         - Отключение DMA и низкого энергопотребления.
  *         - Расчет размеров FIFO буферов.
  *         - Связывание драйвера с USB стеком.
  */
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
  /* Устанавливаем параметры LL драйвера */
  /* Выбор USB OTG FS контроллера */
  hpcd.Instance                 = USB_OTG_FS;
  /* Количество конечных точек (кроме EP0) */
  hpcd.Init.dev_endpoints       = 5;
  /* Отключение выделенной конечной точки EP1 */
  hpcd.Init.use_dedicated_ep1   = 0;
  /* Максимальный размер пакета для EP0 - 64 байта */
  hpcd.Init.ep0_mps             = 0x40;
  /* Отключение DMA */
  hpcd.Init.dma_enable          = 0;
  /* Отключение режима низкого энергопотребления */
  hpcd.Init.low_power_enable    = 0;
  /* Использование встроенного физического уровня */
  hpcd.Init.phy_itface          = PCD_PHY_EMBEDDED;
  /* Включение генерации SOF (Start of Frame) */
  hpcd.Init.Sof_enable          = 1;
  /* Установка полной скорости USB (12 Мбит/с) */
  hpcd.Init.speed               = PCD_SPEED_FULL;
  /* Отключение определения VBUS */
  hpcd.Init.vbus_sensing_enable = 0;
  /* Отключение управления энергопотреблением (LPM) */
  hpcd.Init.lpm_enable          = 0;

  /* Связываем драйвер со стеком */
  hpcd.pData    = pdev;
  pdev->pData   = &hpcd;

  /* Инициализируем LL драйвер */
  HAL_PCD_Init(&hpcd);

  /* Настраиваем размеры fifo */
  USBD_LL_Setup_Fifo();

  return USBD_OK;
}

/**
  * @brief  Деинициализация низкоуровневого драйвера USB.
  *         Освобождает ресурсы, выделенные для работы USB контроллера.
  * @param  pdev: Дескриптор устройства
  * @retval USBD Статус
  * @note   Используется при отключении устройства или перезагрузке USB стека.
  */
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
  HAL_PCD_DeInit(pdev->pData);
  return USBD_OK;
}

/**
  * @brief  Запуск работы USB устройства.
  *         Активирует USB контроллер и переводит устройство в рабочее состояние.
  * @param  pdev: Дескриптор устройства
  * @retval USBD Статус
  * @note   После вызова этой функции устройство готово к взаимодействию с хостом.
  */
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
  HAL_PCD_Start(pdev->pData);
  return USBD_OK;
}

/**
  * @brief  Остановка работы USB устройства.
  *         Деактивирует USB контроллер и переводит устройство в неактивное состояние.
  * @param  pdev: Дескриптор устройства
  * @retval USBD Статус
  * @note   Используется при отключении устройства или переходе в спящий режим.
  */
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
  HAL_PCD_Stop(pdev->pData);
  return USBD_OK;
}

/**
  * @brief  Открытие конечной точки USB.
  *         Конфигурирует указанную конечную точку для передачи данных.
  * @param  pdev: Дескриптор устройства
  * @param  ep_addr: Номер конечной точки
  * @param  ep_type: Тип конечной точки
  * @param  ep_mps: Максимальный размер пакета конечной точки
  * @retval USBD Статус
  * @note   Вызывается при настройке интерфейсов USB устройства.
  */
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev,
                                  uint8_t ep_addr,
                                  uint8_t ep_type,
                                  uint16_t ep_mps)
{
  HAL_PCD_EP_Open(pdev->pData,
                  ep_addr,
                  ep_mps,
                  ep_type);

  return USBD_OK;
}

/**
  * @brief  Закрытие конечной точки USB.
  *         Деактивирует указанную конечную точку и освобождает её ресурсы.
  * @param  pdev: Дескриптор устройства
  * @param  ep_addr: Номер конечной точки
  * @retval USBD Статус
  * @note   Используется при изменении конфигурации устройства или его отключении.
  */
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_Close(pdev->pData, ep_addr);
  USB_CLEAN_EP_AFTER_CLOSE(ep_addr);
  return USBD_OK;
}

/**
  * @brief  Очистка буфера конечной точки USB.
  *         Удаляет все данные из буфера указанной конечной точки.
  * @param  pdev: Дескриптор устройства
  * @param  ep_addr: Номер конечной точки
  * @retval USBD Статус
  * @note   Используется для сброса состояния конечной точки.
  */
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_Flush(pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Установка состояния STALL для конечной точки.
  *         Переводит конечную точку в состояние STALL (протокольная ошибка).
  * @param  pdev: Дескриптор устройства
  * @param  ep_addr: Номер конечной точки
  * @retval USBD Статус
  * @note   Используется для обработки протокольных ошибок.
  */
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_SetStall(pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Снятие состояния STALL для конечной точки.
  *         Возвращает конечную точку в рабочее состояние после STALL.
  * @param  pdev: Дескриптор устройства
  * @param  ep_addr: Номер конечной точки
  * @retval USBD Статус
  * @note   Используется для восстановления работы конечной точки.
  */
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_ClrStall(pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Проверка состояния STALL конечной точки.
  *         Определяет, находится ли указанная конечная точка в состоянии STALL.
  * @param  pdev: Дескриптор устройства
  * @param  ep_addr: Номер конечной точки
  * @retval Stall (1: Да, 0: Нет)
  * @note   Используется для мониторинга состояния конечных точек.
  */
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  PCD_HandleTypeDef *hpcd = pdev->pData;

  if((ep_addr & 0x80) == 0x80)
  {
    return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
  }
  else
  {
    return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
  }
}

/**
  * @brief  Назначение адреса USB устройству.
  *         Устанавливает уникальный адрес для USB устройства на шине.
  * @param  pdev: Дескриптор устройства
  * @param  ep_addr: Номер конечной точки
  * @retval USBD Статус
  * @note   Вызывается после этапа настройки устройства.
  */
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
  HAL_PCD_SetAddress(pdev->pData, dev_addr);
  return USBD_OK;
}

/**
  * @brief  Передача данных через конечную точку.
  *         Отправляет данные на хост через указанную конечную точку.
  * @param  pdev: Дескриптор устройства
  * @param  ep_addr: Номер конечной точки
  * @param  pbuf: Указатель на данные для отправки
  * @param  size: Размер данных
  * @retval USBD Статус
  * @note   Используется для потоковой передачи данных.
  */
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev,
                                    uint8_t ep_addr,
                                    uint8_t *pbuf,
                                    uint16_t size)
{
  HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size);
  return USBD_OK;
}

/**
  * @brief  Подготовка конечной точки для приема данных.
  *         Настройка буфера для приема данных от хоста.
  * @param  pdev: Дескриптор устройства
  * @param  ep_addr: Номер конечной точки
  * @param  pbuf: Указатель на данные для приема
  * @param  size: Размер данных
  * @retval USBD Статус
  * @note   Используется для асинхронного приема данных.
  */
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev,
                                          uint8_t ep_addr,
                                          uint8_t *pbuf,
                                          uint16_t size)
{
  HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size);
  return USBD_OK;
}

/**
  * @brief  Получение размера последнего принятого пакета.
  *         Возвращает количество байт, полученных через указанную конечную точку.
  * @param  pdev: Дескриптор устройства
  * @param  ep_addr: Номер конечной точки
  * @retval Полученный размер данных
  * @note   Используется для проверки объема принятых данных.
  */
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  return HAL_PCD_EP_GetRxCount(pdev->pData, ep_addr);
}

/**
  * @brief  Задержка для USB устройства.
  *         Реализует задержку в миллисекундах.
  * @param  Delay: Задержка в мс
  * @retval Нет
  * @note   Используется для таймеров и синхронизации.
  */
void USBD_LL_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}
/**
  * @brief  Настройка размеров FIFO буферов.
  *         Выполняет расчет и распределение размеров TX и RX FIFO буферов.
  * @retval Статус OK
  * @note   Особенности реализации:
  *         - Расчет размеров FIFO для всех типов конечных точек.
  *         - Обработка различных конфигураций (например, аудио playback и recording).
  *         - Проверка на переполнение общего размера FIFO.
  */
static USBD_StatusTypeDef USBD_LL_Setup_Fifo(void)
{

  uint16_t tx_fifo_size[5]={0};/* Массив размеров для каждой из 5 конечных точек TX_FIFO */
  uint8_t max_tx_ep_num = 0;/* Максимальное количество TX_EP_NUMBER */
  uint16_t tx_fifo_used_size = 0; /* Общий размер использования TX_FIFO */
  uint16_t rx_fifo_size =0; /* Общий размер использования RX_FIFO */

  /* rx_fifo_size = (5 * number of control endpoints + 8) + ((largest USB packet used / 4) + 1 for status information) +
                    (2 * number of OUT endpoints) + 1 for Global NAK*/
#if USE_USB_AUDIO_PLAYBACK
  /* не используется, ниже на rx_fifo_size распределяется всё свободное место */
  rx_fifo_size = 194;//146;//(USBD_AUDIO_CONFIG_PLAY_MAX_PACKET_SIZE>64)?(USBD_AUDIO_CONFIG_PLAY_MAX_PACKET_SIZE + 3U)/4:16;
 max_tx_ep_num = CONFIG_AUDIO_EP_SYNC&0x7F;
#else /* USE_USB_AUDIO_PLAYBACK */
 rx_fifo_size = 16;/*64(mps)/4*/
#endif /* USE_USB_AUDIO_PLAYBACK*/


  /* Расчет размера FIFO для конфигурационного дескриптора */
  /* USB_AUDIO_GetConfigDescriptor возращает размер аудио дескриптора - usb_audio_descriptors.c */
  tx_fifo_used_size = (USB_AUDIO_GetConfigDescriptor(0)+3)/4 ;
  tx_fifo_size[0] = tx_fifo_used_size;

  /* Для остальных конечных точек устанавливает минимальный размер */
  /* Передавать кроме конфигурации ничего не требуется */
  for(int i = 1; i<= max_tx_ep_num; i++)
 {
  /* Минимальный размер для каждой конечной точки 16*/
   if(tx_fifo_size[i]<16)
   {
     tx_fifo_size[i] = 16;
   }
   tx_fifo_used_size += tx_fifo_size[i];
 }

 /* Расчет общего размера RX FIFO
    Не используется в нашем режиме USE_USB_AUDIO_PLAYBACK */
 rx_fifo_size += (5*1/*количество управляющих конечных точек*/+8
                    +1/* для информации о состоянии*/+2*6/*количество конечных точек OUT*/+1/*для глобального NAK*/);

  /* Проверка на переполнение общего размера FIFO */
  if(tx_fifo_used_size + rx_fifo_size<=USB_FIFO_WORD_SIZE)
  {

#if USE_USB_AUDIO_PLAYBACK
  /* Распределение оставшегося места для воспроизведения */
  rx_fifo_size = USB_FIFO_WORD_SIZE - tx_fifo_used_size;
#endif /* USE_USB_AUDIO_PLAYBACK */

  }
  else
  {
     Error_Handler();
  }

  /* Установка размеров FIFO */
  HAL_PCDEx_SetRxFiFo(&hpcd, rx_fifo_size);
  for(int i = 0; i<= max_tx_ep_num; i++)
  {
    HAL_PCD_SetTxFiFo(&hpcd, i, tx_fifo_size[i]);
  }
  return USBD_OK;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
