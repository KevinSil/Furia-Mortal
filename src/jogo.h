// jogo.h
// Declarações públicas para o ecrã JOGO: inicialização, atualização, desenho e unload.
// Comentários em Português (Portugal).

#pragma once
#include "game.h"

// Inicializa o estado do jogo (carregar mapa, texturas, inimigos, etc.)
void Jogo_Init(GameState &G);

// Determina a posição de spawn do jogador consoante o mapa selecionado
void playerPosi(GameState &G);

// (Placeholders) funções auxiliares usadas anteriormente
void NewFunction(GameState &G);
void NewFunction(GameState &G);
void posi(GameState &G);

// Ciclo principal do ecrã JOGO
void Jogo_Update(GameState &G, float dt);
void Jogo_Draw(GameState &G);
void Jogo_Unload(GameState &G);
