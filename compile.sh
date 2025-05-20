#!/bin/bash

# Имя образа и контейнера
IMAGE_NAME="ubuntu:22.04"
PROJECT_DIR="/project"

# Проверяем, установлен ли Docker
if ! command -v docker &> /dev/null
then
    echo "❌ Docker не установлен. Установите Docker перед запуском."
    exit 1
fi

echo "📁 Текущая директория: $(pwd)"

# Запускаем контейнер, монтируем текущую папку и выполняем сборку
echo "🔧 Запуск контейнера и установка зависимостей..."
docker run --rm -it \
    -v "$(pwd):$PROJECT_DIR" \
    --workdir "$PROJECT_DIR" \
    $IMAGE_NAME \
    bash -c "
        echo '📥 Обновление пакетов...'
        apt update > /dev/null && \

        echo '📦 Установка необходимых инструментов...'
        apt install -y build-essential git gcc-arm-none-eabi libnewlib-arm-none-eabi dfu-util python3 > /dev/null && \

        echo '⚙️ Выполняется make dfu...'
        make && \
        make dfu
    "

# Проверяем результат
if [ $? -eq 0 ]; then
    echo "✅ Сборка и прошивка успешно завершены!"
else
    echo "❌ Ошибка при сборке или прошивке."
    exit 1
fi
