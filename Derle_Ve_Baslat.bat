@echo off
set "PATH=C:\raylib\w64devkit\bin;%PATH%"
g++ viento_game.cpp -o viento_game.exe -I C:\raylib\raylib\src -L C:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm
if %ERRORLEVEL% EQU 0 (
    echo Derleme basarili! Oyun baslatiliyor...
    start "" viento_game.exe
) else (
    echo Derleme hatasi olustu.
    pause
)
