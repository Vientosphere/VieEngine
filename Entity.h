#pragma once

#include "raylib.h"

// 3D uzaydaki bir nesneyi temsil eden temel yapı
struct Entity {
    Vector3 pozisyon; // Konum (X, Y, Z)
    Vector3 boyut;    // Boyut (Genişlik, Yükseklik, Derinlik)
    Color renk;       // Gövde rengi
    Color cizgiRengi; // Kenar çizgisi rengi
    bool cizgiler;    // Kenar çizgileri çizilsin mi?

    // Nesnenin sınırlayıcı kutusu (Bounding Box - Tıklama ve seçim için)
    BoundingBox GetBoundingBox() const {
        Vector3 min = {
            pozisyon.x - boyut.x / 2.0f,
            pozisyon.y - boyut.y / 2.0f,
            pozisyon.z - boyut.z / 2.0f
        };
        Vector3 max = {
            pozisyon.x + boyut.x / 2.0f,
            pozisyon.y + boyut.y / 2.0f,
            pozisyon.z + boyut.z / 2.0f
        };
        return { min, max };
    }

    // Nesneyi 3D uzayda çizen fonksiyon
    void Ciz(bool seciliMi = false) const {
        DrawCube(pozisyon, boyut.x, boyut.y, boyut.z, renk);
        
        if (seciliMi) {
            // Seçili nesneye parlak turuncu/sarı kenarlık çizgisi
            DrawCubeWires(pozisyon, boyut.x, boyut.y, boyut.z, ORANGE);
        } else if (cizgiler) {
            DrawCubeWires(pozisyon, boyut.x, boyut.y, boyut.z, cizgiRengi);
        }
    }
};
