#pragma once

#include "raylib.h"
#include "Entity.h"
#include <vector>

// Viento Engine Çekirdek Sınıfı
class Engine {
public:
    Engine();
    ~Engine();

    // Motor yaşam döngüsü
    void Init(int genislik, int yukseklik, const char* baslik);
    void Run();
    void Shutdown();

    // Sahneye nesne ekleme
    void NesneEkle(const Entity& yeniNesne);

private:
    void Update(); // Kamera ve sahne güncellemeleri
    void Render(); // 3D uzay ve editör arayüzü çizimi

    // Pencere durumu
    int ekranGenisligi;
    int ekranYuksekligi;
    bool calisiyorMu;

    // 3D Editör Kamerası (Blender tarzı yörünge/orbit kamera)
    Camera3D kamera;

    // Sahnedeki nesneler
    std::vector<Entity> nesneler;
};
