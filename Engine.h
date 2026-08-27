#pragma once

#include "Entity.h"
#include "Light.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <vector>

#define SHADOWMAP_BOYUT 2048

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
    void IsikEkle(IsikTipi tip, Vector3 pos, Color renk = WHITE, float parlaklik = 1.5f, float zayiflamaYaricapi = 18.0f);

private:
    void InitShader();
    void InitShadowmap();
    void InitMeshes();
    void GuncelleIsikKamerasi();
    void Update();
    void Render();
    void SahneyiCiz(bool golgePassi);
    void CizToolbar();
    void CizSagAltGizmo();
    void CizTransformGizmo(const Entity& nesne);

    RenderTexture2D LoadShadowmap(int width, int height);
    void UnloadShadowmap(RenderTexture2D target);

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

    MenuDurumu aktifMenu;

    // Aydınlatma Shader & Dynamic Multi-Mode Shadow Mapping
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

    // Primitive GPU Meshes & Materials
    Mesh kupMesh;
    Mesh kureMesh;
    Mesh silindirMesh;
    Mesh ucgenMesh;
    Mesh duzlemMesh;

    Material isikMaterial;
    Material golgeMaterial;
    Material wireMaterial;
    Material outlineMaterial;

    std::vector<Isik> isiklar;
    std::vector<Entity> nesneler;
};
