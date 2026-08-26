#include "Engine.h"

Engine::Engine()
    : ekranGenisligi(1280), ekranYuksekligi(720), calisiyorMu(false) {
    
    // Blender tarzı perspektif editör kamerası
    kamera = { 0 };
    kamera.position = (Vector3){ 7.0f, 6.0f, 7.0f }; // 3D uzayda çapraz yukarıda durur
    kamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };   // Merkeze (orijine) bakar
    kamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };       // Yukarı yön Y eksenidir
    kamera.fovy = 45.0f;
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
    // Farenin sağ tıkına basılı tutarken kamerayı serbestçe döndür (Orbital / Free Camera)
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        UpdateCamera(&kamera, CAMERA_FREE);
    }

    // Çıkış kontrolü
    if (IsKeyDown(KEY_ESCAPE)) {
        calisiyorMu = false;
    }
}

void Engine::Render() {
    BeginDrawing();
        // Blender tarzı profesyonel koyu gri editör arka planı
        ClearBackground((Color){ 30, 31, 36, 255 });

        // --- 3D VIEWPORT BAŞLANGICI ---
        BeginMode3D(kamera);

            // 1. Zemin Izgarası (Grid)
            DrawGrid(24, 1.0f);

            // 2. Sahnedeki Varlıkları Çiz (Default 2x2x2 Küp vb.)
            for (const auto& nesne : nesneler) {
                nesne.Ciz();
            }

            // 3. 3D Eksen Çizgileri (Kırmızı: X, Yeşil: Y, Mavi: Z)
            // X Ekseni (Kırmızı)
            DrawLine3D((Vector3){ 0.0f, 0.01f, 0.0f }, (Vector3){ 4.0f, 0.01f, 0.0f }, RED);
            // Y Ekseni (Yeşil - Yukarı)
            DrawLine3D((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 4.0f, 0.0f }, GREEN);
            // Z Ekseni (Mavi)
            DrawLine3D((Vector3){ 0.0f, 0.01f, 0.0f }, (Vector3){ 0.0f, 0.01f, 4.0f }, BLUE);

        EndMode3D();
        // --- 3D VIEWPORT BİTİŞİ ---

        // Sol Üst: Motor Bilgileri (HUD)
        DrawText("Viento Engine 3D - Viewport", 20, 20, 20, RAYWHITE);
        DrawText("Fare Sag Tik + WASD: Kamera Kontrolu | ESC: Cikis", 20, 48, 14, GRAY);

        // Sol Alt: Eksen Bilgisi & İstatistikler
        DrawText("Eksenler: X (Kirmizi) | Y (Yesil) | Z (Mavi)", 20, ekranYuksekligi - 40, 14, LIGHTGRAY);
        DrawText(TextFormat("Nesneler: %i", (int)nesneler.size()), 20, ekranYuksekligi - 60, 14, SKYBLUE);

        // Sağ Üst: FPS Göstergesi
        DrawFPS(ekranGenisligi - 90, 20);

    EndDrawing();
}
