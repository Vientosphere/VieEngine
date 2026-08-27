#pragma once

#include "Entity.h"
#include "Light.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <vector>

// Gölge derinlik haritası çözünürlüğü (2048x2048 HD Depth Map)
#define SHADOWMAP_BOYUT 2048

// Dönüşüm / Manipülasyon Modları
enum class TransformModu {
    KONUM,    // Translate (W Tuşu)
    ROTASYON, // Rotate (E Tuşu)
    OLCEK     // Scale (R Tuşu)
};

// Sürüklenen Gizmo Eksen Tipi
enum class EksenTipi {
    YOK,
    X,
    Y,
    Z,
    MERKEZ
};

// Açık olan Toolbar Menüsü Durumu
enum class MenuDurumu {
    KAPALI,
    ADD_ANA,
    ADD_SHAPES,
    ADD_LIGHTS
};

// Viento Engine Çekirdek Motor Sınıfı
class Engine {
public:
    Engine();
    ~Engine();

    // Motor Yaşam Döngüsü (Lifecycle)
    void Init(int genislik, int yukseklik, const char* baslik);
    void Run();
    void Shutdown();

    // Sahne & Varlık Yönetimi
    void NesneEkle(const Entity& yeniNesne);
    void IsikEkle(IsikTipi tip, Vector3 pos, Color renk = WHITE, float parlaklik = 1.5f, float zayiflamaYaricapi = 18.0f);

private:
    // Başlatma ve Kaynak Yönetimi
    void InitShader();
    void InitShadowmap();
    void InitMeshes();
    RenderTexture2D LoadShadowmap(int width, int height);
    void UnloadShadowmap(RenderTexture2D target);

    // Döngü Mantığı (Update & Render)
    void Update();
    void Render();
    void SahneyiCiz(bool golgePassi);
    void GuncelleIsikKamerasi();

    // Kullanıcı Arayüzü & Editör Elemanları
    void CizToolbar();
    void CizSagAltGizmo();
    void CizTransformGizmo(const Entity& nesne);
    EksenTipi AlgilaGizmoEkseni(const Entity& nesne, Ray ray);

    // Ekran ve Durum Değişkenleri
    int ekranGenisligi;
    int ekranYuksekligi;
    bool calisiyorMu;

    // 3D Kamera ve Serbest Dolaşım (Free-Fly)
    Camera3D kamera;
    float kameraHizi;
    float kameraYaw;
    float kameraPitch;

    // Nesne Seçimi ve Dönüşüm (Transform) Durumu
    int seciliNesneIndeksi;
    TransformModu aktifMod;
    EksenTipi suruklenenEksen;
    Vector2 sonFarePozisyonu;
    MenuDurumu aktifMenu;

    // Aydınlatma Shader & Çoklu Modlu Gölge Sistemi
    Shader isikShader;
    RenderTexture2D golgeHaritasi;
    Camera3D isikKamerasi;
    Matrix lightVP;
    int lightVPLoc;
    int shadowMapLoc;
    int shadowResLoc;
    int ortamIsigiLoc;
    int golgeIsikLoc;
    int golgeIsikIndeksi;

    // GPU Geometri Mesh ve Materyal Tanımları
    Mesh kupMesh;
    Mesh kureMesh;
    Mesh silindirMesh;
    Mesh ucgenMesh;
    Mesh duzlemMesh;

    Material isikMaterial;
    Material golgeMaterial;
    Material wireMaterial;
    Material outlineMaterial;

    // Sahne Veri Listeleri
    std::vector<Isik> isiklar;
    std::vector<Entity> nesneler;
};
