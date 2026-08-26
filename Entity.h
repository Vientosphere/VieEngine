#pragma once

#include "raylib.h"

// 3D dünyadaki tüm nesnelerin temel yapısı
struct Entity {
    Vector3 pozisyon; // Dünyadaki konumu (X, Y, Z)
    Vector3 boyut;    // Genişlik, Yükseklik, Derinlik
    Color renk;       // Nesnenin rengi
    bool cizgiler;    // Kenar çizgileri çizilsin mi?

    // Nesneyi ekrana çizen temel fonksiyon
    void Ciz() const {
        DrawCubeV(pozisyon, boyut, renk);
        if (cizgiler) {
            DrawCubeWiresV(pozisyon, boyut, WHITE);
        }
    }
};
