#pragma once

#include "raylib.h"
#include "raymath.h"

// Sahnedeki Varlık (Entity) Şekil Türleri
enum class EntityTipi {
    KUP,          // Küp (Cube)
    KURE,         // Küre (Sphere)
    SILINDIR,     // Silindir (Cylinder)
    UCGEN,        // Koni / Piramit (Cone / Pyramid)
    DUZLEM,       // Düzlem / Zemin (Plane / Ground)
    ISIK_KAYNAGI  // Işık Kaynağı Tutamacı (Light Anchor / Visualizer)
};

// 3D Uzaydaki Bir Varlığı (Entity) Temsil Eden Temel Veri Yapısı
struct Entity {
    EntityTipi tip;      // Geometri şekli
    Vector3 pozisyon;    // Dünya uzayı konumu (X, Y, Z)
    Vector3 rotasyon;    // Euler dönüş açıları (Pitch, Yaw, Roll - Derece cinsinden)
    Vector3 olcek;       // Boyutlandırma çarpanı (Scale X, Y, Z)

    Color renk;          // Katı gövde difüz rengi
    Color cizgiRengi;    // Tel kafes (Wireframe) kenar çizgisi rengi
    bool cizgiler;       // Tel kafes çizgileri çizilsin mi?

    // Işık bağlantı indeksi (-1 ise standart 3D nesne, >= 0 ise bağlı olduğu ışık kaynağı indeksi)
    int bagliIsikIndeksi;

    // Kurucu Fonksiyon (Tipine göre standart boyut ve konum ataması)
    Entity(EntityTipi nesneTipi = EntityTipi::KUP)
        : tip(nesneTipi),
          pozisyon((Vector3){
              0.0f,
              (nesneTipi == EntityTipi::DUZLEM ? 0.0f : (nesneTipi == EntityTipi::ISIK_KAYNAGI ? 4.0f : 1.0f)),
              0.0f
          }),
          rotasyon((Vector3){ 0.0f, 0.0f, 0.0f }),
          olcek((Vector3){
              (nesneTipi == EntityTipi::DUZLEM ? 24.0f : (nesneTipi == EntityTipi::ISIK_KAYNAGI ? 0.6f : 2.0f)),
              (nesneTipi == EntityTipi::DUZLEM ? 0.06f : (nesneTipi == EntityTipi::ISIK_KAYNAGI ? 0.6f : 2.0f)),
              (nesneTipi == EntityTipi::DUZLEM ? 24.0f : (nesneTipi == EntityTipi::ISIK_KAYNAGI ? 0.6f : 2.0f))
          }),
          renk((nesneTipi == EntityTipi::ISIK_KAYNAGI ? (Color){ 255, 220, 60, 255 } : (Color){ 140, 145, 155, 255 })),
          cizgiRengi((nesneTipi == EntityTipi::ISIK_KAYNAGI ? (Color){ 255, 240, 150, 255 } : (Color){ 220, 225, 235, 255 })),
          cizgiler(true),
          bagliIsikIndeksi(-1) {}

    // Fare ile nesne seçimi (Raycast Picking) için eksen hizalı sınırlayıcı kutu (AABB)
    BoundingBox GetBoundingBox() const {
        float yukseklik = (tip == EntityTipi::DUZLEM) ? 0.15f : olcek.y;
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
};
