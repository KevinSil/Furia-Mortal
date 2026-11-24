#include "raylib.h"
#include "game.h"

// --- Unity build: inclui diretamente os .cpp da pasta src ---
// Isso permite compilar tudo junto sem precisar criar múltiplos arquivos de objeto
#include "src/menu_player.cpp"
#include "src/menu_map.cpp"
#include "src/jogo.cpp"
#include "src/pause.cpp"
#include "src/bubble.cpp"
#include "src/enemies.cpp"
#include "src/game_over.cpp"
#include "src/helper.cpp"
#include "src/menu_logo.cpp"

int main() {
    // Inicializa a janela com largura 500, altura 500 e título
    InitWindow(500, 500, "Fúria Mortal👻");
    SetTargetFPS(60);  // Define o FPS alvo

    GameState G;       // Estrutura principal que guarda o estado do jogo

    // Loop principal do jogo
    while (!WindowShouldClose()) {
        float dt = GetFrameTime(); // Delta time: tempo desde o último frame

        // ---------------- UPDATE ----------------
        // Atualiza o estado do jogo de acordo com a tela atual
        switch (G.screen) {
            case MENU_JOGO:    MenuLogo_Update(G, dt); break;
            case MENU_PLAYER:  MenuPlayer_Update(G, dt); break;
            case MENU_MAP:     MenuMap_Update(G, dt); break;
            case JOGO:         Jogo_Update(G, dt); break;
            case PAUSE:        Pause_Update(G, dt); break;
            case GAMEOVER:     GameOver_Update(G, dt); break;
        }

        // ---------------- DRAW ----------------
        BeginDrawing();  // Inicia a renderização do frame
        ClearBackground((Color){ 156, 30, 72, 255 }); // Cor de fundo padrão

        // Desenha a tela de acordo com o estado atual
        switch (G.screen) {
            case MENU_JOGO:    MenuLogo_Draw(G); break;
            case MENU_PLAYER:  MenuPlayer_Draw(G); break;
            case MENU_MAP:     MenuMap_Draw(G); break; 
            case JOGO:         Jogo_Draw(G); break;
            case PAUSE:        Jogo_Draw(G); Pause_Draw(G); break; // Sobrepõe menu de pause
            case GAMEOVER:     GameOver_Draw(G); break;
        }
        EndDrawing(); // Finaliza a renderização do frame
    }

    // Se o jogo estava inicializado, libera recursos
    if (G.jogoInicializado) 
        Jogo_Unload(G);

    CloseWindow(); // Fecha a janela e finaliza Raylib
    return 0;
}
