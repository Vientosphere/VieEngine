#pragma once

#include "Entity.h"
#include "Light.h"
#include "raylib.h"
#include "rlgl.h"
#include <vector>

// Dönüşüm / Manipülasyon Modları
enum class TransformModu {
    KONUM,    // Translate (W Tuşu)
    ROTASYON, // Rotate (E Tuşu)
    OLCEK     // Scale (R Tuşu)
};

// Sürüklenen Eksen Tipi
enum class EksenTipi {
    YOK,
    X,
    Y,
    Z,
    MERKEZ
};

// Açık olan Toolbar Menüsü
enum class MenuDurumu {
    KAPALI,
    ADD_ANA,
    ADD_SHAPES,
    ADD_LIGHTS
};

// Viento Engine Çekirdek Sınıfı
class Engine {
public:
    Engine();
    ~Engine();

    void Init(int genislik, int yukseklik, const char* baslik);
    void Run();
    void Shutdown();

    void NesneEkle(const Entity& yeniNesne);
    void IsikEkle(IsikTipi tip, Vector3 pos, Color renk = WHITE, float parlaklik = 1.0f);

private:
    void InitShader(); // GLSL Aydınlatma shader'ı kurulumu
    void Update();
    void Render();
    void CizToolbar(); // Üst araç çubuğu ve açılır menüler
    void CizSagAltGizmo();
    void CizTransformGizmo(const Entity& nesne);

    EksenTipi AlgilaGizmoEkseni(const Entity& nesne, Ray ray);

    int ekranGenisligi;
    int ekranYuksekligi;
    bool calisiyorMu;

    Camera3D kamera;
    float kameraHizi;
    float kameraYaw;
    float kameraPitch;

    int seciliNesneIndeksi;
    TransformModu aktifMod;
    EksenTipi suruklenenEksen;
    Vector2 sonFarePozisyonu;

    // Toolbar / Menü Yönetimi
    MenuDurumu aktifMenu;

    // Aydınlatma Shader ve Işık Listesi
    Shader isikShader;
    int kameraPosLoc;
    int ortamIsigiLoc;
    std::vector<Isik> isiklar;

    std::vector<Entity> nesneler;
};
