#include "Engine.h"

Engine::Engine()
    : ekranGenisligi(1280), ekranYuksekligi(720), calisiyorMu(false),
      kontrolEdilenIndeks(0), hareketHizi(0.3f) {
    
    // Kameranın başlangıç değerleri
    kamera = { 0 };
    kamera.position = (Vector3){ 0.0f, 15.0f, 20.0f };
    kamera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
    kamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    kamera.fovy = 60.0f;
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
    // Kontrol edilen geçerli bir nesne varsa hareket ettir
    if (!nesneler.empty() && kontrolEdilenIndeks < static_cast<int>(nesneler.size())) {
        Entity& aktifKup = nesneler[kontrolEdilenIndeks];

        // WASD ile hareket
        if (IsKeyDown(KEY_W)) aktifKup.pozisyon.z -= hareketHizi;
        if (IsKeyDown(KEY_S)) aktifKup.pozisyon.z += hareketHizi;
        if (IsKeyDown(KEY_A)) aktifKup.pozisyon.x -= hareketHizi;
        if (IsKeyDown(KEY_D)) aktifKup.pozisyon.x += hareketHizi;

        // Yukarı / Aşağı yükselme (Boşluk / Sol Shift)
        if (IsKeyDown(KEY_SPACE)) aktifKup.pozisyon.y += hareketHizi;
        if (IsKeyDown(KEY_LEFT_SHIFT)) aktifKup.pozisyon.y -= hareketHizi;

        // Renk değiştirme kontrolleri
        if (IsKeyDown(KEY_R)) aktifKup.renk = RED;
        if (IsKeyDown(KEY_G)) aktifKup.renk = GREEN;
        if (IsKeyDown(KEY_B)) aktifKup.renk = BLUE;
        if (IsKeyDown(KEY_Y)) aktifKup.renk = YELLOW;
    }

    // Çıkış tuşu
    if (IsKeyDown(KEY_ESCAPE) || IsKeyDown(KEY_X)) calisiyorMu = false;
}

void Engine::Render() {
    BeginDrawing();
        ClearBackground((Color){ 18, 18, 24, 255 }); // Modern koyu arka plan

        // 3D Çizim Alanı
        BeginMode3D(kamera);
            // 3D Zemin Izgarası
            DrawGrid(20, 2.0f);

            // Sahnedeki tüm nesneleri döngüyle çiz
            for (const auto& nesne : nesneler) {
                nesne.Ciz();
            }
        EndMode3D();

        // 2D Arayüz (HUD) Bilgileri
        DrawText("Viento Engine v0.03 (Entity System)", 20, 20, 22, SKYBLUE);
        DrawText("WASD: Hareket | Space / L-Shift: Yukari/Asagi | R,G,B,Y: Renk", 20, 50, 15, LIGHTGRAY);
        DrawText(TextFormat("Sahnedeki Nesne Sayisi: %i", (int)nesneler.size()), 20, 75, 15, LIME);
        
        DrawFPS(ekranGenisligi - 90, 20);

    EndDrawing();
}
