#include "menu_map.h"
#include "jogo.h"
#include "raylib.h"

const char *mapas[] = {"America do Norte", "África", "Europa"};

// escolher spawn conforme o mapa
const float HIT_W = 32.0f, HIT_H = 32.0f;

float spawnCX = 380.0f;
float spawnCY = 780.0f;

void MenuMap_Update(GameState &G, float)
{
    if (IsKeyPressed(KEY_RIGHT))
        G.selMap = (G.selMap + 1) % 3;
    if (IsKeyPressed(KEY_LEFT))
        G.selMap = (G.selMap + 2) % 3;
    if (IsKeyPressed(KEY_BACKSPACE))
        G.screen = MENU_PLAYER;

    if (IsKeyPressed(KEY_ENTER))
    {
        if (!G.jogoInicializado)
        {
            Jogo_Init(G);
            G.jogoInicializado = true;
        }
        G.screen = JOGO;
    }
}

void MenuMap_Draw(GameState &G)
{
    ClearBackground(DARKGREEN);
    DrawText("Escolhe o MAPA (← → ENTER; BACKSPACE volta)", 12, 40, 15, RAYWHITE);
    DrawText(TextFormat("Mapa: %s", mapas[G.selMap]), 30, 80, 20, YELLOW);
}

void playerPosi(GameState &G)
{
    // posição player
    switch (G.selMap)
    {
    case 0: // AMN
        spawnCX = 380.0f;
        spawnCY = 780.0f;
        break;
    case 1: // AFRICA
        spawnCX = 6400.0f;
        spawnCY = 3936.0f;
        break;
    case 2: // EUROPA
        spawnCX = 6528.0f;
        spawnCY = 1696.0f;
        break;
    }
}
