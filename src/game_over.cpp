#include "raylib.h"
#include "game.h"

// ============================================================================
// Função de atualização da tela de Game Over
// ============================================================================
// Recebe o estado do jogo e o delta time (não usado aqui)
void GameOver_Update(GameState& G, float)
{
    // Se o jogador apertar ENTER, volta para o menu principal
    if (IsKeyPressed(KEY_ENTER)) {
        G.screen = MENU_PLAYER;      // Muda a tela para MENU_PLAYER
        G.jogoInicializado = false;  // Reseta flag para permitir reinicialização do jogo
    }
}

// ============================================================================
// Função de desenho da tela de Game Over
// ============================================================================
void GameOver_Draw(GameState&)
{
    ClearBackground(RED);  // Preenche a tela de vermelho para indicar Game Over

    // Desenha o texto "GAME OVER" na tela
    DrawText("GAME OVER", 125, 200, 40, RAYWHITE);

    // Desenha instrução para voltar ao menu
    DrawText("ENTER para voltar", 125, 240, 20, RAYWHITE);
}
