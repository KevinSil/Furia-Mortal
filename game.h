#pragma once
// Garante que este arquivo de header só seja incluído uma vez durante a compilação

#include "raylib.h"
// Inclui a biblioteca Raylib, usada para gráficos e controle do jogo

// Enum que identifica qual tela está ativa no jogo
enum ScreenId
{
    MENU_JOGO,     // Tela inicial do jogo
    MENU_PLAYER,   // Tela de seleção de personagem
    MENU_MAP,      // Tela de seleção de mapa
    JOGO,          // Tela onde o jogo acontece
    PAUSE,         // Tela de pausa
    GAMEOVER       // Tela de game over
};

// Enum para representar as classes jogáveis do personagem
enum PlayerClass
{
    CAVALEIRO = 0, // Classe focada em força e defesa
    MAGO = 1,      // Classe focada em magia e mana
    ARQUEIRO = 2   // Classe focada em velocidade e ataques à distância
};

// Estrutura que guarda todos os atributos do jogador
struct PlayerStats
{
    int maxHP, maxStamina, maxMana; // Valores máximos de vida, stamina e mana
    float baseSpeed, runSpeed;      // Velocidade normal e velocidade correndo
    float armor;                    // Armadura do jogador (reduz dano)
    float staminaRegenPs, manaRegenPs; // Regen de stamina e mana por segundo
    int hp, stamina, mana;          // Valores atuais de vida, stamina e mana
};

// Estado global do jogo
struct GameState
{
    ScreenId screen = MENU_JOGO; // Tela atual — inicia no MENU_JOGO
    int selPlayer = 0;           // Indica qual classe está selecionada no menu
    int selMap = 0;              // Indica qual mapa está selecionado
    bool jogoInicializado = false; // Marca se o jogo já foi configurado e iniciado
    PlayerStats stats;           // Estatísticas do jogador atual
    int score = 0;               // Pontos ganhos derrotando inimigos
};
