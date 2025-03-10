#include <cmath>
#include <cstdio>
#include <functional>
#include <iostream>
#include <omp.h>
#include <raylib.h>
#include <raymath.h>
#include <string>
#include <vector>

// Seus includes usuais
#include "./src/Camera/Camera3de.h"
#include "./src/Iluminacao/Iluminacao.h"
#include "./src/Material/Material.h"
#include "./src/Objeto/Objeto.h"
#include "./src/Raio/Raio.h"
#include "funcoes_auxiliares.h"

#include "src/ObjetoComplexo/ObjetoComplexo.h"
#include "src/calcular_intersecao.h"
#include "src/inicializar_objetos.h"
#include "src/objetosTrabalhofinal.h"

using namespace funcoes_auxiliares;

// ---------------------- Dimensões de Janela ----------------------
static const int SCREEN_WIDTH  = 1100; // Área de render + GUI maior
static const int SCREEN_HEIGHT = 650;

// Área de render (à esquerda)
static const int RENDER_W = 750;  
static const int RENDER_H = 650;  
static const int nCol = 750;     
static const int nLin = 650;     

// Vetores e estruturas
static std::vector<ObjetoComplexo> complexObjects;
static std::vector<Objeto> objetos;

// Luzes (sem Spot)
static std::vector<iluminacao::FontePontual>    fontes_pontuais;
static std::vector<iluminacao::FonteDirecional> fontes_direcionais;
static Vetor3d I_A = {0.4f, 0.4f, 0.4f};

// Câmera
static Camera3de camera({0,100,300}, {0,0,0}, {0,1,0});

// Ray casting buffer
static RenderTexture2D tela;
static std::vector<Color> pixel_buffer(nLin * nCol, BLACK);

// Índice do objeto selecionado
static int objetoSelecionado = -1;

// -------------------- GUI: Variáveis de Transformação --------------------
static float guiTransX = 0.0f, guiTransY = 0.0f, guiTransZ = 0.0f;
static float guiRotX   = 0.0f, guiRotY   = 0.0f, guiRotZ   = 0.0f;
static float guiEscX   = 1.0f, guiEscY   = 1.0f, guiEscZ   = 1.0f;

// -------------------- GUI: Variáveis de Câmera --------------------
static float guiCamX = 0.0f;
static float guiCamY = 100.0f;
static float guiCamZ = 300.0f;

static float guiAtX  = 0.0f;
static float guiAtY  = 0.0f;
static float guiAtZ  = 0.0f;

// Fonte TTF
static Font guiFont;

// ------------------------------------------------------------------
// Função de ray casting (renderiza na RenderTexture `tela`)
// ------------------------------------------------------------------
static void renderizar()
{
    // Limpa buffer
    for (auto &px : pixel_buffer) {
        px = BLACK;
    }

    // Matriz camera->mundo
    Matriz M_cw = camera.getMatrixCameraWorld();

    // Canto sup/esq
    Vetor3d pse = (M_cw * camera.get_PSE().ponto4d()).vetor3d();
    Vetor3d rv  = (M_cw * Vetor3d{1,0,0}.vetor4d()).vetor3d();
    Vetor3d dv  = (M_cw * Vetor3d{0,-1,0}.vetor4d()).vetor3d();

    float dx = camera.get_W_J()/nCol;
    float dy = camera.get_H_J()/nLin;

    BeginTextureMode(tela);
    ClearBackground(BLACK);

    for (int i=0; i<nLin; i++){
      for (int j=0; j<nCol; j++){
        Vetor3d P = pse + rv*(dx*(j+0.5f)) + dv*(dy*(i+0.5f));
        Vetor3d dr = (P - camera.position).normalizado();

        Raio raio(camera.position, dr);
        auto [t, idx] = calcular_intersecao(raio, objetos);
        if (t>0.0f) {
          Vetor3d Pt = raio.no_ponto(t);
          Vetor3d normal = objetos[idx].normal(Pt);

          // Pega material
          MaterialSimples mat;
          std::visit([&](auto &&o){
            using T = std::decay_t<decltype(o)>;
            if constexpr (std::is_same_v<T, PlanoTextura>) {
               mat = o.material(Pt);
            } else {
               mat = objetos[idx].material;
            }
          }, objetos[idx].obj);

          // Iluminação
          Vetor3d cor = {0,0,0};
          // Ambiente
          cor = cor + iluminacao::luz_ambiente(I_A, mat.K_a);

          // Fontes pontuais
          for (auto &fp : fontes_pontuais) {
            if(!fp.acesa) continue;
            Vetor3d drl = fp.posicao - Pt;
            float distl = drl.tamanho();
            drl = drl.normalizado();

            Raio r_luz(Pt, drl);
            auto [t_luz, _] = calcular_intersecao(r_luz, objetos, idx);
            if(t_luz<0.0f || t_luz>distl) {
              cor = cor + iluminacao::modelo_phong(Pt, dr, normal, fp, mat);
            }
          }
          // Direcional
          for (auto &fd : fontes_direcionais) {
            if(!fd.acesa) continue;
            Vetor3d dl = fd.direcao.normalizado();
            Raio r_luz(Pt, dl);
            auto [t_luz, _] = calcular_intersecao(r_luz, objetos, idx);
            if(t_luz<0.0f) {
              cor = cor + iluminacao::modelo_phong(Pt, dr, normal, fd, mat);
            }
          }

          float R = fminf(cor.x*255.f,255.f);
          float G = fminf(cor.y*255.f,255.f);
          float B = fminf(cor.z*255.f,255.f);
          pixel_buffer[i*nCol+j] = {(unsigned char)R,
                                    (unsigned char)G,
                                    (unsigned char)B,
                                    255};
        }
      }
    }

    // Desenha no texture
    for (int i=0; i<nLin; i++){
      for (int j=0; j<nCol; j++){
        DrawPixel(j,i, pixel_buffer[i*nCol+j]);
      }
    }

    EndTextureMode();
}

// ------------------------------------------------------------------
// Inicializa Luzes (pontual + direcional, sem spot)
// ------------------------------------------------------------------
static void inicializar_luzes()
{
    iluminacao::FontePontual fpt({300,200,300}, {0.8f,0.8f,0.8f});
    fontes_pontuais.push_back(fpt);

    iluminacao::FonteDirecional fdir({-1,1,0}, {0.4f,0.4f,0.4f});
    fontes_direcionais.push_back(fdir);
}

// ------------------------------------------------------------------
// GUI - desenhar e processar
// ------------------------------------------------------------------
static void desenharGUI()
{
    float painelW = (float)(SCREEN_WIDTH - RENDER_W);
    float painelH = (float)SCREEN_HEIGHT;
    Rectangle painel = { (float)RENDER_W, 0, painelW, painelH };

    // Fundo do painel
    DrawRectangleRec(painel, (Color){ 40,40,60,255 });

    // Título principal
    float espLin = 26; // espaçamento vertical
    float baseY  = painel.y+10;
    float baseX  = painel.x+10;
    DrawTextEx(guiFont, "TRANSFORMACOES", {baseX, baseY}, 24, 2.0f, WHITE);
    baseY += 40;

    // Se objeto selecionado
    if (objetoSelecionado >= 0) {
      std::string txtSel = "Objeto Selecionado: " + std::to_string(objetoSelecionado);
      DrawTextEx(guiFont, txtSel.c_str(), {baseX, baseY}, 20, 2.0f, YELLOW);
      baseY += 40;

      // --- TRANSLACAO ---
      DrawTextEx(guiFont, "Translacao:", {baseX, baseY}, 18,1, WHITE);
      baseY += espLin;
      // X
      std::string txtX = "X = " + std::to_string(guiTransX);
      DrawTextEx(guiFont, txtX.c_str(), {baseX, baseY}, 16,1, WHITE);

      Rectangle rx = { baseX+100, baseY, 150, 12 };
      DrawRectangleRec(rx, GRAY);
      Vector2 mouse = GetMousePosition();
      float relx = (guiTransX+100)/200.f; // -100..+100
      float cx = rx.x + relx*rx.width;
      DrawCircle((int)cx, (int)(rx.y+6), 6, WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, rx)){
        float rel = (mouse.x - rx.x)/rx.width;
        if(rel<0) rel=0; if(rel>1) rel=1;
        guiTransX = rel*200 - 100;
      }
      baseY += espLin;

      // Y
      std::string txtY = "Y = " + std::to_string(guiTransY);
      DrawTextEx(guiFont, txtY.c_str(), {baseX, baseY}, 16,1, WHITE);
      Rectangle ry = { baseX+100, baseY, 150,12 };
      DrawRectangleRec(ry, GRAY);
      float rely = (guiTransY+100)/200.f;
      float cy = ry.x + rely*ry.width;
      DrawCircle((int)cy, (int)(ry.y+6),6, WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mouse,ry)){
        float rel=(mouse.x-ry.x)/ry.width;
        if(rel<0) rel=0; if(rel>1) rel=1;
        guiTransY= rel*200-100;
      }
      baseY += espLin;

      // Z
      std::string txtZ = "Z = " + std::to_string(guiTransZ);
      DrawTextEx(guiFont, txtZ.c_str(), {baseX, baseY},16,1,WHITE);
      Rectangle rz= { baseX+100, baseY, 150,12 };
      DrawRectangleRec(rz, GRAY);
      float relz = (guiTransZ+100)/200.f;
      float cz = rz.x + relz*rz.width;
      DrawCircle((int)cz,(int)(rz.y+6),6,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mouse,rz)){
        float rel=(mouse.x-rz.x)/rz.width;
        if(rel<0)rel=0;if(rel>1)rel=1;
        guiTransZ = rel*200-100;
      }
      baseY += espLin;

      // Botão aplicar T
      Rectangle btnT = {baseX, baseY,100,24};
      DrawRectangleRec(btnT, (Color){80,120,80,255});
      DrawTextEx(guiFont, "APLICAR T", {btnT.x+3, btnT.y+2}, 18,1,WHITE);
      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mouse,btnT)){
        Matriz mt= Matriz::translacao({guiTransX, guiTransY, guiTransZ});
        objetos[objetoSelecionado].transformar(mt);
        renderizar();
      }
      baseY += espLin*1.3f;

      // --- ROTACAO ---
      DrawTextEx(guiFont,"Rotacao (Graus):",{baseX, baseY}, 18,1, WHITE);
      baseY += espLin;
      // RotX
      std::string sx= "X = "+std::to_string(guiRotX);
      DrawTextEx(guiFont, sx.c_str(), {baseX, baseY},16,1,WHITE);
      Rectangle rrx = {baseX+100, baseY, 150,12};
      DrawRectangleRec(rrx,GRAY);
      float relrx = (guiRotX+180)/360.f; // -180..+180
      float cRotX = rrx.x + relrx*rrx.width;
      DrawCircle((int)cRotX,(int)(rrx.y+6),6,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mouse,rrx)){
        float rel=(mouse.x-rrx.x)/rrx.width;
        if(rel<0)rel=0;if(rel>1)rel=1;
        guiRotX= rel*360-180;
      }
      baseY+=espLin;

      // RotY
      std::string sy= "Y = "+std::to_string(guiRotY);
      DrawTextEx(guiFont, sy.c_str(), {baseX, baseY},16,1,WHITE);
      Rectangle rry= {baseX+100,baseY,150,12};
      DrawRectangleRec(rry,GRAY);
      float relry= (guiRotY+180)/360.f;
      float cRotY= rry.x+ relry*rry.width;
      DrawCircle((int)cRotY,(int)(rry.y+6),6,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mouse,rry)){
        float rel=(mouse.x-rry.x)/rry.width;
        if(rel<0)rel=0;if(rel>1)rel=1;
        guiRotY= rel*360-180;
      }
      baseY+=espLin;

      // RotZ
      std::string sz= "Z = "+std::to_string(guiRotZ);
      DrawTextEx(guiFont, sz.c_str(), {baseX, baseY},16,1,WHITE);
      Rectangle rrz= {baseX+100,baseY,150,12};
      DrawRectangleRec(rrz,GRAY);
      float relrz= (guiRotZ+180)/360.f;
      float cRotZ= rrz.x+relrz*rrz.width;
      DrawCircle((int)cRotZ,(int)(rrz.y+6),6,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mouse,rrz)){
        float rel=(mouse.x-rrz.x)/rrz.width;
        if(rel<0)rel=0;if(rel>1)rel=1;
        guiRotZ= rel*360-180;
      }
      baseY+=espLin;

      // Botão aplicar R
      Rectangle btnR={baseX, baseY,100,24};
      DrawRectangleRec(btnR,(Color){120,80,80,255});
      DrawTextEx(guiFont,"APLICAR R",{btnR.x+3, btnR.y+2}, 18,1,WHITE);
      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mouse,btnR)){
        Matriz mx= Matriz::rotacao_x(DEG2RAD*guiRotX);
        Matriz my= Matriz::rotacao_y(DEG2RAD*guiRotY);
        Matriz mz= Matriz::rotacao_z(DEG2RAD*guiRotZ);
        Matriz final= mx*my*mz;
        objetos[objetoSelecionado].transformar(final);
        renderizar();
      }
      baseY+=espLin*1.3f;

      // --- ESCALA ---
      DrawTextEx(guiFont, "Escala:", {baseX, baseY}, 18,1,WHITE);
      baseY+=espLin;
      // X
      std::string seX= "X= "+std::to_string(guiEscX);
      DrawTextEx(guiFont,seX.c_str(),{baseX, baseY},16,1,WHITE);
      Rectangle rxS={baseX+100, baseY,150,12};
      DrawRectangleRec(rxS,GRAY);
      float relsx= (guiEscX-0.1f)/1.9f; // 0.1..2.0
      if(relsx<0)relsx=0;if(relsx>1)relsx=1;
      float cEscX= rxS.x+relsx*rxS.width;
      DrawCircle((int)cEscX,(int)(rxS.y+6),6,WHITE);
      Vector2 mousep= GetMousePosition();
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&& CheckCollisionPointRec(mousep,rxS)){
        float rr=(mousep.x - rxS.x)/rxS.width;
        if(rr<0)rr=0;if(rr>1)rr=1;
        guiEscX= 0.1f+ rr*1.9f;
      }
      baseY+=espLin;

      // Y
      std::string seY= "Y= "+std::to_string(guiEscY);
      DrawTextEx(guiFont,seY.c_str(),{baseX, baseY},16,1,WHITE);
      Rectangle ryS={baseX+100, baseY,150,12};
      DrawRectangleRec(ryS,GRAY);
      float relsy= (guiEscY-0.1f)/1.9f;
      if(relsy<0)relsy=0;if(relsy>1)relsy=1;
      float cEscY= ryS.x+relsy*ryS.width;
      DrawCircle((int)cEscY,(int)(ryS.y+6),6,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&& CheckCollisionPointRec(mousep,ryS)){
        float rr=(mousep.x - ryS.x)/ryS.width;
        if(rr<0)rr=0;if(rr>1)rr=1;
        guiEscY= 0.1f+ rr*1.9f;
      }
      baseY+=espLin;

      // Z
      std::string seZ= "Z= "+std::to_string(guiEscZ);
      DrawTextEx(guiFont,seZ.c_str(),{baseX, baseY},16,1,WHITE);
      Rectangle rzS={baseX+100, baseY,150,12};
      DrawRectangleRec(rzS,GRAY);
      float relsz= (guiEscZ-0.1f)/1.9f;
      if(relsz<0)relsz=0;if(relsz>1)relsz=1;
      float cEscZ= rzS.x+relsz*rzS.width;
      DrawCircle((int)cEscZ,(int)(rzS.y+6),6,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mousep,rzS)){
        float rr=(mousep.x - rzS.x)/rzS.width;
        if(rr<0)rr=0;if(rr>1)rr=1;
        guiEscZ= 0.1f+ rr*1.9f;
      }
      baseY+=espLin;

      // Botão aplicar E
      Rectangle btnE= {baseX, baseY,100,24};
      DrawRectangleRec(btnE,(Color){120,120,80,255});
      DrawTextEx(guiFont,"APLICAR E",{btnE.x+3,btnE.y+2},18,1,WHITE);
      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mousep,btnE)){
        Matriz me= Matriz::escala({guiEscX, guiEscY, guiEscZ});
        objetos[objetoSelecionado].transformar(me);
        renderizar();
      }
      baseY+=espLin*1.3f;
    } 
    else {
      // Nenhum obj
      DrawTextEx(guiFont,"Nenhum objeto selecionado",
                 {baseX, baseY}, 18,2, RED);
      baseY+=40;
    }

    // ---------- Controles de Camera -----------
    DrawTextEx(guiFont,"Camera Olho (Eye):", {baseX, baseY},20,2, WHITE);
    baseY+=espLin;
    // X
    {
      std::string scx= "X= "+std::to_string(guiCamX);
      DrawTextEx(guiFont, scx.c_str(), {baseX, baseY},16,1,WHITE);
      Rectangle rcx= {baseX+80, baseY,150,12};
      DrawRectangleRec(rcx, DARKGRAY);
      Vector2 mp= GetMousePosition();
      float re= (guiCamX+500)/1000.f; //-500..+500
      float cc= rcx.x+ re*rcx.width;
      DrawCircle((int)cc,(int)(rcx.y+6),6,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mp,rcx)){
        float rr= (mp.x-rcx.x)/rcx.width;
        if(rr<0)rr=0;if(rr>1)rr=1;
        guiCamX= rr*1000 -500;
      }
      baseY+=espLin;
    }
    // Y
    {
      std::string scy= "Y= "+std::to_string(guiCamY);
      DrawTextEx(guiFont, scy.c_str(), {baseX, baseY},16,1,WHITE);
      Rectangle rcy= {baseX+80, baseY,150,12};
      DrawRectangleRec(rcy, DARKGRAY);
      Vector2 mp= GetMousePosition();
      float re= (guiCamY+500)/1000.f;
      float cc= rcy.x+ re*rcy.width;
      DrawCircle((int)cc,(int)(rcy.y+6),6,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mp,rcy)){
        float rr=(mp.x-rcy.x)/rcy.width;
        if(rr<0)rr=0;if(rr>1)rr=1;
        guiCamY= rr*1000-500;
      }
      baseY+=espLin;
    }
    // Z
    {
      std::string scz= "Z= "+std::to_string(guiCamZ);
      DrawTextEx(guiFont, scz.c_str(), {baseX, baseY},16,1,WHITE);
      Rectangle rcz= {baseX+80, baseY,150,12};
      DrawRectangleRec(rcz, DARKGRAY);
      Vector2 mp= GetMousePosition();
      float re= (guiCamZ+500)/1000.f;
      float cc= rcz.x+ re*rcz.width;
      DrawCircle((int)cc,(int)(rcz.y+6),6,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mp,rcz)){
        float rr=(mp.x-rcz.x)/rcz.width;
        if(rr<0)rr=0;if(rr>1)rr=1;
        guiCamZ= rr*1000-500;
      }
      baseY+=espLin;
    }

    // Botao "ATUALIZAR OLHO"
    {
      Rectangle bEye= {baseX, baseY, 120,24};
      DrawRectangleRec(bEye, (Color){80,120,150,255});
      DrawTextEx(guiFont,"ATUALIZAR OLHO",{bEye.x+5,bEye.y+2},16,1,WHITE);
      Vector2 mp= GetMousePosition();
      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mp,bEye)){
        camera.position= {guiCamX, guiCamY, guiCamZ};
        camera.updateCoordinates();
        renderizar();
      }
      baseY+= espLin*1.3f;
    }

    // ---------- At -----------
    DrawTextEx(guiFont,"Camera (At):", {baseX, baseY},20,2,WHITE);
    baseY+=espLin;

    // X
    {
      std::string sAx= "X= "+std::to_string(guiAtX);
      DrawTextEx(guiFont,sAx.c_str(), {baseX, baseY},16,1,WHITE);
      Rectangle rAx= {baseX+80, baseY,150,12};
      DrawRectangleRec(rAx,DARKGRAY);
      Vector2 mp= GetMousePosition();
      float re= (guiAtX+500)/1000.f;
      float cc= rAx.x + re*rAx.width;
      DrawCircle((int)cc,(int)(rAx.y+6),6,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mp,rAx)){
        float rr=(mp.x-rAx.x)/rAx.width;
        if(rr<0)rr=0;if(rr>1)rr=1;
        guiAtX= rr*1000-500;
      }
      baseY+=espLin;
    }
    // Y
    {
      std::string sAy= "Y= "+std::to_string(guiAtY);
      DrawTextEx(guiFont, sAy.c_str(),{baseX, baseY},16,1,WHITE);
      Rectangle rAy= {baseX+80, baseY,150,12};
      DrawRectangleRec(rAy,DARKGRAY);
      Vector2 mp= GetMousePosition();
      float re= (guiAtY+500)/1000.f;
      float cc= rAy.x + re*rAy.width;
      DrawCircle((int)cc,(int)(rAy.y+6),6,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mp,rAy)){
        float rr=(mp.x-rAy.x)/rAy.width;
        if(rr<0)rr=0;if(rr>1)rr=1;
        guiAtY= rr*1000-500;
      }
      baseY+=espLin;
    }
    // Z
    {
      std::string sAz= "Z= "+std::to_string(guiAtZ);
      DrawTextEx(guiFont, sAz.c_str(), {baseX, baseY},16,1,WHITE);
      Rectangle rAz= {baseX+80, baseY,150,12};
      DrawRectangleRec(rAz,DARKGRAY);
      Vector2 mp= GetMousePosition();
      float re= (guiAtZ+500)/1000.f;
      float cc= rAz.x+ re*rAz.width;
      DrawCircle((int)cc,(int)(rAz.y+6),6,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mp,rAz)){
        float rr=(mp.x-rAz.x)/rAz.width;
        if(rr<0)rr=0;if(rr>1)rr=1;
        guiAtZ= rr*1000-500;
      }
      baseY+=espLin;
    }

    // Botão "ATUALIZAR AT"
    {
      Rectangle bAt= {baseX, baseY,140,24};
      DrawRectangleRec(bAt,(Color){80,80,150,255});
      DrawTextEx(guiFont,"ATUALIZAR AT",{bAt.x+5,bAt.y+2},16,1,WHITE);
      Vector2 mp= GetMousePosition();
      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mp,bAt)){
        camera.lookAt= {guiAtX, guiAtY, guiAtZ};
        camera.updateCoordinates();
        renderizar();
      }
      baseY+=espLin*1.3f;
    }
}

// ------------------------------------------------------------------
// main
// ------------------------------------------------------------------
int main()
{
    omp_set_num_threads(4);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Trabalho CG - GUI Renovado (PT-BR)");
    SetTargetFPS(60);

    // Carrega fonte TTF
    guiFont = LoadFontEx("assets/roboto.ttf", 36, NULL, 250);
    SetTextureFilter(guiFont.texture, TEXTURE_FILTER_BILINEAR);

    // RenderTexture
    tela = LoadRenderTexture(nCol, nLin);

    // Cena
    inicializar_objetosfinal(objetos, complexObjects);
    for (auto &cobj : complexObjects) {
      flatten_objetos(cobj, objetos);
    }
    std::cout << "Objetos: " << objetos.size() << "\n";

    // Luzes
    inicializar_luzes();

    // Câmera
    camera.updateCoordinates();

    // Render inicial
    renderizar();

    // Ajusta valores de GUI da câmera
    guiCamX = camera.position.x;
    guiCamY = camera.position.y;
    guiCamZ = camera.position.z;

    guiAtX  = camera.lookAt.x;
    guiAtY  = camera.lookAt.y;
    guiAtZ  = camera.lookAt.z;

    while (!WindowShouldClose()) {
      // Ray picking no lado esquerdo
      Vector2 mouse = GetMousePosition();
      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        if(mouse.x < (float)nCol && mouse.y < (float)nLin){
          int mx = mouse.x;
          int my = mouse.y;
          Matriz M_cw = camera.getMatrixCameraWorld();
          Vetor3d pse= (M_cw*camera.get_PSE().ponto4d()).vetor3d();
          Vetor3d rv = (M_cw*Vetor3d{1,0,0}.vetor4d()).vetor3d();
          Vetor3d dv = (M_cw*Vetor3d{0,-1,0}.vetor4d()).vetor3d();
          float dx = camera.get_W_J()/nCol;
          float dy = camera.get_H_J()/nLin;
          Vetor3d P = pse + rv*(dx*(mx+0.5f)) + dv*(dy*(my+0.5f));
          Vetor3d dir = (P - camera.position).normalizado();
          Raio raio(camera.position, dir);
          auto [tt, idx] = calcular_intersecao(raio, objetos);
          if(tt>0.0f){
            objetoSelecionado= idx;
            TraceLog(LOG_INFO, "Selecionou objeto: %d", idx);
          }
        }
      }

      BeginDrawing();
      {
        ClearBackground(BLACK);
        // Desenha RayCast
        DrawTextureRec(tela.texture,
                       {0,0,(float)nCol,(float)-nLin},
                       {0,0},
                       WHITE);

        // GUI
        desenharGUI();
      }
      EndDrawing();
    }

    // Final
    UnloadFont(guiFont);
    UnloadRenderTexture(tela);
    CloseWindow();
    return 0;
}
