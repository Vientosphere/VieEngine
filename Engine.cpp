#include "Engine.h"
#include "raymath.h"
#include "rlgl.h"

// 1. Ana Aydınlatma & Gölge Vertex Shader
static const char* isikVSKaynak = R"(
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

// 2. Ana Aydınlatma & Gölge Fragment Shader (Physical Attenuation, Inner/Outer Cone, PCF Soft Shadow)
static const char* isikFSKaynak = R"(
#version 330
#define MAKSIMUM_ISIK_SAYISI 8

struct Isik {
    int aktif;
    int tip; // 0: Point, 1: Spot, 2: Environment/Directional
    vec3 pozisyon;
    vec3 hedef;
    vec4 renk;
    float parlaklik;
    float zayiflamaYaricapi;
    float icKoniAcisi;
    float disKoniAcisi;
};

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec4 ortamIsigi;
uniform vec3 viewPos;
uniform mat4 lightVP;
uniform sampler2D shadowMap;
uniform int shadowMapResolution;
uniform Isik isiklar[MAKSIMUM_ISIK_SAYISI];

out vec4 finalColor;

// 16-Sample PCF (Percentage Closer Filtering) Yumuşak Gölge Hesaplayıcı
float HesaplaGolge(vec3 fragPos, vec3 normal, vec3 isikYonu) {
    vec4 fragPosLightSpace = lightVP * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = (projCoords * 0.5) + 0.5;

    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 1.0;
    }

    // Eğim bazlı adaptif shadow bias
    float cosTheta = clamp(dot(normal, isikYonu), 0.0, 1.0);
    float bias = max(0.0035 * (1.0 - cosTheta), 0.0008);

    float golgeFaktoru = 0.0;
    vec2 texelSize = vec2(1.0 / float(shadowMapResolution));

    // 4x4 PCF Yumuşatma Filtresi
    for (int x = -1; x <= 2; ++x) {
        for (int y = -1; y <= 2; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            golgeFaktoru += (projCoords.z - bias > pcfDepth) ? 0.0 : 1.0;
        }
    }
    return golgeFaktoru / 16.0;
}

void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec4 albedo = texelColor * colDiffuse * fragColor;

    vec3 normal = normalize(fragNormal);
    vec3 gorunumYonu = normalize(viewPos - fragPosition);
    vec3 toplamAydinlatma = ortamIsigi.rgb;

    for (int i = 0; i < MAKSIMUM_ISIK_SAYISI; i++) {
        if (isiklar[i].aktif == 1) {
            vec3 isikYonu = vec3(0.0);
            float sonum = 1.0;
            float golge = 1.0;

            if (isiklar[i].tip == 2) {
                // Environment / Directional Light (Güneş Işığı & Dinamik Gölge)
                isikYonu = normalize(isiklar[i].pozisyon - isiklar[i].hedef);
                golge = HesaplaGolge(fragPosition, normal, isikYonu);
            } else {
                // Point & Spot Light (Inverse Square Law Falloff)
                vec3 fark = isiklar[i].pozisyon - fragPosition;
                float mesafe = length(fark);
                isikYonu = normalize(fark);

                float yaricap = max(isiklar[i].zayiflamaYaricapi, 0.1);
                float mesafeOrani = clamp(1.0 - pow(mesafe / yaricap, 4.0), 0.0, 1.0);
                sonum = (mesafeOrani * mesafeOrani) / (mesafe * mesafe + 1.0) * (yaricap * 0.7);

                if (isiklar[i].tip == 1) {
                    // Spot Light Inner & Outer Cone Açı Yumuşatması
                    vec3 spotYonu = normalize(isiklar[i].hedef - isiklar[i].pozisyon);
                    float theta = dot(-isikYonu, spotYonu);
                    float epsilon = isiklar[i].icKoniAcisi - isiklar[i].disKoniAcisi;
                    float spotYogunluk = clamp((theta - isiklar[i].disKoniAcisi) / max(epsilon, 0.0001), 0.0, 1.0);
                    sonum *= spotYogunluk;
                }
            }

            // Diffuse (Lambert)
            float NdotL = max(dot(normal, isikYonu), 0.0);
            vec3 diffuse = isiklar[i].renk.rgb * NdotL * isiklar[i].parlaklik;

            // Specular (Blinn-Phong)
            vec3 yariVektor = normalize(isikYonu + gorunumYonu);
            float NdotH = max(dot(normal, yariVektor), 0.0);
            float spec = pow(NdotH, 32.0);
            vec3 specular = isiklar[i].renk.rgb * spec * 0.4 * isiklar[i].parlaklik;

            toplamAydinlatma += ((diffuse + specular) * sonum) * golge;
        }
    }

    finalColor = vec4(albedo.rgb * toplamAydinlatma, albedo.a);
}
)";

RenderTexture2D Engine::LoadShadowmap(int width, int height) {
    RenderTexture2D target = { 0 };
    target.id = rlLoadFramebuffer();
    target.texture.width = width;
    target.texture.height = height;

    if (target.id > 0) {
        rlEnableFramebuffer(target.id);

        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19; // DEPTH_COMPONENT_24BIT
        target.depth.mipmaps = 1;

        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

        if (rlFramebufferComplete(target.id)) {
            TraceLog(LOG_INFO, "FBO: Shadowmap Framebuffer olusturuldu (ID %i)", target.id);
        }

        rlDisableFramebuffer();
    }
    return target;
}

void Engine::UnloadShadowmap(RenderTexture2D target) {
    if (target.id > 0) {
        rlUnloadFramebuffer(target.id);
        rlUnloadTexture(target.depth.id);
    }
}

Engine::Engine()
    : ekranGenisligi(1280), ekranYuksekligi(720), calisiyorMu(false),
      kameraHizi(10.0f), kameraYaw(45.0f), kameraPitch(-30.0f),
      seciliNesneIndeksi(-1), aktifMod(TransformModu::KONUM),
      suruklenenEksen(EksenTipi::YOK), sonFarePozisyonu((Vector2){ 0.0f, 0.0f }),
      aktifMenu(MenuDurumu::KAPALI) {

    kamera = { 0 };
    kamera.position = (Vector3){ 9.0f, 8.0f, 9.0f };
    kamera.target = (Vector3){ 0.0f, 1.0f, 0.0f };
    kamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    kamera.fovy = 65.0f;
    kamera.projection = CAMERA_PERSPECTIVE;

    Vector3 bakisYonu = Vector3Normalize(Vector3Subtract(kamera.target, kamera.position));
    kameraPitch = asinf(bakisYonu.y) * RAD2DEG;
    kameraYaw   = atan2f(bakisYonu.x, bakisYonu.z) * RAD2DEG;

    // Gölge Işık Kamerası (Ortografik Güneş Projeksiyonu)
    isikKamerasi = { 0 };
    isikKamerasi.position = (Vector3){ 14.0f, 20.0f, 14.0f };
    isikKamerasi.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    isikKamerasi.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    isikKamerasi.fovy = 40.0f;
    isikKamerasi.projection = CAMERA_ORTHOGRAPHIC;
}

Engine::~Engine() {
    Shutdown();
}

void Engine::InitMeshes() {
    kupMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    kureMesh = GenMeshSphere(0.5f, 32, 32);
    silindirMesh = GenMeshCylinder(0.5f, 1.0f, 32);
    ucgenMesh = GenMeshCone(0.6f, 1.0f, 4);
    duzlemMesh = GenMeshCube(1.0f, 1.0f, 1.0f);

    isikMaterial = LoadMaterialDefault();
    isikMaterial.shader = isikShader;

    golgeMaterial = LoadMaterialDefault();
    wireMaterial = LoadMaterialDefault();
    outlineMaterial = LoadMaterialDefault();
}

void Engine::InitShadowmap() {
    golgeHaritasi = LoadShadowmap(SHADOWMAP_BOYUT, SHADOWMAP_BOYUT);
    SetTextureFilter(golgeHaritasi.depth, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(golgeHaritasi.depth, TEXTURE_WRAP_CLAMP);
}

void Engine::InitShader() {
    isikShader = LoadShaderFromMemory(isikVSKaynak, isikFSKaynak);

    isikShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(isikShader, "viewPos");
    isikShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(isikShader, "matModel");
    isikShader.locs[SHADER_LOC_MATRIX_NORMAL] = GetShaderLocation(isikShader, "matNormal");
    isikShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(isikShader, "mvp");
    isikShader.locs[SHADER_LOC_MAP_DIFFUSE] = GetShaderLocation(isikShader, "texture0");

    lightVPLoc = GetShaderLocation(isikShader, "lightVP");
    shadowMapLoc = GetShaderLocation(isikShader, "shadowMap");
    shadowResLoc = GetShaderLocation(isikShader, "shadowMapResolution");
    ortamIsigiLoc = GetShaderLocation(isikShader, "ortamIsigi");

    int res = SHADOWMAP_BOYUT;
    SetShaderValue(isikShader, shadowResLoc, &res, SHADER_UNIFORM_INT);

    float ortamDegeri[4] = { 0.15f, 0.16f, 0.20f, 1.0f };
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
    InitShadowmap();
    InitMeshes();

    // 1. Zemin Düzlemi (Plane - 24x24 geniş zemin)
    Entity zemin(EntityTipi::DUZLEM);
    zemin.pozisyon = (Vector3){ 0.0f, 0.0f, 0.0f };
    zemin.olcek = (Vector3){ 24.0f, 0.06f, 24.0f };
    zemin.renk = (Color){ 170, 175, 185, 255 };
    NesneEkle(zemin);

    // 2. Kırmızı Küp (Cube)
    Entity kup(EntityTipi::KUP);
    kup.pozisyon = (Vector3){ -3.0f, 1.0f, -1.0f };
    kup.olcek = (Vector3){ 2.0f, 2.0f, 2.0f };
    kup.renk = (Color){ 230, 75, 60, 255 };
    NesneEkle(kup);

    // 3. Mavi Küre (Sphere)
    Entity kure(EntityTipi::KURE);
    kure.pozisyon = (Vector3){ 3.0f, 1.25f, 1.0f };
    kure.olcek = (Vector3){ 2.5f, 2.5f, 2.5f };
    kure.renk = (Color){ 45, 140, 240, 255 };
    NesneEkle(kure);

    // 4. Yeşil Silindir (Cylinder)
    Entity silindir(EntityTipi::SILINDIR);
    silindir.pozisyon = (Vector3){ 0.0f, 1.5f, -3.5f };
    silindir.olcek = (Vector3){ 1.8f, 3.0f, 1.8f };
    silindir.renk = (Color){ 60, 200, 110, 255 };
    NesneEkle(silindir);

    // 5. Sarı Güneş Işığı (Directional Sun Light with Deep Shadows)
    IsikEkle(IsikTipi::ORTAM_GUNES, (Vector3){ 14.0f, 20.0f, 14.0f }, (Color){ 255, 245, 225, 255 }, 1.5f, 60.0f);

    // 6. Sıcak Noktasal Işık (Point Light with Attenuation Falloff)
    IsikEkle(IsikTipi::NOKTASAL, (Vector3){ 0.0f, 3.5f, 2.0f }, (Color){ 255, 200, 100, 255 }, 1.2f, 10.0f);

    calisiyorMu = true;
}

void Engine::Shutdown() {
    if (calisiyorMu) {
        UnloadMesh(kupMesh);
        UnloadMesh(kureMesh);
        UnloadMesh(silindirMesh);
        UnloadMesh(ucgenMesh);
        UnloadMesh(duzlemMesh);
        UnloadShader(isikShader);
        UnloadShadowmap(golgeHaritasi);
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

void Engine::IsikEkle(IsikTipi tip, Vector3 pos, Color renk, float parlaklik, float zayiflamaYaricapi) {
    if (isiklar.size() >= MAKSIMUM_ISIK_SAYISI) return;

    int index = static_cast<int>(isiklar.size());
    Isik yeniIsik;
    yeniIsik.tip = tip;
    yeniIsik.pozisyon = pos;
    yeniIsik.hedef = (Vector3){ 0.0f, 0.0f, 0.0f };
    yeniIsik.renk = renk;
    yeniIsik.parlaklik = parlaklik;
    yeniIsik.zayiflamaYaricapi = zayiflamaYaricapi;
    yeniIsik.icKoniAcisi = (tip == IsikTipi::SPOT) ? 22.0f : 45.0f;
    yeniIsik.disKoniAcisi = (tip == IsikTipi::SPOT) ? 38.0f : 60.0f;

    yeniIsik.aktifLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].aktif", index));
    yeniIsik.tipLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].tip", index));
    yeniIsik.posLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].pozisyon", index));
    yeniIsik.hedefLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].hedef", index));
    yeniIsik.renkLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].renk", index));
    yeniIsik.parlaklikLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].parlaklik", index));
    yeniIsik.zayiflamaYaricapiLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].zayiflamaYaricapi", index));
    yeniIsik.icKoniAcisiLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].icKoniAcisi", index));
    yeniIsik.disKoniAcisiLoc = GetShaderLocation(isikShader, TextFormat("isiklar[%i].disKoniAcisi", index));

    isiklar.push_back(yeniIsik);

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

    float tekerlek = GetMouseWheelMove();
    if (tekerlek != 0.0f) {
        kameraHizi += tekerlek * 2.5f;
        if (kameraHizi < 1.0f) kameraHizi = 1.0f;
        if (kameraHizi > 50.0f) kameraHizi = 50.0f;
    }

    if (IsKeyDown(KEY_LEFT_BRACKET) && kamera.fovy > 30.0f)  kamera.fovy -= 30.0f * dt;
    if (IsKeyDown(KEY_RIGHT_BRACKET) && kamera.fovy < 110.0f) kamera.fovy += 30.0f * dt;

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

    if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        if (IsKeyPressed(KEY_W)) aktifMod = TransformModu::KONUM;
        if (IsKeyPressed(KEY_E)) aktifMod = TransformModu::ROTASYON;
        if (IsKeyPressed(KEY_R)) aktifMod = TransformModu::OLCEK;

        if ((IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) && seciliNesneIndeksi != -1) {
            Entity& silinen = nesneler[seciliNesneIndeksi];
            if (silinen.bagliIsikIndeksi >= 0 && silinen.bagliIsikIndeksi < static_cast<int>(isiklar.size())) {
                isiklar[silinen.bagliIsikIndeksi].aktif = false;
            }
            nesneler.erase(nesneler.begin() + seciliNesneIndeksi);
            seciliNesneIndeksi = -1;
            suruklenenEksen = EksenTipi::YOK;
        }

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

                    if (secili.bagliIsikIndeksi >= 0 && secili.bagliIsikIndeksi < static_cast<int>(isiklar.size())) {
                        isiklar[secili.bagliIsikIndeksi].pozisyon = secili.pozisyon;
                        // Güneş Işığı ise Gölge Kamerasını da ışığın pozisyonuna senkronize et
                        if (isiklar[secili.bagliIsikIndeksi].tip == IsikTipi::ORTAM_GUNES) {
                            isikKamerasi.position = secili.pozisyon;
                        }
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

    // Shader ve Işık GPU Güncellemeleri
    float camPos[3] = { kamera.position.x, kamera.position.y, kamera.position.z };
    SetShaderValue(isikShader, isikShader.locs[SHADER_LOC_VECTOR_VIEW], camPos, SHADER_UNIFORM_VEC3);

    for (int i = 0; i < static_cast<int>(isiklar.size()); i++) {
        GuncelleIsikGPU(isikShader, isiklar[i]);
    }
}

void Engine::SahneyiCiz(bool golgePassi) {
    for (int i = 0; i < static_cast<int>(nesneler.size()); i++) {
        const Entity& n = nesneler[i];
        if (n.tip == EntityTipi::ISIK_KAYNAGI) continue;

        Matrix matScale = MatrixScale(n.olcek.x, n.olcek.y, n.olcek.z);
        Matrix matRot = MatrixMultiply(MatrixRotateZ(n.rotasyon.z * DEG2RAD),
                        MatrixMultiply(MatrixRotateX(n.rotasyon.x * DEG2RAD), MatrixRotateY(n.rotasyon.y * DEG2RAD)));
        Matrix matTrans = MatrixTranslate(n.pozisyon.x, n.pozisyon.y, n.pozisyon.z);
        Matrix transform = MatrixMultiply(MatrixMultiply(matScale, matRot), matTrans);

        Mesh mesh;
        if (n.tip == EntityTipi::KUP) mesh = kupMesh;
        else if (n.tip == EntityTipi::KURE) mesh = kureMesh;
        else if (n.tip == EntityTipi::SILINDIR) mesh = silindirMesh;
        else if (n.tip == EntityTipi::UCGEN) mesh = ucgenMesh;
        else mesh = duzlemMesh;

        if (golgePassi) {
            DrawMesh(mesh, golgeMaterial, transform);
        } else {
            // 1. Katı Gövde Renderi (Işık & Gölge Shader'ı ile)
            isikMaterial.maps[MATERIAL_MAP_DIFFUSE].color = n.renk;
            DrawMesh(mesh, isikMaterial, transform);

            // 2. Model Tel Çerçeveleri (Wireframe) - Mesh ile %100 Senkronize
            if (n.cizgiler) {
                rlEnableWireMode();
                wireMaterial.maps[MATERIAL_MAP_DIFFUSE].color = n.cizgiRengi;
                DrawMesh(mesh, wireMaterial, transform);
                rlDisableWireMode();
            }

            // 3. Seçili Nesne İçin Turuncu Dış Çizgi (Selection Outline)
            bool secili = (i == seciliNesneIndeksi);
            if (secili) {
                rlEnableWireMode();
                outlineMaterial.maps[MATERIAL_MAP_DIFFUSE].color = (Color){ 255, 140, 0, 255 };

                Vector3 scaleOutline1 = Vector3Scale(n.olcek, 1.015f);
                if (n.tip == EntityTipi::DUZLEM) scaleOutline1.y = n.olcek.y + 0.005f;
                Matrix matScaleOut1 = MatrixScale(scaleOutline1.x, scaleOutline1.y, scaleOutline1.z);
                Matrix transformOut1 = MatrixMultiply(MatrixMultiply(matScaleOut1, matRot), matTrans);
                DrawMesh(mesh, outlineMaterial, transformOut1);

                Vector3 scaleOutline2 = Vector3Scale(n.olcek, 1.03f);
                if (n.tip == EntityTipi::DUZLEM) scaleOutline2.y = n.olcek.y + 0.01f;
                Matrix matScaleOut2 = MatrixScale(scaleOutline2.x, scaleOutline2.y, scaleOutline2.z);
                Matrix transformOut2 = MatrixMultiply(MatrixMultiply(matScaleOut2, matRot), matTrans);
                DrawMesh(mesh, outlineMaterial, transformOut2);

                rlDisableWireMode();
            }
        }
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

                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        Vector3 olusmaPozisyonu = (Vector3){ 0.0f, 5.0f, 0.0f };
                        IsikEkle(lightTipleri[i], olusmaPozisyonu, lightRenkleri[i], 1.5f, 14.0f);
                        seciliNesneIndeksi = static_cast<int>(nesneler.size()) - 1;
                        aktifMenu = MenuDurumu::KAPALI;
                    }
                }
                DrawText(lightIsimleri[i], 305, 81 + (i * 33), 13, WHITE);
            }
        }
    }
}

void Engine::Render() {
    // ====================================================
    // 1. PASS: GÖLGE DERİNLİK HARİTASI (SHADOWMAP PASS)
    // ====================================================
    BeginTextureMode(golgeHaritasi);
        ClearBackground(WHITE);
        BeginMode3D(isikKamerasi);
            Matrix lightView = rlGetMatrixModelview();
            Matrix lightProj = rlGetMatrixProjection();
            lightVP = MatrixMultiply(lightView, lightProj);

            // Sahneyi derinlik tamponuna render et
            SahneyiCiz(true);
        EndMode3D();
    EndTextureMode();

    // ====================================================
    // 2. PASS: ANA SAHNE VE IŞIK / GÖLGE RENDERİ
    // ====================================================
    SetShaderValueMatrix(isikShader, lightVPLoc, lightVP);

    // Shadow Map derinlik dokusunu Texture Slot 1'e bağla
    rlEnableShader(isikShader.id);
    int shadowSlot = 1;
    rlActiveTextureSlot(shadowSlot);
    rlEnableTexture(golgeHaritasi.depth.id);
    rlSetUniformSampler(shadowMapLoc, shadowSlot);

    BeginDrawing();
        ClearBackground((Color){ 24, 26, 31, 255 });

        BeginMode3D(kamera);

            DrawGrid(24, 1.0f);

            // 3D Nesneleri Işık & Gölge Shader'ı ile çiz
            SahneyiCiz(false);

            // Işık Kaynağı Simgeleri & Transform Gizmo
            for (int i = 0; i < static_cast<int>(nesneler.size()); i++) {
                if (nesneler[i].tip == EntityTipi::ISIK_KAYNAGI) {
                    bool secili = (i == seciliNesneIndeksi);
                    DrawSphere(nesneler[i].pozisyon, 0.35f, (Color){ 255, 230, 80, 255 });
                    DrawSphereWires(nesneler[i].pozisyon, 0.38f, 10, 10, WHITE);
                    if (secili) {
                        DrawSphereWires(nesneler[i].pozisyon, 0.48f, 12, 12, (Color){ 255, 140, 0, 255 });
                    }
                }

                if (i == seciliNesneIndeksi) {
                    CizTransformGizmo(nesneler[i]);
                }
            }

        EndMode3D();

        rlDisableShader();

        // ====================================================
        // 3. 2D HUD VE ARAYÜZ
        // ====================================================
        DrawRectangleRounded((Rectangle){ 20.0f, 50.0f, 520.0f, 105.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ 20.0f, 50.0f, 520.0f, 105.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        DrawText("Fare Sol Tik: Obje/Isik Sec & Eksenle Tasi | Del: Sil", 35, 62, 13, (Color){ 240, 245, 255, 255 });
        
        const char* modAdi = (aktifMod == TransformModu::KONUM) ? "[W] Konum (Translate)" :
                             (aktifMod == TransformModu::ROTASYON) ? "[E] Rotasyon (Rotate)" : "[R] Olcek (Scale)";
        Color modRengi = (aktifMod == TransformModu::KONUM) ? SKYBLUE :
                         (aktifMod == TransformModu::ROTASYON) ? GREEN : ORANGE;

        DrawText(TextFormat("Aktif Mod: %s", modAdi), 35, 84, 14, modRengi);
        DrawText("Kamera: Sag Tik + WASD/QE | Tekerlek: Hiz | ESC: Cikis", 35, 106, 12, (Color){ 255, 205, 80, 255 });

        // Sol Alt: Seçili Obje Bilgisi
        DrawRectangleRounded((Rectangle){ 20.0f, (float)(ekranYuksekligi - 85), 510.0f, 65.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ 20.0f, (float)(ekranYuksekligi - 85), 510.0f, 65.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        if (seciliNesneIndeksi != -1) {
            const Entity& secili = nesneler[seciliNesneIndeksi];
            const char* tipMetni = (secili.tip == EntityTipi::ISIK_KAYNAGI) ? "ISIK KAYNAGI" : "3D NESNE";
            Color baslikRenk = (secili.tip == EntityTipi::ISIK_KAYNAGI) ? YELLOW : ORANGE;

            DrawText(TextFormat("SECILI %s #%i (Sahne: %i Obje, %i Isik | Dynamic Shadowmap: Aktif)", tipMetni, seciliNesneIndeksi, (int)nesneler.size(), (int)isiklar.size()), 35, ekranYuksekligi - 75, 14, baslikRenk);
            DrawText(TextFormat("Konum: (%.1f, %.1f, %.1f) | Rot: (%.0f, %.0f, %.0f) | Boyut: (%.1f, %.1f, %.1f)", 
                     secili.pozisyon.x, secili.pozisyon.y, secili.pozisyon.z,
                     secili.rotasyon.x, secili.rotasyon.y, secili.rotasyon.z,
                     secili.olcek.x, secili.olcek.y, secili.olcek.z), 35, ekranYuksekligi - 52, 12, (Color){ 220, 230, 245, 255 });
        } else {
            DrawText(TextFormat("Secili Obje: Yok (Toplam Sahne: %i Obje | %i Aktif Isik | 2048p Soft Shadows)", (int)nesneler.size(), (int)isiklar.size()), 35, ekranYuksekligi - 60, 14, (Color){ 130, 140, 160, 255 });
        }

        CizSagAltGizmo();

        int fpsPanelWidth = 230;
        DrawRectangleRounded((Rectangle){ (float)(ekranGenisligi - fpsPanelWidth - 20), 50.0f, (float)fpsPanelWidth, 75.0f }, 0.15f, 4, (Color){ 18, 20, 26, 210 });
        DrawRectangleRoundedLines((Rectangle){ (float)(ekranGenisligi - fpsPanelWidth - 20), 50.0f, (float)fpsPanelWidth, 75.0f }, 0.15f, 4, (Color){ 55, 60, 75, 255 });

        DrawText(TextFormat("FPS: %i", GetFPS()), ekranGenisligi - fpsPanelWidth + 15, 60, 18, (Color){ 80, 255, 120, 255 });
        DrawText(TextFormat("Frame Time: %.2f ms", GetFrameTime() * 1000.0f), ekranGenisligi - fpsPanelWidth + 15, 80, 13, (Color){ 255, 205, 80, 255 });
        DrawText(TextFormat("Cozunurluk: %ix%i", ekranGenisligi, ekranYuksekligi), ekranGenisligi - fpsPanelWidth + 15, 98, 12, (Color){ 170, 180, 200, 255 });

        CizToolbar();

    EndDrawing();
}
