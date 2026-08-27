# 🌪️ Viento Engine

**Viento Engine**, C++ ve Raylib altyapısı üzerine inşa edilmiş; modern grafik boru hattı (Graphics Pipeline), gerçek zamanlı dinamik gölgeler (Real-Time Shadow Mapping), fiziksel ışıklandırma modelleri ve etkileşimli 3D sahne editörü içeren modüler bir 3D oyun motoru projesidir.

---

## 🏗️ Mimari ve Teknik Özellikler

### 1. 2-Pass Dinamik Gölge Boru Hattı (Shadow Mapping Pipeline)
* **2048x2048 HD Z-Buffer:** `rlLoadFramebuffer` ve `rlLoadTextureDepth` (`DEPTH_COMPONENT_24BIT`) kullanılarak donanımsal derinlik dokusu oluşturulur.
* **1. Geçiş (Shadow Pass):** Işık kamerasının perspektifinden sahne derinlik tamponuna yazılır ve `lightVP` (Light View-Projection) matrisi kaydedilir.
* **2. Geçiş (Shaded Scene Pass):** Kamera uzayında `lightVP` matrisi ile her bir pikselin ışık görüşünde olup olmadığı hesaplanır.
* **16-Sample PCF (Percentage-Closer Filtering):** Sert pikselleşme yerine yumuşak geçişli gölge kenarları (Penumbra) render edilir.
* **Çoklu Projeksiyon Desteği:**
  * **Güneş Işığı (Directional):** Paralel ışınlar için Ortografik projeksiyon.
  * **Noktasal Işık (Point Light):** Işık merkezinden dışarıya doğru radyal perspektif gölge.
  * **Spot Işık:** Koni açısına odaklı perspektif projeksiyon.

### 2. Fiziksel & Optik Işıklandırma (Physical Lighting)
* **Ters Kare Kanunu (Inverse-Square Law Falloff):** Işık kaynağından uzaklaştıkça karesiyle orantılı fiziksel sönümlenme.
* **Attenuation Radius:** Işığın etkili olduğu küresel etki yarıçapı sınırı.
* **Inner & Outer Cone Falloff:** Spot ışıklarda iç açıdan dış açıya doğru pürüzsüz ve kademeli sönümlenme.
* **Blinn-Phong Parlama Modeli:** Yüzey normalleri ve yarım vektör (Half-Vector) ile yüksek kaliteli speküler parlama.

### 3. Etkileşimli 3D Sahne Editörü & Araçlar
* **Serbest Dolaşım Kamerası (Free-Fly Camera):** Sağ Tık + `WASD` / `QE` ile tam 3D eksen serbestliği, fare tekerleğiyle ayarlanabilir hareket hızı.
* **Etkileşimli 3D Transform Gizmo:** 2D ekran vektör izdüşümü ile farenin sol tık hareketiyle X (Kırmızı), Y (Yeşil), Z (Mavi) eksenlerinde hassas Taşıma (`W`), Döndürme (`E`) ve Ölçekleme (`R`).
* **Senkronize Tel Kafes & Seçim Çerçevesi:** `rlEnableWireMode` ile nesnenin kendi GPU mesh geometrisi ve dönüşüm matrisiyle %100 kenetlenen canlı turuncu seçim çerçevesi.
* **Üst Menü (+ Add):**
  * **Geometriler (Shapes):** Küp, Küre, Silindir, Piramit, Zemin Düzlemi (Plane).
  * **Işık Kaynakları (Lights):** Noktasal Işık (Point), Spot Işık, Güneş Işığı (Directional).

---

## 📂 Dosya Yapısı

| Dosya | Açıklama |
| :--- | :--- |
| **`Engine.h`** | Çekirdek motor sınıfı tanımları, render pipeline ve FBO durum değişkenleri. |
| **`Engine.cpp`** | GLSL 330 Shader kodları, 2-Pass gölge döngüsü, Gizmo matematiği ve kullanıcı arayüzü. |
| **`Entity.h`** | 3D nesnelerin konum, rotasyon, ölçek ve AABB sınırlayıcı kutu verilerini tutan varlık yapısı. |
| **`Light.h`** | Fiziksel ışık parametreleri (Attenuation, Inner/Outer Cone) ve GPU uniform aktarıcısı. |
| **`main.cpp`** | Motorun başlatıldığı ve ana döngünün çağrıldığı giriş noktası. |

---

## 🚀 Derleme ve Çalıştırma

Projeyi derlemek ve çalıştırmak için:

1. **`Derle_Ve_Baslat.bat`** dosyasını çalıştırın.
2. Derleme `g++` (MinGW-w64) ve Raylib kütüphanesi üzerinden optimize bayraklarla gerçekleştirilerek motor başlatılır.

---

## 🎮 Editör Kontrolleri

* **Kamera Hareketi:** `Sağ Tık + WASD` (İleri/Geri/Sol/Sağ), `Q / E` (Aşağı/Yukarı)
* **Kamera Hızı:** `Fare Tekerleği`
* **Görüş Açısı (FOV):** `[` ve `]` tuşları
* **Mod Değişimi:** `W` (Konum), `E` (Rotasyon), `R` (Ölçek)
* **Hassas Hareket:** `Shift` tuşuna basılı tutarak sürükleme
* **Nesne Seçimi:** `Fare Sol Tık`
* **Nesne Silme:** `Delete` veya `Backspace`
* **Menü / Çıkış:** `ESC`
