#include "Engine.h"
#include "raymath.h"
#include "rlgl.h"

// Gömülü GLSL Vertex Shader (OpenGL 3.3 Core)
static const char* vertexShaderKaynak = R"(
#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main() {
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

// Gömülü GLSL Fragment Shader (Point, Spot, Environment Işık Desteği)
static const char* fragmentShaderKaynak = R"(
#version 330
#define MAKSIMUM_ISIK_SAYISI 8

struct Isik {
    int aktif;
    int tip; // 0: Noktasal (Point), 1: Spot, 2: Ortam/Güneş (Environment/Directional)
    vec3 pozisyon;
    vec3 hedef;
    vec4 renk;
    float parlaklik;
    float koniAcisi;
};

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform vec4 colDiffuse;
uniform vec4 ortamIsigi;
uniform vec3 viewPos;
uniform Isik isiklar[MAKSIMUM_ISIK_SAYISI];

out vec4 finalColor;

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 gorunumYonu = normalize(viewPos - fragPosition);
    vec3 toplamAydinlatma = ortamIsigi.rgb;

    for (int i = 0; i < MAKSIMUM_ISIK_SAYISI; i++) {
        if (isiklar[i].aktif == 1) {
            vec3 isikYonu = vec3(0.0);
            float sonum = 1.0;

            if (isiklar[i].tip == 2) {
                // Environment / Directional (Güneş Işığı)
                isikYonu = normalize(isiklar[i].pozisyon - isiklar[i].hedef);
            } else {
                // Point & Spot Light
                vec3 fark = isiklar[i].pozisyon - fragPosition;
                float mesafe = length(fark);
                isikYonu = normalize(fark);
                sonum = 1.0 / (1.0 + 0.09 * mesafe + 0.032 * (mesafe * mesafe));

                if (isiklar[i].tip == 1) {
                    // Spot Light Koni Hesaplaması
                    vec3 spotYonu = normalize(isiklar[i].hedef - isiklar[i].pozisyon);
                    float theta = dot(isikYonu, -spotYonu);
                    if (theta > isiklar[i].koniAcisi) {
                        float spotYogunluk = clamp((theta - isiklar[i].koniAcisi) / (1.0 - isiklar[i].koniAcisi), 0.0, 1.0);
                        sonum *= spotYogunluk;
                    } else {
                        sonum = 0.0;
                    }
                }
            }

            // Diffuse (Yaygın Işık)
            float NdotL = max(dot(normal, isikYonu), 0.0);
            vec3 diffuse = isiklar[i].renk.rgb * NdotL * isiklar[i].parlaklik;

            // Specular (Blinn-Phong Parlama)
            vec3 yariVektor = normalize(isikYonu + gorunumYonu);
            float NdotH = max(dot(normal, yariVektor), 0.0);
            float spec = pow(NdotH, 32.0);
            vec3 specular = isiklar[i].renk.rgb * spec * 0.3 * isiklar[i].parlaklik;

            toplamAydinlatma += (diffuse + specular) * sonum;
        }
    }

    vec4 nesneRengi = colDiffuse * fragColor;
    finalColor = vec4(nesneRengi.rgb * toplamAydinlatma, nesneRengi.a);
}
)";

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

void Engine::InitShader() {
    isikShader = LoadShaderFromMemory(vertexShaderKaynak, fragmentShaderKaynak);

    isikShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(isikShader, "viewPos");
    isikShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(isikShader, "matModel");

    ortamIsigiLoc = GetShaderLocation(isikShader, "ortamIsigi");
    float ortamDegeri[4] = { 0.22f, 0.24f, 0.28f, 1.0f }; // Modern nötr ortam ışığı
    SetShaderValue(isikShader, ortamIsigiLoc, ortamDegeri, SHADER_UNIFORM_VEC4);
}

void Engine::Init(int genislik, int yukseklik, const char* baslik) {
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_MSAA_4X_HINT);

    int monitor = GetCurrentMonitor();
    ekranGenisligi = GetMonitorWidth(monitor);
    ekranYuksekligi = GetMonitorHeight(monitor);

    InitWindow(ekranGenisligi, ekranYuksekligi, baslik);
    SetWindowPosition(0, 0);
    SetTargetFPS(0);

    InitShader();

    // Sahneye varsayılan 1 adet Güneş / Ortam Işığı ekle
    IsikEkle(IsikTipi::ORTAM_GUNES, (Vector3){ 8.0f, 12.0f, 8.0f }, (Color){ 255, 245, 230, 255 }, 1.2f);

    calisiyorMu = true;
}

void Engine::Shutdown() {
    if (calisiyorMu) {
        UnloadShader(isikShader);
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

void Engine::IsikEkle(IsikTipi tip, Vector3 pos, Color renk, float parlaklik) {
    if (isiklar.size() >= MAKSIMUM_ISIK_SAYISI) return;

    int index = static_cast<int>(isiklar.size());
    Isik yeniIsik;
    yeniIsik.tip = tip;
    yeniIsik.pozisyon = pos;
    yeniIsik.hedef = (Vector3){ 0.0f, 0.0f, 0.0f };
    yeniIsik.renk = renk;
    yeniIsik.parlaklik = parlaklik;
    yeniIsik.koniAcisi = (tip == IsikTipi::SPOT) ? 35.0f : 45.0f;

    // Shader Uniform Lokasyonlarını bağla
    yeniIsik.aktifLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].aktif", index));
    yeniIsik.tipLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].tip", index));
    yeniIsik.posLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].pozisyon", index));
    yeniIsik.hedefLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].hedef", index));
    yeniIsik.renkLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].renk", index));
    yeniIsik.parlaklikLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].parlaklik", index));
    yeniIsik.koniAcisiLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].koniAcisi", index));

    isiklar.push_back(yeniIsik);

    // Sahneye ışığı taşımak için görünür bir Işık Kaynağı Nesnesi ekle
    Entity isikObjesi(EntityTipi::ISIK_KAYNAGI);
    isikObjesi.pozisyon = pos;
    isikObjesi.bagliIsikIndeksi = index;
    NesneEkle(isikObjesi);
}

EksenTipi Engine::AlgilaGizmoEkseni(const Entity& nesne, Ray ray) {
    Vector3 pos = nesne.pozisyon;
    
    float maxBoyut = fmaxf(nesne.olcek.x, fmaxf(nesne.olcek.y, nesne.olcek.z));
    float mesafe = fmaxf(2.0f, maxBoyut * 0.75f + 1.2f);
    float yaricap = fmaxf(0.35f, mesafe * 0.12f);

    BoundingBox xBox = {
        (Vector3){ pos.x + mesafe - yaricap, pos.y - yaricap, pos.z - yaricap },
        (Vector3){ pos.x + mesafe + yaricap, pos.y + yaricap, pos.z + yaricap }
    };
    if (GetRayCollisionBox(ray, xBox).hit) return EksenTipi::X;

    if (!(nesne.tip == EntityTipi::DUZLEM && aktifMod == TransformModu::OLCEK)) {
        BoundingBox yBox = {
            (Vector3){ pos.x - yaricap, pos.y + mesafe - yaricap, pos.z - yaricap },
            (Vector3){ pos.x + yaricap, pos.y + mesafe + yaricap, pos.z + yaricap }
        };
        if (GetRayCollisionBox(ray, yBox).hit) return EksenTipi::Y;
    }

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
        aktifMenu = MenuDurumu::KAPALI;
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
        if (IsKeyPressed(KEY_W)) aktifMod = TransformModu::KONUM;
        if (IsKeyPressed(KEY_E)) aktifMod = TransformModu::ROTASYON;
        if (IsKeyPressed(KEY_R)) aktifMod = TransformModu::OLCEK;

        // Seçili Nesneyi Silme
        if ((IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) && seciliNesneIndeksi != -1) {
            Entity& silinen = nesneler[seciliNesneIndeksi];
            if (silinen.bagliIsikIndeksi >= 0 && silinen.bagliIsikIndeksi < static_cast<int>(isiklar.size())) {
                isiklar[silinen.bagliIsikIndeksi].aktif = false;
            }
            nesneler.erase(nesneler.begin() + seciliNesneIndeksi);
            seciliNesneIndeksi = -1;
            suruklenenEksen = EksenTipi::YOK;
        }

        // Sol Tıka Basıldığı An (3D Seçim)
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (farePos.y > 45.0f && aktifMenu == MenuDurumu::KAPALI) {
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

                    // Eğer taşınan nesne bir IŞIK ise ışığın GPU pozisyonunu senkronize et
                    if (secili.bagliIsikIndeksi >= 0 && secili.bagliIsikIndeksi < static_cast<int>(isiklar.size())) {
                        isiklar[secili.bagliIsikIndeksi].pozisyon = secili.pozisyon;
                    }
                }
                else if (aktifMod == TransformModu::ROTASYON) {
                    float rotSens = 2.0f;
                    if (IsKeyDown(KEY_LEFT_SHIFT)) rotSens *= 0.25f;

                    if (suruklenenEksen == EksenTipi::X) secili.rotasyon.x += miktarX * rotSens * 40.0f;
                    if (suruklenenEksen == EksenTipi::Y) secili.rotasyon.y -= miktarY * rotSens * 40.0f;
                    if (suruklenenEksen == EksenTipi::Z) secili.rotasyon.z -= miktarZ * rotSens * 40.0f;
                }
                else if (aktifMod == TransformModu::OLCEK) {
                    if (suruklenenEksen == EksenTipi::X) secili.olcek.x = fmaxf(0.1f, secili.olcek.x + miktarX);
                    
                    if (suruklenenEksen == EksenTipi::Y) {
                        if (secili.tip != EntityTipi::DUZLEM) {
                            secili.olcek.y = fmaxf(0.1f, secili.olcek.y + miktarY);
                        }
                    }
                    
                    if (suruklenenEksen == EksenTipi::Z) secili.olcek.z = fmaxf(0.1f, secili.olcek.z + miktarZ);
                    
                    if (suruklenenEksen == EksenTipi::MERKEZ) {
                        float artis = (fareDelta.x - fareDelta.y) * 0.02f;
                        if (IsKeyDown(KEY_LEFT_SHIFT)) artis *= 0.25f;
                        secili.olcek.x = fmaxf(0.1f, secili.olcek.x + artis);
                        if (secili.tip != EntityTipi::DUZLEM) {
                            secili.olcek.y = fmaxf(0.1f, secili.olcek.y + artis);
                        }
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

    // 5. Shader Güncellemeleri (Kamera pozisyonu ve tüm aktif ışıklar)
    float camPos[3] = { kamera.position.x, kamera.position.y, kamera.position.z };
    SetShaderValue(isikShader, isikShader.locs[SHADER_LOC_VECTOR_VIEW], camPos, SHADER_UNIFORM_VEC3);

    for (int i = 0; i < static_cast<int>(isiklar.size()); i++) {
        GuncelleIsikGPU(isikShader, isiklar[i]);
    }
}

void Engine::CizTransformGizmo(const Entity& nesne) {
    Vector3 pos = nesne.pozisyon;

    float maxBoyut = fmaxf(nesne.olcek.x, fmaxf(nesne.olcek.y, nesne.olcek.z));
    float gizmoUzunluk = fmaxf(2.0f, maxBoyut * 0.75f + 1.2f);
    float kafaBoyut = fmaxf(0.25f, gizmoUzunluk * 0.08f);

    Color xRenk = (suruklenenEksen == EksenTipi::X) ? YELLOW : RED;
    Color yRenk = (suruklenenEksen == EksenTipi::Y) ? YELLOW : GREEN;
    Color zRenk = (suruklenenEksen == EksenTipi::Z) ? YELLOW : BLUE;
    Color mRenk = (suruklenenEksen == EksenTipi::MERKEZ) ? YELLOW : WHITE;

    rlDisableDepthTest();

    DrawSphere(pos, kafaBoyut * 1.1f, mRenk);

    if (aktifMod == TransformModu::KONUM) {
        DrawLine3D(pos, (Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, xRenk);
        DrawSphere((Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, kafaBoyut, xRenk);

        DrawLine3D(pos, (Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, yRenk);
        DrawSphere((Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, kafaBoyut, yRenk);

        DrawLine3D(pos, (Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, zRenk);
        DrawSphere((Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, kafaBoyut, zRenk);
    } 
    else if (aktifMod == TransformModu::ROTASYON) {
        DrawCircle3D(pos, gizmoUzunluk, (Vector3){ 1.0f, 0.0f, 0.0f }, 90.0f, xRenk);
        DrawCircle3D(pos, gizmoUzunluk, (Vector3){ 0.0f, 1.0f, 0.0f }, 0.0f, yRenk);
        DrawCircle3D(pos, gizmoUzunluk, (Vector3){ 0.0f, 0.0f, 1.0f }, 90.0f, zRenk);

        DrawSphere((Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, kafaBoyut, xRenk);
        DrawSphere((Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, kafaBoyut, yRenk);
        DrawSphere((Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, kafaBoyut, zRenk);
    } 
    else if (aktifMod == TransformModu::OLCEK) {
        DrawLine3D(pos, (Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, xRenk);
        DrawCube((Vector3){ pos.x + gizmoUzunluk, pos.y, pos.z }, kafaBoyut * 1.5f, kafaBoyut * 1.5f, kafaBoyut * 1.5f, xRenk);

        if (nesne.tip != EntityTipi::DUZLEM) {
            DrawLine3D(pos, (Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, yRenk);
            DrawCube((Vector3){ pos.x, pos.y + gizmoUzunluk, pos.z }, kafaBoyut * 1.5f, kafaBoyut * 1.5f, kafaBoyut * 1.5f, yRenk);
        }

        DrawLine3D(pos, (Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, zRenk);
        DrawCube((Vector3){ pos.x, pos.y, pos.z + gizmoUzunluk }, kafaBoyut * 1.5f, kafaBoyut * 1.5f, kafaBoyut * 1.5f, zRenk);
    }

    rlEnableDepthTest();
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

    DrawText("VIENTO ENGINE", 18, 11, 16, (Color){ 0, 230, 255, 255 });

    Rectangle addBtn = { 160.0f, 5.0f, 75.0f, 28.0f };
    bool addHover = CheckCollisionPointRec(fare, addBtn);
    
    if (aktifMenu != MenuDurumu::KAPALI || addHover) {
        DrawRectangleRec(addBtn, (Color){ 45, 50, 65, 255 });
    }
    DrawText("+ Add", 178, 12, 14, (Color){ 240, 245, 255, 255 });

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (addHover) {
            aktifMenu = (aktifMenu == MenuDurumu::KAPALI) ? MenuDurumu::ADD_ANA : MenuDurumu::KAPALI;
        } else if (fare.y > 38.0f) {
            Rectangle anaMenuKutusu = { 160.0f, 38.0f, 130.0f, 70.0f };
            Rectangle subMenuKutusu = { 290.0f, 38.0f, 160.0f, 180.0f };
            if (!CheckCollisionPointRec(fare, anaMenuKutusu) && !CheckCollisionPointRec(fare, subMenuKutusu)) {
                aktifMenu = MenuDurumu::KAPALI;
            }
        }
    }

    // 2. Açılır Menü Mantığı
    if (aktifMenu != MenuDurumu::KAPALI) {
        Rectangle anaMenuKutusu = { 160.0f, 38.0f, 130.0f, 70.0f };
        DrawRectangleRec(anaMenuKutusu, (Color){ 25, 28, 36, 250 });
        DrawRectangleLinesEx(anaMenuKutusu, 1.0f, (Color){ 60, 65, 80, 255 });

        Rectangle shapesItem = { 160.0f, 40.0f, 130.0f, 32.0f };
        Rectangle lightsItem = { 160.0f, 72.0f, 130.0f, 32.0f };

        bool hoverShapes = CheckCollisionPointRec(fare, shapesItem);
        bool hoverLights = CheckCollisionPointRec(fare, lightsItem);

        if (hoverShapes) aktifMenu = MenuDurumu::ADD_SHAPES;
        if (hoverLights) aktifMenu = MenuDurumu::ADD_LIGHTS;

        if (aktifMenu == MenuDurumu::ADD_SHAPES || hoverShapes) {
            DrawRectangleRec(shapesItem, (Color){ 45, 52, 70, 255 });
        }
        if (aktifMenu == MenuDurumu::ADD_LIGHTS || hoverLights) {
            DrawRectangleRec(lightsItem, (Color){ 45, 52, 70, 255 });
        }

        DrawText("Shapes        >", 172, 49, 13, (Color){ 230, 235, 245, 255 });
        DrawText("Lights          >", 172, 81, 13, (Color){ 230, 235, 245, 255 });

        // 3. Alt Menü: Shapes (İşlevsel)
        if (aktifMenu == MenuDurumu::ADD_SHAPES) {
            Rectangle shapesMenuKutusu = { 290.0f, 38.0f, 140.0f, 175.0f };
            DrawRectangleRec(shapesMenuKutusu, (Color){ 28, 32, 42, 250 });
            DrawRectangleLinesEx(shapesMenuKutusu, 1.0f, (Color){ 60, 65, 80, 255 });

            const char* shapeIsimleri[] = { "Cube", "Sphere", "Cylinder", "Triangle", "Plane" };
            EntityTipi shapeTipleri[] = { EntityTipi::KUP, EntityTipi::KURE, EntityTipi::SILINDIR, EntityTipi::UCGEN, EntityTipi::DUZLEM };

            for (int i = 0; i < 5; i++) {
                Rectangle itemRect = { 290.0f, 40.0f + (i * 33.0f), 140.0f, 32.0f };
                bool itemHover = CheckCollisionPointRec(fare, itemRect);

                if (itemHover) {
                    DrawRectangleRec(itemRect, (Color){ 0, 120, 215, 255 });

                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        Entity yeniNesne(shapeTipleri[i]);
                        NesneEkle(yeniNesne);
                        seciliNesneIndeksi = static_cast<int>(nesneler.size()) - 1;
                        aktifMenu = MenuDurumu::KAPALI;
                    }
                }
                DrawText(shapeIsimleri[i], 305, 49 + (i * 33), 13, WHITE);
            }
        }

        // 4. Alt Menü: Lights (İşlevsel Işık Ekleme)
        else if (aktifMenu == MenuDurumu::ADD_LIGHTS) {
            Rectangle lightsMenuKutusu = { 290.0f, 70.0f, 170.0f, 105.0f };
            DrawRectangleRec(lightsMenuKutusu, (Color){ 28, 32, 42, 250 });
            DrawRectangleLinesEx(lightsMenuKutusu, 1.0f, (Color){ 60, 65, 80, 255 });

            const char* lightIsimleri[] = { "Point Light", "Spot Light", "Environment Light" };
            IsikTipi lightTipleri[] = { IsikTipi::NOKTASAL, IsikTipi::SPOT, IsikTipi::ORTAM_GUNES };
            Color lightRenkleri[] = { (Color){ 255, 220, 120, 255 }, (Color){ 120, 220, 255, 255 }, (Color){ 255, 245, 230, 255 } };

            for (int i = 0; i < 3; i++) {
                Rectangle itemRect = { 290.0f, 72.0f + (i * 33.0f), 170.0f, 32.0f };
                bool itemHover = CheckCollisionPointRec(fare, itemRect);

                if (itemHover) {
                    DrawRectangleRec(itemRect, (Color){ 0, 120, 215, 255 });

                    // Işık Ekleme Tıklaması
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        Vector3 olusmaPozisyonu = (Vector3){ 0.0f, 4.0f, 0.0f };
                        IsikEkle(lightTipleri[i], olusmaPozisyonu, lightRenkleri[i], 1.2f);
                        seciliNesneIndeksi = static_cast<int>(nesneler.size()) - 1; // Yeni ışık objesini anında seç
                        aktifMenu = MenuDurumu::KAPALI;
                    }
                }
                DrawText(lightIsimleri[i], 305, 81 + (i * 33), 13, WHITE);
            }
        }
    }
}

void Engine::Render() {
    BeginDrawing();
        ClearBackground((Color){ 26, 27, 32, 255 });

        // --- 3D VIEWPORT ---
        BeginMode3D(kamera);

            DrawGrid(24, 1.0f);

            // 1. IŞIK SHADER'I İLE AYDINLATILAN NESNELER
            BeginShaderMode(isikShader);
                for (int i = 0; i < static_cast<int>(nesneler.size()); i++) {
                    if (nesneler[i].tip != EntityTipi::ISIK_KAYNAGI) {
                        bool secili = (i == seciliNesneIndeksi);
                        nesneler[i].Ciz(secili);
                    }
                }
            EndShaderMode();

            // 2. SHADER'SIZ (KENDİNDEN PARLAK) ÇİZİLEN IŞIK KAYNAKLARI VE GIZMOLAR
            for (int i = 0; i < static_cast<int>(nesneler.size()); i++) {
                if (nesneler[i].tip == EntityTipi::ISIK_KAYNAGI) {
                    bool secili = (i == seciliNesneIndeksi);
                    nesneler[i].Ciz(secili);
                }

                if (i == seciliNesneIndeksi) {
                    CizTransformGizmo(nesneler[i]);
                }
            }

        EndMode3D();
        // --- 3D VIEWPORT BİTİŞİ ---

        // --- 2D ARAYÜZ (HUD) & PANELLER ---
        
        // Sol Üst Bilgi Paneli
        DrawRectangleRounded((Rectangle){ 20.0f, 50.0f, 520.0f, 105.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ 20.0f, 50.0f, 520.0f, 105.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        DrawText("Fare Sol Tik: Obje/Isik Sec & Eksenle Tasi | Del: Sil", 35, 62, 13, (Color){ 240, 245, 255, 255 });
        
        const char* modAdi = (aktifMod == TransformModu::KONUM) ? "[W] Konum (Translate)" :
                             (aktifMod == TransformModu::ROTASYON) ? "[E] Rotasyon (Rotate)" : "[R] Olcek (Scale)";
        Color modRengi = (aktifMod == TransformModu::KONUM) ? SKYBLUE :
                         (aktifMod == TransformModu::ROTASYON) ? GREEN : ORANGE;

        DrawText(TextFormat("Aktif Mod: %s", modAdi), 35, 84, 14, modRengi);
        DrawText("Kamera: Sag Tik + WASD/QE | Tekerlek: Hiz | ESC: Cikis", 35, 106, 12, (Color){ 255, 205, 80, 255 });

        // Sol Alt: Seçili Obje / Işık Transform Paneli
        DrawRectangleRounded((Rectangle){ 20.0f, (float)(ekranYuksekligi - 85), 510.0f, 65.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ 20.0f, (float)(ekranYuksekligi - 85), 510.0f, 65.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        if (seciliNesneIndeksi != -1) {
            const Entity& secili = nesneler[seciliNesneIndeksi];
            const char* tipMetni = (secili.tip == EntityTipi::ISIK_KAYNAGI) ? "ISIK KAYNAGI" : "3D NESNE";
            Color baslikRenk = (secili.tip == EntityTipi::ISIK_KAYNAGI) ? YELLOW : ORANGE;

            DrawText(TextFormat("SECILI %s #%i (Sahne: %i Obje, %i Isik)", tipMetni, seciliNesneIndeksi, (int)nesneler.size(), (int)isiklar.size()), 35, ekranYuksekligi - 75, 14, baslikRenk);
            DrawText(TextFormat("Konum: (%.1f, %.1f, %.1f) | Rot: (%.0f, %.0f, %.0f) | Boyut: (%.1f, %.1f, %.1f)", 
                     secili.pozisyon.x, secili.pozisyon.y, secili.pozisyon.z,
                     secili.rotasyon.x, secili.rotasyon.y, secili.rotasyon.z,
                     secili.olcek.x, secili.olcek.y, secili.olcek.z), 35, ekranYuksekligi - 52, 12, (Color){ 220, 230, 245, 255 });
        } else {
            DrawText(TextFormat("Secili Obje: Yok (Toplam Sahne: %i Obje | %i Aktif Isik)", (int)nesneler.size(), (int)isiklar.size()), 35, ekranYuksekligi - 60, 14, (Color){ 130, 140, 160, 255 });
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

        // En Üst: Toolbar
        CizToolbar();

    EndDrawing();
}
