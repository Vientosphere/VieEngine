#pragma once

#include "Entity.h"
#include "raylib.h"
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
    void Update(); // Kamera, seçim ve mantık güncellemeleri
    void Render(); // 3D uzay, gizmo ve arayüz çizimi
    void CizSagAltGizmo(); // Sağ alttaki 3D yön pusulası

    // Pencere durumu
    int ekranGenisligi;
    int ekranYuksekligi;
    bool calisiyorMu;

    // 3D Kamera ve serbest hareket değişkenleri
    Camera3D kamera;
    float kameraHizi;       // Tekerlek ile ayarlanan anlık hız
    float kameraYaw;        // Yatay bakış açısı (derece)
    float kameraPitch;      // Dikey bakış açısı (derece)

    // Seçim sistemi (-1: Hiçbir şey seçili değil)
    int seciliNesneIndeksi;

    // Sahnedeki nesneler
    std::vector<Entity> nesneler;
};
