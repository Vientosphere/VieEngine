# 🌪️ Viento Engine

**Viento Engine**, C++ dili ile geliştirilen, hafif ve modüler bir 2D/3D oyun motoru projesidir.

---

## 📌 Genel Bakış & Hedefler

Projenin temel amacı; karmaşık ve ağır oyun motorlarının getirdiği yüklerden uzak, doğrudan bellek ve donanım kontrolünü temel alan sade bir motor mimarisi oluşturmaktır.

* **Render:** 2D ve 3D çizim işlemleri için **Raylib** kütüphanesi kullanılmaktadır.
* **Fizik:** 3D uzayda katı cisim (rigid body) ve çarpışma simülasyonları için **Bullet3 Physics** entegrasyonu hedeflenmektedir.
* **Dil & Standart:** C++ (MinGW-w64 derleyici altyapısı).

---

## ⚙️ Mevcut Özellikler (v0.03)

- [x] **Çekirdek Motor Mimarisi:** `Engine` sınıfı ve yaşam döngüsü (`Init`, `Update`, `Render`, `Shutdown`).
- [x] **Varlık (Entity) Sistemi:** Sahneye dinamik olarak nesne (`Entity`) ekleme ve toplu çizim döngüsü.
- [x] **3D Perspektif Kamera:** 3D uzay ve derinlik hesaplamaları.
- [x] **Girdi Yönetimi:** Klavye ile nesne hareketi (X, Y, Z eksenleri) ve anlık renk yönetimi.

---

## 🗺️ Planlanan Geliştirmeler (Roadmap)

- [x] **Serbest 3D Kamera (Free-Fly Camera):** Editör içinde serbest dolaşım.
- [ ] **Transform & Hiyerarşi:** Nesnelerin dönüş açıları (Rotation / Yaw-Pitch-Roll).
- [ ] **Bullet3 Entegrasyonu:** Yerçekimi, kütle ve katı cisim çarpışma hesaplamaları.

---

## 🚀 Derleme & Çalıştırma

Projeyi Windows üzerinde derlemek ve çalıştırmak için:

1. `C:\raylib` (w64devkit) kurulu olmalıdır.
2. `Derle_Ve_Baslat.bat` dosyası çalıştırılarak otomatik derleme (`g++`) ve başlatma sağlanır.

---

## 👤 Geliştirici
* **Engin Yılmaz (Viento)**
