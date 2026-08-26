#pragma once

#include "raylib.h"

// 3D uzaydaki bir nesneyi temsil eden temel yapı
struct Entity {
    Vector3 pozisyon; // Konum (X, Y, Z)
    Vector3 boyut;    // Boyut (Genişlik, Yükseklik, Derinlik)
    Color renk;       // Gövde rengi
    Color cizgiRengi; // Kenar çizgisi rengi
    bool cizgiler;    // Kenar çizgileri çizilsin mi?

    // Nesneyi 3D uzayda çizen fonksiyon
    void Ciz() const {
        DrawCubeV(pozisyon, boyut, renk);
        if (cizgiler) {
            DrawCubeWiresV(pozisyon, boyut, cizgiRengi);
        }
    }
};
