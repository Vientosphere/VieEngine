#pragma once

#include "raylib.h"

#define MAKSIMUM_ISIK_SAYISI 8

// Işık Türleri
enum class IsikTipi {
    NOKTASAL,    // Point Light
    SPOT,        // Spot Light
    ORTAM_GUNES  // Environment / Directional Light
};

// Işık Kaynağı Yapısı
struct Isik {
    bool aktif;
    IsikTipi tip;
    Vector3 pozisyon;
    Vector3 hedef;
    Color renk;
    float parlaklik;
    float koniAcisi; // Spot ışık için

    // Shader Uniform Konumları (GPU belleğindeki indeksler)
    int aktifLoc;
    int tipLoc;
    int posLoc;
    int hedefLoc;
    int renkLoc;
    int parlaklikLoc;
    int koniAcisiLoc;

    Isik()
        : aktif(true),
          tip(IsikTipi::NOKTASAL),
          pozisyon((Vector3){ 0.0f, 5.0f, 0.0f }),
          hedef((Vector3){ 0.0f, 0.0f, 0.0f }),
          renk(WHITE),
          parlaklik(1.0f),
          koniAcisi(45.0f),
          aktifLoc(-1), tipLoc(-1), posLoc(-1), hedefLoc(-1), renkLoc(-1), parlaklikLoc(-1), koniAcisiLoc(-1) {}
};

// Shader'a ışık verilerini gönderen yardımcı fonksiyonlar
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
    
    float cosKoni = cosf(isik.koniAcisi * DEG2RAD);
    SetShaderValue(shader, isik.koniAcisiLoc, &cosKoni, SHADER_UNIFORM_FLOAT);
}
