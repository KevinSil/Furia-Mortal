// jogo.cpp
// Implementação do ecrã principal do jogo: carregamento do mapa, gestão do jogador,
// actualização do estado (input, física, colisões), desenho do mundo e HUD.

#include "jogo.h"
#include "bubble.h"
#include "helper.h"
#include "enemies.h"
#include "raylib.h"
#include "raymath.h" // Clamp, Vector2Length, etc.
#include "json.hpp"  // nlohmann::json (single-header)

#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <math.h> // sqrtf

using nlohmann::json;

// ----------------- Definição das configurações de animação do jogador ------------------
struct SpriteSetCfg
{
    const char *baseDir;             // Caminho para os diretórios dos sprites do jogador (ex: "assets/players/cavaleiro/")
    int idleCols, walkCols, runCols; // Quantidade de colunas (frames) para cada animação
    float idleFps, walkFps, runFps;  // Frames por segundo para cada animação
};

// Função que retorna as estatísticas base de acordo com a classe do jogador
static PlayerStats BaseStatsFor(PlayerClass c)
{
    switch (c)
    {
    case CAVALEIRO:
        return PlayerStats{
            150,   // maxHP
            100,   // maxSt
            0,     // maxMana
            50.f,  // baseSpeed
            160.f, // runSpeed
            0.25f, // armor
            80.f,  // stRegen
            0.f,   // mnRegen
            150,   // hp
            100,   // st
            0};    // mana
    case MAGO:
        return PlayerStats{85, 70, 140, 115.f, 150.f, 0.05f, 70.f, 10.f, 85, 70, 140};
    case ARQUEIRO:
        return PlayerStats{110, 130, 30, 140.f, 190.f, 0.10f, 22.f, 2.f, 110, 130, 30};
    }
    return PlayerStats{};
}

// Configurações para os sprites de cada classe do jogador
static const SpriteSetCfg gSpriteSets[] = {
    {"assets/players/cavaleiro/", 7, 8, 8, 6.0f, 10.0f, 12.0f},
    {"assets/players/mago/", 7, 6, 8, 6.0f, 10.0f, 12.0f},
    {"assets/players/arqueiro/", 7, 8, 8, 6.0f, 10.0f, 12.0f},
};

// Estrutura responsável pela animação do jogador
struct AnimadorJogador
{
    TiraAnim idle, walk, run;
    enum Estado
    {
        IDLE, // O jogador está parado
        WALK, // O jogador está andando
        RUN   // O jogador está correndo
    } estado = IDLE;
    bool inverterX = false;                      // Se o personagem deve ser invertido (andar para a esquerda)
    float limiarWalk = 1.0f, limiarRun = 120.0f; // Limiares de velocidade para determinar o estado de animação

    // Retorna a largura do quadro da animação atual
    int FrameW() const { return idle.frameW ? idle.frameW : (walk.frameW ? walk.frameW : run.frameW); }

    // Retorna a altura do quadro da animação atual
    int FrameH() const { return idle.frameH ? idle.frameH : (walk.frameH ? walk.frameH : run.frameH); }

    // Atualiza a animação do jogador com base na velocidade
    void Atualizar(float dt, Vector2 velPxFrame)
    {
        if (velPxFrame.x != 0)
            inverterX = (velPxFrame.x < 0); // Se mover para a esquerda, inverte o personagem

        const float velPxSeg = (dt > 0.0f) ? Vector2Length(velPxFrame) / dt : 0.0f;
        if (velPxSeg < limiarWalk)
            estado = IDLE;
        else if (velPxSeg < limiarRun)
            estado = WALK;
        else
            estado = RUN;

        // Atualiza as animações com base no estado atual
        idle.Atualizar(dt, estado == IDLE);
        walk.Atualizar(dt, estado == WALK);
        run.Atualizar(dt, estado == RUN);
    }

    // Desenha a animação do jogador no centro de sua hitbox
    void DesenharNoCentroDoHitbox(const Rectangle &hit) const
    {
        const float cx = hit.x + hit.width * 0.5f;
        const float cy = hit.y + hit.height * 0.5f;
        const float drawX = cx - FrameW() * 0.5f;
        const float drawY = cy - FrameH() * 0.5f;

        const TiraAnim *a = (estado == RUN) ? &run : (estado == WALK) ? &walk
                                                                      : &idle;
        DrawTexturePro(a->tex, a->ObterSrc(inverterX),
                       Rectangle{drawX, drawY, (float)a->frameW, (float)a->frameH},
                       Vector2{0, 0}, 0.0f, WHITE);
    }

    // Libera os recursos das animações
    void Libertar()
    {
        idle.Libertar();
        walk.Libertar();
        run.Libertar();
    }
};

// Variáveis privadas (globais para o módulo)
json gMapa;         // Dados do mapa (Tiled)
Texture2D gAtlas{}; // Atlas de sprites (tileset)
int gMapaW = 0, gMapaH = 0, gTileW = 0, gTileH = 0, gFirstGid = 1, gAtlasCols = 1;
json gLayerGround, gLayerDecor, gLayerOver;
std::vector<Rectangle> gColisores; // Lista de colisores do mapa (objetos estáticos)

// Jogador e câmara
Rectangle gHitbox{};             // Hitbox do jogador
AnimadorJogador player;          // Gerenciador da animação do jogador
Camera2D gCam{};                 // Câmera 2D
bool gVerDebugColisores = false; // Flag para mostrar colisores no modo debug

// Função para carregar os sprites do jogador com base no índice de seleção
static void CarregarSpritesDoPlayerIndex(int selPlayer)
{
    const SpriteSetCfg &s = gSpriteSets[selPlayer % 3];
    std::string pIdle = std::string(s.baseDir) + "IDLE.png";
    std::string pWalk = std::string(s.baseDir) + "WALK.png";
    std::string pRun = std::string(s.baseDir) + "RUN.png";

    player.Libertar(); // Libera texturas antigas
    player.idle.Carregar(pIdle.c_str(), s.idleCols, s.idleFps);
    player.walk.Carregar(pWalk.c_str(), s.walkCols, s.walkFps);
    player.run.Carregar(pRun.c_str(), s.runCols, s.runFps);
}

// ================== Ciclo de vida do ecrã ==================
// Inicializa o jogo (carrega mapa, atlas de tiles, inimigos, etc.)
void Jogo_Init(GameState &G)
{
    const char *CAMINHO_MAPA = "assets/maps/Novomap.json";
    const char *CAMINHO_ATLAS = "assets/tilesets/tiles.png";

    // Carrega o mapa
    gMapa = json::parse(LerFicheiroTexto(CAMINHO_MAPA));
    gMapaW = gMapa["width"].get<int>();
    gMapaH = gMapa["height"].get<int>();
    gTileW = gMapa["tilewidth"].get<int>();
    gTileH = gMapa["tileheight"].get<int>();

    // Carrega o atlas de tiles
    const auto &ts = gMapa["tilesets"][0];
    gFirstGid = ts.value("firstgid", 1);
    gAtlasCols = ts.value("columns", 1);
    gAtlas = LoadTexture(CAMINHO_ATLAS);
    SetTextureFilter(gAtlas, TEXTURE_FILTER_POINT);

    // Carrega as camadas do mapa (Ground, Decor, Over)
    const auto &layers = gMapa["layers"];
    auto itGround = std::find_if(layers.begin(), layers.end(), [](const json &L)
                                 { return L["type"] == "tilelayer" && L["name"] == "Ground"; });
    auto itDecor = std::find_if(layers.begin(), layers.end(), [](const json &L)
                                { return L["type"] == "tilelayer" && L["name"] == "Decor"; });
    auto itOver = std::find_if(layers.begin(), layers.end(), [](const json &L)
                               { return L["type"] == "tilelayer" && L["name"] == "Over"; });
    gLayerGround = (itGround != layers.end()) ? *itGround : json();
    gLayerDecor = (itDecor != layers.end()) ? *itDecor : json();
    gLayerOver = (itOver != layers.end()) ? *itOver : json();

    // Carrega os colisores (objetos estáticos do mapa)
    gColisores.clear();
    for (const auto &L : layers)
    {
        if (L["type"] == "objectgroup" && L["name"] == "Collision")
        {
            for (const auto &o : L["objects"])
            {
                gColisores.push_back(Rectangle{
                    o.value("x", 0.0f), o.value("y", 0.0f),
                    o.value("width", 0.0f), o.value("height", 0.0f)});
            }
        }
    }

    // Carrega o jogador com base na classe selecionada
    CarregarSpritesDoPlayerIndex(G.selPlayer);
    PlayerClass cls = static_cast<PlayerClass>(G.selPlayer);
    G.stats = BaseStatsFor(cls);

    // Carrega inimigos
    Enemies_Load();

    // Define a posição inicial do jogador
    playerPosi(G);

    // Define a hitbox inicial do jogador
    gHitbox = RectCentrado(spawnCX, spawnCY, HIT_W, HIT_H);

    // Inicializa a câmara
    gCam = Camera2D{};
    gCam.zoom = 1.0f;
    gCam.offset = Vector2{(float)GetScreenWidth() * 0.5f, (float)GetScreenHeight() * 0.5f};
    gCam.target = Vector2{gHitbox.x + gHitbox.width * 0.5f, gHitbox.y + gHitbox.height * 0.5f};

    gVerDebugColisores = false;
}

// Função que atualiza o estado do jogo (movimento, colisões, animações)
void Jogo_Update(GameState &G, float dt)
{
    // Se pressionado "P", pausa o jogo
    if (IsKeyPressed(KEY_P))
    {
        G.screen = PAUSE;
        return;
    }

    // Alterna a visualização dos colisores (debug)
    if (IsKeyPressed(KEY_F1))
        gVerDebugColisores = !gVerDebugColisores;

    //-----------EXPLICAR APARTIR DAQUI------------------

    // Detecta se o jogador pressionou a tecla de ataque (espaço)
    if (IsKeyPressed(KEY_SPACE)) // função
    {
        float raio = 80.0f; // Define o alcance do ataque (hit scan circular)
        int dano = 25;      // Define o dano que o ataque causa

        // Verifica quais inimigos estão dentro do raio e aplica dano
        // Retorna o número de inimigos atingidos
        int n = Enemies_HitNearby(G, gHitbox, raio, dano);

        // Se pelo menos um inimigo foi atingido
        if (n > 0)
        {
            char txt[64];
            // Cria uma mensagem dinâmica indicando quantos inimigos foram atingidos
            snprintf(txt, sizeof(txt), "Acertei %d", n);
            // Mostra a mensagem na tela como bolha (bubble) por 1 segundo
            Bubble_Show(txt, 1.0f);
        }
        else
        {
            // Caso nenhum inimigo tenha sido atingido, exibe mensagem de falha
            Bubble_Show("Falhei...", 0.6f);
        }
    }

    // Atualiza o temporizador da bubble (mensagens temporárias)
    Bubble_Update(dt);

    // Atualiza os inimigos
    Enemies_Update(G, dt);

    // Lógica de movimento (baseado nas teclas pressionadas)
    auto &S = G.stats; // Referência direta às estatísticas do jogador
    bool runIntent = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    bool podeCorrer = (S.stamina > 0);
    float vel = (runIntent && podeCorrer) ? S.runSpeed : S.baseSpeed;

    // Calcula o deslocamento do jogador com base na velocidade
    Vector2 delta = CalcularDeslocamento(vel, dt);

    // Lógica de regeneração de stamina (andar/gastar ao correr)
    bool aMover = (fabsf(delta.x) > 0.0f || fabsf(delta.y) > 0.0f);
    bool isRunning = (runIntent && aMover && S.stamina > 0.0f);
    if (isRunning)
    {
        S.stamina -= 25.0f * dt; // Gasta stamina ao correr
        if (S.stamina < 0.0f)
            S.stamina = 0.0f;
    }
    else
    {
        S.stamina += S.staminaRegenPs * dt; // Regenera stamina quando anda ou pára
        if (S.stamina > S.maxStamina)
            S.stamina = S.maxStamina;
    }

    // Resolve as colisões no eixo X e Y
    ResolverEixo(gHitbox, delta.x, 0.0f, gColisores);
    ResolverEixo(gHitbox, 0.0f, delta.y, gColisores);

    // Limita a posição do jogador dentro do mundo
    const float mundoW = (float)gMapaW * gTileW;
    const float mundoH = (float)gMapaH * gTileH;
    gHitbox.x = Clamp(gHitbox.x, 0.0f, mundoW - gHitbox.width);
    gHitbox.y = Clamp(gHitbox.y, 0.0f, mundoH - gHitbox.height);

    // Atualiza a animação do jogador
    player.Atualizar(dt, delta);

    // A câmara segue o jogador, mas respeita os limites do mundo
    const float halfW = (float)GetScreenWidth() * 0.5f / gCam.zoom;
    const float halfH = (float)GetScreenHeight() * 0.5f / gCam.zoom;
    gCam.target = Vector2{gHitbox.x + gHitbox.width * 0.5f, gHitbox.y + gHitbox.height * 0.5f};
    gCam.target.x = (mundoW > halfW * 2) ? Clamp(gCam.target.x, halfW, mundoW - halfW) : mundoW * 0.5f;
    gCam.target.y = (mundoH > halfH * 2) ? Clamp(gCam.target.y, halfH, mundoH - halfH) : mundoH * 0.5f;

    // ----------------EXPLICAR APARTIR DAQUI----------------------
    // Se o jogador morreu, muda para a tela de GAMEOVER
    if (G.stats.hp <= 0)
    {
        G.screen = GAMEOVER;
        return; // Não continua atualizar após a morte
    }

    //--------------EXPLICAR APARTIR DAQUI------------------
    // --------------------------------------
    // Regeneração de vida do jogador
    // --------------------------------------
    static float regenTimer = 0.0f;
    regenTimer += dt;

    // Regenera a vida do jogador a cada 3 segundos
    if (regenTimer >= 3.0f)
    {
        regenTimer = 0.0f;
        int vidaExtra = 5; // Quanto vai curar
        int vidaMax = 100; // Define o máximo manualmente
        G.stats.hp = (G.stats.hp + vidaExtra > vidaMax) ? vidaMax : G.stats.hp + vidaExtra;
        TraceLog(LOG_INFO, "Vida regenerada: +%d (HP atual = %d)", vidaExtra, G.stats.hp);
    }
}

// Função que desenha todos os elementos do jogo (jogador, inimigos, fundo, HUD, etc.)
void Jogo_Draw(GameState &G)
{
    BeginMode2D(gCam); // Começa o modo 2D para desenhar com a câmera

    // Desenha as camadas do mapa
    if (!gLayerGround.is_null())
        DesenharLayerTiles(gAtlas, gFirstGid, gAtlasCols, gTileW, gTileH, gMapaW, gMapaH, gLayerGround);
    if (!gLayerDecor.is_null())
        DesenharLayerTiles(gAtlas, gFirstGid, gAtlasCols, gTileW, gTileH, gMapaW, gMapaH, gLayerDecor);

    // Desenha os inimigos
    Enemies_Draw();

    // Desenha o jogador
    player.DesenharNoCentroDoHitbox(gHitbox);

    // Desenha a camada superior (Over)
    if (!gLayerOver.is_null())
        DesenharLayerTiles(gAtlas, gFirstGid, gAtlasCols, gTileW, gTileH, gMapaW, gMapaH, gLayerOver);

    // Desenha a bubble com o texto (exibição temporária de mensagens)
    Vector2 anchor = {gHitbox.x + gHitbox.width * 0.5f, gHitbox.y - 6.0f};
    Bubble_Draw(anchor);

    // Debug: Desenha as colisões (caixas vermelhas para colisores e caixa verde para a hitbox do jogador)
    if (gVerDebugColisores)
    {
        for (const auto &c : gColisores)
            DrawRectangleLines((int)c.x, (int)c.y, (int)c.width, (int)c.height, RED);
        DrawRectangleLines((int)gHitbox.x, (int)gHitbox.y, (int)gHitbox.width, (int)gHitbox.height, GREEN);
    }

    EndMode2D(); // Finaliza o modo 2D

    // Desenha o HUD (barra de vida, stamina, etc.)
    auto &S = G.stats; // Referência direta às estatísticas do jogador
    int x = 10, y = 10, w = 180, h = 16;

    DrawText(TextFormat("%s", NomeClasse(G.selPlayer)), x, y, 18, RAYWHITE);
    DrawText(TextFormat("HP: %d/%d", S.hp, S.maxHP), x, y = 440, 14, RAYWHITE);
    DrawBar(x + 90, y = 440, w, h, (float)S.hp / S.maxHP, RED);

    DrawText(TextFormat("STA: %d/%d", S.stamina, S.maxStamina), x, y = 460, 14, RAYWHITE);
    DrawBar(x + 90, y = 460, w, h, (float)S.stamina / S.maxStamina, GREEN);

    DrawText("SHIFT: Correr  |  ESPAÇO: Habilidade", x, y = 480, 12, LIGHTGRAY);
    DrawText(TextFormat("SCORE: %d", G.score), x, y = 30, 14, YELLOW);
}

// Libera os recursos usados no jogo (texturas, animações, inimigos, etc.)
void Jogo_Unload(GameState &)
{
    player.Libertar();
    Enemies_Unload();

    if (gAtlas.id)
    {
        UnloadTexture(gAtlas);
        gAtlas.id = 0;
    }
    gMapa = json();
    gColisores.clear();
}
