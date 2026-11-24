// bubble.h
// Cabeçalho público da bubble (uma pequena caixa de texto flutuante).

#pragma once
#include "raylib.h" // Inclui a biblioteca Raylib para gráficos e manipulação de objetos 2D

// Declaração das funções da API minimalista para manipulação da 'bubble'

// Função para mostrar a bubble com o texto especificado durante um tempo definido
// - text: Texto a ser exibido na bubble
// - seconds: Tempo de exibição da bubble em segundos
void Bubble_Show(const char *text, float seconds);

// Função para atualizar o temporizador da bubble
// - dt: Delta time (tempo que passou desde a última atualização)
// Esta função verifica se o tempo de exibição da bubble expirou e a esconde se necessário.
void Bubble_Update(float dt);

// Função para desenhar a bubble na posição do mundo indicada pelo parâmetro 'anchorWorld'
// - anchorWorld: Posição no mundo onde a bubble será desenhada (por exemplo, acima do jogador ou inimigo)
void Bubble_Draw(Vector2 anchorWorld);
