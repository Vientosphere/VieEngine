#pragma once

#include "raylib.h"

// Maksimum aktif ışık sayısı
#define MAKSIMUM_ISIK_SAYISI 8

// Işık Türleri
enum class IsikTipi {
    NOKTASAL,    // Point Light (Mesafe bazlı fiziksel zayıflama / Attenuation)
    SPOT,        // Spot Light (İç ve dış koni açısı yumuşatması)
    ORTAM_GUNES  // Environment / Directional Light (Sonsuz paralel ışınlar & Ortografik gölge)
};

// Işık Kaynağı Veri Yapısı (Fiziksel & Optik Parametreler)
struct Isik {
    bool aktif;
    IsikTipi tip;
    Vector3 pozisyon;
    Vector3 hedef;
    Color renk;
    float parlaklik;

    // Fiziksel & Optik Işık Parametreleri
    float zayiflamaYaricapi; // Attenuation Radius (Işığın etkili olduğu maksimum küresel mesafe)
    float icKoniAcisi;       // Inner Cone Angle (Spot ışığın tam güçte kaldığı iç açı)
    float disKoniAcisi;      // Outer Cone Angle (Spot ışığın yumuşayarak sıfırlandığı dış sınır açısı)
    float kaynakYaricapi;    // Source Radius (Işık kaynağının fiziksel boyutu)

    // Shader GPU Uniform Lokasyonları
    int aktifLoc;
    int tipLoc;
    int posLoc;
    int hedefLoc;
    int renkLoc;
    int parlaklikLoc;
    int zayiflamaYaricapiLoc;
    int icKoniAcisiLoc;
    int disKoniAcisiLoc;

    Isik()
        : aktif(true),
          tip(IsikTipi::NOKTASAL),
          pozisyon((Vector3){ 0.0f, 5.0f, 0.0f }),
          hedef((Vector3){ 0.0f, 0.0f, 0.0f }),
          renk(WHITE),
          parlaklik(1.5f),
          zayiflamaYaricapi(16.0f),
          icKoniAcisi(25.0f),
          disKoniAcisi(45.0f),
          kaynakYaricapi(0.5f),
          aktifLoc(-1), tipLoc(-1), posLoc(-1), hedefLoc(-1), renkLoc(-1), parlaklikLoc(-1),
          zayiflamaYaricapiLoc(-1), icKoniAcisiLoc(-1), disKoniAcisiLoc(-1) {}
};

// Işık verilerini GPU Shader uniform değişkenlerine yükleyen yardımcı fonksiyon
inline void GuncelleIsikGPU(Shader shader, Isik& isik) {
    int aktifInt = isik.aktif ? 1 : 0;
    SetShaderValue(shader, isik.aktifLoc, &aktifInt, SHADER_UNIFORM_INT);

    int tipInt = static_cast<int>(isik.tip);
    SetShaderValue(shader, isik.tipLoc, &tipInt, SHADER_UNIFORM_INT);

    float pos[3] = { isik.pozisyon.x, isik.pozisyon.y, isik.pozisyon.z };
    SetShaderValue(shader, isik.posLoc, pos, SHADER_UNIFORM_VEC3);

    float hedef[3] = { isik.hedef.x, isik.hedef.y, isik.hedef.z };
    SetShaderValue(shader, isik.hedefLoc, hedef, SHADER_UNIFORM_VEC3);

    float renkVec[4] = {
        static_cast<float>(isik.renk.r) / 255.0f,
        static_cast<float>(isik.renk.g) / 255.0f,
        static_cast<float>(isik.renk.b) / 255.0f,
        static_cast<float>(isik.renk.a) / 255.0f
    };
    SetShaderValue(shader, isik.renkLoc, renkVec, SHADER_UNIFORM_VEC4);

    SetShaderValue(shader, isik.parlaklikLoc, &isik.parlaklik, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, isik.zayiflamaYaricapiLoc, &isik.zayiflamaYaricapi, SHADER_UNIFORM_FLOAT);

    // Koni açılarının kosinüslerini hesaplayarak fragment shader'da dot product ile doğrudan karşılaştırma sağlanır
    float cosIc = cosf(isik.icKoniAcisi * DEG2RAD);
    SetShaderValue(shader, isik.icKoniAcisiLoc, &cosIc, SHADER_UNIFORM_FLOAT);

    float cosDis = cosf(isik.disKoniAcisi * DEG2RAD);
    SetShaderValue(shader, isik.disKoniAcisiLoc, &cosDis, SHADER_UNIFORM_FLOAT);
}
