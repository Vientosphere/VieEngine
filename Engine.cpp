#include "Engine.h"
#include "raymath.h"
#include "rlgl.h"

Engine::Engine()
    : ekranGenisligi(1280), ekranYuksekligi(720), calisiyorMu(false),
      kameraHizi(10.0f), kameraYaw(45.0f), kameraPitch(-30.0f),
      seciliNesneIndeksi(-1), aktifMod(TransformModu::KONUM) {

    // Varsayılan FOV 70 ile perspektif kamera
    kamera = { 0 };
    kamera.position = (Vector3){ 7.0f, 6.0f, 7.0f };
    kamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    kamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    kamera.fovy = 70.0f;
    kamera.projection = CAMERA_PERSPECTIVE;

    // Başlangıç açısı hesaplama
    Vector3 bakisYonu = Vector3Normalize(Vector3Subtract(kamera.target, kamera.position));
    kameraPitch = asinf(bakisYonu.y) * RAD2DEG;
    kameraYaw   = atan2f(bakisYonu.x, bakisYonu.z) * RAD2DEG;
}

Engine::~Engine() {
    Shutdown();
}

void Engine::Init(int genislik, int yukseklik, const char* baslik) {
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST | FLAG_MSAA_4X_HINT);

    int monitor = GetCurrentMonitor();
    ekranGenisligi = GetMonitorWidth(monitor);
    ekranYuksekligi = GetMonitorHeight(monitor);

    InitWindow(ekranGenisligi, ekranYuksekligi, baslik);
    SetWindowPosition(0, 0);
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

void Engine::Update() {
    float dt = GetFrameTime();

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

    // 4. Standart Editör Modu (Sağ tık basılı DEĞİLKEN)
    if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        // Obje Seçimi (Sol Tık Raycast)
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Ray ray = GetMouseRay(GetMousePosition(), kamera);
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

        // Mod Değiştirme Tuşları (W: Konum, E: Rotasyon, R: Ölçek)
        if (IsKeyPressed(KEY_W)) aktifMod = TransformModu::KONUM;
        if (IsKeyPressed(KEY_E)) aktifMod = TransformModu::ROTASYON;
        if (IsKeyPressed(KEY_R)) aktifMod = TransformModu::OLCEK;

        // 5. Seçili Nesneyi Numpad veya Yön Tuşlarıyla / I-J-K-L-U-O ile Değiştirme
        if (seciliNesneIndeksi != -1 && seciliNesneIndeksi < static_cast<int>(nesneler.size())) {
            Entity& secili = nesneler[seciliNesneIndeksi];
            float deltaSpeed = 3.0f * dt;

            // Shift'e basılırsa 4 kat daha hızlı değiştir
            if (IsKeyDown(KEY_LEFT_SHIFT)) deltaSpeed *= 4.0f;

            if (aktifMod == TransformModu::KONUM) {
                // X (Sağ-Sol: D / A) | Y (Yukarı-Aşağı: Space / C) | Z (İleri-Geri: W / S) -> Tuşlar I,K,J,L,U,O ile
                if (IsKeyDown(KEY_L)) secili.pozisyon.x += deltaSpeed;
                if (IsKeyDown(KEY_J)) secili.pozisyon.x -= deltaSpeed;
                if (IsKeyDown(KEY_I)) secili.pozisyon.z -= deltaSpeed;
                if (IsKeyDown(KEY_K)) secili.pozisyon.z += deltaSpeed;
                if (IsKeyDown(KEY_O)) secili.pozisyon.y += deltaSpeed;
                if (IsKeyDown(KEY_U)) secili.pozisyon.y -= deltaSpeed;
            } 
            else if (aktifMod == TransformModu::ROTASYON) {
                float rotSpeed = 90.0f * dt;
                if (IsKeyDown(KEY_LEFT_SHIFT)) rotSpeed *= 2.0f;

                if (IsKeyDown(KEY_J)) secili.rotasyon.y -= rotSpeed; // Yaw (Y Ekseni etrafında)
                if (IsKeyDown(KEY_L)) secili.rotasyon.y += rotSpeed;
                if (IsKeyDown(KEY_I)) secili.rotasyon.x -= rotSpeed; // Pitch (X Ekseni)
                if (IsKeyDown(KEY_K)) secili.rotasyon.x += rotSpeed;
                if (IsKeyDown(KEY_U)) secili.rotasyon.z -= rotSpeed; // Roll (Z Ekseni)
                if (IsKeyDown(KEY_O)) secili.rotasyon.z += rotSpeed;
            } 
            else if (aktifMod == TransformModu::OLCEK) {
                if (IsKeyDown(KEY_L)) secili.olcek.x += deltaSpeed;
                if (IsKeyDown(KEY_J)) secili.olcek.x = fmaxf(0.1f, secili.olcek.x - deltaSpeed);
                if (IsKeyDown(KEY_O)) secili.olcek.y += deltaSpeed;
                if (IsKeyDown(KEY_U)) secili.olcek.y = fmaxf(0.1f, secili.olcek.y - deltaSpeed);
                if (IsKeyDown(KEY_I)) secili.olcek.z += deltaSpeed;
                if (IsKeyDown(KEY_K)) secili.olcek.z = fmaxf(0.1f, secili.olcek.z - deltaSpeed);
                
                // Hepsini birden orantılı büyütüp küçültme (P / M)
                if (IsKeyDown(KEY_P)) {
                    secili.olcek.x += deltaSpeed;
                    secili.olcek.y += deltaSpeed;
                    secili.olcek.z += deltaSpeed;
                }
                if (IsKeyDown(KEY_M)) {
                    secili.olcek.x = fmaxf(0.1f, secili.olcek.x - deltaSpeed);
                    secili.olcek.y = fmaxf(0.1f, secili.olcek.y - deltaSpeed);
                    secili.olcek.z = fmaxf(0.1f, secili.olcek.z - deltaSpeed);
                }
            }

            // Objeyi sıfırla (R Tuşuna çift tıklama veya Home tuşu)
            if (IsKeyPressed(KEY_HOME)) {
                secili.rotasyon = (Vector3){ 0.0f, 0.0f, 0.0f };
                secili.olcek = (Vector3){ 2.0f, 2.0f, 2.0f };
            }
        }
    }

    // Seçimi iptal et veya çık
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (seciliNesneIndeksi != -1) {
            seciliNesneIndeksi = -1;
        } else {
            calisiyorMu = false;
        }
    }
}

void Engine::CizTransformGizmo(const Entity& nesne) {
    Vector3 pos = nesne.pozisyon;
    float gizmoUzunluk = 2.5f;

    if (aktifMod == TransformModu::KONUM) {
        // Konum Modu: Uçlarında ok olan doğrusal eksenler
        DrawLine3D(pos, (Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, RED);
        DrawSphere((Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, 0.15f, RED);

        DrawLine3D(pos, (Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, GREEN);
        DrawSphere((Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, 0.15f, GREEN);

        DrawLine3D(pos, (Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, BLUE);
        DrawSphere((Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, 0.15f, BLUE);
    } 
    else if (aktifMod == TransformModu::ROTASYON) {
        // Rotasyon Modu: 3 Boyutlu dönme çemberleri/halkaları
        DrawCircle3D(pos, 2.0f, (Vector3){ 1.0f, 0.0f, 0.0f }, 90.0f, RED);   // X etrafında halka
        DrawCircle3D(pos, 2.0f, (Vector3){ 0.0f, 1.0f, 0.0f }, 0.0f, GREEN);  // Y etrafında halka
        DrawCircle3D(pos, 2.0f, (Vector3){ 0.0f, 0.0f, 1.0f }, 90.0f, BLUE);  // Z etrafında halka
    } 
    else if (aktifMod == TransformModu::OLCEK) {
        // Ölçek Modu: Uçlarında küpler olan eksenler
        DrawLine3D(pos, (Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, RED);
        DrawCube((Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, 0.25f, 0.25f, 0.25f, RED);

        DrawLine3D(pos, (Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, GREEN);
        DrawCube((Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, 0.25f, 0.25f, 0.25f, GREEN);

        DrawLine3D(pos, (Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, BLUE);
        DrawCube((Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, 0.25f, 0.25f, 0.25f, BLUE);
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

        // --- 2D ARAYÜZ (HUD) & MOD BİLGİLERİ ---
        
        // Sol Üst Bilgi Paneli
        DrawRectangleRounded((Rectangle){ 20.0f, 20.0f, 520.0f, 115.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ 20.0f, 20.0f, 520.0f, 115.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        DrawText("VIENTO ENGINE 3D - VIEWPORT", 35, 30, 18, (Color){ 240, 245, 255, 255 });
        DrawText("Fare Sol Tik: Obje Sec | Sag Tik + WASD/QE: Kamera", 35, 54, 13, (Color){ 170, 180, 200, 255 });
        
        // Aktif Mod Vurgusu (W, E, R tuşları)
        const char* modAdi = (aktifMod == TransformModu::KONUM) ? "[W] Konum (Translate)" :
                             (aktifMod == TransformModu::ROTASYON) ? "[E] Rotasyon (Rotate)" : "[R] Olcek (Scale)";
        Color modRengi = (aktifMod == TransformModu::KONUM) ? SKYBLUE :
                         (aktifMod == TransformModu::ROTASYON) ? GREEN : ORANGE;

        DrawText(TextFormat("Aktif Mod: %s", modAdi), 35, 74, 14, modRengi);
        DrawText("Manipulasyon: I, K, J, L (X/Z) | U, O (Y) | Shift: Hizli", 35, 94, 12, (Color){ 255, 205, 80, 255 });

        // Sol Alt: Seçili Obje Transform Paneli
        DrawRectangleRounded((Rectangle){ 20.0f, (float)(ekranYuksekligi - 85), 480.0f, 65.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ 20.0f, (float)(ekranYuksekligi - 85), 480.0f, 65.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

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
