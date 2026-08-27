#include "Engine.h"
#include "raymath.h"
#include "rlgl.h"

Engine::Engine()
    : ekranGenisligi(1280), ekranYuksekligi(720), calisiyorMu(false),
      kameraHizi(10.0f), kameraYaw(45.0f), kameraPitch(-30.0f),
      seciliNesneIndeksi(-1), aktifMod(TransformModu::KONUM),
      suruklenenEksen(EksenTipi::YOK), sonFarePozisyonu((Vector2){ 0.0f, 0.0f }),
      aktifMenu(MenuDurumu::KAPALI) {

    kamera = { 0 };
    kamera.position = (Vector3){ 7.0f, 6.0f, 7.0f };
    kamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    kamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    kamera.fovy = 70.0f;
    kamera.projection = CAMERA_PERSPECTIVE;

    Vector3 bakisYonu = Vector3Normalize(Vector3Subtract(kamera.target, kamera.position));
    kameraPitch = asinf(bakisYonu.y) * RAD2DEG;
    kameraYaw   = atan2f(bakisYonu.x, bakisYonu.z) * RAD2DEG;
}

Engine::~Engine() {
    Shutdown();
}

void Engine::Init(int genislik, int yukseklik, const char* baslik) {
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_MSAA_4X_HINT);

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

EksenTipi Engine::AlgilaGizmoEkseni(const Entity& nesne, Ray ray) {
    Vector3 pos = nesne.pozisyon;
    float mesafe = 2.5f;
    float yaricap = 0.35f;

    BoundingBox xBox = {
        (Vector3){ pos.x + mesafe - yaricap, pos.y - yaricap, pos.z - yaricap },
        (Vector3){ pos.x + mesafe + yaricap, pos.y + yaricap, pos.z + yaricap }
    };
    if (GetRayCollisionBox(ray, xBox).hit) return EksenTipi::X;

    BoundingBox yBox = {
        (Vector3){ pos.x - yaricap, pos.y + mesafe - yaricap, pos.z - yaricap },
        (Vector3){ pos.x + yaricap, pos.y + mesafe + yaricap, pos.z + yaricap }
    };
    if (GetRayCollisionBox(ray, yBox).hit) return EksenTipi::Y;

    BoundingBox zBox = {
        (Vector3){ pos.x - yaricap, pos.y - yaricap, pos.z + mesafe - yaricap },
        (Vector3){ pos.x + yaricap, pos.y + yaricap, pos.z + mesafe + yaricap }
    };
    if (GetRayCollisionBox(ray, zBox).hit) return EksenTipi::Z;

    BoundingBox merkezBox = {
        (Vector3){ pos.x - yaricap, pos.y - yaricap, pos.z - yaricap },
        (Vector3){ pos.x + yaricap, pos.y + yaricap, pos.z + yaricap }
    };
    if (GetRayCollisionBox(ray, merkezBox).hit) return EksenTipi::MERKEZ;

    return EksenTipi::YOK;
}

void Engine::Update() {
    float dt = GetFrameTime();
    Vector2 farePos = GetMousePosition();

    // Toolbar Alanında mıyız? (Y < 45 veya Açık Menü Bölgesi)
    bool toolbarUzerinde = (farePos.y < 45.0f || (aktifMenu != MenuDurumu::KAPALI && farePos.x < 350.0f && farePos.y < 250.0f));

    // 1. Mouse Tekerleği ile kamera hızını değiştirme
    float tekerlek = GetMouseWheelMove();
    if (tekerlek != 0.0f) {
        kameraHizi += tekerlek * 2.5f;
        if (kameraHizi < 1.0f) kameraHizi = 1.0f;
        if (kameraHizi > 50.0f) kameraHizi = 50.0f;
    }

    // 2. FOV Ayarı
    if (IsKeyDown(KEY_LEFT_BRACKET) && kamera.fovy > 30.0f)  kamera.fovy -= 30.0f * dt;
    if (IsKeyDown(KEY_RIGHT_BRACKET) && kamera.fovy < 110.0f) kamera.fovy += 30.0f * dt;

    // 3. Kamera Hareketi (Sağ Tık Basılıyken)
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        DisableCursor();
        GetMouseDelta();
        aktifMenu = MenuDurumu::KAPALI; // Kamerayı çevirirken menüyü kapat
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
        // Mod Değiştirme Kısayolları (W, E, R)
        if (IsKeyPressed(KEY_W)) aktifMod = TransformModu::KONUM;
        if (IsKeyPressed(KEY_E)) aktifMod = TransformModu::ROTASYON;
        if (IsKeyPressed(KEY_R)) aktifMod = TransformModu::OLCEK;

        // Seçili Nesneyi Silme (Delete veya Backspace)
        if ((IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) && seciliNesneIndeksi != -1) {
            nesneler.erase(nesneler.begin() + seciliNesneIndeksi);
            seciliNesneIndeksi = -1;
            suruklenenEksen = EksenTipi::YOK;
        }

        // Sol Tıka Basıldığı An
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            // Eğer Toolbar'a tıklanmadıysa 3D uzayda seçim ve gizmo sürükleme yap
            if (!toolbarUzerinde) {
                aktifMenu = MenuDurumu::KAPALI; // Boşluğa tıklayınca menüyü kapat

                Ray ray = GetMouseRay(farePos, kamera);
                sonFarePozisyonu = farePos;

                if (seciliNesneIndeksi != -1 && seciliNesneIndeksi < static_cast<int>(nesneler.size())) {
                    suruklenenEksen = AlgilaGizmoEkseni(nesneler[seciliNesneIndeksi], ray);
                    if (suruklenenEksen != EksenTipi::YOK) {
                        HideCursor();
                    }
                }

                if (suruklenenEksen == EksenTipi::YOK) {
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
        }

        // Sol Tık Basılıyken Fareyi Sürükleme (Ekran Projeksiyonlu Hatasız Eksen Hareketi)
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && suruklenenEksen != EksenTipi::YOK) {
            Vector2 fareDelta = { farePos.x - sonFarePozisyonu.x, farePos.y - sonFarePozisyonu.y };
            sonFarePozisyonu = farePos;

            if (seciliNesneIndeksi != -1 && seciliNesneIndeksi < static_cast<int>(nesneler.size())) {
                Entity& secili = nesneler[seciliNesneIndeksi];

                Vector2 merkez2D = GetWorldToScreen(secili.pozisyon, kamera);
                Vector2 xEksen2D = GetWorldToScreen(Vector3Add(secili.pozisyon, (Vector3){ 1.0f, 0.0f, 0.0f }), kamera);
                Vector2 yEksen2D = GetWorldToScreen(Vector3Add(secili.pozisyon, (Vector3){ 0.0f, 1.0f, 0.0f }), kamera);
                Vector2 zEksen2D = GetWorldToScreen(Vector3Add(secili.pozisyon, (Vector3){ 0.0f, 0.0f, 1.0f }), kamera);

                Vector2 dirX = Vector2Normalize(Vector2Subtract(xEksen2D, merkez2D));
                Vector2 dirY = Vector2Normalize(Vector2Subtract(yEksen2D, merkez2D));
                Vector2 dirZ = Vector2Normalize(Vector2Subtract(zEksen2D, merkez2D));

                float miktarX = (fareDelta.x * dirX.x + fareDelta.y * dirX.y) * 0.03f;
                float miktarY = (fareDelta.x * dirY.x + fareDelta.y * dirY.y) * 0.03f;
                float miktarZ = (fareDelta.x * dirZ.x + fareDelta.y * dirZ.y) * 0.03f;

                if (IsKeyDown(KEY_LEFT_SHIFT)) {
                    miktarX *= 0.25f;
                    miktarY *= 0.25f;
                    miktarZ *= 0.25f;
                }

                if (aktifMod == TransformModu::KONUM) {
                    if (suruklenenEksen == EksenTipi::X) secili.pozisyon.x += miktarX;
                    if (suruklenenEksen == EksenTipi::Y) secili.pozisyon.y += miktarY;
                    if (suruklenenEksen == EksenTipi::Z) secili.pozisyon.z += miktarZ;
                }
                else if (aktifMod == TransformModu::ROTASYON) {
                    float rotSens = 2.5f;
                    if (IsKeyDown(KEY_LEFT_SHIFT)) rotSens *= 0.25f;

                    if (suruklenenEksen == EksenTipi::X) secili.rotasyon.x -= miktarY * rotSens * 40.0f;
                    if (suruklenenEksen == EksenTipi::Y) secili.rotasyon.y += miktarX * rotSens * 40.0f;
                    if (suruklenenEksen == EksenTipi::Z) secili.rotasyon.z += miktarZ * rotSens * 40.0f;
                }
                else if (aktifMod == TransformModu::OLCEK) {
                    if (suruklenenEksen == EksenTipi::X) secili.olcek.x = fmaxf(0.1f, secili.olcek.x + miktarX);
                    if (suruklenenEksen == EksenTipi::Y) secili.olcek.y = fmaxf(0.1f, secili.olcek.y + miktarY);
                    if (suruklenenEksen == EksenTipi::Z) secili.olcek.z = fmaxf(0.1f, secili.olcek.z + miktarZ);
                    if (suruklenenEksen == EksenTipi::MERKEZ) {
                        float artis = (fareDelta.x - fareDelta.y) * 0.02f;
                        if (IsKeyDown(KEY_LEFT_SHIFT)) artis *= 0.25f;
                        secili.olcek.x = fmaxf(0.1f, secili.olcek.x + artis);
                        secili.olcek.y = fmaxf(0.1f, secili.olcek.y + artis);
                        secili.olcek.z = fmaxf(0.1f, secili.olcek.z + artis);
                    }
                }
            }
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            if (suruklenenEksen != EksenTipi::YOK) {
                ShowCursor();
            }
            suruklenenEksen = EksenTipi::YOK;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (aktifMenu != MenuDurumu::KAPALI) {
            aktifMenu = MenuDurumu::KAPALI;
        } else if (seciliNesneIndeksi != -1) {
            seciliNesneIndeksi = -1;
            suruklenenEksen = EksenTipi::YOK;
        } else {
            calisiyorMu = false;
        }
    }
}

void Engine::CizTransformGizmo(const Entity& nesne) {
    Vector3 pos = nesne.pozisyon;
    float gizmoUzunluk = 2.5f;

    Color xRenk = (suruklenenEksen == EksenTipi::X) ? YELLOW : RED;
    Color yRenk = (suruklenenEksen == EksenTipi::Y) ? YELLOW : GREEN;
    Color zRenk = (suruklenenEksen == EksenTipi::Z) ? YELLOW : BLUE;
    Color mRenk = (suruklenenEksen == EksenTipi::MERKEZ) ? YELLOW : WHITE;

    DrawSphere(pos, 0.25f, mRenk);

    if (aktifMod == TransformModu::KONUM) {
        DrawLine3D(pos, (Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, xRenk);
        DrawSphere((Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, 0.25f, xRenk);

        DrawLine3D(pos, (Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, yRenk);
        DrawSphere((Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, 0.25f, yRenk);

        DrawLine3D(pos, (Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, zRenk);
        DrawSphere((Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, 0.25f, zRenk);
    } 
    else if (aktifMod == TransformModu::ROTASYON) {
        DrawCircle3D(pos, 2.5f, (Vector3){ 1.0f, 0.0f, 0.0f }, 90.0f, xRenk);
        DrawCircle3D(pos, 2.5f, (Vector3){ 0.0f, 1.0f, 0.0f }, 0.0f, yRenk);
        DrawCircle3D(pos, 2.5f, (Vector3){ 0.0f, 0.0f, 1.0f }, 90.0f, zRenk);

        DrawSphere((Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, 0.25f, xRenk);
        DrawSphere((Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, 0.25f, yRenk);
        DrawSphere((Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, 0.25f, zRenk);
    } 
    else if (aktifMod == TransformModu::OLCEK) {
        DrawLine3D(pos, (Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, xRenk);
        DrawCube((Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, 0.35f, 0.35f, 0.35f, xRenk);

        DrawLine3D(pos, (Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, yRenk);
        DrawCube((Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, 0.35f, 0.35f, 0.35f, yRenk);

        DrawLine3D(pos, (Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, zRenk);
        DrawCube((Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, 0.35f, 0.35f, 0.35f, zRenk);
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

void Engine::CizToolbar() {
    Vector2 fare = GetMousePosition();

    // 1. Üst Toolbar Şeridi
    DrawRectangle(0, 0, ekranGenisligi, 38, (Color){ 22, 24, 30, 240 });
    DrawLine(0, 38, ekranGenisligi, 38, (Color){ 50, 55, 68, 255 });

    // Sol Başlık
    DrawText("VIENTO ENGINE", 18, 11, 16, (Color){ 0, 230, 255, 255 });

    // "+ Add" Menü Butonu
    Rectangle addBtn = { 160.0f, 5.0f, 75.0f, 28.0f };
    bool addHover = CheckCollisionPointRec(fare, addBtn);
    
    if (aktifMenu != MenuDurumu::KAPALI || addHover) {
        DrawRectangleRec(addBtn, (Color){ 45, 50, 65, 255 });
    }
    DrawText("+ Add", 178, 12, 14, (Color){ 240, 245, 255, 255 });

    // Buton Tıklaması ile menüyü aç/kapat
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && addHover) {
        aktifMenu = (aktifMenu == MenuDurumu::KAPALI) ? MenuDurumu::ADD_ANA : MenuDurumu::KAPALI;
    }

    // 2. Açılır Menü Mantığı
    if (aktifMenu != MenuDurumu::KAPALI) {
        // Ana Menü Kutusu (Shapes ve Lights butonları)
        Rectangle anaMenuKutusu = { 160.0f, 38.0f, 130.0f, 70.0f };
        DrawRectangleRec(anaMenuKutusu, (Color){ 25, 28, 36, 250 });
        DrawRectangleLinesEx(anaMenuKutusu, 1.0f, (Color){ 60, 65, 80, 255 });

        Rectangle shapesItem = { 160.0f, 40.0f, 130.0f, 32.0f };
        Rectangle lightsItem = { 160.0f, 72.0f, 130.0f, 32.0f };

        // Alt menü alanları (Fare alt menü üzerindeyken açık kalması için)
        Rectangle shapesAltMenuAlani = { 290.0f, 38.0f, 140.0f, 140.0f };
        Rectangle lightsAltMenuAlani = { 290.0f, 70.0f, 160.0f, 110.0f };

        bool hoverShapes = CheckCollisionPointRec(fare, shapesItem);
        bool hoverLights = CheckCollisionPointRec(fare, lightsItem);
        bool insideShapesSub = CheckCollisionPointRec(fare, shapesAltMenuAlani);
        bool insideLightsSub = CheckCollisionPointRec(fare, lightsAltMenuAlani);

        // Menü geçiş mantığı: Sadece o satırın üzerindeyken veya o alt menünün içindeyken aktif et
        if (hoverShapes) {
            aktifMenu = MenuDurumu::ADD_SHAPES;
        } else if (hoverLights) {
            aktifMenu = MenuDurumu::ADD_LIGHTS;
        } else if (!insideShapesSub && !insideLightsSub && !CheckCollisionPointRec(fare, anaMenuKutusu)) {
            // Eğer fare ana menünün ve alt menülerin tamamen dışındaysa ve tıklandıysa kapat
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                aktifMenu = MenuDurumu::KAPALI;
            }
        }

        // Ana Menü Öğelerini Çiz
        if (aktifMenu == MenuDurumu::ADD_SHAPES || hoverShapes) {
            DrawRectangleRec(shapesItem, (Color){ 45, 52, 70, 255 });
        }
        if (aktifMenu == MenuDurumu::ADD_LIGHTS || hoverLights) {
            DrawRectangleRec(lightsItem, (Color){ 45, 52, 70, 255 });
        }

        DrawText("Shapes        >", 172, 49, 13, (Color){ 230, 235, 245, 255 });
        DrawText("Lights          >", 172, 81, 13, (Color){ 230, 235, 245, 255 });

        // 3. Alt Menü: SADECE Shapes Aktifken Çiz
        if (aktifMenu == MenuDurumu::ADD_SHAPES) {
            DrawRectangleRec(shapesAltMenuAlani, (Color){ 28, 32, 42, 250 });
            DrawRectangleLinesEx(shapesAltMenuAlani, 1.0f, (Color){ 60, 65, 80, 255 });

            const char* shapeIsimleri[] = { "Cube", "Sphere", "Cylinder", "Triangle" };
            EntityTipi shapeTipleri[] = { EntityTipi::KUP, EntityTipi::KURE, EntityTipi::SILINDIR, EntityTipi::UCGEN };

            for (int i = 0; i < 4; i++) {
                Rectangle itemRect = { 290.0f, 40.0f + (i * 33.0f), 140.0f, 32.0f };
                bool itemHover = CheckCollisionPointRec(fare, itemRect);

                if (itemHover) {
                    DrawRectangleRec(itemRect, (Color){ 0, 120, 215, 255 });

                    // Şekil Ekleme Tıklaması
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        Entity yeniNesne(shapeTipleri[i]);
                        yeniNesne.pozisyon = (Vector3){ 0.0f, 1.0f, 0.0f };
                        NesneEkle(yeniNesne);
                        seciliNesneIndeksi = static_cast<int>(nesneler.size()) - 1;
                        aktifMenu = MenuDurumu::KAPALI;
                    }
                }
                DrawText(shapeIsimleri[i], 305, 49 + (i * 33), 13, WHITE);
            }
        }

        // 4. Alt Menü: SADECE Lights Aktifken Çiz
        else if (aktifMenu == MenuDurumu::ADD_LIGHTS) {
            DrawRectangleRec(lightsAltMenuAlani, (Color){ 28, 32, 42, 250 });
            DrawRectangleLinesEx(lightsAltMenuAlani, 1.0f, (Color){ 60, 65, 80, 255 });

            const char* lightIsimleri[] = { "Point Light", "Spot Light", "Environment Light" };

            for (int i = 0; i < 3; i++) {
                Rectangle itemRect = { 290.0f, 72.0f + (i * 33.0f), 160.0f, 32.0f };
                bool itemHover = CheckCollisionPointRec(fare, itemRect);

                if (itemHover) {
                    DrawRectangleRec(itemRect, (Color){ 50, 55, 70, 255 });
                }
                DrawText(lightIsimleri[i], 305, 81 + (i * 33), 12, (Color){ 160, 170, 190, 255 });
            }
        }
    }
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

        // --- 2D ARAYÜZ (HUD) & PANELLER ---
        
        // Sol Üst Bilgi Paneli (Toolbar'ın hemen altında)
        DrawRectangleRounded((Rectangle){ 20.0f, 50.0f, 520.0f, 105.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ 20.0f, 50.0f, 520.0f, 105.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        DrawText("Fare Sol Tik: Obje Sec / Eksen Tut Surukle | Del: Sil", 35, 62, 13, (Color){ 240, 245, 255, 255 });
        
        const char* modAdi = (aktifMod == TransformModu::KONUM) ? "[W] Konum (Translate)" :
                             (aktifMod == TransformModu::ROTASYON) ? "[E] Rotasyon (Rotate)" : "[R] Olcek (Scale)";
        Color modRengi = (aktifMod == TransformModu::KONUM) ? SKYBLUE :
                         (aktifMod == TransformModu::ROTASYON) ? GREEN : ORANGE;

        DrawText(TextFormat("Aktif Mod: %s", modAdi), 35, 84, 14, modRengi);
        DrawText("Kamera: Sag Tik + WASD/QE | Tekerlek: Hiz | ESC: Cikis", 35, 106, 12, (Color){ 255, 205, 80, 255 });

        // Sol Alt: Seçili Obje Transform Paneli
        DrawRectangleRounded((Rectangle){ 20.0f, (float)(ekranYuksekligi - 85), 490.0f, 65.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ 20.0f, (float)(ekranYuksekligi - 85), 490.0f, 65.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        if (seciliNesneIndeksi != -1) {
            const Entity& secili = nesneler[seciliNesneIndeksi];
            DrawText(TextFormat("SECILI OBJE #%i (Toplam: %i)", seciliNesneIndeksi, (int)nesneler.size()), 35, ekranYuksekligi - 75, 14, ORANGE);
            DrawText(TextFormat("Konum: (%.1f, %.1f, %.1f) | Rot: (%.0f, %.0f, %.0f) | Boyut: (%.1f, %.1f, %.1f)", 
                     secili.pozisyon.x, secili.pozisyon.y, secili.pozisyon.z,
                     secili.rotasyon.x, secili.rotasyon.y, secili.rotasyon.z,
                     secili.olcek.x, secili.olcek.y, secili.olcek.z), 35, ekranYuksekligi - 52, 12, (Color){ 220, 230, 245, 255 });
        } else {
            DrawText(TextFormat("Secili Obje: Yok (Toplam Sahne: %i Obje)", (int)nesneler.size()), 35, ekranYuksekligi - 60, 14, (Color){ 130, 140, 160, 255 });
        }

        // Sağ Alt: 3D Yön Pusulası
        CizSagAltGizmo();

        // Sağ Üst: FPS Paneli
        int fpsPanelWidth = 230;
        DrawRectangleRounded((Rectangle){ (float)(ekranGenisligi - fpsPanelWidth - 20), 50.0f, (float)fpsPanelWidth, 75.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ (float)(ekranGenisligi - fpsPanelWidth - 20), 50.0f, (float)fpsPanelWidth, 75.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        DrawText(TextFormat("FPS: %i", GetFPS()), ekranGenisligi - fpsPanelWidth + 15, 60, 18, (Color){ 80, 255, 120, 255 });
        DrawText(TextFormat("Frame Time: %.2f ms", GetFrameTime() * 1000.0f), ekranGenisligi - fpsPanelWidth + 15, 80, 13, (Color){ 255, 205, 80, 255 });
        DrawText(TextFormat("Cozunurluk: %ix%i", ekranGenisligi, ekranYuksekligi), ekranGenisligi - fpsPanelWidth + 15, 98, 12, (Color){ 170, 180, 200, 255 });

        // En Üst: Toolbar ve Açılır Menüler (Tüm arayüzün en üst katmanında çizilir)
        CizToolbar();

    EndDrawing();
}
