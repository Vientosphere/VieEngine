#include "Engine.h"

int main() {
    // 1. Viento Engine başlat
    Engine motor;
    motor.Init(1280, 720, "Viento Engine - 3D Viewport");

    // 2. Klasik Blender Başlangıç Küpü (2x2x2 Boyutunda, Tam Merkezde)
    Entity baslangicKupu;
    baslangicKupu.pozisyon = (Vector3){ 0.0f, 1.0f, 0.0f };    // Tabanı ızgaraya oturacak şekilde Y=1
    baslangicKupu.boyut = (Vector3){ 2.0f, 2.0f, 2.0f };       // Standart 2x2x2
    baslangicKupu.renk = (Color){ 140, 145, 155, 255 };        // Blender nötr gri
    baslangicKupu.cizgiRengi = (Color){ 230, 235, 245, 255 };  // Açık gri kenarlıklar
    baslangicKupu.cizgiler = true;

    // Sahneye ekle
    motor.NesneEkle(baslangicKupu);

    // 3. Motoru çalıştır
    motor.Run();

    return 0;
}
