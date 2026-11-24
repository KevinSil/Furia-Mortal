// bubble.cpp
// Pequeno utilitário para mostrar uma 'bubble' de texto sobre o jogador/inimigos.
// Comentários em Português (Portugal). Funções: Bubble_Show, Bubble_Update, Bubble_Draw.

#include "bubble.h"
#include "raylib.h"
#include <string>

// Estado interno da bubble (visibilidade, temporizador e texto)
static bool gVisible = false; // Indica se a bubble deve ser desenhada
static float gT = 0.0f;       // Tempo decorrido desde que a bubble apareceu
static float gDur = 0.0f;     // Duração total da bubble em segundos
static std::string gText;     // Texto que será mostrado na bubble

// Desenha uma caixa arredondada com o texto centrado acima do 'anchor' (coordenadas no mundo)
// - anchor: ponto de referência (por exemplo, cima do hitbox do jogador)
// - text: texto a mostrar dentro da bubble
// - fontSize: tamanho da fonte do texto
// - padding: espaço extra ao redor do texto, para melhorar o visual
static void DrawSpeechBubbleAt(Vector2 anchor, const char *text, float fontSize = 16.0f, float padding = 8.0f)
{
    Font font = GetFontDefault(); // Obtém a fonte padrão do Raylib
    const float spacing = 1.0f;   // Espaçamento entre os caracteres do texto
    Vector2 tsize = MeasureTextEx(font, text, fontSize, spacing); // Medir o tamanho do texto

    // Calcula o retângulo da bubble, ajustando a posição com um pequeno deslocamento vertical
    Rectangle box{
        anchor.x - (tsize.x * 0.5f) - padding, // Ajuste horizontal para centralizar
        anchor.y - (tsize.y + padding * 2.0f) - 8.0f, // Ajuste vertical para colocar acima do anchor
        tsize.x + padding * 2.0f, // Largura com o padding extra
        tsize.y + padding * 2.0f   // Altura com o padding extra
    };

    // Desenha a caixa arredondada com fundo suave e borda preta
    DrawRectangleRounded(box, 0.35f, 10, Fade(RAYWHITE, 0.95f)); // Fundo com opacidade
    DrawRectangleRoundedLines(box, 0.35f, 10, BLACK); // Borda preta arredondada

    // Desenha o texto dentro da bubble, com o padding definido
    DrawTextEx(font, text, {box.x + padding, box.y + padding}, fontSize, spacing, BLACK);
}

// Função para mostrar a bubble com o 'text' durante um determinado número de segundos
// - text pode ser NULL para limpar a bubble
void Bubble_Show(const char *text, float seconds)
{
    gText = text ? text : ""; // Define o texto ou limpa a bubble se text for NULL
    gDur = (seconds > 0.0f) ? seconds : 2.0f; // Define a duração, ou usa 2 segundos por defeito
    gT = 0.0f; // Reinicia o temporizador
    gVisible = true; // Torna a bubble visível
}

// Função para atualizar o temporizador da bubble; esconde a bubble quando o tempo expira
void Bubble_Update(float dt)
{
    if (!gVisible) // Se a bubble não está visível, não faz nada
        return;
    gT += dt; // Atualiza o temporizador com o tempo decorrido (delta time)
    if (gT >= gDur) // Se o tempo decorrido for maior ou igual à duração
        gVisible = false; // Esconde a bubble
}

// Função para desenhar a bubble no mundo na posição 'anchorWorld'
// Esta função é chamada por quem controla a HUD do jogo
void Bubble_Draw(Vector2 anchorWorld)
{
    if (!gVisible) // Se a bubble não estiver visível, não desenha nada
        return;
    DrawSpeechBubbleAt(anchorWorld, gText.c_str()); // Desenha a bubble na posição 'anchorWorld' com o texto
}
