#ifndef UTILS_H
#define UTILS_H

#include <allegro5/color.h>

// Definições de tamanho do tabuleiro e dos blocos (pixels)
#define ALTURA 24
#define LARGURA 24
#define TILE 40

// Enums para organizar as direções e as telas (estados) do jogo
typedef enum Direcao { CIMA, BAIXO, ESQUERDA, DIREITA } Direcao;

typedef enum Estado { MENU, JOGO, SAIR } Estado;


// Struct para cada segmento (gomo) do corpo da cobra
typedef struct Corpo {
  int x;
  int y;
} Corpo;

struct No {
  Corpo dado;
  struct No *proximo;
  struct No *anterior;
};

typedef struct Lista {
  struct No *inicio;
  struct No *fim;
} Lista;

// Struct que controla a cabeça, pontuação e o ponteiro para o corpo
typedef struct Cabeca {
  int x;
  int y;
  int pontuacao;
  int tamanho;
  Lista *cauda;
} Cabeca;

// Struct para a posição e valor de pontos da maçã
typedef struct Maca {
  int x;
  int y;
  int pontos;
} Maca;

// Struct para renderização e detecção de cliques em botões
typedef struct Botao {
  int x, y;
  int largura, altura;
  ALLEGRO_COLOR cor;
  char texto[32];
} Botao;

#endif
