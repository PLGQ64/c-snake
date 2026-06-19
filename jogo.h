#ifndef JOGO_H
#define JOGO_H

#include "utils.h" // Cabeca, Maca, Direcao, Estado

// Funções de física e estado do jogo: movimento, crescimento da cobra,
// detecção de colisões e posicionamento da maçã. Não dependem do Allegro
// nem desenham nada na tela.

void inicializar(Cabeca *cobra, Maca *maca, Direcao *dir);
void logica(Cabeca *cobra, Maca *maca, Estado *estado, Direcao *dir);
void preencherMapa(Cabeca *cobra, Maca *maca, int **mapa);
void inserir_final(Cabeca *cobra);
void liberar_lista(Cabeca *cobra);

#endif
