#pragma once

#include "raylib.h"
#include "raymath.h"

// 3D uzaydaki bir nesneyi temsil eden temel yapı
struct Entity {
    Vector3 pozisyon; // Konum (X, Y, Z)
    Vector3 rotasyon; // Dönüş açıları (Pitch, Yaw, Roll - Derece cinsinden)
    Vector3 olcek;    // Ölçek/Boyut çarpanı (Scale X, Y, Z)
    
    Color renk;       // Gövde rengi
    Color cizgiRengi; // Kenar çizgisi rengi
    bool cizgiler;    // Kenar çizgileri çizilsin mi?

    // Varsayılan kurucu
    Entity()
        : pozisyon((Vector3){ 0.0f, 1.0f, 0.0f }),
          rotasyon((Vector3){ 0.0f, 0.0f, 0.0f }),
          olcek((Vector3){ 2.0f, 2.0f, 2.0f }),
          renk((Color){ 140, 145, 155, 255 }),
          cizgiRengi((Color){ 230, 235, 245, 255 }),
          cizgiler(true) {}

    // Nesnenin sınırlayıcı kutusu (Tıklama/Picking için)
    BoundingBox GetBoundingBox() const {
        Vector3 min = {
            pozisyon.x - olcek.x / 2.0f,
            pozisyon.y - olcek.y / 2.0f,
            pozisyon.z - olcek.z / 2.0f
        };
        Vector3 max = {
            pozisyon.x + olcek.x / 2.0f,
            pozisyon.y + olcek.y / 2.0f,
            pozisyon.z + olcek.z / 2.0f
        };
        return { min, max };
    }

    // Nesneyi 3D uzayda konum, rotasyon ve ölçek dönüşümleriyle çizen fonksiyon
    void Ciz(bool seciliMi = false) const {
        rlPushMatrix();
            // 1. Konuma taşı (Translation)
            rlTranslatef(pozisyon.x, pozisyon.y, pozisyon.z);

            // 2. Açılara göre döndür (Rotation: Y -> X -> Z)
            rlRotatef(rotasyon.y, 0.0f, 1.0f, 0.0f);
            rlRotatef(rotasyon.x, 1.0f, 0.0f, 0.0f);
            rlRotatef(rotasyon.z, 0.0f, 0.0f, 1.0f);

            // 3. Ölçeklendir (Scale)
            rlScalef(olcek.x, olcek.y, olcek.z);

            // Birim küp (1x1x1) çizilir, ölçek matrisi bunu gerçek boyutuna dönüştürür
            DrawCube((Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 1.0f, renk);

            if (seciliMi) {
                DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 1.0f, ORANGE);
            } else if (cizgiler) {
                DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 1.0f, cizgiRengi);
            }
        rlPopMatrix();
    }
};
