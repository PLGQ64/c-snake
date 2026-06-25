#include "jogo.h"
#include "utils.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

void inserir_final(Cabeca *cobra) {
  struct No *pi = cobra->cauda->fim;
  struct No *novo;

  novo = (struct No *)malloc(sizeof(struct No));
  novo->anterior = NULL;

  if (cobra->cauda->fim == NULL) {
    novo->dado.x = cobra->x + 1;
    novo->dado.y = cobra->y;
    cobra->cauda->inicio = novo;
    cobra->cauda->fim = novo;
    return;
  }
  novo->anterior = pi;
  cobra->cauda->fim = novo;
}

void liberar_lista(Cabeca *cobra) {
  struct No *pi = cobra->cauda->fim;
  struct No *proximo;
  while (pi != NULL) {
    proximo = pi->anterior;
    free(pi);
    pi = proximo;
  }
  free(cobra->cauda);
  cobra->cauda = NULL;
}

void inicializar(Cabeca *cobra, Maca *maca, Direcao *dir) {
  *dir = ESQUERDA;
  cobra->x = LARGURA / 2;
  cobra->y = ALTURA / 2;
  cobra->pontuacao = 0;
  cobra->tamanho = 1;

  cobra->cauda = (Lista *)malloc(sizeof(Lista));
  cobra->cauda->inicio = NULL;
  cobra->cauda->fim = NULL;

  // cobra->cauda = (Corpo *)malloc(cobra->tamanho * sizeof(Corpo));
  // for (int i = 0; i < cobra->tamanho; i++) {
  //   cobra->cauda[i].x = cobra->x + (i + 1);
  //   cobra->cauda[i].y = cobra->y;
  // }
  inserir_final(cobra);

  maca->x = rand() % (LARGURA - 2) + 1;
  maca->y = rand() % (ALTURA - 2) + 1;
  maca->pontos = 1;
}

// Processa toda a física do jogo: movimento, crescimento e colisões
void logica(Cabeca *cobra, Maca *maca, Estado *estado, Direcao *dir) {
  // Move os gomos do corpo para a posição do gomo anterior (efeito dominó)

  // for (int i = cobra->tamanho - 1; i > 0; i--) {
  //   cobra->cauda[i].x = cobra->cauda[i - 1].x;
  //   cobra->cauda[i].y = cobra->cauda[i - 1].y;
  // }

  // Depois que todos os gomos "andaram" para a frente, o gomo zero pega a mesma
  // posicao da cabeça

  // cobra->cauda[0].x = cobra->x;
  // cobra->cauda[0].y = cobra->y;

  // move a cabeça

  struct No *atual = cobra->cauda->fim;
  // while (atual->proximo != NULL) {
  //   atual = atual->proximo;
  // }

  while (atual->anterior != NULL) {
    atual->dado.x = atual->anterior->dado.x;
    atual->dado.y = atual->anterior->dado.y;
    atual = atual->anterior;
  }

  cobra->cauda->inicio->dado.x = cobra->x;
  cobra->cauda->inicio->dado.y = cobra->y;

  if (*dir == CIMA) {
    cobra->y -= 1;
  } else if (*dir == BAIXO) {
    cobra->y += 1;
  } else if (*dir == ESQUERDA) {
    cobra->x -= 1;
  } else if (*dir == DIREITA) {
    cobra->x += 1;
  }

  // verifica se a cabeça pegou a maça
  if (cobra->x == maca->x && cobra->y == maca->y) {
    inserir_final(cobra);
    cobra->tamanho += 1;
    cobra->pontuacao += maca->pontos;
    // cobra->cauda = realloc(cobra->cauda, cobra->tamanho * sizeof(Corpo));
    // cobra->cauda[cobra->tamanho - 1] = cobra->cauda[cobra->tamanho - 2];

    bool visivel;
    do {
      visivel = true;
      maca->x = rand() % (LARGURA - 2) + 1;
      maca->y = rand() % (ALTURA - 2) + 1;

      if (cobra->x == maca->x && cobra->y == maca->y) {
        visivel = false;
        continue; // sorteio caiu na cabeça: sorteia tudo de novo
      }
      atual = cobra->cauda->fim;
      while (atual != NULL) {
        if (atual->dado.x == maca->x && atual->dado.y == maca->y) {
          visivel = false;
          break; // já sabemos que colidiu; não precisa checar o resto da cauda
        }
        atual = atual->anterior;
      }
    } while (visivel == false);
  }

  // verifica se a cobra bateu em uma parede
  if (cobra->y == 0 || cobra->y == ALTURA - 1 || cobra->x == 0 ||
      cobra->x == LARGURA - 1) {
    *estado = MENU;
  }

  // verifica se a cobra bateu no proprio corpo
  atual = cobra->cauda->fim;
  while (atual != NULL) {
    if (cobra->x == atual->dado.x && cobra->y == atual->dado.y) {
      *estado = MENU;
    }
    atual = atual->anterior;
  }

  if (cobra->tamanho => 480) {
    *estado = MENU;
  }
}

void preencherMapa(Cabeca *cobra, Maca *maca, int **mapa) {
  for (int y = 0; y < ALTURA; y++) {
    for (int x = 0; x < LARGURA; x++) {
      if (y == 0 || y == ALTURA - 1 || x == 0 || x == LARGURA - 1) {
        mapa[y][x] = 4;
      } else {
        mapa[y][x] = 0;
      }

      if (y == maca->y && x == maca->x) {
        mapa[y][x] = 3;
      }

      struct No *atual = cobra->cauda->fim;
      while (atual != NULL) {
        if (y == atual->dado.y && x == atual->dado.x) {
          mapa[y][x] = 2;
          break;
        }
        atual = atual->anterior;
      }

      if (y == cobra->y && x == cobra->x) {
        mapa[y][x] = 1;
      }
    }
  }
}
