// helper.h
// Conjunto de utilitários partilhados: TiraAnim (spritesheet/strip), I/O, movimento,
// colisão AABB, desenho de layers Tiled e utilitários de câmara/HUD.
// Comentários em Português (Portugal).

#pragma once
#include "raylib.h"
#include "json.hpp" // nlohmann::json
#include <string>
#include <vector>

// ====== TiraAnim (partilhado) ======
// Estrutura que representa uma tira de frames horizontais (spritesheet em linha).
// - `tex`: textura carregada
// - `colunas`: número de frames na horizontal
// - `frameW/frameH`: dimensões do frame
// - `frameAtual`, `tempo`, `durFrame`: controlo de animação
struct TiraAnim
{
    Texture2D tex{};
    int colunas = 1;
    int frameW = 0, frameH = 0;
    int frameAtual = 0;
    float tempo = 0.0f;
    float durFrame = 0.1f;

    // Carrega uma tira de frames a partir de `caminho`, com `COLUNAS` frames e `fps` de animação
    void Carregar(const char *caminho, int COLUNAS, float fps)
    {
        tex = LoadTexture(caminho);
        SetTextureFilter(tex, TEXTURE_FILTER_POINT);
        colunas = (COLUNAS > 0) ? COLUNAS : 1;
        frameW = tex.width / colunas;
        frameH = tex.height;
        durFrame = (fps > 0.0f) ? 1.0f / fps : 999999.0f;
        frameAtual = 0;
        tempo = 0.0f;
    }

    // Actualiza a animação se `tocar` for true; caso contrário mantém o 1º frame
    void Atualizar(float dt, bool tocar)
    {
        if (!tocar || durFrame >= 999999.0f)
        {
            frameAtual = 0;
            tempo = 0.0f;
            return;
        }
        tempo += dt;
        while (tempo >= durFrame)
        {
            tempo -= durFrame;
            frameAtual = (frameAtual + 1) % colunas;
        }
    }

    // Retorna a fonte (Rectangle) a extrair da textura para o frame atual.
    Rectangle ObterSrc(bool inverterX = false) const
    {
        Rectangle r{(float)(frameAtual * frameW), 0, (float)frameW, (float)frameH};
        if (inverterX)
        {
            r.width = -r.width;
            r.x += frameW;
        }
        return r;
    }

    // Liberta a textura associada (se existir)
    void Libertar()
    {
        if (tex.id)
            UnloadTexture(tex);
        tex.id = 0;
    }
};

// ====== Helpers reutilizáveis (mesmas assinaturas do teu código) ======

// Ficheiros
std::string LerFicheiroTexto(const char *caminho);

// Input / Movimento
Vector2 CalcularDeslocamento(float velPxSeg, float dt);

// Geometria
inline Rectangle RectCentrado(float cx, float cy, float w, float h)
{
    return Rectangle{cx - w * 0.5f, cy - h * 0.5f, w, h};
}

// Colisão AABB
bool IntersetaAABB(const Rectangle &a, const Rectangle &b);
void ResolverEixo(Rectangle &r, float dx, float dy, const std::vector<Rectangle> &colisores);

// Tiled / Layers
void DesenharLayerTiles(Texture2D atlas, int firstgid, int colunasAtlas,
                        int tileW, int tileH, int mapaW, int mapaH,
                        const nlohmann::json &layer);

// (Opcional) Mundo/Câmara
void ClampToWorld(Rectangle &r, float worldW, float worldH);
void FollowAndClamp(Camera2D &cam, const Rectangle &target,
                    float worldW, float worldH);

// (Opcional) HUD utilitário — mantém os nomes para não mudares chamadas
void DrawBar(int x, int y, int w, int h, float pct, Color fill);
const char *NomeClasse(int idx);
