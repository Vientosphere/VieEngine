#include "Engine.h"

Engine::Engine()
    : ekranGenisligi(800), ekranYuksekligi(600), calisiyorMu(false),
      kupBoyutu(4.0f), hareketHizi(0.3f), kupRengi(WHITE) {
    
    // Kameranın başlangıç değerleri
    kamera = { 0 };
    kamera.position = (Vector3){ 0.0f, 10.0f, 15.0f };
    kamera.target = (Vector3){ 0.0f, 5.0f, 0.0f };
    kamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    kamera.fovy = 75.0f;
    kamera.projection = CAMERA_PERSPECTIVE;

    // Küpün başlangıç konumu
    kupPozisyonu = (Vector3){ 0.0f, 1.0f, 2.0f };
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

void Engine::Run() {
    while (calisiyorMu && !WindowShouldClose()) {
        Update();
        Render();
    }
}

void Engine::Update() {
    // WASD ile küp hareketi
    if (IsKeyDown(KEY_W)) kupPozisyonu.z -= hareketHizi;
    if (IsKeyDown(KEY_S)) kupPozisyonu.z += hareketHizi;
    if (IsKeyDown(KEY_A)) kupPozisyonu.x -= hareketHizi;
    if (IsKeyDown(KEY_D)) kupPozisyonu.x += hareketHizi;

    // Renk değiştirme kontrolleri
    if (IsKeyDown(KEY_R)) kupRengi = RED;
    if (IsKeyDown(KEY_G)) kupRengi = GREEN;
    if (IsKeyDown(KEY_B)) kupRengi = BLUE;

    // Çıkış tuşu
    if (IsKeyDown(KEY_X)) calisiyorMu = false;
}

void Engine::Render() {
    BeginDrawing();
        ClearBackground(BLACK);

        // 3D Çizim Alanı
        BeginMode3D(kamera);
            DrawGrid(12, 2.0f);
            DrawCube(kupPozisyonu, kupBoyutu, kupBoyutu, kupBoyutu, kupRengi);
            DrawCubeWires(kupPozisyonu, kupBoyutu, kupBoyutu, kupBoyutu, WHITE);
        EndMode3D();

        // 2D Arayüz Bilgileri
        DrawText("VieEngine v0.02", 20, 20, 20, SKYBLUE);
        DrawText("WASD: Hareket | R,G,B: Renk | X: Çıkış", 20, 50, 16, GRAY);
        DrawFPS(ekranGenisligi - 90, 20);

    EndDrawing();
}
