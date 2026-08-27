#pragma once

#include "raylib.h"

#define MAKSIMUM_ISIK_SAYISI 8

// Işık Türleri
enum class IsikTipi {
    NOKTASAL,    // Point Light (Attenuation Radius / Falloff)
    SPOT,        // Spot Light (Inner / Outer Cone)
    ORTAM_GUNES  // Environment / Directional Light
};

// Işık Kaynağı Yapısı (Fiziksel / Optik Işık Parametreleri)
struct Isik {
    bool aktif;
    IsikTipi tip;
    Vector3 pozisyon;
    Vector3 hedef;
    Color renk;
    float parlaklik;
    
    // Unreal Engine / Fiziksel Işık Parametreleri:
    float zayiflamaYaricapi; // Attenuation Radius (Point Light etki mesafesi)
    float icKoniAcisi;       // Inner Cone Angle (Spot Light yumuşak geçiş başlangıcı)
    float disKoniAcisi;      // Outer Cone Angle (Spot Light sınır açısı)
    float kaynakYaricapi;    // Source Radius / Boyut

    // Shader Uniform Lokasyonları
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
          zayiflamaYaricapi(15.0f),
          icKoniAcisi(25.0f),
          disKoniAcisi(45.0f),
          kaynakYaricapi(0.5f),
          aktifLoc(-1), tipLoc(-1), posLoc(-1), hedefLoc(-1), renkLoc(-1), parlaklikLoc(-1),
          zayiflamaYaricapiLoc(-1), icKoniAcisiLoc(-1), disKoniAcisiLoc(-1) {}
};

// Shader'a modern fiziksel ışık verilerini gönderen yardımcı fonksiyon
inline void GuncelleIsikGPU(Shader shader, Isik& isik) {
    int aktifInt = isik.aktif ? 1 : 0;
    SetShaderValue(shader, isik.aktifLoc, &aktifInt, SHADER_UNIFORM_INT);

    int tipInt = (int)isik.tip;
    SetShaderValue(shader, isik.tipLoc, &tipInt, SHADER_UNIFORM_INT);

    float pos[3] = { isik.pozisyon.x, isik.pozisyon.y, isik.pozisyon.z };
    SetShaderValue(shader, isik.posLoc, pos, SHADER_UNIFORM_VEC3);

    float hedef[3] = { isik.hedef.x, isik.hedef.y, isik.hedef.z };
    SetShaderValue(shader, isik.hedefLoc, hedef, SHADER_UNIFORM_VEC3);

    float renkVec[4] = {
        (float)isik.renk.r / 255.0f,
        (float)isik.renk.g / 255.0f,
        (float)isik.renk.b / 255.0f,
        (float)isik.renk.a / 255.0f
    };
    SetShaderValue(shader, isik.renkLoc, renkVec, SHADER_UNIFORM_VEC4);

    SetShaderValue(shader, isik.parlaklikLoc, &isik.parlaklik, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, isik.zayiflamaYaricapiLoc, &isik.zayiflamaYaricapi, SHADER_UNIFORM_FLOAT);

    float cosIc = cosf(isik.icKoniAcisi * DEG2RAD);
    SetShaderValue(shader, isik.icKoniAcisiLoc, &cosIc, SHADER_UNIFORM_FLOAT);

    float cosDis = cosf(isik.disKoniAcisi * DEG2RAD);
    SetShaderValue(shader, isik.disKoniAcisiLoc, &cosDis, SHADER_UNIFORM_FLOAT);
}
