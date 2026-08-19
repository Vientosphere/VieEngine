# 🌪️ VieEngine

**VieEngine**, C++ dili ile geliştirilen, hafif ve modüler bir 2D/3D oyun motoru projesidir.

---

## 📌 Genel Bakış & Hedefler

Projenin temel amacı; karmaşık ve ağır oyun motorlarının getirdiği yüklerden uzak, doğrudan bellek ve donanım kontrolünü temel alan sade bir motor mimarisi oluşturmaktır.

* **Render:** 2D ve 3D çizim işlemleri için **Raylib** kütüphanesi kullanılmaktadır.
* **Fizik:** 3D uzayda katı cisim (rigid body) ve çarpışma simülasyonları için **Bullet3 Physics** entegrasyonu hedeflenmektedir.
* **Dil & Standart:** C++ (MinGW-w64 derleyici altyapısı).

---

## ⚙️ Mevcut Özellikler (v0.01)

- [x] **Oyun Döngüsü (Game Loop):** Sabit 60 FPS hedefli ana çalışma döngüsü.
- [x] **Girdi Yönetimi (Input):** Gerçek zamanlı klavye dinleme (WASD, aksiyon tuşları).
- [x] **2D Render:** Ekran uzayında temel şekil ve metin çizimleri.
- [x] **Sınır Kontrolleri:** Pencere kenarlık çarpışma ve konum kısıtlama mantığı.

---

## 🗺️ Planlanan Geliştirmeler (Roadmap)

- [ ] **Modüler Mimari (OOP):** `Engine`, `Input`, `Renderer` ve `Entity` sınıflarına ayrıştırma.
- [ ] **3D Kamera & Koordinat Sistemi:** Perspektif kamera kontrolleri.
- [ ] **Bullet3 Entegrasyonu:** Yerçekimi, kütle ve katı cisim çarpışma hesaplamaları.

---

## 🚀 Derleme & Çalıştırma

Projeyi Windows üzerinde derlemek ve çalıştırmak için:

1. `C:\raylib` (w64devkit) kurulu olmalıdır.
2. `Derle_Ve_Baslat.bat` dosyası çalıştırılarak otomatik derleme (`g++`) ve başlatma sağlanır.

---

## 👤 Geliştirici
* **Viento**
