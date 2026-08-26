#include "Engine.h"

int main() {
    // Motoru başlat
    Engine motor;
    motor.Init(1280, 720, "Viento Engine");

    // 1. Oyuncu Küpü (Bizim kontrol ettiğimiz nesne)
    Entity oyuncuKupu;
    oyuncuKupu.pozisyon = (Vector3){ 0.0f, 1.5f, 0.0f };
    oyuncuKupu.boyut = (Vector3){ 3.0f, 3.0f, 3.0f };
    oyuncuKupu.renk = RED;
    oyuncuKupu.cizgiler = true;
    motor.NesneEkle(oyuncuKupu);

    // 2. Sahneye Sabit Engeller / Sütunlar Ekleyelim
    Entity solSutun;
    solSutun.pozisyon = (Vector3){ -8.0f, 2.5f, -5.0f };
    solSutun.boyut = (Vector3){ 2.0f, 5.0f, 2.0f };
    solSutun.renk = DARKGRAY;
    solSutun.cizgiler = true;
    motor.NesneEkle(solSutun);

    Entity sagSutun;
    sagSutun.pozisyon = (Vector3){ 8.0f, 2.5f, -5.0f };
    sagSutun.boyut = (Vector3){ 2.0f, 5.0f, 2.0f };
    sagSutun.renk = DARKGRAY;
    sagSutun.cizgiler = true;
    motor.NesneEkle(sagSutun);

    Entity arkaDuvar;
    arkaDuvar.pozisyon = (Vector3){ 0.0f, 2.0f, -12.0f };
    arkaDuvar.boyut = (Vector3){ 18.0f, 4.0f, 1.5f };
    arkaDuvar.renk = MAROON;
    arkaDuvar.cizgiler = true;
    motor.NesneEkle(arkaDuvar);

    // Motor döngüsünü çalıştır
    motor.Run();

    return 0;
}
