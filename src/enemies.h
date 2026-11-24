// src/enemies.h
#pragma once
// Garante que este header seja incluído apenas uma vez na compilação

#include "raylib.h"
#include "jogo.h"
#include "game.h" // Necessário para conhecer a estrutura GameState

// ------------------------------------------------------------
// Enum que define os tipos de inimigos existentes no jogo
// ------------------------------------------------------------
enum class EnemyKind
{
    SLIME,     // Inimigo básico, geralmente lento e fraco
    GOBLIN,    // Médio: mais rápido e causa mais dano
    SKELETON   // Forte: maior alcance e agressividade
};

// ------------------------------------------------------------
// Estrutura com os atributos básicos de um inimigo
// Muito semelhante à estrutura do jogador (PlayerStats)
// ------------------------------------------------------------
struct EnemyStats
{
    int damage;          // Dano causado ao tocar no jogador
    float patrolSpeed;   // Velocidade quando o inimigo está em patrulha
    float chaseSpeed;    // Velocidade quando está perseguindo o jogador
    float aggroRadius;   // Distância máxima para detectar o jogador e iniciar a perseguição
    float touchCooldown; // Tempo mínimo entre danos sucessivos ao jogador
};

// ------------------------------------------------------------
// API de gestão de inimigos — responsável por:
// • carregar inimigos
// • criar/spawnar inimigos
// • atualizar comportamento (IA, movimento, colisões)
// • desenhar na tela
// • descarregar recursos
// ------------------------------------------------------------

// Carrega sprites, sons e dados dos inimigos
void Enemies_Load();

// Cria um inimigo no mapa em "pos", configurado para patrulhar entre
// patrolLeftX → lado esquerdo do percurso
// patrolRightX → lado direito do percurso
// dir → direção inicial (+1 direita, -1 esquerda)
void Enemies_Spawn(EnemyKind kind, Vector2 pos, float patrolLeftX, float patrolRightX, int dir = +1);

// Atualiza toda a lógica dos inimigos
// • IA (patrulha, perseguição, ataque)
// • movimento
// • colisões com o jogador
// • morte e remoção
void Enemies_Update(GameState &G, float dt);

// Desenha todos os inimigos atualmente ativos na tela
void Enemies_Draw();

// Libera memoria, texturas ou outros recursos quando o jogo fecha
void Enemies_Unload();

// ------------------------------------------------------------
// NOVO: Sistema de ataque do jogador
// Permite que o player atinja inimigos próximos
//   playerHit → hitbox do ataque do jogador (swing da espada, magia…)
//   raio      → área extra de influência
//   dano      → dano causado
// Retorna quantos inimigos foram atingidos
// ------------------------------------------------------------
int Enemies_HitNearby(GameState &G, const Rectangle &playerHit, float raio, int dano);

// Spawna vários inimigos escolhendo posições aleatórias dentro da área do mapa
void Enemies_SpawnRandom(int quantidade, EnemyKind tipo, Rectangle areaMapa);
