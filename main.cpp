#include "raylib.h" // raylib kutuphanesini cagiriyoruz burda

int main() {
    // Pencereyi ac ve 60 FPS'e sabitle
    InitWindow(800, 600, "VieEngine");
    SetTargetFPS(60);

    // Kutu ayarlari
    int x = 375;
    int y = 275;
    const int BOYUT = 50;
    const int HIZ = 6;
    Color kutuRengi = RED;

    // Ana dongu
    while (!WindowShouldClose()) {
        // Hareket kontrolleri
        if (IsKeyDown(KEY_W)) y -= HIZ;
        if (IsKeyDown(KEY_S)) y += HIZ;
        if (IsKeyDown(KEY_A)) x -= HIZ;
        if (IsKeyDown(KEY_D)) x += HIZ;
        if (IsKeyDown(KEY_X)) CloseWindow();

        // Renk degistirme
        if (IsKeyDown(KEY_R)) kutuRengi = RED;
        if (IsKeyDown(KEY_G)) kutuRengi = GREEN;
        if (IsKeyDown(KEY_B)) kutuRengi = BLUE;

        // Ekrandan cikmasin diye sinirlar
        if (x < 0) x = 0;
        if (x > 800 - BOYUT) x = 800 - BOYUT;
        if (y < 0) y = 0;
        if (y > 600 - BOYUT) y = 600 - BOYUT;

        // Cizim kismi
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
