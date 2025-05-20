@echo off
:: build_in_docker.bat

:: Проверяем, установлен ли Docker
where docker >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ❌ Docker не установлен. Убедитесь, что Docker Desktop запущен.
    exit /b 1
)

echo 📁 Текущая директория: %cd%

echo 🔧 Запуск Docker-контейнера и установка зависимостей...

docker run --rm ^
    -v "%cd%:/project" ^
    -w "/project" ^
    ubuntu:22.04 ^
    bash -c ^
    "apt update > /dev/null && \
     apt install -y build-essential git gcc-arm-none-eabi libnewlib-arm-none-eabi dfu-util python3 > /dev/null && \
     make && \
     make dfu"

:: Проверяем результат
if %ERRORLEVEL% equ 0 (
    echo ✅ Сборка и прошивка успешно завершены!
) else (
    echo ❌ Ошибка при сборке или прошивке.
    exit /b 1
)