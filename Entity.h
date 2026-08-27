#pragma once

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

// Nesne Şekil Türleri
enum class EntityTipi {
    KUP,      // Cube
    KURE,     // Sphere
    SILINDIR, // Cylinder
    UCGEN,    // Triangle / Pyramid
    DUZLEM    // Plane / Ground
};

// 3D uzaydaki bir nesneyi temsil eden temel yapı
struct Entity {
    EntityTipi tip;   // Nesnenin geometrisi
    Vector3 pozisyon; // Konum (X, Y, Z)
    Vector3 rotasyon; // Dönüş açıları (Pitch, Yaw, Roll)
    Vector3 olcek;    // Ölçek çarpanı (Scale X, Y, Z)
    
    Color renk;       // Gövde rengi
    Color cizgiRengi; // Kenar çizgisi rengi
    bool cizgiler;    // Kenar çizgileri çizilsin mi?

    // Varsayılan kurucu
    Entity(EntityTipi nesneTipi = EntityTipi::KUP)
        : tip(nesneTipi),
          pozisyon((Vector3){ 0.0f, (nesneTipi == EntityTipi::DUZLEM ? 0.0f : 1.0f), 0.0f }),
          rotasyon((Vector3){ 0.0f, 0.0f, 0.0f }),
          olcek((Vector3){ (nesneTipi == EntityTipi::DUZLEM ? 10.0f : 2.0f), (nesneTipi == EntityTipi::DUZLEM ? 0.05f : 2.0f), (nesneTipi == EntityTipi::DUZLEM ? 10.0f : 2.0f) }),
          renk((Color){ 140, 145, 155, 255 }),
          cizgiRengi((Color){ 230, 235, 245, 255 }),
          cizgiler(true) {}

    // Nesnenin sınırlayıcı kutusu (Tıklama/Picking için)
    BoundingBox GetBoundingBox() const {
        float yukseklik = (tip == EntityTipi::DUZLEM) ? 0.1f : olcek.y;
        Vector3 min = {
            pozisyon.x - olcek.x / 2.0f,
            pozisyon.y - yukseklik / 2.0f,
            pozisyon.z - olcek.z / 2.0f
        };
        Vector3 max = {
            pozisyon.x + olcek.x / 2.0f,
            pozisyon.y + yukseklik / 2.0f,
            pozisyon.z + olcek.z / 2.0f
        };
        return { min, max };
    }

    // Nesneyi türüne göre 3D uzayda çizen fonksiyon
    void Ciz(bool seciliMi = false) const {
        rlPushMatrix();
            rlTranslatef(pozisyon.x, pozisyon.y, pozisyon.z);
            rlRotatef(rotasyon.y, 0.0f, 1.0f, 0.0f);
            rlRotatef(rotasyon.x, 1.0f, 0.0f, 0.0f);
            rlRotatef(rotasyon.z, 0.0f, 0.0f, 1.0f);
            rlScalef(olcek.x, olcek.y, olcek.z);

            Color govde = renk;
            Color kenar = seciliMi ? ORANGE : cizgiRengi;

            if (tip == EntityTipi::KUP) {
                DrawCube((Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 1.0f, govde);
                if (seciliMi || cizgiler) DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 1.0f, kenar);
            }
            else if (tip == EntityTipi::KURE) {
                DrawSphere((Vector3){ 0.0f, 0.0f, 0.0f }, 0.5f, govde);
                if (seciliMi || cizgiler) DrawSphereWires((Vector3){ 0.0f, 0.0f, 0.0f }, 0.5f, 16, 16, kenar);
            }
            else if (tip == EntityTipi::SILINDIR) {
                DrawCylinder((Vector3){ 0.0f, -0.5f, 0.0f }, 0.5f, 0.5f, 1.0f, 20, govde);
                if (seciliMi || cizgiler) DrawCylinderWires((Vector3){ 0.0f, -0.5f, 0.0f }, 0.5f, 0.5f, 1.0f, 20, kenar);
            }
            else if (tip == EntityTipi::UCGEN) {
                DrawCylinder((Vector3){ 0.0f, -0.5f, 0.0f }, 0.0f, 0.6f, 1.0f, 4, govde);
                if (seciliMi || cizgiler) DrawCylinderWires((Vector3){ 0.0f, -0.5f, 0.0f }, 0.0f, 0.6f, 1.0f, 4, kenar);
            }
            else if (tip == EntityTipi::DUZLEM) {
                // Düzlem / Zemin (Geniş 2D Yüzey)
                DrawCube((Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, 0.02f, 1.0f, govde);
                if (seciliMi || cizgiler) DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, 0.02f, 1.0f, kenar);
            }
        rlPopMatrix();
    }
};
