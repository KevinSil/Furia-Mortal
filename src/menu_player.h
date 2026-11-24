// menu_player.h
// Declarações do ecrã de selecção de jogador (Menu Player)

#pragma once
#include "game.h"

// Actualiza o menu de escolha do jogador
void MenuPlayer_Update(GameState &G, float dt);

// Desenha o menu de escolha do jogador
void MenuPlayer_Draw(GameState &G);