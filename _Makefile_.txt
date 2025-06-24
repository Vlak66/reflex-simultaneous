# Директории для сборки
BUILD_DIR_DEBUG   := build
OBJ_DIR     := $(BUILD_DIR_DEBUG)/obj
OBJ_HAL_DIR     := $(BUILD_DIR_DEBUG)/obj/hal

# Загружаемые репозитории
# stm32f4xx-hal-driver 		 Основные HAL драйвера для STM32F4 от ST
# cmsis                      CMSIS (Cortex Microcontroller Software Interface Standard) от ST
# 							 Стандартный интерфейс для микроконтроллеров
# cmsis-device-f4			 Cпециальная часть CMSIS для F4 семейства микроконтроллеров от ST

# Исходные файлы
# Audio/Src/:
#   audio_usb_nodes.c          - Реализация узлов USB аудио системы (входной/выходной узлы и узел управления функциями)
#   audio_usb_playback_session.c - Реализация сессии воспроизведения USB аудио (инициализация, управление, синхронизация)
#   usbd_audio_if.c           - Интерфейс USB аудио устройства (инициализация и управление сессиями)
#
# STM32_USB_Device_Library/Class/AUDIO_10/Src/:
#   usbd_audio.c              - Реализация USB Audio Class 1.0 (дескрипторы и запросы)
#
# STM32_USB_Device_Library/Core/Src/:
#   usbd_core_ex.c           - Расширенное ядро USB устройства (обработка транзакций)
#   usbd_ctlreq.c            - Обработка управляющих запросов USB
#   usbd_ioreq.c             - Обработка ввода/вывода USB
#
# User/Src/:
#   audio_configuration.c    - Конфигурация аудио параметров
#   audio_speaker_node.c     - Управление выводом звука на динамик
#   main.c                   - Основной файл программы
#   SAI.c                    - Драйвер аудио интерфейса SAI
#   stm32f4xx_it.c           - Обработчики прерываний
#   usb_audio_descriptors.c  - USB дескрипторы аудио устройства
#   usbd_conf.c              - Конфигурация USB устройства
#   usbd_desc.c              - Дескрипторы USB устройства


C_SOURCES = \
	Audio/Src/audio_usb_nodes.c \
	Audio/Src/audio_usb_playback_session.c \
	Audio/Src/usbd_audio_if.c \
	STM32_USB_Device_Library/Class/AUDIO_10/Src/usbd_audio.c \
	STM32_USB_Device_Library/Core/Src/usbd_core_ex.c \
	STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
	STM32_USB_Device_Library/Core/Src/usbd_ioreq.c \
	cmsis-device-f4/Source/Templates/system_stm32f4xx.c \
	User/Src/audio_configuration.c \
	User/Src/audio_speaker_node.c \
	User/Src/main.c \
	User/Src/SAI.c \
	User/Src/stm32f4xx_it.c \
	User/Src/usb_audio_descriptors.c \
	User/Src/usbd_conf.c \
	User/Src/usbd_desc.c \
	User/Src/SPDIF_RX.c

HAL_DRIVER_SOURCES = \
	stm32f4xx-hal-driver/Src/stm32f4xx_hal.c \
	stm32f4xx-hal-driver/Src/stm32f4xx_hal_cortex.c \
	stm32f4xx-hal-driver/Src/stm32f4xx_hal_pcd.c \
	stm32f4xx-hal-driver/Src/stm32f4xx_hal_pcd_ex.c \
	stm32f4xx-hal-driver/Src/stm32f4xx_ll_usb.c \
	stm32f4xx-hal-driver/Src/stm32f4xx_hal_rcc.c \
	stm32f4xx-hal-driver/Src/stm32f4xx_hal_tim.c \
	stm32f4xx-hal-driver/Src/stm32f4xx_hal_spdifrx.c \
	stm32f4xx-hal-driver/Src/stm32f4xx_hal_gpio.c \
	stm32f4xx-hal-driver/Src/stm32f4xx_hal_tim_ex.c \
	stm32f4xx-hal-driver/Src/stm32f4xx_hal_dma.c \
	stm32f4xx-hal-driver/Src/stm32f4xx_hal_rcc_ex.c

ASM_SOURCES = \
	cmsis-device-f4/Source/Templates/gcc/startup_stm32f446xx.s

# Объектные файлы
OBJS_DEBUG   := $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(C_SOURCES))) \
				$(patsubst %.s, $(OBJ_DIR)/%.o, $(notdir $(ASM_SOURCES))) \
				$(patsubst %.c, $(OBJ_HAL_DIR)/%.o, $(notdir $(HAL_DRIVER_SOURCES)))

# Указываем Make, где искать исходные файлы
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.c $(sort $(dir $(HAL_DRIVER_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

###############################################################################
# Команды компилятора
CC      := arm-none-eabi-gcc
AS      := arm-none-eabi-gcc
LD      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size

# Флаги процессора/архитектуры
CPUFLAGS := -mcpu=cortex-m4 -mthumb -mfloat-abi=soft

# Флаги конфигурации
CFLAGS = $(CPUFLAGS) -O2 -g -std=c11 \
	-ffunction-sections -fdata-sections \
	-Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers \
	-MMD -MP -MF"$(@:%.o=%.d)"

CFLAGS_HAL = $(CPUFLAGS) -O2 -g -std=c11 \
	-ffunction-sections -fdata-sections \
	-w \
	-MMD -MP -MF"$(@:%.o=%.d)"

DEFS = \
	-DSTM32F446xx \
	-DUSE_USB_FS \
	-DUSE_USB_AUDIO_PLAYBACK

INCLUDES = \
	-IAudio/Inc \
    -ISTM32_USB_Device_Library/Class/AUDIO_10/Inc \
    -ISTM32_USB_Device_Library/Core/Inc \
    -Istm32f4xx-hal-driver/Inc \
    -Icmsis-device-f4/Include/ \
    -Icore/Core/Include \
    -IUser/Inc


LDFLAGS = $(CPUFLAGS) -Wl,--gc-sections -specs=nano.specs -specs=nosys.specs -Tstm32f446.ld -lc -g

# Имя проекта
PROJECT_NAME := REFLEX

.PHONY: all debug clean clean-repo update cmsis cmsis-device hal
all: cmsis hal cmsis-device
	@$(MAKE) debug

debug: $(BUILD_DIR_DEBUG)/$(PROJECT_NAME).elf $(BUILD_DIR_DEBUG)/$(PROJECT_NAME).hex $(BUILD_DIR_DEBUG)/$(PROJECT_NAME).bin

###############################################################################
# Связывание
###############################################################################
$(BUILD_DIR_DEBUG)/$(PROJECT_NAME).elf: $(OBJS_DEBUG)
	@echo "Линковка: $(notdir $@)"
	@$(LD) $(LDFLAGS) -o $@ $^
	@$(SIZE) $@

$(BUILD_DIR_DEBUG)/$(PROJECT_NAME).hex: $(BUILD_DIR_DEBUG)/$(PROJECT_NAME).elf | $(BUILD_DIR_DEBUG)
	@echo "Создание HEX: $(notdir $@)"
	@$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR_DEBUG)/$(PROJECT_NAME).bin: $(BUILD_DIR_DEBUG)/$(PROJECT_NAME).elf | $(BUILD_DIR_DEBUG)
	@echo "Создание BIN: $(notdir $@)"
	@$(OBJCOPY) -O binary -S $< $@

###############################################################################
# Правила компиляции
###############################################################################
# Компиляция C-файлов в объектные файлы
# $< - имя исходного файла
# $@ - имя целевого файла
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	@echo "Компиляция: $(notdir $<)"
	@$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -o $@ -c $<

$(OBJ_HAL_DIR)/%.o: %.c | $(OBJ_HAL_DIR)
	@echo "Компиляция: $(notdir $<)"
	@$(CC) $(CFLAGS_HAL) $(DEFS) $(INCLUDES) -o $@ -c $<

# Компиляция ассемблерных файлов
$(OBJ_DIR)/%.o: %.s | $(OBJ_DIR)
	@echo "Компиляция ассемблера: $(notdir $<)"
	@$(AS) $(CPUFLAGS) -c -o $@ $<

# Создание директории для объектных файлов
$(OBJ_DIR) $(OBJ_HAL_DIR):
	@mkdir -p $@

# Создание DFU файла для прошивки
# dfu-suffix добавляет информацию о производителе и продукте
$(BUILD_DIR_DEBUG)/$(PROJECT_NAME).dfu: $(BUILD_DIR_DEBUG)/$(PROJECT_NAME).bin
	@echo "Создание файла обновления: $(notdir $@)"
	@./dfu-convert -b 0x8000000:$< $@

$(BUILD_DIR_DEBUG)/$(PROJECT_NAME).dfutil: $(BUILD_DIR_DEBUG)/$(PROJECT_NAME).bin
	@echo "Создание файла обновления: $(notdir $@)"
	@cp -r $< $@
	@dfu-suffix -a $@ -v 0483 -p 0012 -d 5400 1>/dev/null

# Цель для создания DFU файла
dfu: $(BUILD_DIR_DEBUG)/$(PROJECT_NAME).dfu

# Зависимость для CMSIS - проверяет наличие библиотеки
cmsis: core/.git

# Зависимость для HAL - проверяет наличие библиотеки
hal: stm32f4xx-hal-driver/.git

# Зависимость для DEVICE_CMSIS - проверяет наличие библиотеки
cmsis-device: cmsis-device-f4/.git

# Загрузка только необходимой части CMSIS библиотеки
# 1. Создаем директорию core и переходим в нее
# 2. Инициализируем git репозиторий
# 3. Добавляем удаленный репозиторий STM
# 4. Включаем sparse-checkout для выборочной загрузки
# 5. Указываем загружать только папку Core
# 6. Загружаем только последний коммит (--depth 1) для экономии трафика
core/.git:
	@mkdir -p core && cd core && \
	git init -q && \
	git remote add origin https://github.com/STMicroelectronics/cmsis-core && \
	git config extensions.partialClone true && \
	git config core.sparseCheckout true && \
	git sparse-checkout set --no-cone Core && \
	echo "Загрузка cmsis с официального репозитория..." && \
	git fetch --depth 1 --filter=blob:none origin master -q && \
	git checkout master -q

stm32f4xx-hal-driver/.git:
	@echo "Загрузка hal v1.7.6 с официального репозитория..." && \
	git clone --branch v1.7.6 -q --depth 1 https://github.com/STMicroelectronics/stm32f4xx-hal-driver.git

cmsis-device-f4/.git:
	@echo "Загрузка cmsis device с официального репозитория..." && \
	git clone -q --depth 1 https://github.com/STMicroelectronics/cmsis-device-f4.git


# Прошивка микроконтроллера через DFU
# -a 0 - альтернативный интерфейс
# -s 0x08000000:leave - адрес загрузки и выход из режима DFU после загрузки
update: $(BUILD_DIR_DEBUG)/$(PROJECT_NAME).dfutil
	dfu-util -a 0 -s 0x08000000:leave -D $<

###############################################################################
# Правила очистки
###############################################################################
# Очистка всех сгенерированных файлов
clean:
	@echo "Очистка..."
	@$(RM) -rf $(BUILD_DIR_DEBUG)

clean-repo:
	@echo "Удаление репозиториев..."
	@rm -rf core
	@rm -rf stm32f4xx-hal-driver
	@rm -rf cmsis-device-f4

###############################################################################
# Зависимости
###############################################################################
# Подключение файлов зависимостей для отслеживания изменений в заголовочных файлах
-include $(wildcard $(OBJ_DIR)/*.d)
-include $(wildcard $(OBJ_HAL_DIR)/*.d)

.NOTPARALLEL: cmsis hal cmsis-device
