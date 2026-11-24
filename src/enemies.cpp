// ============================================================================
// enemies.cpp — IA simples de inimigos (Patrulha + Perseguição) com sprites
// ============================================================================

// Inclui a header dos inimigos, que define EnemyKind, EnemyStats e funções relacionadas
#include "enemies.h"

// Biblioteca raymath para operações com vetores (normalização, escala, etc.)
#include "raymath.h"

// Bibliotecas padrão C++ usadas no código
#include <vector>     // std::vector para listas de inimigos e obstáculos
#include <string>     // std::string para montar caminhos de arquivos
#include <algorithm>  // std::max e std::min para limitar valores
#include <cmath>      // sqrt, fabs, etc. para cálculos matemáticos
#include <random>     // Geração de números aleatórios para spawn de inimigos

// ============================================================================
// VARIÁVEIS EXTERNAS DO jogo.cpp
// ============================================================================
// Estas variáveis existem em jogo.cpp, mas são usadas aqui:
// • gHitbox → hitbox do jogador
// • gCam → câmera do jogo
// • gColisores → obstáculos (paredes, chão, etc.)
extern Rectangle gHitbox;
extern Camera2D gCam;
extern std::vector<Rectangle> gColisores;

// Função externa que resolve movimentação com colisão
extern void ResolverEixo(Rectangle &r, float dx, float dy, const std::vector<Rectangle> &colisores);

// ============================================================================
// CONFIGURAÇÃO DE TIPOS DE INIMIGO
// ============================================================================
// Struct que define parâmetros de cada tipo de inimigo
struct EnemySetCfg
{
    const char *baseDir;  // Diretório dos sprites do inimigo
    int walkCols;         // Número de frames da animação de andar
    float walkFps;        // Velocidade da animação
    EnemyStats baseStats; // Estatísticas base (HP, velocidade, dano, etc.)
};

// Lista com diferentes tipos de inimigos e suas configurações
static const EnemySetCfg gEnemySets[] = {
    {"assets/enemies/slime/",    5, 2.0f, { 8, 60.f, 65.f, 120.f, 0.5f}}, // Slime lento
    {"assets/enemies/goblin/",   2, 6.0f, {12, 70.f, 30.f, 140.f, 0.5f}}, // Goblin rápido
    {"assets/enemies/skeleton/", 2, 8.0f, {14, 55.f, 68.f, 110.f, 0.6f}}, // Esqueleto forte
};

// ============================================================================
// HELPERS MATEMÁTICOS
// ============================================================================

// Calcula o comprimento (módulo) de um vetor 2D
static inline float Len(Vector2 v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

// Normaliza um vetor (torna de comprimento 1)
static inline Vector2 Norm(Vector2 v)
{
    float l = Len(v);
    return (l > 0) ? Vector2{v.x / l, v.y / l} : Vector2{0, 0};
}

// ============================================================================
// ESTRUTURA DE UM INIMIGO INDIVIDUAL
// ============================================================================
struct Enemy
{
    Rectangle hit{0, 0, 24, 24};       // Hitbox do inimigo
    bool faceLeft = false;             // Direção do sprite
    EnemyKind kind = EnemyKind::SLIME; // Tipo do inimigo

    int hp = 50;       // Vida inicial
    bool alive = true; // Se falso, inimigo é ignorado

    float patrolLeftX = 0.f, patrolRightX = 0.f; // Limites de patrulha
    int dir = +1;                                 // Direção (+1 direita, -1 esquerda)

    enum State { Patrol, Chase } state = Patrol; // Estado atual da IA
    float touchTimer = 0.f;                      // Cooldown de dano ao jogador

    EnemyStats stats{}; // Estatísticas do inimigo
    TiraAnim walk;      // Animação de andar (spritesheet)

    // ========================================================================
    // FUNÇÕES DO INIMIGO
    // ========================================================================

    // Carrega sprites e stats do tipo
    void Load(const EnemySetCfg &C)
    {
        std::string p = std::string(C.baseDir) + "WALK.png"; // Caminho do spritesheet
        walk.Carregar(p.c_str(), C.walkCols, C.walkFps);    // Carrega animação
        stats = C.baseStats;                                // Carrega stats base
    }

    // Verifica se o jogador está dentro do aggroRadius ou visível pela câmera
    bool PlayerPertoOuVisivel() const
    {
        Vector2 pc{gHitbox.x + gHitbox.width*0.5f, gHitbox.y + gHitbox.height*0.5f};
        Vector2 ec{hit.x + hit.width*0.5f, hit.y + hit.height*0.5f};

        // Checa distância para aggro
        if (Len(Vector2{pc.x - ec.x, pc.y - ec.y}) <= stats.aggroRadius)
            return true;

        // Checa se inimigo está dentro da visão da câmera
        float halfW = GetScreenWidth()  * 0.5f / gCam.zoom;
        float halfH = GetScreenHeight() * 0.5f / gCam.zoom;
        Rectangle view{gCam.target.x - halfW, gCam.target.y - halfH, halfW*2, halfH*2};

        return IntersetaAABB(hit, view);
    }

    // Atualiza comportamento do inimigo
    void Update(GameState &G, float dt)
    {
        // Define estado: patrulha ou perseguição
        state = PlayerPertoOuVisivel() ? Chase : Patrol;

        Vector2 vel{0, 0};

        if (state == Patrol)
        {
            vel.x = dir * stats.patrolSpeed; // Movimenta na direção da patrulha
        }
        else
        {
            // Calcula vetor direção até jogador e multiplica pela velocidade de chase
            Vector2 pc{gHitbox.x + gHitbox.width*0.5f, gHitbox.y + gHitbox.height*0.5f};
            Vector2 ec{hit.x + hit.width*0.5f, hit.y + hit.height*0.5f};
            vel = Vector2Scale(Norm(Vector2{pc.x - ec.x, pc.y - ec.y}), stats.chaseSpeed);
        }

        // Aplica movimento com colisão
        ResolverEixo(hit, vel.x * dt, 0, gColisores);
        ResolverEixo(hit, 0, vel.y * dt, gColisores);

        // Reverte direção se bateu nos limites da patrulha
        if (state == Patrol)
        {
            if (hit.x <= patrolLeftX) dir = +1;
            if (hit.x + hit.width >= patrolRightX) dir = -1;
        }

        // Atualiza direção visual do sprite
        faceLeft = (vel.x < 0);
        bool moving = (std::fabs(vel.x) > 0.1f || std::fabs(vel.y) > 0.1f);
        walk.Atualizar(dt, moving);

        // Dano ao jogador
        if (touchTimer > 0)
            touchTimer -= dt;

        if (IntersetaAABB(hit, gHitbox) && touchTimer <= 0)
        {
            G.stats.hp = std::max(0, G.stats.hp - stats.damage);
            touchTimer = stats.touchCooldown;
        }
    }

    // Desenha o inimigo usando DrawTexturePro
    void Draw() const
    {
        float cx = hit.x + hit.width * 0.5f;
        float cy = hit.y + hit.height * 0.5f;

        DrawTexturePro(
            walk.tex,
            walk.ObterSrc(faceLeft),
            Rectangle{cx - walk.frameW*0.5f, cy - walk.frameH*0.5f,
                      (float)walk.frameW, (float)walk.frameH},
            Vector2{0, 0}, 0.f, WHITE);
    }

    void Unload()
    {
        walk.Libertar();
    }
};

// ============================================================================
// LISTA GLOBAL DE INIMIGOS
// ============================================================================
static std::vector<Enemy> gEnemies;

// ============================================================================
// SPAWN MANUAL DE INIMIGOS
// ============================================================================
void Enemies_Spawn(EnemyKind kind, Vector2 pos, float leftX, float rightX, int dir)
{
    const EnemySetCfg &C = gEnemySets[(int)kind];

    Enemy e;
    e.Load(C);                    // Carrega sprites e stats
    e.hit = Rectangle{pos.x, pos.y, 24, 24};
    e.patrolLeftX  = leftX;       // Limite esquerdo da patrulha
    e.patrolRightX = rightX;      // Limite direito
    e.dir = dir;
    e.hp = 50;
    e.alive = true;
    e.kind = kind;

    gEnemies.push_back(e);         // Adiciona à lista global
}

// ============================================================================
// SPAWN ALEATÓRIO DE INIMIGOS
// ============================================================================
void Enemies_SpawnRandom(int quantidade, EnemyKind tipo, Rectangle areaMapa)
{
    // Inicializa um gerador de números aleatórios estático (uma vez apenas)
    static std::random_device rd;   // Fonte de entropia do sistema
    static std::mt19937 rng(rd());  // Motor Mersenne Twister

    // Distribuições uniformes para gerar posições X e Y dentro da área do mapa
    std::uniform_real_distribution<float> dx(areaMapa.x, areaMapa.x + areaMapa.width);
    std::uniform_real_distribution<float> dy(areaMapa.y, areaMapa.y + areaMapa.height);

    // Limita o número de tentativas para evitar loop infinito
    int tentativasMax = quantidade * 200;
    int criados = 0;  // Contador de inimigos criados

    // Loop até criar a quantidade desejada ou esgotar tentativas
    while (criados < quantidade && tentativasMax-- > 0)
    {
        // Gera uma posição aleatória dentro da área
        Vector2 pos{dx(rng), dy(rng)};
        Rectangle hit{pos.x, pos.y, 24, 24}; // Hitbox do inimigo

        // Verifica colisão com obstáculos para evitar spawn sobre eles
        bool colide = false;
        for (const auto &c : gColisores)
        {
            if (CheckCollisionRecs(hit, c))
            {
                colide = true;
                break;
            }
        }
        if (colide) continue; // Se colidir, pula para a próxima tentativa

        // Evita spawn em cima do jogador
        if (CheckCollisionRecs(hit, gHitbox)) continue;

        // Spawn final do inimigo com patrulha horizontal aleatória
        float patrolRange = 80.f; // Alcance da patrulha
        Enemies_Spawn(tipo, pos,
                      pos.x - patrolRange*0.5f, // Limite esquerdo da patrulha
                      pos.x + patrolRange*0.5f, // Limite direito da patrulha
                      (rng() % 2 == 0) ? +1 : -1); // Direção inicial aleatória (+1 ou -1)
        criados++; // Incrementa contador de inimigos criados
    }

    // Loga no console quantos inimigos foram realmente spawnados
    TraceLog(LOG_INFO, "Spawnados %d inimigos (tipo=%d)", criados, (int)tipo);
}


// ============================================================================
// LOAD INICIAL DOS INIMIGOS
// ============================================================================
void Enemies_Load()
{
    gEnemies.clear();

    Rectangle areaMapa = {0, 0, 7000, 4000}; // Área total do mapa

    // Spawn inicial de inimigos
    Enemies_SpawnRandom(80, EnemyKind::SLIME,    areaMapa);
    Enemies_SpawnRandom(50, EnemyKind::SKELETON, areaMapa);
}

// ============================================================================
// UPDATE GERAL DOS INIMIGOS
// ============================================================================
void Enemies_Update(GameState &G, float dt)
{
    for (auto &e : gEnemies)
    {
        if (!e.alive) continue;
        e.Update(G, dt);
    }

    // Respawn contínuo próximo da câmera
    static float spawnTimer = 0;
    spawnTimer += dt;

    if (spawnTimer >= 2.0f)
    {
        spawnTimer = 0.0f;

        Rectangle aroundCam = {
            gCam.target.x - 600,
            gCam.target.y - 400,
            1200,
            800
        };

        Enemies_SpawnRandom(3, EnemyKind::SLIME,    aroundCam);
        Enemies_SpawnRandom(2, EnemyKind::SKELETON, aroundCam);
    }
}

// ============================================================================
// DESENHA TODOS OS INIMIGOS
// ============================================================================
void Enemies_Draw()
{
    for (const auto &e : gEnemies)
    {
        if (!e.alive) continue;
        e.Draw();
    }
}

// ============================================================================
// LIBERA RECURSOS E LIMPA LISTA AQUIIIIIII
// ============================================================================
// Função que libera todos os recursos usados pelos inimigos
void Enemies_Unload()
{
    // Para cada inimigo na lista global
    for (auto &e : gEnemies)
        e.Unload();  // Libera recursos individuais (como texturas da animação)

    // Limpa a lista global de inimigos, removendo todos os elementos
    gEnemies.clear();
}

// ============================================================================
// ATAQUE DO JOGADOR — Hit Scan Circular
// ============================================================================
// Função que verifica inimigos próximos a um ataque do jogador
// playerHit: hitbox do jogador
// raio: distância do ataque (hit scan circular)
// dano: quantidade de dano a aplicar
// Retorna a quantidade de inimigos atingidos
int Enemies_HitNearby(GameState &G, const Rectangle &playerHit, float raio, int dano)
{
    // Calcula o centro da hitbox do jogador
    float px = playerHit.x + playerHit.width  * 0.5f;
    float py = playerHit.y + playerHit.height * 0.5f;

    int atingidos = 0; // Contador de inimigos atingidos

    // Percorre todos os inimigos da lista global
    for (auto &e : gEnemies)
    {
        if (!e.alive) continue; // Ignora inimigos mortos

        // Calcula o centro da hitbox do inimigo
        float ex = e.hit.x + e.hit.width * 0.5f;
        float ey = e.hit.y + e.hit.height * 0.5f;

        // Distância ao quadrado entre jogador e inimigo
        float dx = ex - px;
        float dy = ey - py;
        float dist2 = dx*dx + dy*dy;

        // Se o inimigo estiver dentro do raio do ataque
        if (dist2 <= raio * raio)
        {
            e.hp -= dano; // Aplica dano

            // Se a vida acabar, marca como morto e dá pontos
            if (e.hp <= 0)
            {
                e.alive = false;

                // Pontuação por tipo de inimigo
                int pts = 0;
                switch (e.kind)
                {
                    case EnemyKind::SLIME:    pts = 10; break;
                    case EnemyKind::GOBLIN:   pts = 20; break;
                    case EnemyKind::SKELETON: pts = 30; break;
                    default:                  pts = 5;  break;
                }
                G.score += pts; // Atualiza a pontuação do jogador
            }

            atingidos++; // Incrementa o contador
        }
    }

    return atingidos; // Retorna quantos inimigos foram atingidos
}
