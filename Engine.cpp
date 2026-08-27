#include "Engine.h"
#include "raymath.h"

Engine::Engine()
    : ekranGenisligi(1280), ekranYuksekligi(720), calisiyorMu(false),
      kameraHizi(10.0f), kameraYaw(45.0f), kameraPitch(-30.0f),
      seciliNesneIndeksi(-1) {

    // Varsayılan FOV 70 ile perspektif kamera
    kamera = { 0 };
    kamera.position = (Vector3){ 7.0f, 6.0f, 7.0f };
    kamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    kamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    kamera.fovy = 70.0f;
    kamera.projection = CAMERA_PERSPECTIVE;

    // Başlangıç bakış açısından Yaw ve Pitch hesapla
    Vector3 bakisYonu = Vector3Normalize(Vector3Subtract(kamera.target, kamera.position));
    kameraPitch = asinf(bakisYonu.y) * RAD2DEG;
    kameraYaw   = atan2f(bakisYonu.x, bakisYonu.z) * RAD2DEG;
}

Engine::~Engine() {
    Shutdown();
}

void Engine::Init(int genislik, int yukseklik, const char* baslik) {
    // 1. Kenarlıksız pencere ve MSAA 4X bayrakları
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST | FLAG_MSAA_4X_HINT);

    // 2. Birincil monitörün çözünürlüğünü algıla
    int monitor = GetCurrentMonitor();
    ekranGenisligi = GetMonitorWidth(monitor);
    ekranYuksekligi = GetMonitorHeight(monitor);

    // 3. Monitör boyutunda tam ekran penceresiz (Borderless Fullscreen) aç
    InitWindow(ekranGenisligi, ekranYuksekligi, baslik);
    SetWindowPosition(0, 0);

    // 4. FPS sınırını tamamen kaldır (Uncapped FPS - Delta Time ile tam uyumlu)
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
    // [Delta Time Core]: Tüm hareket ve dönüş hesaplamalarını FPS'ten bağımsız saniye bazına bağlar
    float dt = GetFrameTime();

    // 1. Mouse Tekerleği ile kamera hızını değiştirme
    float tekerlek = GetMouseWheelMove();
    if (tekerlek != 0.0f) {
        kameraHizi += tekerlek * 2.5f;
        if (kameraHizi < 1.0f) kameraHizi = 1.0f;
        if (kameraHizi > 50.0f) kameraHizi = 50.0f;
    }

    // 2. FOV Ayarı (Delta time ile yumuşak geçiş)
    if (IsKeyDown(KEY_LEFT_BRACKET) && kamera.fovy > 30.0f)  kamera.fovy -= 30.0f * dt;
    if (IsKeyDown(KEY_RIGHT_BRACKET) && kamera.fovy < 110.0f) kamera.fovy += 30.0f * dt;

    // 3. Serbest Kamera Hareketi (Sağ tık basılıyken)
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        DisableCursor();
        GetMouseDelta(); // İlk tıklamadaki delta sıçramasını nötrle
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 mouseDelta = GetMouseDelta();
        
        // Fare dönüşü (Delta Time ile pürüzsüzleştirilmiş açı değişimi)
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

        // [Delta Time bazlı hareket]: 60 FPS'te de 1000 FPS'te de hız aynı kalır
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

    // 4. [3D Picking Core]: Fare ışını (Ray) ile sahnedeki en yakın nesneyi seçme
    if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
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
    }

    // Seçimi iptal et veya çıkış
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (seciliNesneIndeksi != -1) {
            seciliNesneIndeksi = -1;
        } else {
            calisiyorMu = false;
        }
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
        // Blender tarzı koyu gri editör arkaplanı
        ClearBackground((Color){ 30, 31, 36, 255 });

        // --- 3D VIEWPORT ---
        BeginMode3D(kamera);

            // Zemin Izgarası
            DrawGrid(24, 1.0f);

            // Sahnedeki Varlıkları Çiz
            for (int i = 0; i < static_cast<int>(nesneler.size()); i++) {
                bool secili = (i == seciliNesneIndeksi);
                nesneler[i].Ciz(secili);

                // Yalnızca obje seçiliyken üzerinde yerel 3D eksen çizgilerini (Gizmo) göster
                if (secili) {
                    Vector3 pos = nesneler[i].pozisyon;
                    float gizmoUzunluk = 2.5f;

                    DrawLine3D(pos, (Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, RED);
                    DrawSphere((Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, 0.15f, RED);

                    DrawLine3D(pos, (Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, GREEN);
                    DrawSphere((Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, 0.15f, GREEN);

                    DrawLine3D(pos, (Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, BLUE);
                    DrawSphere((Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, 0.15f, BLUE);
                }
            }

        EndMode3D();
        // --- 3D VIEWPORT BİTİŞİ ---

        // --- 2D ARAYÜZ (HUD) & OKUNAKLI TİPOGRAFİ ---
        
        // Sol Üst Bilgi Paneli
        DrawRectangleRounded((Rectangle){ 20.0f, 20.0f, 490.0f, 95.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ 20.0f, 20.0f, 490.0f, 95.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        DrawText("VIENTO ENGINE 3D - VIEWPORT", 35, 32, 18, (Color){ 240, 245, 255, 255 });
        DrawText("Fare Sol Tik: Obje Sec | Sag Tik + WASD/QE: Serbest Kamera", 35, 58, 14, (Color){ 170, 180, 200, 255 });
        DrawText(TextFormat("Kamera Hizi: %.1fx (Tekerlek) | FOV: %.0f ([ / ]) | ESC: Cikis", kameraHizi, kamera.fovy), 35, 80, 14, (Color){ 255, 205, 80, 255 });

        // Sol Alt: Seçili Obje Durum Paneli
        DrawRectangleRounded((Rectangle){ 20.0f, (float)(ekranYuksekligi - 65), 420.0f, 45.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ 20.0f, (float)(ekranYuksekligi - 65), 420.0f, 45.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        if (seciliNesneIndeksi != -1) {
            Vector3 pos = nesneler[seciliNesneIndeksi].pozisyon;
            DrawText(TextFormat("SECILI OBJE #%i | Konum: (%.2f, %.2f, %.2f)", seciliNesneIndeksi, pos.x, pos.y, pos.z), 35, ekranYuksekligi - 52, 14, (Color){ 255, 140, 50, 255 });
        } else {
            DrawText("Secili Obje: Yok (Secmek icin kupe sol tiklayin)", 35, ekranYuksekligi - 52, 14, (Color){ 130, 140, 160, 255 });
        }

        // Sağ Alt: 3D Yön Pusulası (World Gizmo)
        CizSagAltGizmo();

        // Sağ Üst: FPS, Delta Time & Çözünürlük Paneli
        int fpsPanelWidth = 230;
        DrawRectangleRounded((Rectangle){ (float)(ekranGenisligi - fpsPanelWidth - 20), 20.0f, (float)fpsPanelWidth, 75.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ (float)(ekranGenisligi - fpsPanelWidth - 20), 20.0f, (float)fpsPanelWidth, 75.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        DrawText(TextFormat("FPS: %i", GetFPS()), ekranGenisligi - fpsPanelWidth, 30, 18, (Color){ 80, 255, 120, 255 });
        DrawText(TextFormat("Frame Time: %.2f ms", GetFrameTime() * 1000.0f), ekranGenisligi - fpsPanelWidth, 50, 13, (Color){ 255, 205, 80, 255 });
        DrawText(TextFormat("Cozunurluk: %ix%i", ekranGenisligi, ekranYuksekligi), ekranGenisligi - fpsPanelWidth, 68, 12, (Color){ 170, 180, 200, 255 });

    EndDrawing();
}
