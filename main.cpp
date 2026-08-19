#include "raylib.h"

int main() {
    // Pencereyi ac ve 60 FPS'e sabitle
    InitWindow(800, 600, "VieEngine 3D");
    SetTargetFPS(60);

    // 3D Kamera Ayarlari
    Camera3D kamera = { 0 };
    kamera.position = (Vector3){ 0.0f, 10.0f, 10.0f }; // Kameranin durdugu yer (X, Y, Z)
    kamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };     // Kameranin baktigi nokta
    kamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         // Kameranin yukari yonu (Y ekseni)
    kamera.fovy = 45.0f;                                // Gorus acisi (FOV)
    kamera.projection = CAMERA_PERSPECTIVE;             // 3D perspektif gorus

    // 3D Kupumuzun baslangic konumu ve hizi
    Vector3 kupPozisyonu = { 0.0f, 1.0f, 0.0f };
    const float HIZ = 0.2f;
    Color kupRengi = RED;

    // Ana dongu
    while (!WindowShouldClose()) {
        // Hareket kontrolleri (3D Duzlemde: X saga/sola, Z ileri/geri)
        if (IsKeyDown(KEY_W)) kupPozisyonu.z -= HIZ;
        if (IsKeyDown(KEY_S)) kupPozisyonu.z += HIZ;
        if (IsKeyDown(KEY_A)) kupPozisyonu.x -= HIZ;
        if (IsKeyDown(KEY_D)) kupPozisyonu.x += HIZ;
        if (IsKeyDown(KEY_X)) CloseWindow();

        // Renk degistirme
        if (IsKeyDown(KEY_R)) kupRengi = RED;
        if (IsKeyDown(KEY_G)) kupRengi = GREEN;
        if (IsKeyDown(KEY_B)) kupRengi = BLUE;

        // Cizim kismi
        BeginDrawing();
            ClearBackground(BLACK);

            // --- 3D MODU BASLANGICI ---
            BeginMode3D(kamera);

                // 3D Izgara Zemin (Grid)
                DrawGrid(20, 1.0f);

                // Bizim 3D Kupumuz
                DrawCube(kupPozisyonu, 2.0f, 2.0f, 2.0f, kupRengi);
                DrawCubeWires(kupPozisyonu, 2.0f, 2.0f, 2.0f, WHITE); // Kupun kenar cizgileri

            EndMode3D();
            // --- 3D MODU BITISI ---

            // 2D Arayuz (HUD) Yazilari
            DrawText("VIENGINE 3D PRE ALPHA 0.02", 200, 30, 24, SKYBLUE);
            DrawText("WASD ile 3D Kupu Duzlemde Hareket Ettir", 230, 65, 16, GRAY);
            DrawFPS(20, 20);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
