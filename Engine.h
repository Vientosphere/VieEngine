#pragma once

#include "Entity.h"
#include "raylib.h"
#include "rlgl.h"
#include <vector>

// Dönüşüm / Manipülasyon Modları
enum class TransformModu {
    KONUM,    // Translate / Position (W Tuşu)
    ROTASYON, // Rotate (E Tuşu)
    OLCEK     // Scale (R Tuşu)
};

// Sürüklenen Eksen Tipi
enum class EksenTipi {
    YOK,
    X,
    Y,
    Z,
    MERKEZ // Tüm eksenler (Ölçek için)
};

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
    void Update(); // Kamera, seçim ve fare sürükleme güncellemeleri
    void Render(); // 3D uzay, gizmo ve arayüz çizimi
    void CizSagAltGizmo(); // Sağ alttaki 3D yön pusulası
    void CizTransformGizmo(const Entity& nesne); // Seçili nesnenin üzerindeki etkileşimli eksenler

    // Eksen tutma ve sürükleme kontrolleri
    EksenTipi AlgilaGizmoEkseni(const Entity& nesne, Ray ray);

    // Pencere durumu
    int ekranGenisligi;
    int ekranYuksekligi;
    bool calisiyorMu;

    // 3D Kamera ve serbest hareket değişkenleri
    Camera3D kamera;
    float kameraHizi;
    float kameraYaw;
    float kameraPitch;

    // Seçim, Mod ve Sürükleme durumu
    int seciliNesneIndeksi;
    TransformModu aktifMod;
    EksenTipi suruklenenEksen;
    Vector2 sonFarePozisyonu;

    // Sahnedeki nesneler
    std::vector<Entity> nesneler;
};
