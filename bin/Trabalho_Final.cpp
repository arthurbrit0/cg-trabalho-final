#include <cmath>
#include <cstdio>
#include <functional>
#include <iostream>
#include <omp.h>
#include <raylib.h>
#include <raymath.h>
#include <string>
#include <vector>
#include <cstring>   
#include <cctype>    

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

static const int SCREEN_WIDTH  = 1200;
static const int SCREEN_HEIGHT = 700;

static const int RENDER_W = 800;  
static const int RENDER_H = 700;  
static const int nCol = 800;  
static const int nLin = 700;  

static std::vector<ObjetoComplexo> complexObjects;
static std::vector<Objeto> objetos;

static std::vector<iluminacao::FontePontual>    fontes_pontuais;
static std::vector<iluminacao::FonteDirecional> fontes_direcionais;
static Vetor3d I_A = {0.4f, 0.4f, 0.4f};

static Camera3de camera({0,100,300}, {0,0,0}, {0,1,0});

static RenderTexture2D tela;
static std::vector<Color> pixel_buffer(nLin*nCol, BLACK);

static int objetoSelecionado = -1;

static float transX=0, transY=0, transZ=0;
static char  txtTransX[16]="0.0", txtTransY[16]="0.0", txtTransZ[16]="0.0";
static bool  editTransX=false, editTransY=false, editTransZ=false;

static float rotX=0, rotY=0, rotZ=0;
static char  txtRotX[16]="0.0", txtRotY[16]="0.0", txtRotZ[16]="0.0";
static bool  editRotX=false, editRotY=false, editRotZ=false;

static float escX=1, escY=1, escZ=1;
static char  txtEscX[16]="1.0", txtEscY[16]="1.0", txtEscZ[16]="1.0";
static bool  editEscX=false, editEscY=false, editEscZ=false;

static float camX=0, camY=100, camZ=300;
static char  txtCamX[16]="0.0", txtCamY[16]="100.0", txtCamZ[16]="300.0";
static bool  editCamX=false, editCamY=false, editCamZ=false;

static float atX=0, atY=0, atZ=0;
static char  txtAtX[16]="0.0", txtAtY[16]="0.0", txtAtZ[16]="0.0";
static bool  editAtX=false, editAtY=false, editAtZ=false;

// Fonte TTF
static Font guiFont;

static float parseFloat(const char *str, float defaultVal=0.0f)
{
    if (!str || !(*str)) return defaultVal;
    float val= std::strtof(str, nullptr); 
    return val;
}

static void textBoxInput(char *buf, int bufSize, bool &editing)
{
    if(!editing) return;

    int key = GetCharPressed();
    while(key>0){
      if ((key >= 32) && (key < 127)) {
        int len= strlen(buf);
        if(len < bufSize-1) {
          buf[len] = (char)key;
          buf[len+1]= '\0';
        }
      }
      key= GetCharPressed();
    }

    if(IsKeyPressed(KEY_BACKSPACE)){
      int len= strlen(buf);
      if(len>0) buf[len-1]= '\0';
    }
    if(IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)){
      editing= false;
    }
}

static bool drawTextBox(char *buf, int bufSize, float x, float y, float w, float h, bool editing)
{
    Rectangle box= {x,y,w,h};
    Color bg= editing? (Color){70,70,110,255} : (Color){50,50,70,255};
    DrawRectangleRec(box, bg);

    DrawTextEx(guiFont, buf, {x+4,y+2}, 18, 1, WHITE);

    Vector2 mp= GetMousePosition();
    bool clicked= false;
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mp, box)){
      clicked= true;
    }
    return clicked;
}

static void renderizar()
{
    // limpa
    for (auto &px : pixel_buffer) {
      px= BLACK;
    }

    Matriz M_cw= camera.getMatrixCameraWorld();

    Vetor3d pse= (M_cw*camera.get_PSE().ponto4d()).vetor3d();
    Vetor3d rv = (M_cw*Vetor3d{1,0,0}.vetor4d()).vetor3d();
    Vetor3d dv = (M_cw*Vetor3d{0,-1,0}.vetor4d()).vetor3d();

    float dx= camera.get_W_J()/nCol;
    float dy= camera.get_H_J()/nLin;

    BeginTextureMode(tela);
    ClearBackground(BLACK);

    for(int i=0; i<nLin; i++){
      for(int j=0; j<nCol; j++){
        Vetor3d P= pse + rv*(dx*(j+0.5f)) + dv*(dy*(i+0.5f));
        Vetor3d dr= (P - camera.position).normalizado();
        Raio raio(camera.position, dr);
        auto [t, idx]= calcular_intersecao(raio, objetos);
        if(t>0.0f){
          Vetor3d Pt= raio.no_ponto(t);
          Vetor3d normal= objetos[idx].normal(Pt);

          MaterialSimples mat;
          std::visit([&](auto &&o){
            using T= std::decay_t<decltype(o)>;
            if constexpr(std::is_same_v<T, PlanoTextura>){
              mat= o.material(Pt);
            } else {
              mat= objetos[idx].material;
            }
          }, objetos[idx].obj);

          Vetor3d cor= {0,0,0};
          cor= cor + iluminacao::luz_ambiente(I_A, mat.K_a);
          for(auto &fp: fontes_pontuais){
            if(!fp.acesa) continue;
            Vetor3d ldir= fp.posicao - Pt;
            float dist= ldir.tamanho();
            ldir= ldir.normalizado();
            Raio rLuz(Pt,ldir);
            auto [tl, _] = calcular_intersecao(rLuz, objetos, idx);
            if(tl<0.0f || tl> dist){
              cor= cor + iluminacao::modelo_phong(Pt, dr, normal, fp, mat);
            }
          }
          for(auto &fd: fontes_direcionais){
            if(!fd.acesa) continue;
            Vetor3d dl= fd.direcao.normalizado();
            Raio rLuz(Pt, dl);
            auto [tl,_]= calcular_intersecao(rLuz, objetos, idx);
            if(tl<0.0f){
              cor= cor + iluminacao::modelo_phong(Pt, dr, normal, fd, mat);
            }
          }
          float R= fminf(cor.x*255.f,255.f);
          float G= fminf(cor.y*255.f,255.f);
          float B= fminf(cor.z*255.f,255.f);
          pixel_buffer[i*nCol+j]= {(unsigned char)R,
                                   (unsigned char)G,
                                   (unsigned char)B,
                                   255};
        }
      }
    }

    for(int i=0; i<nLin; i++){
      for(int j=0; j<nCol; j++){
        DrawPixel(j,i, pixel_buffer[i*nCol+j]);
      }
    }

    EndTextureMode();
}

static void inicializar_luzes()
{
    iluminacao::FontePontual fp({300,200,300}, {0.9f,0.9f,0.9f});
    fontes_pontuais.push_back(fp);

    iluminacao::FonteDirecional fd({-1,1,0}, {0.4f,0.4f,0.4f});
    fontes_direcionais.push_back(fd);
}

static void desenharGUI()
{
    float painelW= (float)(SCREEN_WIDTH - RENDER_W);
    float painelH= (float)SCREEN_HEIGHT;
    Rectangle painel= { (float)RENDER_W, 0, painelW, painelH};

    DrawRectangleRec(painel, (Color){40,40,60,255});

    float baseX= painel.x+10;
    float baseY= painel.y+10;
    float espLin= 30;

    DrawTextEx(guiFont, "Transformacoes", {baseX, baseY}, 24,2, WHITE);
    baseY+= 40;

    if(objetoSelecionado>=0){
      std::string txtSel= "Obj Sel.: " + std::to_string(objetoSelecionado);
      DrawTextEx(guiFont, txtSel.c_str(), {baseX, baseY}, 20,1, YELLOW);
      baseY+=espLin*1.2f;

      DrawTextEx(guiFont, "Translacao:", {baseX, baseY}, 18,1, WHITE);
      baseY+=espLin;

      {
        DrawTextEx(guiFont, "X:", {baseX, baseY}, 18,1, WHITE);
        bool clicked= drawTextBox(txtTransX,16, baseX+30, baseY,80,24, editTransX);
        if(clicked){
          editTransX=true;
          editTransY=false;
          editTransZ=false;
        }
        textBoxInput(txtTransX,16, editTransX);
        if(!editTransX){
          transX= parseFloat(txtTransX, transX);
        }
        Rectangle rSli= { baseX+120, baseY+6, 120,10}; 
        DrawRectangleRec(rSli, GRAY);
        float rel= (transX+100)/200.f;
        if(rel<0) rel=0;if(rel>1) rel=1;
        float cx= rSli.x+ rel*rSli.width;
        DrawCircle((int)cx,(int)(rSli.y+5),5, WHITE);
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(),rSli) 
           && !editTransX) // so ajusta slider se nao estiver editando
        {
          float newRel= (GetMousePosition().x - rSli.x)/rSli.width;
          if(newRel<0)newRel=0;if(newRel>1)newRel=1;
          transX= newRel*200 -100;
          snprintf(txtTransX,16,"%.1f", transX);
        }
      }
      baseY+= espLin;

      {
        DrawTextEx(guiFont, "Y:", {baseX, baseY}, 18,1, WHITE);
        bool clicked= drawTextBox(txtTransY,16, baseX+30, baseY,80,24, editTransY);
        if(clicked){
          editTransY=true;
          editTransX=false;
          editTransZ=false;
        }
        textBoxInput(txtTransY,16, editTransY);
        if(!editTransY){
          transY= parseFloat(txtTransY, transY);
        }
        Rectangle rSli= { baseX+120, baseY+6,120,10};
        DrawRectangleRec(rSli, GRAY);
        float rel= (transY+100)/200.f;
        if(rel<0)rel=0;if(rel>1)rel=1;
        float cx= rSli.x+ rel*rSli.width;
        DrawCircle((int)cx,(int)(rSli.y+5),5, WHITE);
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&& CheckCollisionPointRec(GetMousePosition(),rSli)
           && !editTransY)
        {
          float newRel=(GetMousePosition().x- rSli.x)/rSli.width;
          if(newRel<0)newRel=0;if(newRel>1)newRel=1;
          transY= newRel*200-100;
          snprintf(txtTransY,16,"%.1f", transY);
        }
      }
      baseY+= espLin;

      {
        DrawTextEx(guiFont, "Z:", {baseX, baseY}, 18,1, WHITE);
        bool clicked= drawTextBox(txtTransZ,16, baseX+30, baseY,80,24, editTransZ);
        if(clicked){
          editTransZ=true;
          editTransX=false;
          editTransY=false;
        }
        textBoxInput(txtTransZ,16, editTransZ);
        if(!editTransZ){
          transZ= parseFloat(txtTransZ, transZ);
        }
        Rectangle rSli= { baseX+120, baseY+6,120,10};
        DrawRectangleRec(rSli, GRAY);
        float rel= (transZ+100)/200.f;
        if(rel<0)rel=0;if(rel>1)rel=1;
        float cx= rSli.x+ rel*rSli.width;
        DrawCircle((int)cx,(int)(rSli.y+5),5, WHITE);
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&& CheckCollisionPointRec(GetMousePosition(),rSli)
           && !editTransZ)
        {
          float newRel=(GetMousePosition().x-rSli.x)/rSli.width;
          if(newRel<0)newRel=0;if(newRel>1)newRel=1;
          transZ= newRel*200-100;
          snprintf(txtTransZ,16,"%.1f", transZ);
        }
      }
      baseY+= espLin;

      {
        Rectangle bT= {baseX, baseY, 120, 28};
        DrawRectangleRec(bT,(Color){80,120,80,255});
        DrawTextEx(guiFont, "APLICAR T", {bT.x+5,bT.y+3}, 20,2,WHITE);
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&& CheckCollisionPointRec(GetMousePosition(),bT)){
          Matriz mt= Matriz::translacao({transX, transY, transZ});
          objetos[objetoSelecionado].transformar(mt);
          renderizar();
        }
      }
      baseY+= espLin*1.3f;

      DrawTextEx(guiFont,"Rotacao (Graus):",{baseX, baseY},18,1,WHITE);
      baseY+= espLin;

      {
        DrawTextEx(guiFont,"X:",{baseX, baseY},18,1,WHITE);
        bool clicked= drawTextBox(txtRotX,16, baseX+30, baseY,80,24, editRotX);
        if(clicked){
          editRotX=true; editRotY=false; editRotZ=false;
        }
        textBoxInput(txtRotX,16, editRotX);
        if(!editRotX){
          rotX= parseFloat(txtRotX,rotX);
        }
        Rectangle sli={baseX+120, baseY+6,120,10};
        DrawRectangleRec(sli,GRAY);
        float rel= (rotX+180)/360.f;
        if(rel<0)rel=0;if(rel>1)rel=1;
        float cx= sli.x+ rel*sli.width;
        DrawCircle((int)cx,(int)(sli.y+5),5,WHITE);
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&& CheckCollisionPointRec(GetMousePosition(),sli)
           && !editRotX)
        {
          float r=(GetMousePosition().x - sli.x)/sli.width;
          if(r<0)r=0;if(r>1)r=1;
          rotX= r*360 -180;
          snprintf(txtRotX,16,"%.1f", rotX);
        }
      }
      baseY+= espLin;

      {
        DrawTextEx(guiFont,"Y:",{baseX, baseY},18,1,WHITE);
        bool clicked= drawTextBox(txtRotY,16, baseX+30, baseY,80,24, editRotY);
        if(clicked){
          editRotY=true; editRotX=false; editRotZ=false;
        }
        textBoxInput(txtRotY,16, editRotY);
        if(!editRotY){
          rotY= parseFloat(txtRotY,rotY);
        }
        Rectangle sli={baseX+120, baseY+6,120,10};
        DrawRectangleRec(sli,GRAY);
        float rel= (rotY+180)/360.f;
        if(rel<0)rel=0;if(rel>1)rel=1;
        float cx= sli.x+ rel*sli.width;
        DrawCircle((int)cx,(int)(sli.y+5),5,WHITE);
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(GetMousePosition(),sli)
           && !editRotY)
        {
          float r=(GetMousePosition().x-sli.x)/sli.width;
          if(r<0)r=0;if(r>1)r=1;
          rotY= r*360-180;
          snprintf(txtRotY,16,"%.1f", rotY);
        }
      }
      baseY+= espLin;

      {
        DrawTextEx(guiFont,"Z:",{baseX, baseY},18,1,WHITE);
        bool clicked= drawTextBox(txtRotZ,16, baseX+30, baseY,80,24, editRotZ);
        if(clicked){
          editRotZ=true; editRotX=false; editRotY=false;
        }
        textBoxInput(txtRotZ,16, editRotZ);
        if(!editRotZ){
          rotZ= parseFloat(txtRotZ, rotZ);
        }
        Rectangle sli={baseX+120, baseY+6,120,10};
        DrawRectangleRec(sli,GRAY);
        float rel= (rotZ+180)/360.f;
        if(rel<0)rel=0;if(rel>1)rel=1;
        float cx= sli.x+ rel*sli.width;
        DrawCircle((int)cx,(int)(sli.y+5),5,WHITE);
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(GetMousePosition(),sli)
           && !editRotZ)
        {
          float r=(GetMousePosition().x-sli.x)/sli.width;
          if(r<0)r=0;if(r>1)r=1;
          rotZ= r*360-180;
          snprintf(txtRotZ,16,"%.1f", rotZ);
        }
      }
      baseY+= espLin;

      {
        Rectangle bR= {baseX, baseY,120,28};
        DrawRectangleRec(bR,(Color){120,80,80,255});
        DrawTextEx(guiFont,"APLICAR R",{bR.x+3,bR.y+3},20,2,WHITE);
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&& CheckCollisionPointRec(GetMousePosition(),bR)){
          Matriz mx= Matriz::rotacao_x(DEG2RAD*rotX);
          Matriz my= Matriz::rotacao_y(DEG2RAD*rotY);
          Matriz mz= Matriz::rotacao_z(DEG2RAD*rotZ);
          Matriz final= mx*my*mz;
          objetos[objetoSelecionado].transformar(final);
          renderizar();
        }
      }
      baseY+= espLin*1.3f;

      DrawTextEx(guiFont,"Escala:", {baseX, baseY},18,1,WHITE);
      baseY+= espLin;

      {
        DrawTextEx(guiFont, "X:",{baseX, baseY},18,1,WHITE);
        bool clicked= drawTextBox(txtEscX,16, baseX+30, baseY,80,24, editEscX);
        if(clicked){
          editEscX=true; editEscY=false; editEscZ=false;
        }
        textBoxInput(txtEscX,16, editEscX);
        if(!editEscX){
          escX= parseFloat(txtEscX, escX);
        }
        Rectangle sli={baseX+120, baseY+6,120,10};
        DrawRectangleRec(sli,GRAY);
        float rel= (escX-0.1f)/1.9f; 
        if(rel<0)rel=0;if(rel>1)rel=1;
        float cx= sli.x+ rel*sli.width;
        DrawCircle((int)cx,(int)(sli.y+5),5,WHITE);
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&& CheckCollisionPointRec(GetMousePosition(),sli)
           && !editEscX)
        {
          float r=(GetMousePosition().x- sli.x)/sli.width;
          if(r<0)r=0;if(r>1)r=1;
          escX= 0.1f+ r*1.9f;
          snprintf(txtEscX,16,"%.2f", escX);
        }
      }
      baseY+= espLin;

      {
        DrawTextEx(guiFont, "Y:",{baseX, baseY},18,1,WHITE);
        bool clicked= drawTextBox(txtEscY,16, baseX+30, baseY,80,24, editEscY);
        if(clicked){
          editEscY=true; editEscX=false; editEscZ=false;
        }
        textBoxInput(txtEscY,16, editEscY);
        if(!editEscY){
          escY= parseFloat(txtEscY, escY);
        }
        Rectangle sli={baseX+120, baseY+6,120,10};
        DrawRectangleRec(sli,GRAY);
        float rel= (escY-0.1f)/1.9f;
        if(rel<0)rel=0;if(rel>1)rel=1;
        float cx= sli.x+ rel*sli.width;
        DrawCircle((int)cx,(int)(sli.y+5),5,WHITE);
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&& CheckCollisionPointRec(GetMousePosition(),sli)
           && !editEscY)
        {
          float r=(GetMousePosition().x- sli.x)/sli.width;
          if(r<0)r=0;if(r>1)r=1;
          escY= 0.1f+ r*1.9f;
          snprintf(txtEscY,16,"%.2f", escY);
        }
      }
      baseY+= espLin;

      {
        DrawTextEx(guiFont, "Z:",{baseX, baseY},18,1,WHITE);
        bool clicked= drawTextBox(txtEscZ,16, baseX+30, baseY,80,24, editEscZ);
        if(clicked){
          editEscZ=true; editEscX=false; editEscY=false;
        }
        textBoxInput(txtEscZ,16, editEscZ);
        if(!editEscZ){
          escZ= parseFloat(txtEscZ, escZ);
        }
        Rectangle sli={baseX+120, baseY+6,120,10};
        DrawRectangleRec(sli,GRAY);
        float rel= (escZ-0.1f)/1.9f;
        if(rel<0)rel=0;if(rel>1)rel=1;
        float cx= sli.x+ rel*sli.width;
        DrawCircle((int)cx,(int)(sli.y+5),5,WHITE);
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&& CheckCollisionPointRec(GetMousePosition(),sli)
           && !editEscZ)
        {
          float r=(GetMousePosition().x- sli.x)/sli.width;
          if(r<0)r=0;if(r>1)r=1;
          escZ= 0.1f+ r*1.9f;
          snprintf(txtEscZ,16,"%.2f", escZ);
        }
      }
      baseY+= espLin;

      {
        Rectangle bE= {baseX, baseY,120,28};
        DrawRectangleRec(bE,(Color){120,120,80,255});
        DrawTextEx(guiFont,"APLICAR E",{bE.x+3,bE.y+3},20,2,WHITE);
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&& CheckCollisionPointRec(GetMousePosition(),bE)){
          Matriz me= Matriz::escala({escX, escY, escZ});
          objetos[objetoSelecionado].transformar(me);
          renderizar();
        }
      }
      baseY+= espLin*1.3f;
    }
    else {
      DrawTextEx(guiFont,"Nenhum objeto selecionado",
                 {baseX, baseY},18,1,RED);
      baseY+= 40;
    }

    DrawTextEx(guiFont,"Camera Eye:", {baseX, baseY}, 20,1, WHITE);
    baseY+= espLin;

    {
      DrawTextEx(guiFont, "X:", {baseX, baseY},18,1,WHITE);
      bool clicked= drawTextBox(txtCamX,16, baseX+30, baseY,80,24, editCamX);
      if(clicked){
        editCamX=true; editCamY=false; editCamZ=false;
      }
      textBoxInput(txtCamX,16, editCamX);
      if(!editCamX){
        camX= parseFloat(txtCamX, camX);
      }
      Rectangle sli={baseX+120, baseY+6,120,10};
      DrawRectangleRec(sli,GRAY);
      float rel= (camX+500)/1000.f;
      if(rel<0)rel=0;if(rel>1)rel=1;
      float cx= sli.x+ rel*sli.width;
      DrawCircle((int)cx,(int)(sli.y+5),5,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(GetMousePosition(),sli)
         && !editCamX)
      {
        float r=(GetMousePosition().x- sli.x)/sli.width;
        if(r<0)r=0;if(r>1)r=1;
        camX= r*1000-500;
        snprintf(txtCamX,16,"%.1f",camX);
      }
    }
    baseY+= espLin;

    {
      DrawTextEx(guiFont, "Y:", {baseX, baseY},18,1,WHITE);
      bool clicked= drawTextBox(txtCamY,16, baseX+30, baseY,80,24, editCamY);
      if(clicked){
        editCamY=true; editCamX=false; editCamZ=false;
      }
      textBoxInput(txtCamY,16, editCamY);
      if(!editCamY){
        camY= parseFloat(txtCamY, camY);
      }
      Rectangle sli={baseX+120, baseY+6,120,10};
      DrawRectangleRec(sli,GRAY);
      float rel= (camY+500)/1000.f;
      if(rel<0)rel=0;if(rel>1)rel=1;
      float cx= sli.x+ rel*sli.width;
      DrawCircle((int)cx,(int)(sli.y+5),5,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(GetMousePosition(),sli)
         && !editCamY)
      {
        float r=(GetMousePosition().x- sli.x)/sli.width;
        if(r<0)r=0;if(r>1)r=1;
        camY= r*1000-500;
        snprintf(txtCamY,16,"%.1f",camY);
      }
    }
    baseY+= espLin;

    {
      DrawTextEx(guiFont, "Z:", {baseX, baseY},18,1,WHITE);
      bool clicked= drawTextBox(txtCamZ,16, baseX+30, baseY,80,24, editCamZ);
      if(clicked){
        editCamZ=true; editCamX=false; editCamY=false;
      }
      textBoxInput(txtCamZ,16, editCamZ);
      if(!editCamZ){
        camZ= parseFloat(txtCamZ, camZ);
      }
      Rectangle sli={baseX+120, baseY+6,120,10};
      DrawRectangleRec(sli,GRAY);
      float rel= (camZ+500)/1000.f;
      if(rel<0)rel=0;if(rel>1)rel=1;
      float cx= sli.x+ rel*sli.width;
      DrawCircle((int)cx,(int)(sli.y+5),5,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&& CheckCollisionPointRec(GetMousePosition(),sli)
         && !editCamZ)
      {
        float r=(GetMousePosition().x- sli.x)/sli.width;
        if(r<0)r=0;if(r>1)r=1;
        camZ= r*1000-500;
        snprintf(txtCamZ,16,"%.1f",camZ);
      }
    }
    baseY+= espLin;

    {
      Rectangle bEye= {baseX, baseY,140,28};
      DrawRectangleRec(bEye,(Color){80,120,150,255});
      DrawTextEx(guiFont, "ATUALIZAR EYE", {bEye.x+5,bEye.y+3}, 18,2,WHITE);
      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&& CheckCollisionPointRec(GetMousePosition(),bEye)){
        camera.position= {camX, camY, camZ};
        camera.updateCoordinates();
        renderizar();
      }
    }
    baseY+= espLin*1.2f;

    DrawTextEx(guiFont,"Camera (At):", {baseX, baseY},20,1, WHITE);
    baseY+= espLin;

    {
      DrawTextEx(guiFont,"X:",{baseX,baseY},18,1,WHITE);
      bool clicked= drawTextBox(txtAtX,16, baseX+30, baseY,80,24, editAtX);
      if(clicked){
        editAtX=true;editAtY=false;editAtZ=false;
      }
      textBoxInput(txtAtX,16, editAtX);
      if(!editAtX){
        atX= parseFloat(txtAtX, atX);
      }
      Rectangle sli={baseX+120, baseY+6,120,10};
      DrawRectangleRec(sli,GRAY);
      float rel= (atX+500)/1000.f;
      if(rel<0)rel=0;if(rel>1)rel=1;
      float cx= sli.x+ rel*sli.width;
      DrawCircle((int)cx,(int)(sli.y+5),5,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(GetMousePosition(),sli)
         && !editAtX)
      {
        float r=(GetMousePosition().x- sli.x)/sli.width;
        if(r<0)r=0;if(r>1)r=1;
        atX= r*1000-500;
        snprintf(txtAtX,16,"%.1f", atX);
      }
    }
    baseY+= espLin;

    {
      DrawTextEx(guiFont,"Y:",{baseX,baseY},18,1,WHITE);
      bool clicked= drawTextBox(txtAtY,16, baseX+30, baseY,80,24, editAtY);
      if(clicked){
        editAtY=true;editAtX=false;editAtZ=false;
      }
      textBoxInput(txtAtY,16, editAtY);
      if(!editAtY){
        atY= parseFloat(txtAtY, atY);
      }
      Rectangle sli={baseX+120, baseY+6,120,10};
      DrawRectangleRec(sli,GRAY);
      float rel= (atY+500)/1000.f;
      if(rel<0)rel=0;if(rel>1)rel=1;
      float cx= sli.x+ rel*sli.width;
      DrawCircle((int)cx,(int)(sli.y+5),5,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(GetMousePosition(),sli)
         && !editAtY)
      {
        float r=(GetMousePosition().x-sli.x)/sli.width;
        if(r<0)r=0;if(r>1)r=1;
        atY= r*1000-500;
        snprintf(txtAtY,16,"%.1f",atY);
      }
    }
    baseY+= espLin;

    {
      DrawTextEx(guiFont,"Z:",{baseX,baseY},18,1,WHITE);
      bool clicked= drawTextBox(txtAtZ,16, baseX+30, baseY,80,24, editAtZ);
      if(clicked){
        editAtZ=true;editAtX=false;editAtY=false;
      }
      textBoxInput(txtAtZ,16, editAtZ);
      if(!editAtZ){
        atZ= parseFloat(txtAtZ, atZ);
      }
      Rectangle sli={baseX+120, baseY+6,120,10};
      DrawRectangleRec(sli,GRAY);
      float rel= (atZ+500)/1000.f;
      if(rel<0)rel=0;if(rel>1)rel=1;
      float cx= sli.x+ rel*sli.width;
      DrawCircle((int)cx,(int)(sli.y+5),5,WHITE);
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)&& CheckCollisionPointRec(GetMousePosition(),sli)
         && !editAtZ)
      {
        float r=(GetMousePosition().x-sli.x)/sli.width;
        if(r<0)r=0;if(r>1)r=1;
        atZ= r*1000-500;
        snprintf(txtAtZ,16,"%.1f",atZ);
      }
    }
    baseY+= espLin;

    {
      Rectangle bA= {baseX, baseY,150,28};
      DrawRectangleRec(bA,(Color){60,120,160,255});
      DrawTextEx(guiFont,"ATUALIZAR AT",{bA.x+5,bA.y+3},18,2,WHITE);
      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(GetMousePosition(),bA)){
        camera.lookAt= {atX,atY,atZ};
        camera.updateCoordinates();
        renderizar();
      }
    }
}
int main()
{
    omp_set_num_threads(4);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Trabalho CG - GUI (Sliders + Digitação)");
    SetTargetFPS(60);

    guiFont = LoadFontEx("assets/roboto.ttf", 36, NULL, 250);
    SetTextureFilter(guiFont.texture, TEXTURE_FILTER_BILINEAR);

    tela = LoadRenderTexture(nCol, nLin);

    inicializar_objetosfinal(objetos, complexObjects);
    for(auto &cobj: complexObjects){
      flatten_objetos(cobj, objetos);
    }
    std::cout << "Objetos: " << objetos.size() << "\n";

    inicializar_luzes();

    camera.updateCoordinates();
    camX= camera.position.x;    snprintf(txtCamX,16,"%.1f", camX);
    camY= camera.position.y;    snprintf(txtCamY,16,"%.1f", camY);
    camZ= camera.position.z;    snprintf(txtCamZ,16,"%.1f", camZ);

    atX= camera.lookAt.x;       snprintf(txtAtX,16,"%.1f", atX);
    atY= camera.lookAt.y;       snprintf(txtAtY,16,"%.1f", atY);
    atZ= camera.lookAt.z;       snprintf(txtAtZ,16,"%.1f", atZ);

    renderizar();

    while(!WindowShouldClose()){
      Vector2 mouse= GetMousePosition();
      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        if(mouse.x< (float)nCol && mouse.y<(float)nLin){
          int mx= mouse.x;
          int my= mouse.y;
          Matriz M_cw= camera.getMatrixCameraWorld();
          Vetor3d pse= (M_cw*camera.get_PSE().ponto4d()).vetor3d();
          Vetor3d rv= (M_cw*Vetor3d{1,0,0}.vetor4d()).vetor3d();
          Vetor3d dv= (M_cw*Vetor3d{0,-1,0}.vetor4d()).vetor3d();

          float dx= camera.get_W_J()/nCol;
          float dy= camera.get_H_J()/nLin;

          Vetor3d P= pse + rv*(dx*(mx+0.5f)) + dv*(dy*(my+0.5f));
          Vetor3d dir= (P - camera.position).normalizado();
          Raio r(camera.position, dir);
          auto [tt, idx]= calcular_intersecao(r, objetos);
          if(tt>0.0f){
            objetoSelecionado= idx;
            TraceLog(LOG_INFO,"Selecionado %d", idx);
          }
        }
      }

      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        if(mouse.x>RENDER_W) {
        } else {
          editTransX=false;editTransY=false;editTransZ=false;
          editRotX=false;  editRotY=false;  editRotZ=false;
          editEscX=false;  editEscY=false;  editEscZ=false;
          editCamX=false;  editCamY=false;  editCamZ=false;
          editAtX=false;   editAtY=false;   editAtZ=false;
        }
      }

      BeginDrawing();
      {
        ClearBackground(BLACK);

        DrawTextureRec(tela.texture,{0,0,(float)nCol,(float)-nLin},{0,0},WHITE);

        desenharGUI();
      }
      EndDrawing();
    }

    UnloadFont(guiFont);
    UnloadRenderTexture(tela);
    CloseWindow();
    return 0;
}
