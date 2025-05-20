# build_in_docker.ps1

# Проверка наличия Docker
if (!(Get-Command docker -ErrorAction SilentlyContinue)) {
    Write-Host "❌ Docker не установлен. Убедитесь, что Docker Desktop запущен." -ForegroundColor Red
    exit 1
}

Write-Host "📁 Текущая директория: $PWD" -ForegroundColor Green

# Запуск контейнера с монтированием текущей папки и установкой зависимостей
Write-Host "🔧 Запуск Docker-контейнера..." -ForegroundColor Yellow

docker run --rm `
    -v "${PWD}:/project" `
    -w "/project" `
    ubuntu:22.04 `
    bash -c "
        echo '📥 Обновление пакетов...' && \
        apt update > /dev/null && \
        \
        echo '📦 Установка необходимых инструментов...' && \
        apt install -y build-essential git gcc-arm-none-eabi libnewlib-arm-none-eabi dfu-util python3 > /dev/null && \
        \
        echo '⚙️ Выполняется make dfu...' && \
        make && \
        make dfu
    "

# Проверяем результат
if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Сборка и прошивка успешно завершены!" -ForegroundColor Green
} else {
    Write-Host "❌ Ошибка при сборке или прошивке." -ForegroundColor Red
    exit 1
}