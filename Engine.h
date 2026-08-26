#pragma once

#include "raylib.h"
#include "Entity.h"
#include <vector>

// Motorun ana çalışma mantığını yöneten sınıf
class Engine {
public:
    Engine();  // Kurucu: Başlangıç
    ~Engine(); // Yıkıcı: Bellek ve pencere temizliği

    // Motorun yaşam döngüsü fonksiyonları
    void Init(int genislik, int yukseklik, const char* baslik);
    void Run();
    void Shutdown();

    // Sahneye nesne ekleme fonksiyonu
    void NesneEkle(const Entity& yeniNesne);

private:
    void Update(); // Mantık ve girdi hesaplamaları
    void Render(); // Ekrana çizim aşaması

    // Pencere ve çalışma durumu
    int ekranGenisligi;
    int ekranYuksekligi;
    bool calisiyorMu;

    // 3D Kamera
    Camera3D kamera;

    // Dünyadaki tüm varlıkların (Entity) listesi
    std::vector<Entity> nesneler;

    // Kontrol edilen ana küpün nesne indeksi
    int kontrolEdilenIndeks;
    float hareketHizi;
};
