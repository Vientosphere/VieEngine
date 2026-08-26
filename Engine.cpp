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
}

Engine::~Engine() {
    Shutdown();
}

void Engine::Init(int genislik, int yukseklik, const char* baslik) {
    ekranGenisligi = genislik;
    ekranYuksekligi = yukseklik;

    InitWindow(ekranGenisligi, ekranYuksekligi, baslik);
    SetTargetFPS(60);
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

    // 1. Mouse Tekerleği ile Unreal Engine tarzı kamera hızını değiştirme
    float tekerlek = GetMouseWheelMove();
    if (tekerlek != 0.0f) {
        kameraHizi += tekerlek * 2.5f;
        if (kameraHizi < 1.0f) kameraHizi = 1.0f;
        if (kameraHizi > 50.0f) kameraHizi = 50.0f;
    }

    // 2. FOV Ayarı (Klavyede [ ve ] tuşları ile FOV açısını daralt/genişlet)
    if (IsKeyDown(KEY_LEFT_BRACKET) && kamera.fovy > 30.0f)  kamera.fovy -= 20.0f * dt;
    if (IsKeyDown(KEY_RIGHT_BRACKET) && kamera.fovy < 110.0f) kamera.fovy += 20.0f * dt;

    // 3. Unreal Tarzı Kamera Hareketi (Sağ tık basılı tutulurken)
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        // Fareyi pencereye kilitle
        DisableCursor();

        Vector2 mouseDelta = GetMouseDelta();
        kameraYaw   += mouseDelta.x * 0.15f;
        kameraPitch -= mouseDelta.y * 0.15f;

        // Kameranın ters takla atmasını engelle
        if (kameraPitch > 89.0f)  kameraPitch = 89.0f;
        if (kameraPitch < -89.0f) kameraPitch = -89.0f;

        // Açıları radyana çevir ve ileri/sağ vektörlerini hesapla
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

        // WASD + Q/E Tuşları ile hareket
        float anlikHiz = kameraHizi * dt;
        if (IsKeyDown(KEY_W)) kamera.position = Vector3Add(kamera.position, Vector3Scale(ileri, anlikHiz));
        if (IsKeyDown(KEY_S)) kamera.position = Vector3Subtract(kamera.position, Vector3Scale(ileri, anlikHiz));
        if (IsKeyDown(KEY_D)) kamera.position = Vector3Add(kamera.position, Vector3Scale(sag, anlikHiz));
        if (IsKeyDown(KEY_A)) kamera.position = Vector3Subtract(kamera.position, Vector3Scale(sag, anlikHiz));
        if (IsKeyDown(KEY_E)) kamera.position = Vector3Add(kamera.position, Vector3Scale(yukari, anlikHiz)); // Yukarı çık
        if (IsKeyDown(KEY_Q)) kamera.position = Vector3Subtract(kamera.position, Vector3Scale(yukari, anlikHiz)); // Aşağı in

        // Hedefi güncelle
        kamera.target = Vector3Add(kamera.position, ileri);
    } else {
        EnableCursor();

        // 4. Mouse Sol Tık ile 3D Nesne Seçme (Raycasting)
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

    // Seçimi iptal et (ESC veya boşluğa tıklama)
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (seciliNesneIndeksi != -1) {
            seciliNesneIndeksi = -1;
        } else {
            calisiyorMu = false;
        }
    }
}

void Engine::CizSagAltGizmo() {
    // Sağ altta 3D Yön Pusulası (Kameranın açısına göre dönen RGB koordinat eksenleri)
    Vector3 kameraYonu = Vector3Normalize(Vector3Subtract(kamera.target, kamera.position));
    
    // Sağ alt köşede küçük bir kamera oluştur
    Camera3D gizmoKamera = { 0 };
    gizmoKamera.position = Vector3Scale(kameraYonu, -3.5f); // Merkeze aynı açıdan bakan mini kamera
    gizmoKamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    gizmoKamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    gizmoKamera.fovy = 50.0f;
    gizmoKamera.projection = CAMERA_PERSPECTIVE;

    // Sağ alt 120x120 piksel alan
    int gizmoBoyut = 120;
    int gizmoX = ekranGenisligi - gizmoBoyut - 20;
    int gizmoY = ekranYuksekligi - gizmoBoyut - 20;

    // Mini pusula arkaplanı
    DrawRectangleRounded((Rectangle){ (float)gizmoX, (float)gizmoY, (float)gizmoBoyut, (float)gizmoBoyut }, 0.2f, 4, (Color){ 20, 22, 28, 200 });
    DrawRectangleRoundedLines((Rectangle){ (float)gizmoX, (float)gizmoY, (float)gizmoBoyut, (float)gizmoBoyut }, 0.2f, 4, (Color){ 60, 65, 80, 255 });

    // Mini 3D çizim alanını kırp
    BeginScissorMode(gizmoX, gizmoY, gizmoBoyut, gizmoBoyut);
    BeginMode3D(gizmoKamera);

        // X (Kırmızı), Y (Yeşil), Z (Mavi) Eksen Çizgileri
        DrawLine3D((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 1.0f, 0.0f, 0.0f }, RED);
        DrawSphere((Vector3){ 1.0f, 0.0f, 0.0f }, 0.1f, RED);

        DrawLine3D((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 1.0f, 0.0f }, GREEN);
        DrawSphere((Vector3){ 0.0f, 1.0f, 0.0f }, 0.1f, GREEN);

        DrawLine3D((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, 1.0f }, BLUE);
        DrawSphere((Vector3){ 0.0f, 0.0f, 1.0f }, 0.1f, BLUE);

        // Merkez nokta
        DrawSphere((Vector3){ 0.0f, 0.0f, 0.0f }, 0.06f, WHITE);

    EndMode3D();
    EndScissorMode();

    // Eksen isimleri
    DrawText("X", gizmoX + gizmoBoyut - 20, gizmoY + gizmoBoyut / 2 - 5, 12, RED);
    DrawText("Y", gizmoX + gizmoBoyut / 2 - 4, gizmoY + 8, 12, GREEN);
    DrawText("Z", gizmoX + 12, gizmoY + gizmoBoyut - 20, 12, BLUE);
}

void Engine::Render() {
    BeginDrawing();
        // Blender tarzı koyu gri editör arkaplanı
        ClearBackground((Color){ 30, 31, 36, 255 });

        // --- 3D VIEWPORT BAŞLANGICI ---
        BeginMode3D(kamera);

            // 1. Zemin Izgarası (Grid)
            DrawGrid(24, 1.0f);

            // 2. Sahnedeki Varlıkları Çiz
            for (int i = 0; i < static_cast<int>(nesneler.size()); i++) {
                bool secili = (i == seciliNesneIndeksi);
                nesneler[i].Ciz(secili);

                // 3. Yalnızca obje seçiliyken üzerinde yerel 3D eksen çizgilerini (Gizmo) göster
                if (secili) {
                    Vector3 pos = nesneler[i].pozisyon;
                    float gizmoUzunluk = 2.5f;

                    // X Ekseni (Kırmızı)
                    DrawLine3D(pos, (Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, RED);
                    DrawSphere((Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, 0.15f, RED);

                    // Y Ekseni (Yeşil)
                    DrawLine3D(pos, (Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, GREEN);
                    DrawSphere((Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, 0.15f, GREEN);

                    // Z Ekseni (Mavi)
                    DrawLine3D(pos, (Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, BLUE);
                    DrawSphere((Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, 0.15f, BLUE);
                }
            }

        EndMode3D();
        // --- 3D VIEWPORT BİTİŞİ ---

        // Sol Üst: Motor Bilgileri (HUD)
        DrawText("Viento Engine 3D - Viewport", 20, 20, 20, RAYWHITE);
        DrawText("Fare Sol Tik: Obje Sec | Sag Tik + WASD/QE: Unreal Kamera", 20, 48, 14, LIGHTGRAY);
        DrawText(TextFormat("Kamera Hizi: %.1fx (Tekerlek ile ayarla) | FOV: %.0f ([ / ] ile ayarla)", kameraHizi, kamera.fovy), 20, 70, 14, YELLOW);

        // Sol Alt: Seçili Obje Bilgisi
        if (seciliNesneIndeksi != -1) {
            Vector3 pos = nesneler[seciliNesneIndeksi].pozisyon;
            DrawText(TextFormat("Secili Obje #%i | Konum: (X: %.2f, Y: %.2f, Z: %.2f)", seciliNesneIndeksi, pos.x, pos.y, pos.z), 20, ekranYuksekligi - 40, 15, ORANGE);
        } else {
            DrawText("Secili Obje: Yok (Secmek icin 3D kupe sol tikla)", 20, ekranYuksekligi - 40, 14, DARKGRAY);
        }

        // Sağ Alt: 3D Yön Pusulası (World Position Gizmo)
        CizSagAltGizmo();

        // Sağ Üst: FPS Göstergesi
        DrawFPS(ekranGenisligi - 90, 20);

    EndDrawing();
}
