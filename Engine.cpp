#include "Engine.h"
#include "raymath.h"
#include "rlgl.h"

Engine::Engine()
    : ekranGenisligi(1280), ekranYuksekligi(720), calisiyorMu(false),
      kameraHizi(10.0f), kameraYaw(45.0f), kameraPitch(-30.0f),
      seciliNesneIndeksi(-1), aktifMod(TransformModu::KONUM),
      suruklenenEksen(EksenTipi::YOK), sonFarePozisyonu((Vector2){ 0.0f, 0.0f }) {

    // Varsayılan FOV 70 ile perspektif kamera
    kamera = { 0 };
    kamera.position = (Vector3){ 7.0f, 6.0f, 7.0f };
    kamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    kamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    kamera.fovy = 70.0f;
    kamera.projection = CAMERA_PERSPECTIVE;

    // Başlangıç bakış açısı
    Vector3 bakisYonu = Vector3Normalize(Vector3Subtract(kamera.target, kamera.position));
    kameraPitch = asinf(bakisYonu.y) * RAD2DEG;
    kameraYaw   = atan2f(bakisYonu.x, bakisYonu.z) * RAD2DEG;
}

Engine::~Engine() {
    Shutdown();
}

void Engine::Init(int genislik, int yukseklik, const char* baslik) {
    // 1. Kenarlıksız pencere ve Anti-Aliasing (MSAA 4X)
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_MSAA_4X_HINT);

    // 2. Birincil monitörün çözünürlüğünü algıla
    int monitor = GetCurrentMonitor();
    ekranGenisligi = GetMonitorWidth(monitor);
    ekranYuksekligi = GetMonitorHeight(monitor);

    // 3. Standart pencere oluştur
    InitWindow(ekranGenisligi, ekranYuksekligi, baslik);
    SetWindowPosition(0, 0);

    // 4. FPS sınırını kaldır
    SetTargetFPS(0);
    calisiyorMu = true;
}

void Engine::Shutdown() {
    if (calisiyorMu) {
        CloseWindow();
        calisiyorMu = false;
    }
}

void Engine::NesneEkle(const Entity& yeniNesne) {
    nesneler.push_back(yeniNesne);
}

void Engine::Run() {
    while (calisiyorMu && !WindowShouldClose()) {
        Update();
        Render();
    }
}

// Ray ile Gizmo eksenine tıklanıp tıklanmadığını algıla
EksenTipi Engine::AlgilaGizmoEkseni(const Entity& nesne, Ray ray) {
    Vector3 pos = nesne.pozisyon;
    float mesafe = 2.5f;
    float yaricap = 0.35f;

    // X Ekseni Ucu (Kırmızı Küre)
    BoundingBox xBox = {
        (Vector3){ pos.x + mesafe - yaricap, pos.y - yaricap, pos.z - yaricap },
        (Vector3){ pos.x + mesafe + yaricap, pos.y + yaricap, pos.z + yaricap }
    };
    if (GetRayCollisionBox(ray, xBox).hit) return EksenTipi::X;

    // Y Ekseni Ucu (Yeşil Küre)
    BoundingBox yBox = {
        (Vector3){ pos.x - yaricap, pos.y + mesafe - yaricap, pos.z - yaricap },
        (Vector3){ pos.x + yaricap, pos.y + mesafe + yaricap, pos.z + yaricap }
    };
    if (GetRayCollisionBox(ray, yBox).hit) return EksenTipi::Y;

    // Z Ekseni Ucu (Mavi Küre)
    BoundingBox zBox = {
        (Vector3){ pos.x - yaricap, pos.y - yaricap, pos.z + mesafe - yaricap },
        (Vector3){ pos.x + yaricap, pos.y + yaricap, pos.z + mesafe + yaricap }
    };
    if (GetRayCollisionBox(ray, zBox).hit) return EksenTipi::Z;

    // Merkez Nokta (Tüm eksenler)
    BoundingBox merkezBox = {
        (Vector3){ pos.x - yaricap, pos.y - yaricap, pos.z - yaricap },
        (Vector3){ pos.x + yaricap, pos.y + yaricap, pos.z + yaricap }
    };
    if (GetRayCollisionBox(ray, merkezBox).hit) return EksenTipi::MERKEZ;

    return EksenTipi::YOK;
}

void Engine::Update() {
    float dt = GetFrameTime();
    Vector2 farePos = GetMousePosition();

    // 1. Mouse Tekerleği ile kamera hızını değiştirme
    float tekerlek = GetMouseWheelMove();
    if (tekerlek != 0.0f) {
        kameraHizi += tekerlek * 2.5f;
        if (kameraHizi < 1.0f) kameraHizi = 1.0f;
        if (kameraHizi > 50.0f) kameraHizi = 50.0f;
    }

    // 2. FOV Ayarı ([ ve ] tuşları)
    if (IsKeyDown(KEY_LEFT_BRACKET) && kamera.fovy > 30.0f)  kamera.fovy -= 30.0f * dt;
    if (IsKeyDown(KEY_RIGHT_BRACKET) && kamera.fovy < 110.0f) kamera.fovy += 30.0f * dt;

    // 3. Kamera Hareketi (Sağ Tık Basılıyken)
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        DisableCursor();
        GetMouseDelta();
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 mouseDelta = GetMouseDelta();
        kameraYaw   -= mouseDelta.x * 0.15f;
        kameraPitch -= mouseDelta.y * 0.15f;

        if (kameraPitch > 89.0f)  kameraPitch = 89.0f;
        if (kameraPitch < -89.0f) kameraPitch = -89.0f;

        float yawRad   = kameraYaw * DEG2RAD;
        float pitchRad = kameraPitch * DEG2RAD;

        Vector3 ileri = {
            cosf(pitchRad) * sinf(yawRad),
            sinf(pitchRad),
            cosf(pitchRad) * cosf(yawRad)
        };
        ileri = Vector3Normalize(ileri);

        Vector3 sag = Vector3Normalize(Vector3CrossProduct(ileri, (Vector3){ 0.0f, 1.0f, 0.0f }));
        Vector3 yukari = { 0.0f, 1.0f, 0.0f };

        float anlikHiz = kameraHizi * dt;
        if (IsKeyDown(KEY_W)) kamera.position = Vector3Add(kamera.position, Vector3Scale(ileri, anlikHiz));
        if (IsKeyDown(KEY_S)) kamera.position = Vector3Subtract(kamera.position, Vector3Scale(ileri, anlikHiz));
        if (IsKeyDown(KEY_D)) kamera.position = Vector3Add(kamera.position, Vector3Scale(sag, anlikHiz));
        if (IsKeyDown(KEY_A)) kamera.position = Vector3Subtract(kamera.position, Vector3Scale(sag, anlikHiz));
        if (IsKeyDown(KEY_E)) kamera.position = Vector3Add(kamera.position, Vector3Scale(yukari, anlikHiz));
        if (IsKeyDown(KEY_Q)) kamera.position = Vector3Subtract(kamera.position, Vector3Scale(yukari, anlikHiz));

        kamera.target = Vector3Add(kamera.position, ileri);
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        EnableCursor();
    }

    // 4. Standart Editör Modu & Fare ile Eksen Sürükleme (Sağ tık basılı DEĞİLKEN)
    if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        // Mod Değiştirme Kısayolları (W: Konum, E: Rotasyon, R: Ölçek)
        if (IsKeyPressed(KEY_W)) aktifMod = TransformModu::KONUM;
        if (IsKeyPressed(KEY_E)) aktifMod = TransformModu::ROTASYON;
        if (IsKeyPressed(KEY_R)) aktifMod = TransformModu::OLCEK;

        // Sol Tıka Basıldığı An
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Ray ray = GetMouseRay(farePos, kamera);
            sonFarePozisyonu = farePos;

            // Önce seçili nesnenin gizmo eksenine mi tıkladık?
            if (seciliNesneIndeksi != -1 && seciliNesneIndeksi < static_cast<int>(nesneler.size())) {
                suruklenenEksen = AlgilaGizmoEkseni(nesneler[seciliNesneIndeksi], ray);
            }

            // Eğer eksene tıklamadıysak yeni bir nesne seçmeyi dene
            if (suruklenenEksen == EksenTipi::YOK) {
                seciliNesneIndeksi = -1;
                float enYakinMesafe = 999999.0f;

                for (int i = 0; i < static_cast<int>(nesneler.size()); i++) {
                    RayCollision carpi = GetRayCollisionBox(ray, nesneler[i].GetBoundingBox());
                    if (carpi.hit && carpi.distance < enYakinMesafe) {
                        enYakinMesafe = carpi.distance;
                        seciliNesneIndeksi = i;
                    }
                }
            }
        }

        // Sol Tık Basılıyken Fareyi Sürükleme (Eksen Manipülasyonu)
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && suruklenenEksen != EksenTipi::YOK) {
            Vector2 fareDelta = { farePos.x - sonFarePozisyonu.x, farePos.y - sonFarePozisyonu.y };
            sonFarePozisyonu = farePos;

            if (seciliNesneIndeksi != -1 && seciliNesneIndeksi < static_cast<int>(nesneler.size())) {
                Entity& secili = nesneler[seciliNesneIndeksi];
                float hassasiyet = 0.05f;
                float suruklemeMiktari = (fareDelta.x - fareDelta.y) * hassasiyet;

                // Shift ile daha hassas ayar
                if (IsKeyDown(KEY_LEFT_SHIFT)) suruklemeMiktari *= 0.25f;

                if (aktifMod == TransformModu::KONUM) {
                    if (suruklenenEksen == EksenTipi::X) secili.pozisyon.x += fareDelta.x * hassasiyet;
                    if (suruklenenEksen == EksenTipi::Y) secili.pozisyon.y -= fareDelta.y * hassasiyet;
                    if (suruklenenEksen == EksenTipi::Z) secili.pozisyon.z += (fareDelta.x + fareDelta.y) * hassasiyet;
                }
                else if (aktifMod == TransformModu::ROTASYON) {
                    float rotSens = 0.8f;
                    if (suruklenenEksen == EksenTipi::X) secili.rotasyon.x -= fareDelta.y * rotSens;
                    if (suruklenenEksen == EksenTipi::Y) secili.rotasyon.y += fareDelta.x * rotSens;
                    if (suruklenenEksen == EksenTipi::Z) secili.rotasyon.z += (fareDelta.x - fareDelta.y) * rotSens;
                }
                else if (aktifMod == TransformModu::OLCEK) {
                    if (suruklenenEksen == EksenTipi::X) secili.olcek.x = fmaxf(0.1f, secili.olcek.x + fareDelta.x * hassasiyet);
                    if (suruklenenEksen == EksenTipi::Y) secili.olcek.y = fmaxf(0.1f, secili.olcek.y - fareDelta.y * hassasiyet);
                    if (suruklenenEksen == EksenTipi::Z) secili.olcek.z = fmaxf(0.1f, secili.olcek.z + (fareDelta.x + fareDelta.y) * hassasiyet);
                    if (suruklenenEksen == EksenTipi::MERKEZ) {
                        float artis = suruklemeMiktari;
                        secili.olcek.x = fmaxf(0.1f, secili.olcek.x + artis);
                        secili.olcek.y = fmaxf(0.1f, secili.olcek.y + artis);
                        secili.olcek.z = fmaxf(0.1f, secili.olcek.z + artis);
                    }
                }
            }
        }

        // Sol tık bırakıldığında sürüklemeyi bitir
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            suruklenenEksen = EksenTipi::YOK;
        }
    }

    // Seçimi iptal et veya çık
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (seciliNesneIndeksi != -1) {
            seciliNesneIndeksi = -1;
            suruklenenEksen = EksenTipi::YOK;
        } else {
            calisiyorMu = false;
        }
    }
}

void Engine::CizTransformGizmo(const Entity& nesne) {
    Vector3 pos = nesne.pozisyon;
    float gizmoUzunluk = 2.5f;

    // Sürüklenen eksen sarı parlar
    Color xRenk = (suruklenenEksen == EksenTipi::X) ? YELLOW : RED;
    Color yRenk = (suruklenenEksen == EksenTipi::Y) ? YELLOW : GREEN;
    Color zRenk = (suruklenenEksen == EksenTipi::Z) ? YELLOW : BLUE;
    Color mRenk = (suruklenenEksen == EksenTipi::MERKEZ) ? YELLOW : WHITE;

    // Merkez Küre (Genel Seçim / Bütünsel Ölçekleme Tutamacı)
    DrawSphere(pos, 0.25f, mRenk);

    if (aktifMod == TransformModu::KONUM) {
        // X Ekseni (Kırmızı Ok)
        DrawLine3D(pos, (Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, xRenk);
        DrawSphere((Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, 0.25f, xRenk);

        // Y Ekseni (Yeşil Ok)
        DrawLine3D(pos, (Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, yRenk);
        DrawSphere((Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, 0.25f, yRenk);

        // Z Ekseni (Mavi Ok)
        DrawLine3D(pos, (Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, zRenk);
        DrawSphere((Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, 0.25f, zRenk);
    } 
    else if (aktifMod == TransformModu::ROTASYON) {
        DrawCircle3D(pos, 2.5f, (Vector3){ 1.0f, 0.0f, 0.0f }, 90.0f, xRenk);
        DrawCircle3D(pos, 2.5f, (Vector3){ 0.0f, 1.0f, 0.0f }, 0.0f, yRenk);
        DrawCircle3D(pos, 2.5f, (Vector3){ 0.0f, 0.0f, 1.0f }, 90.0f, zRenk);

        // Tutamaç Küreleri
        DrawSphere((Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, 0.25f, xRenk);
        DrawSphere((Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, 0.25f, yRenk);
        DrawSphere((Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, 0.25f, zRenk);
    } 
    else if (aktifMod == TransformModu::OLCEK) {
        DrawLine3D(pos, (Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, xRenk);
        DrawCube((Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, 0.35f, 0.35f, 0.35f, xRenk);

        DrawLine3D(pos, (Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, yRenk);
        DrawCube((Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, 0.35f, 0.35f, 0.35f, yRenk);

        DrawLine3D(pos, (Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, zRenk);
        DrawCube((Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, 0.35f, 0.35f, 0.35f, zRenk);
    }
}

void Engine::CizSagAltGizmo() {
    Vector3 kameraYonu = Vector3Normalize(Vector3Subtract(kamera.target, kamera.position));
    
    Camera3D gizmoKamera = { 0 };
    gizmoKamera.position = Vector3Scale(kameraYonu, -3.5f);
    gizmoKamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    gizmoKamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    gizmoKamera.fovy = 50.0f;
    gizmoKamera.projection = CAMERA_PERSPECTIVE;

    int gizmoBoyut = 120;
    int gizmoX = ekranGenisligi - gizmoBoyut - 20;
    int gizmoY = ekranYuksekligi - gizmoBoyut - 20;

    DrawRectangleRounded((Rectangle){ (float)gizmoX, (float)gizmoY, (float)gizmoBoyut, (float)gizmoBoyut }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
    DrawRectangleRoundedLines((Rectangle){ (float)gizmoX, (float)gizmoY, (float)gizmoBoyut, (float)gizmoBoyut }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

    BeginScissorMode(gizmoX, gizmoY, gizmoBoyut, gizmoBoyut);
    BeginMode3D(gizmoKamera);

        DrawLine3D((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 1.0f, 0.0f, 0.0f }, RED);
        DrawSphere((Vector3){ 1.0f, 0.0f, 0.0f }, 0.1f, RED);

        DrawLine3D((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 1.0f, 0.0f }, GREEN);
        DrawSphere((Vector3){ 0.0f, 1.0f, 0.0f }, 0.1f, GREEN);

        DrawLine3D((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, 1.0f }, BLUE);
        DrawSphere((Vector3){ 0.0f, 0.0f, 1.0f }, 0.1f, BLUE);

        DrawSphere((Vector3){ 0.0f, 0.0f, 0.0f }, 0.06f, WHITE);

    EndMode3D();
    EndScissorMode();

    DrawText("X", gizmoX + gizmoBoyut - 20, gizmoY + gizmoBoyut / 2 - 5, 12, RED);
    DrawText("Y", gizmoX + gizmoBoyut / 2 - 4, gizmoY + 8, 12, GREEN);
    DrawText("Z", gizmoX + 12, gizmoY + gizmoBoyut - 20, 12, BLUE);
}

void Engine::Render() {
    BeginDrawing();
        ClearBackground((Color){ 30, 31, 36, 255 });

        // --- 3D VIEWPORT ---
        BeginMode3D(kamera);

            DrawGrid(24, 1.0f);

            // Sahnedeki Varlıkları Çiz
            for (int i = 0; i < static_cast<int>(nesneler.size()); i++) {
                bool secili = (i == seciliNesneIndeksi);
                nesneler[i].Ciz(secili);

                if (secili) {
                    CizTransformGizmo(nesneler[i]);
                }
            }

        EndMode3D();
        // --- 3D VIEWPORT BİTİŞİ ---

        // --- 2D ARAYÜZ (HUD) ---
        
        // Sol Üst Bilgi Paneli
        DrawRectangleRounded((Rectangle){ 20.0f, 20.0f, 520.0f, 115.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ 20.0f, 20.0f, 520.0f, 115.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        DrawText("VIENTO ENGINE 3D - VIEWPORT", 35, 30, 18, (Color){ 240, 245, 255, 255 });
        DrawText("Fare Sol Tik: Obje Sec & Eksenden Tut Surukle", 35, 54, 13, (Color){ 170, 180, 200, 255 });
        
        // Aktif Mod Vurgusu (W, E, R tuşları)
        const char* modAdi = (aktifMod == TransformModu::KONUM) ? "[W] Konum (Translate)" :
                             (aktifMod == TransformModu::ROTASYON) ? "[E] Rotasyon (Rotate)" : "[R] Olcek (Scale)";
        Color modRengi = (aktifMod == TransformModu::KONUM) ? SKYBLUE :
                         (aktifMod == TransformModu::ROTASYON) ? GREEN : ORANGE;

        DrawText(TextFormat("Aktif Mod: %s", modAdi), 35, 74, 14, modRengi);
        DrawText("Kamera: Sag Tik + WASD/QE | Tekerlek: Hiz | ESC: Cikis", 35, 94, 12, (Color){ 255, 205, 80, 255 });

        // Sol Alt: Seçili Obje Transform Paneli
        DrawRectangleRounded((Rectangle){ 20.0f, (float)(ekranYuksekligi - 85), 490.0f, 65.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ 20.0f, (float)(ekranYuksekligi - 85), 490.0f, 65.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        if (seciliNesneIndeksi != -1) {
            const Entity& secili = nesneler[seciliNesneIndeksi];
            DrawText(TextFormat("SECILI OBJE #%i", seciliNesneIndeksi), 35, ekranYuksekligi - 75, 14, ORANGE);
            DrawText(TextFormat("Konum: (%.1f, %.1f, %.1f) | Rot: (%.0f, %.0f, %.0f) | Boyut: (%.1f, %.1f, %.1f)", 
                     secili.pozisyon.x, secili.pozisyon.y, secili.pozisyon.z,
                     secili.rotasyon.x, secili.rotasyon.y, secili.rotasyon.z,
                     secili.olcek.x, secili.olcek.y, secili.olcek.z), 35, ekranYuksekligi - 52, 12, (Color){ 220, 230, 245, 255 });
        } else {
            DrawText("Secili Obje: Yok (Secmek icin kupe sol tiklayin)", 35, ekranYuksekligi - 60, 14, (Color){ 130, 140, 160, 255 });
        }

        // Sağ Alt: 3D Yön Pusulası
        CizSagAltGizmo();

        // Sağ Üst: FPS Paneli
        int fpsPanelWidth = 230;
        DrawRectangleRounded((Rectangle){ (float)(ekranGenisligi - fpsPanelWidth - 20), 20.0f, (float)fpsPanelWidth, 75.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ (float)(ekranGenisligi - fpsPanelWidth - 20), 20.0f, (float)fpsPanelWidth, 75.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        DrawText(TextFormat("FPS: %i", GetFPS()), ekranGenisligi - fpsPanelWidth, 30, 18, (Color){ 80, 255, 120, 255 });
        DrawText(TextFormat("Frame Time: %.2f ms", GetFrameTime() * 1000.0f), ekranGenisligi - fpsPanelWidth, 50, 13, (Color){ 255, 205, 80, 255 });
        DrawText(TextFormat("Cozunurluk: %ix%i", ekranGenisligi, ekranYuksekligi), ekranGenisligi - fpsPanelWidth, 68, 12, (Color){ 170, 180, 200, 255 });

    EndDrawing();
}
