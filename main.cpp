#include "Engine.h"

int main() {
    // 1. Viento Engine motor örneğini oluştur
    Engine motor;

    // 2. Pencereyi ve grafik boru hattını başlat (1280x720 çözünürlük)
    motor.Init(1280, 720, "Viento Engine - 3D Viewport");

    // 3. Ana motor döngüsünü çalıştır (Update & Render)
    motor.Run();

    // 4. Motor kapatıldığında kaynakları temizle
    motor.Shutdown();

    return 0;
}
