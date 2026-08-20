#include "Engine.h"

int main() {
    // Motor nesnesini oluştur ve başlat
    Engine motor;
    motor.Init(800, 600, "VieEngine 3D");

    // Ana döngüyü çalıştır
    motor.Run();

    // Motor kapandığında bellek temizliği otomatik yapılır (~Engine)
    return 0;
}
