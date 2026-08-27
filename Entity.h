#pragma once

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

// Nesne Şekil Türleri
enum class EntityTipi {
    KUP,          // Cube
    KURE,         // Sphere
    SILINDIR,     // Cylinder
    UCGEN,        // Triangle / Pyramid
    DUZLEM,       // Plane / Ground
    ISIK_KAYNAGI  // Light Source Gizmo / Anchor
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

    // Işık bağlantı indeksi (-1 ise standart şekil, >= 0 ise ışık kaynağı nesnesi)
    int bagliIsikIndeksi;

    // Varsayılan kurucu
    Entity(EntityTipi nesneTipi = EntityTipi::KUP)
        : tip(nesneTipi),
          pozisyon((Vector3){ 0.0f, (nesneTipi == EntityTipi::DUZLEM ? 0.0f : (nesneTipi == EntityTipi::ISIK_KAYNAGI ? 4.0f : 1.0f)), 0.0f }),
          rotasyon((Vector3){ 0.0f, 0.0f, 0.0f }),
          olcek((Vector3){ (nesneTipi == EntityTipi::DUZLEM ? 10.0f : (nesneTipi == EntityTipi::ISIK_KAYNAGI ? 0.6f : 2.0f)), 
                           (nesneTipi == EntityTipi::DUZLEM ? 0.05f : (nesneTipi == EntityTipi::ISIK_KAYNAGI ? 0.6f : 2.0f)), 
                           (nesneTipi == EntityTipi::DUZLEM ? 10.0f : (nesneTipi == EntityTipi::ISIK_KAYNAGI ? 0.6f : 2.0f)) }),
          renk((nesneTipi == EntityTipi::ISIK_KAYNAGI ? (Color){ 255, 220, 60, 255 } : (Color){ 140, 145, 155, 255 })),
          cizgiRengi((nesneTipi == EntityTipi::ISIK_KAYNAGI ? (Color){ 255, 240, 150, 255 } : (Color){ 230, 235, 245, 255 })),
          cizgiler(true),
          bagliIsikIndeksi(-1) {}

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
            Color kenar = cizgiRengi;
            Color secimTuruncusu = (Color){ 255, 140, 0, 255 }; // Canlı & Parlak Turuncu Vurgu

            if (tip == EntityTipi::KUP) {
                DrawCube((Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 1.0f, govde);
                if (cizgiler) DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, 1.0f, 1.0f, kenar);
                
                if (seciliMi) {
                    DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 1.03f, 1.03f, 1.03f, secimTuruncusu);
                    DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 1.05f, 1.05f, 1.05f, secimTuruncusu);
                }
            }
            else if (tip == EntityTipi::KURE) {
                DrawSphere((Vector3){ 0.0f, 0.0f, 0.0f }, 0.5f, govde);
                if (cizgiler) DrawSphereWires((Vector3){ 0.0f, 0.0f, 0.0f }, 0.5f, 16, 16, kenar);
                
                if (seciliMi) {
                    DrawSphereWires((Vector3){ 0.0f, 0.0f, 0.0f }, 0.52f, 16, 16, secimTuruncusu);
                }
            }
            else if (tip == EntityTipi::SILINDIR) {
                DrawCylinder((Vector3){ 0.0f, -0.5f, 0.0f }, 0.5f, 0.5f, 1.0f, 20, govde);
                if (cizgiler) DrawCylinderWires((Vector3){ 0.0f, -0.5f, 0.0f }, 0.5f, 0.5f, 1.0f, 20, kenar);
                
                if (seciliMi) {
                    DrawCylinderWires((Vector3){ 0.0f, -0.5f, 0.0f }, 0.52f, 0.52f, 1.02f, 20, secimTuruncusu);
                }
            }
            else if (tip == EntityTipi::UCGEN) {
                DrawCylinder((Vector3){ 0.0f, -0.5f, 0.0f }, 0.0f, 0.6f, 1.0f, 4, govde);
                if (cizgiler) DrawCylinderWires((Vector3){ 0.0f, -0.5f, 0.0f }, 0.0f, 0.6f, 1.0f, 4, kenar);
                
                if (seciliMi) {
                    DrawCylinderWires((Vector3){ 0.0f, -0.5f, 0.0f }, 0.0f, 0.62f, 1.02f, 4, secimTuruncusu);
                }
            }
            else if (tip == EntityTipi::DUZLEM) {
                DrawCube((Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, 0.02f, 1.0f, govde);
                if (cizgiler) DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, 0.02f, 1.0f, kenar);
                
                if (seciliMi) {
                    DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 1.01f, 0.04f, 1.01f, secimTuruncusu);
                    DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 1.02f, 0.05f, 1.02f, secimTuruncusu);
                }
            }
            else if (tip == EntityTipi::ISIK_KAYNAGI) {
                // Işık Kaynağı Görselleştiricisi (Parlak Ampul / Yıldız Küresi)
                DrawSphere((Vector3){ 0.0f, 0.0f, 0.0f }, 0.4f, (Color){ 255, 230, 80, 255 });
                DrawSphereWires((Vector3){ 0.0f, 0.0f, 0.0f }, 0.45f, 8, 8, WHITE);
                
                // Seçiliyse sarı/turuncu ışık halkaları
                if (seciliMi) {
                    DrawSphereWires((Vector3){ 0.0f, 0.0f, 0.0f }, 0.6f, 10, 10, (Color){ 255, 180, 0, 255 });
                }
            }
        rlPopMatrix();
    }
};
