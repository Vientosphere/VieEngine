#include "raylib.h"


int main() {
    // 1. 800x600 boyutunda penceremizi aç
    InitWindow(800, 600, "VieEngine");
    SetTargetFPS(60);

    // Kırmızı kutunun başlangıç koordinatları ve boyutu
    int x = 375;
    int y = 275;
    const int BOYUT = 50;
    const int HIZ = 6;
    Color kutuRengi = RED;

    // 2. Oyun Döngüsü
    while (!WindowShouldClose()) {
        // Kontroller (WASD)
        if (IsKeyDown(KEY_W)) y -= HIZ;
        if (IsKeyDown(KEY_S)) y += HIZ;
        if (IsKeyDown(KEY_A)) x -= HIZ;
        if (IsKeyDown(KEY_D)) x += HIZ;
        if (IsKeyDown(KEY_X)) CloseWindow();

        if (IsKeyDown(KEY_R)) kutuRengi = RED;
        if (IsKeyDown(KEY_G)) kutuRengi = GREEN;
        if (IsKeyDown(KEY_B)) kutuRengi = BLUE;

        if (x < 0) x = 0;
        if (x > 800 - BOYUT) x = 800 - BOYUT;
        if (y < 0) y = 0;
        if (y > 600 - BOYUT) y = 600 - BOYUT;


        // Çizim
        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("VIENGINE PRE ALPHA 0.01", 220, 40, 32, SKYBLUE);
            DrawRectangle(x, y, BOYUT, BOYUT, kutuRengi);
            DrawRectangleLines(x, y, BOYUT, BOYUT, WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
