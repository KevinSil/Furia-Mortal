#include "helper.h"
#include "raymath.h"     // Clamp, Vector2Length
#include <fstream>
#include <iterator>
#include <cmath>
#include <algorithm>     // std::max, std::min (se precisares)

using nlohmann::json;

// ---------------- Ficheiros ----------------
std::string LerFicheiroTexto(const char* caminho){
    std::ifstream f(caminho, std::ios::in | std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
}

// ---------------- Input / Movimento ----------------
Vector2 CalcularDeslocamento(float velPxSeg, float dt){
    Vector2 d{0,0};
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) d.x += 1;
    if (IsKeyDown(KEY_LEFT ) || IsKeyDown(KEY_A)) d.x -= 1;
    if (IsKeyDown(KEY_DOWN ) || IsKeyDown(KEY_S)) d.y += 1;
    if (IsKeyDown(KEY_UP   ) || IsKeyDown(KEY_W)) d.y -= 1;

    if (d.x!=0 || d.y!=0){
        const float inv = 1.0f / std::sqrtf(d.x*d.x + d.y*d.y);
        d.x*=inv; d.y*=inv;
    }
    d.x *= velPxSeg * dt;
    d.y *= velPxSeg * dt;
    return d;
}

// ---------------- Colisão AABB ----------------
bool IntersetaAABB(const Rectangle& a, const Rectangle& b){
    return !(a.x + a.width  <= b.x ||
             b.x + b.width  <= a.x ||
             a.y + a.height <= b.y ||
             b.y + b.height <= a.y);
}

void ResolverEixo(Rectangle& r, float dx, float dy, const std::vector<Rectangle>& colisores){
    r.x += dx;
    for (const auto& c : colisores){
        if (IntersetaAABB(r, c)){
            if (dx>0) r.x = c.x - r.width;
            else if (dx<0) r.x = c.x + c.width;
        }
    }
    r.y += dy;
    for (const auto& c : colisores){
        if (IntersetaAABB(r, c)){
            if (dy>0) r.y = c.y - r.height;
            else if (dy<0) r.y = c.y + c.height;
        }
    }
}

// ---------------- Tiled / Layers ----------------
void DesenharLayerTiles(Texture2D atlas, int firstgid, int colunasAtlas,
                        int tileW, int tileH, int mapaW, int mapaH,
                        const json& layer)
{
    const auto& data = layer["data"];
    for (int y=0; y<mapaH; ++y){
        for (int x=0; x<mapaW; ++x){
            int gid = data[y*mapaW + x].get<int>();
            if (gid==0) continue;
            const int idx = gid - firstgid;
            const int sx  = (idx % colunasAtlas) * tileW;
            const int sy  = (idx / colunasAtlas) * tileH;
            DrawTextureRec(atlas,
                Rectangle{ (float)sx, (float)sy, (float)tileW, (float)tileH },
                Vector2  { (float)(x*tileW), (float)(y*tileH) },
                WHITE);
        }
    }
}

// ---------------- Mundo/Câmara (opcional) ----------------
void ClampToWorld(Rectangle& r, float worldW, float worldH){
    r.x = Clamp(r.x, 0.0f, worldW  - r.width);
    r.y = Clamp(r.y, 0.0f, worldH  - r.height);
}

void FollowAndClamp(Camera2D& cam, const Rectangle& target,
                    float worldW, float worldH){
    const float halfW = (float)GetScreenWidth()  * 0.5f / (cam.zoom == 0 ? 1.0f : cam.zoom);
    const float halfH = (float)GetScreenHeight() * 0.5f / (cam.zoom == 0 ? 1.0f : cam.zoom);
    cam.target = { target.x + target.width*0.5f, target.y + target.height*0.5f };
    cam.target.x = (worldW > halfW*2) ? Clamp(cam.target.x, halfW, worldW - halfW) : worldW*0.5f;
    cam.target.y = (worldH > halfH*2) ? Clamp(cam.target.y, halfH, worldH - halfH) : worldH*0.5f;
}

// ---------------- HUD (opcional) ----------------
void DrawBar(int x,int y,int w,int h,float pct, Color fill){
    DrawRectangleLines(x, y, w, h, RAYWHITE);
    float p = pct; if (p < 0.0f) p = 0.0f; if (p > 1.0f) p = 1.0f;
    int ww = (int)std::lround(w * p);
    if (ww > 2) DrawRectangle(x+1, y+1, ww-2, h-2, fill);
}

const char* NomeClasse(int idx){
    static const char* N[] = {"Cavaleiro","Mago","Arqueiro"};
    return N[(idx % 3 + 3) % 3];
}

