#ifndef UI_H
#define UI_H

#include <stdbool.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#include "utils.h" // Cabeca, Botao

// Funções de renderização (desenho na tela com Allegro), interação com
// botões/mouse, e persistência do placar em arquivo. Tudo que depende
// de I/O (arquivo ou tela) mora aqui, separado da lógica pura do jogo.

void desenhar(int **mapa);
void desenharBotao(Botao *botao, ALLEGRO_FONT *fonte);
bool clickBotao(Botao botao, int mouse_x, int mouse_y);
void desenharPontuacao(Cabeca *c, ALLEGRO_FONT *fonte);
void desenhar_scoreboard(int *board, ALLEGRO_FONT *font);
void grid(void);

void ordenar_pontuacao(int nova_pontuacao, int *board);
void ler_pontuacao(int *board);
void salvar_pontuacao(int *board);
void apagar_pontuacao(int p, int *board);

#endif
