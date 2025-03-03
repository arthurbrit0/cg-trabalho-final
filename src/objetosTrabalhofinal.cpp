#include <iostream>
#include <raylib.h>
#include <stack>
#include <cmath>

#include "inicializar_objetos.h"   // se você tiver esse header
#include "./PlanoTextura/PlanoTextura.h"
#include "./Malha/Malha.h"
#include "./Cilindro/Cilindro.h"
#include "./Cone/Cone.h"
#include "./Esfera/Esfera.h"
#include "./ObjetoComplexo/ObjetoComplexo.h"
#include "./Circulo/Circulo.h"

// Esta é a função que você vai chamar no seu main:
void inicializar_objetosfinal(std::vector<Objeto>& objects_flat,
                              std::vector<ObjetoComplexo>& complexObjects)
{
    // 1) Carregar uma textura para o chão.
    //    (Troque "assets/chao2.png" por algo que você tenha.)
    Image textura_chao = LoadImage("assets/grama.png");
    Color* pixels_textura_chao = LoadImageColors(textura_chao);
    if (!textura_chao.data) {
        std::cerr << "Erro ao carregar textura do chão.\n";
        return;
    }
    // Mapeia, por exemplo, 200x200 unidades de cena
    Textura texturaChao(
        pixels_textura_chao,
        textura_chao.width,
        textura_chao.height,
        200.0f,
        200.0f,
        10.0f
    );

    // Plano texturizado como "solo"
    PlanoTextura solo(
        /* ponto no plano */ {0.0f, 0.0f, 0.0f},
        /* eixo1 */          {1.0f, 0.0f, 0.0f},
        /* eixo2 */          {0.0f, 0.0f, 1.0f},
        /* textura */        texturaChao
    );
    objects_flat.push_back(solo);

    std::cout << "Inicializou plano do solo.\n";

    // 2) Criar um CASTELO como ObjetoComplexo
    ObjetoComplexo castelo;

    // 2.1) Base (um cubo grande via Malha)
    Malha base;
    Vetor3d K_pedra = { 0.6f, 0.6f, 0.6f }; // cor de pedra
    base.inicializar_cubo(
        {0.0f, 10.0f, 0.0f}, // centro
        60.0f,               // aresta do cubo
        K_pedra, K_pedra, K_pedra, 16.0f
    );
    // base em Y=10 => eleva 10 acima do plano
    castelo.adicionar_objeto(base);

    // 2.2) Quatro torres (Cilindro) nos cantos
    auto criarTorre = [&](float x, float z){
        Cilindro torre(
            {x, 0.0f, z}, // centro na base
            5.0f,         // raio
            40.0f,        // altura
            Vetor3d{0.0f,1.0f,0.0f}, // para cima
            K_pedra, K_pedra, K_pedra, 16.0f
        );
        return torre;
    };
    // Como o cubo tem half=30, os cantos ficam +/- em (±25,0,±25).
    castelo.adicionar_objeto(criarTorre(+25.0f, +25.0f));
    castelo.adicionar_objeto(criarTorre(+25.0f, -25.0f));
    castelo.adicionar_objeto(criarTorre(-25.0f, +25.0f));
    castelo.adicionar_objeto(criarTorre(-25.0f, -25.0f));

    // 2.3) Cones no topo das torres (telhadinhos)
    auto criarTelhado = [&](float x, float z){
        Cone telhadinho(
            {x, 40.0f, z},  // topo da torre (40 de altura)
            8.0f,           // raio
            12.0f,          // altura
            Vetor3d{0,1,0}, // para cima
            {0.7f,0.3f,0.3f}, // K_d
            {1.0f,1.0f,1.0f}, // K_e
            {0.1f,0.1f,0.1f}, // K_a
            32.0f
        );
        return telhadinho;
    };
    castelo.adicionar_objeto(criarTelhado(+25.0f, +25.0f));
    castelo.adicionar_objeto(criarTelhado(+25.0f, -25.0f));
    castelo.adicionar_objeto(criarTelhado(-25.0f, +25.0f));
    castelo.adicionar_objeto(criarTelhado(-25.0f, -25.0f));

    complexObjects.push_back(castelo);
    std::cout << "Adicionou castelo.\n";

    // 3) Criar uma FONTE no meio (Cilindro base + esfera d'água)
    ObjetoComplexo fonte;

    // 3.1) Base (Cilindro "raso")
    Cilindro baseFonte(
        {0.0f, 0.0f, 80.0f}, // no plano
        10.0f,               // raio
        2.0f,                // altura
        {0,1,0},            // para cima
        {0.3f,0.3f,0.3f},   // K_d
        {1.0f,1.0f,1.0f},   // K_e
        {0.1f,0.1f,0.1f},   // K_a
        8.0f
    );
    fonte.adicionar_objeto(baseFonte);

    // 3.2) Esfera de "água"
    Esfera agua(
        {0.0f, 3.0f, 80.0f},
        5.0f,
        {0.0f,0.5f,1.0f},  // K_d => meio azulado
        {1.0f,1.0f,1.0f},  // K_e
        {0.1f,0.1f,0.2f},  // K_a
        32.0f
    );
    fonte.adicionar_objeto(agua);

    complexObjects.push_back(fonte);
    std::cout << "Adicionou fonte.\n";

    // 4) Criar algumas ÁRVORES
    auto criarArvore = [&](float x, float z){
        ObjetoComplexo arvore;
        // Tronco
        Cilindro tronco(
            {x, 0.0f, z}, // base
            2.0f,         // raio
            15.0f,        // altura
            {0,1,0},
            {0.5f,0.35f,0.2f}, // marrom
            {1.0f,1.0f,1.0f},
            {0.1f,0.1f,0.1f},
            8.0f
        );
        arvore.adicionar_objeto(tronco);

        // Copa (cone)
        Cone copa(
            {x, 15.0f, z},
            8.0f,
            20.0f,
            {0,1,0},
            {0.0f,0.6f,0.0f}, // verde
            {1.0f,1.0f,1.0f},
            {0.1f,0.1f,0.1f},
            16.0f
        );
        arvore.adicionar_objeto(copa);
        return arvore;
    };
    // Insere algumas
    complexObjects.push_back(criarArvore(60.0f, -40.0f));
    complexObjects.push_back(criarArvore(-70.0f, -60.0f));
    complexObjects.push_back(criarArvore(40.0f, 120.0f));
    std::cout << "Adicionou árvores.\n";

    // 5) Criar um MURO como Malha
    Malha muro;
    Vetor3d K_muro = { 0.7f, 0.7f, 0.7f };
    muro.inicializar_cubo(
        {0.0f, 5.0f, -100.0f}, // centro
        200.0f,                // aresta
        K_muro, K_muro, K_muro,
        16.0f
    );
    // Afinar na profundidade => 200 x 10 x 5, p.ex.
    muro.transformar(
        Matriz::escala({1.0f, 0.5f, 0.05f})
    );
    // Adiciona ao vector "objects_flat" (se for objeto simples)
    // ou a um ObjetoComplexo, mas aqui podemos mandar direto
    objects_flat.push_back(muro);

    // Dica: Se ficar distante, ajuste a posição, por ex. z = -80

    std::cout << "Cena criativa inicializada com sucesso!\n";
}
