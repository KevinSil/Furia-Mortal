// pause.h
// Declarações do ecrã de pausa.

#pragma once
#include "game.h"

// Actualiza o estado do ecrã de pausa (input)
void Pause_Update(GameState &G, float dt);

// Desenha o overlay de pausa
void Pause_Draw(GameState &G);
