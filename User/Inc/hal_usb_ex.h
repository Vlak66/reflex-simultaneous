/**
  ******************************************************************************
  * @file    hal_usb_interface_extension
  * @author  MCD Application Team
  * @brief   Расширение интерфейса HAL для работы с регистрами USB.
  *          Содержит низкоуровневые макросы для управления USB-конечными точками (Endpoints),
  *          а также дополнительные функциональные возможности взаимодействия с USB OTG.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован компанией ST по лицензии Ultimate Liberty,
  * SLA0044, "License"; Вы можете использовать этот файл только в соответствии с
  * условиями лицензии. Полная версия лицензии доступна по адресу:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Определение для предотвращения повторного подключения заголовочного файла */
#ifndef __HAL_USB_INTERFACE_EXTENSION
#define __HAL_USB_INTERFACE_EXTENSION

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_ll_usb.h"

/**
  * @brief Макрос определяет базовый адрес USB OTG контроллера в зависимости от выбранной конфигурации.
  *        Поддерживаются следующие варианты:
  *           - USE_USB_FS_INTO_HS: Использовать FS PHY, но как High Speed устройство
  *           - USE_USB_FS: Использовать Full Speed
  *           - USE_USB_HS: Использовать High Speed
  */
#ifdef USE_USB_FS_INTO_HS
  #define USB_OTG_BASE_ADDRESS  USB_OTG_HS /*!< Использование FS PHY в режиме HS устройства */
#else
  #ifdef USE_USB_FS
    #define USB_OTG_BASE_ADDRESS  USB_OTG_FS /*!< Базовый адрес USB OTG Full Speed */
  #endif
  #ifdef USE_USB_HS
    #define USB_OTG_BASE_ADDRESS  USB_OTG_HS /*!< Базовый адрес USB OTG High Speed */
  #endif
#endif

/* MACRO ------------------------------------------------------------------*/

/**
  * @brief Получает регистр управления входной конечной точкой (IN Endpoint Control).
  * @param ep_addr: Адрес конечной точки (бит 7 указывает направление: 1 = IN, 0 = OUT)
  */
#define USB_DIEPCTL(ep_addr) ((USB_OTG_INEndpointTypeDef *)((uint32_t)USB_OTG_BASE_ADDRESS + USB_OTG_IN_ENDPOINT_BASE   \
        + (ep_addr&0x7FU)*USB_OTG_EP_REG_SIZE))->DIEPCTL

/**
  * @brief Получает регистр управления исходящей конечной точкой (OUT Endpoint Control).
  * @param ep_addr: Адрес конечной точки
  */
#define USB_DOEPCTL(ep_addr) ((USB_OTG_OUTEndpointTypeDef *)((uint32_t)USB_OTG_BASE_ADDRESS +  \
      USB_OTG_OUT_ENDPOINT_BASE + (ep_addr)*USB_OTG_EP_REG_SIZE))->DOEPCTL

/**
  * @brief Отключает и устанавливает Nak для незавершённой IN-конечной точки.
  *        Используется для корректного завершения передачи, если она была прервана.
  * @param ep_addr: Адрес IN-точки
  */
#define USB_CLEAR_INCOMPLETE_IN_EP(ep_addr)     if((((ep_addr) & 0x80U) == 0x80U)) {  \
            USB_DIEPCTL(ep_addr) |= (USB_OTG_DIEPCTL_EPDIS | USB_OTG_DIEPCTL_SNAK);  \
                                         }

/**
  * @brief Очищает конфигурацию конечной точки после закрытия.
  *        Убирает флаги USBAEP, MPSIZ, TXFNUM и EPTYP для IN-точек.
  *        Для OUT-точек — убирает USBAEP и MPSIZ.
  * @param ep_addr: Адрес конечной точки
  */
#define USB_CLEAN_EP_AFTER_CLOSE(ep_addr)       \
if ((((ep_addr) & 0x80U) == 0x80U))           \
{                                             \
   USB_DIEPCTL(ep_addr) &= ~ (USB_OTG_DIEPCTL_USBAEP |   \
                              USB_OTG_DIEPCTL_MPSIZ |    \
                              USB_OTG_DIEPCTL_TXFNUM |   \
                              USB_OTG_DIEPCTL_EPTYP);     \
}                                             \
else                                          \
{                                             \
   USB_DOEPCTL(ep_addr) &= ~(USB_OTG_DOEPCTL_USBAEP |     \
                             USB_OTG_DOEPCTL_MPSIZ);      \
}

/**
  * @brief Возвращает текущий номер кадра SOF (Start of Frame).
  *        Полезно при работе с изохронными передачами.
  */
#define USB_SOF_NUMBER() ((((USB_OTG_DeviceTypeDef *)((uint32_t )USB_OTG_FS + USB_OTG_DEVICE_BASE))->DSTS & USB_OTG_DSTS_FNSOF) >> USB_OTG_DSTS_FNSOF_Pos)

/**
  * @brief Проверяет, является ли передача по ISO IN-каналу незавершённой.
  *        Это позволяет принять решение о необходимости повторной отправки данных.
  * @param ep_addr: Адрес IN-точки
  * @param current_sof: Текущий номер кадра SOF
  * @param transmit_soffn: Номер кадра, когда началась передача
  */
#define IS_ISO_IN_INCOMPLETE_EP(ep_addr, current_sof, transmit_soffn) \
( (USB_DIEPCTL(ep_addr) & USB_OTG_DIEPCTL_EPENA_Msk) && \
  ( ((current_sof & 0x01) == ((USB_DIEPCTL(ep_addr) & USB_OTG_DIEPCTL_EONUM_DPID_Msk) >> USB_OTG_DIEPCTL_EONUM_DPID_Pos)) || \
    (current_sof == ((transmit_soffn + 2) & 0x7FF)) ) )

#ifdef __cplusplus
}
#endif

#endif  /* __HAL_USB_INTERFACE_EXTENSION */

/************************ (C) COPYRIGHT STMicroelectronics *****КОНЕЦ ФАЙЛА****/