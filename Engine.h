#pragma once

#include "raylib.h"

// Motorun ana çalışma mantığını yöneten sınıf
class Engine {
public:
    Engine();  // Kurucu: Başlangıç
    ~Engine(); // Yıkıcı: Bellek ve pencere temizliği

    // Motorun yaşam döngüsü fonksiyonları
    void Init(int genislik, int yukseklik, const char* baslik);
    void Run();
    void Shutdown();

private:
    void Update(); // Mantık ve girdi hesaplamaları
    void Render(); // Ekrana çizim aşaması

    // Pencere ve çalışma durumu
    int ekranGenisligi;
    int ekranYuksekligi;
    bool calisiyorMu;

    // 3D Kamera
    Camera3D kamera;

    // Test nesnesi (Küp) verileri
    Vector3 kupPozisyonu;
    float kupBoyutu;
    float hareketHizi;
    Color kupRengi;
};
