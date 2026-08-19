#include "raylib.h"

int main() {
    // Pencereyi aç ve 60 FPS'e sabitle
    InitWindow(800, 600, "VieEngine");
    SetTargetFPS(30); // konsollar için 30 fps eheheh (yapacağın şakayı)

    // 3D Kamera Ayarlari
    Camera3D kamera = { 0 };
    kamera.position = (Vector3){ 20.0f, 5.0f, 5.0f }; // Kameranın durduğu yer (X, Y, Z)
    kamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };     // Kameranın baktığı nokta
    kamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         // Kameranın yukarı yönü (Y ekseni)
    kamera.fovy = 60.0f;                               // Görüş açısı (FOV)
    kamera.projection = CAMERA_ORTHOGRAPHIC;           // 3D perspektif görüş

    // 3D küpün baslangıç konumu ve hızı
    Vector3 kupPozisyonu = { 0.0f, 1.0f, 0.0f };
    const float HIZ = 0.2f;
    Color kupRengi = RED;

    // Ana dongu
    while (!WindowShouldClose()) {
        // Hareket kontrolleri (3D düzlemde: X sağa/sola, Z ileri/geri (mala anlatır gibi anlattım prdn))
        if (IsKeyDown(KEY_W)) kupPozisyonu.z -= HIZ;
        if (IsKeyDown(KEY_S)) kupPozisyonu.z += HIZ;
        if (IsKeyDown(KEY_A)) kupPozisyonu.x -= HIZ;
        if (IsKeyDown(KEY_D)) kupPozisyonu.x += HIZ;
        if (IsKeyDown(KEY_X)) CloseWindow(); // pencereyi x ile kapatabiliyosun tabii bi çılgınlık yapıp kendi çarpısına da basabilirsin

        // Renk değiştirme
        if (IsKeyDown(KEY_R)) kupRengi = RED;
        if (IsKeyDown(KEY_G)) kupRengi = GREEN;
        if (IsKeyDown(KEY_B)) kupRengi = BLUE;

        // Çizim ksmı
        BeginDrawing();
            ClearBackground(BLACK);

            // --- 3D MODU BAŞLANGICI ---
            BeginMode3D(kamera);

                // 3D ızgara Zemin (Grid)
                DrawGrid(20, 1.0f);

                // 3D küpün gendi burdadır (kıbrıslı galiba)
                DrawCube(kupPozisyonu, 2.0f, 2.0f, 2.0f, kupRengi);
                DrawCubeWires(kupPozisyonu, 2.0f, 2.0f, 2.0f, WHITE); // Küpün kenar çizgileri

            EndMode3D();
            // --- 3D MODU BİTİŞİ ---

            // 2D UI (yazılar falan)
            DrawText("VieEngine Pre Alpha falan filan", 200, 30, 24, SKYBLUE);
            DrawText("WASD ile düzlemde hareket ettirebilirsin kanka", 230, 65, 16, GRAY);
            DrawFPS(20, 20);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
