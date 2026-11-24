#include "raylib.h"  // Biblioteca principal do Raylib
#include "game.h"    // Cabeçalho do sistema de jogo
#include "helper.h"  // Funções auxiliares
#include "raymath.h" // Biblioteca de matemática do Raylib
#include <vector>    // Vetor dinâmico (C++)
#include <cmath>     // Funções matemáticas

// ============================================================
//  SISTEMA DE IMAGEM TEMPORÁRIA COM FADE-OUT
// ============================================================

// Estado que indica se a imagem está ativa
static bool tempImgActive = false;
// Tempo restante antes da imagem sumir
static float tempImgTimer = 0.0f;
// Tempo total que a imagem deve ficar na tela
static float tempImgDuration = 0.0f;
// Textura da imagem temporária
static Texture2D tempImg;

// Exibe uma imagem temporária na tela por "seconds" segundos
void TempImg_Show(const char *path, float seconds)
{
    // Se já existe uma imagem ativa, descarrega antes de carregar outra
    if (tempImgActive)
        UnloadTexture(tempImg);

    tempImg = LoadTexture(path); // Carrega nova imagem
    tempImgDuration = seconds;
    tempImgTimer = seconds;
    tempImgActive = true; // Marca como ativa
}

void TempImg_Update(float dt)
{
    if (!tempImgActive)
        return; // Se não tem imagem, sai

    tempImgTimer -= dt; // Conta regressiva
    if (tempImgTimer <= 0.0f)
    {
        UnloadTexture(tempImg); // Remove imagem da memória
        tempImgActive = false;  // Marca como inativa
    }
}

void TempImg_Draw()
{
    if (!tempImgActive)
        return;

    float alpha = 1.0f;           // Transparência padrão (visível)
    const float FADE_TIME = 0.6f; // Tempo de fade-out

    // Reduz alpha no final do tempo
    if (tempImgTimer < FADE_TIME)
        alpha = tempImgTimer / FADE_TIME;

    // Garante que alpha sempre permaneça entre 0 e 1
    if (alpha < 0.0f)
        alpha = 0.0f;
    if (alpha > 1.0f)
        alpha = 1.0f;

    unsigned char A = (unsigned char)(alpha * 255); // Converte para 0–255

    // Centraliza imagem na tela
    int x = GetScreenWidth() / 2 - tempImg.width / 2;
    int y = GetScreenHeight() / 2 - tempImg.height / 2;

    // Desenha imagem com transparência
    DrawTexture(tempImg, x, y, (Color){255, 255, 255, A});

    // Texto fixo
    DrawText("Kevin Silva |Nº3524| Dev Junior", 20, 430, 30, WHITE);
}

// ============================================================
//  MENU COM ANIMAÇÃO QUE SÓ COMEÇA DEPOIS DA IMAGEM SUMIR
// ============================================================

void MenuLogo_Update(GameState &G, float dt)
{
    TempImg_Update(dt);

    // Clique do mouse avança o menu
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        G.screen = MENU_PLAYER;
}

void MenuLogo_Draw(GameState &G)
{
    // Variáveis estáticas: preservam valor entre frames
    static bool inited = false;           // Controle de inicialização única
    static bool animationStarted = false; // Controle da animação

    static TiraAnim knightAnim;  // Animação do cavaleiro
    static TiraAnim enemyAnim;   // Animação do inimigo
    static Rectangle kHit;       // Hitbox do cavaleiro
    static Rectangle eHit;       // Hitbox do inimigo
    static float kSpeed = 90.0f; // Velocidade do cavaleiro
    static float eSpeed = 70.0f; // Velocidade do inimigo

    static bool kFacingLeft = false, eFacingLeft = false;

    // Estrutura para partículas de estrelas
    struct Star
    {
        Vector2 pos;
        float size;
        float phase;
    };
    static std::vector<Star> stars;

    // Inicialização (só roda uma vez)
    if (!inited)
    {
        knightAnim.Carregar("assets/players/cavaleiro/WALK.png", 8, 10.0f);
        enemyAnim.Carregar("assets/enemies/slime/WALK.png", 5, 6.0f);

        // Posiciona hitboxes
        kHit = RectCentrado(80.0f, 360.0f, 32.0f, 32.0f);
        eHit = RectCentrado(20.0f, 360.0f, 24.0f, 24.0f);

        // Gera estrelas
        stars.clear();
        const int STAR_COUNT = 80;
        int screenW = GetScreenWidth();
        for (int i = 0; i < STAR_COUNT; ++i)
        {
            float sx = (float)GetRandomValue(0, screenW);
            float sy = (float)GetRandomValue(0, 340);
            float ssize = (float)GetRandomValue(1, 3);
            float sphase = (float)GetRandomValue(0, 314) / 100.0f;
            stars.push_back(Star{{sx, sy}, ssize, sphase});
        }

        // Mostra splash de 3 segundos
        TempImg_Show("assets/splash.png", 3.0f);
        inited = true;
    }

    float dt = GetFrameTime();

    // ============================================================
    // Inicia animação somente depois que a imagem sumir
    // ============================================================

    if (!tempImgActive)
        animationStarted = true;

    if (animationStarted)
    {
        // Move cavaleiro para direita
        kHit.x += kSpeed * dt;
        if (kHit.x > (float)GetScreenWidth() + 40.0f)
            kHit.x = -60.0f; // Volta para a esquerda da tela

        // Move slime em direção ao cavaleiro
        Vector2 pc{kHit.x + kHit.width * 0.5f, kHit.y + kHit.height * 0.5f};
        Vector2 ec{eHit.x + eHit.width * 0.5f, eHit.y + eHit.height * 0.5f};
        Vector2 dir = Vector2Normalize(Vector2Subtract(pc, ec));

        eHit.x += dir.x * eSpeed * dt;
        eHit.y += dir.y * eSpeed * dt;

        // Atualiza animações
        knightAnim.Atualizar(dt, true);
        enemyAnim.Atualizar(dt, true);

        kFacingLeft = false;
        eFacingLeft = (dir.x < 0); // Inimigo vira dependendo da direção
    }

    // ============================================================

    ClearBackground((Color){20, 28, 48, 255}); // Fundo azul escuro

    // Desenha estrelas com efeito de brilho
    for (const Star &s : stars)
    {
        float a = 155.0f + 100.0f * sinf(s.phase + GetTime() * 2.0f);
        if (a < 0)
            a = 0;
        if (a > 255)
            a = 255;

        DrawCircleV(s.pos, s.size, (Color){255, 255, 255, (unsigned char)a});
    }

    // Grama embaixo
    DrawRectangle(0, 380, GetScreenWidth(), 120, (Color){40, 80, 30, 255});

    // INIMIGO
    {
        float cx = eHit.x + eHit.width * 0.5f;
        float cy = eHit.y + eHit.height * 0.5f;
        float drawX = cx - enemyAnim.frameW * 0.5f;
        float drawY = cy - enemyAnim.frameH * 0.5f;

        DrawTexturePro(
            enemyAnim.tex,
            enemyAnim.ObterSrc(eFacingLeft),
            Rectangle{drawX, drawY, (float)enemyAnim.frameW, (float)enemyAnim.frameH},
            Vector2{0, 0}, 0.0f, WHITE);
    }

    // CAVALEIRO
    {
        float cx = kHit.x + kHit.width * 0.5f;
        float cy = kHit.y + kHit.height * 0.5f;
        float drawX = cx - knightAnim.frameW * 0.5f;
        float drawY = cy - knightAnim.frameH * 0.5f;

        DrawTexturePro(
            knightAnim.tex,
            knightAnim.ObterSrc(kFacingLeft),
            Rectangle{drawX, drawY, (float)knightAnim.frameW, (float)knightAnim.frameH},
            Vector2{0, 0}, 0.0f, WHITE);
    }

    // Título do jogo
    DrawText("FÚRIA MORTAL", 100, 150, 40, RED);
    // Botão (ainda não interativo)
    DrawText("JOGAR", 170, 200, 40, WHITE);

    // Desenha splash por cima de tudo
    TempImg_Draw();
}
