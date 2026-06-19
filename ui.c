#include "ui.h"

#include <stdio.h>
#include <allegro5/allegro_primitives.h>

void ler_pontuacao(int *board) {
  FILE *file = fopen("pontuacao.bin", "rb");
  if (file == NULL) {
    // Arquivo ainda não existe: cria com zeros e tenta reabrir para leitura
    file = fopen("pontuacao.bin", "wb");
    if (file == NULL) {
      // Não foi possível criar o arquivo (ex: sem permissão); mantém o
      // board zerado em memória e desiste de ler/gravar pontuação
      return;
    }
    fwrite(board, sizeof(int), 6, file);
    fclose(file);
    file = fopen("pontuacao.bin", "rb");
    if (file == NULL) {
      return;
    }
  }
  fread(board, sizeof(int), 6, file);
  fclose(file);
}

// salva a pontuacao no arquivo
void salvar_pontuacao(int *board) {
  FILE *file = fopen("pontuacao.bin", "wb");
  fwrite(board, sizeof(int), 6, file);
  fclose(file);
}

// desenha a score board
void desenhar_scoreboard(int *board, ALLEGRO_FONT *font) {
  al_draw_filled_rectangle(11 * TILE, 4 * TILE, 13 * TILE, 10 * TILE,
                           al_map_rgb(30, 175, 175));
  for (int i = 0; i < 6; i++) {
    al_draw_textf(font, al_map_rgb(255, 255, 255), 12 * TILE,
                  (4 + i) * TILE + 10, ALLEGRO_ALIGN_CENTER, "%d", board[i]);
  }
}

// Zera uma pontuação específica dentro do vetor do placar (Highscores)
void apagar_pontuacao(int p, int *board) {
  for (int i = 0; i < 6; i++) {
    if (i == p) {
      board[i] = 0;
    }
  }
  ordenar_pontuacao(0, board);
  salvar_pontuacao(board);
}

// Insere a nova pontuação no final do vetor e o ordena de forma decrescente
// (Bubble Sort)
void ordenar_pontuacao(int nova_pontuacao, int *board) {
  if (nova_pontuacao > board[5]) {
    board[5] = nova_pontuacao;
  }
  bool modificou = true;
  for (int i = 0; i < 5 && modificou == true; i++) {
    modificou = false;
    for (int k = 0; k < 5 - i; k++) {
      if (board[k] < board[k + 1]) {
        int temp = board[k + 1];
        board[k + 1] = board[k];
        board[k] = temp;
        modificou = true;
      }
    }
  }
}

void desenharPontuacao(Cabeca *c, ALLEGRO_FONT *fonte) {
  al_draw_textf(fonte, al_map_rgb(255, 255, 255), (1 * TILE) / 2 - 5,
                (1 * TILE) / 2 - 5, 0, "%d", c->pontuacao);
}

bool clickBotao(Botao botao, int mouse_x, int mouse_y) {
  if (mouse_x >= botao.x * TILE &&
      mouse_x <= (botao.x * TILE + botao.largura * TILE) &&
      mouse_y >= botao.y * TILE &&
      mouse_y <= (botao.y * TILE + botao.altura * TILE)) {
    return true; 
  }
  return false; 
}

void desenharBotao(Botao *botao, ALLEGRO_FONT *fonte) {
  al_draw_filled_rectangle(botao->x * TILE, botao->y * TILE,
                           botao->x * TILE + botao->largura * TILE,
                           botao->y * TILE + botao->altura * TILE, botao->cor);
  al_draw_text(fonte, al_map_rgb(255, 255, 255),
               botao->x * TILE + (botao->largura * TILE / 2),
               botao->y * TILE + (botao->altura * TILE / 2),
               ALLEGRO_ALIGN_CENTRE, botao->texto);
}

void desenhar(int **mapa) {
  for (int y = 0; y < ALTURA; y++) {
    for (int x = 0; x < LARGURA; x++) {
      int x1 = x * TILE;
      int y1 = y * TILE;
      int x2 = x1 + TILE;
      int y2 = y1 + TILE;

      int espacamento_gomo = (TILE * 31 / 32);
      int espacamento_maca = (TILE * 8 / 32);

      if (mapa[y][x] == 1) { 
        al_draw_filled_rectangle(x1 + espacamento_gomo, y1 + espacamento_gomo,
                                 x2 - espacamento_gomo, y2 - espacamento_gomo,
                                 al_map_rgb(0, 150, 0));
        al_draw_rectangle(x1 + espacamento_gomo - 2, y1 + espacamento_gomo - 2,
                          x2 - espacamento_gomo + 2, y2 - espacamento_gomo + 2,
                          al_map_rgb(0, 100, 0), 4);
      } else if (mapa[y][x] == 2) { 
        al_draw_filled_rectangle(x1 + espacamento_gomo, y1 + espacamento_gomo,
                                 x2 - espacamento_gomo, y2 - espacamento_gomo,
                                 al_map_rgb(0, 255, 0));
        al_draw_rectangle(x1 + espacamento_gomo - 2, y1 + espacamento_gomo - 2,
                          x2 - espacamento_gomo + 2, y2 - espacamento_gomo + 2,
                          al_map_rgb(0, 150, 0), 4);
      } else if (mapa[y][x] == 3) { 
        al_draw_filled_rectangle(x1 + espacamento_maca, y1 + espacamento_maca,
                                 x2 - espacamento_maca, y2 - espacamento_maca,
                                 al_map_rgb(255, 0, 0));
        al_draw_rectangle(x1 + espacamento_maca - 2, y1 + espacamento_maca - 2,
                          x2 - espacamento_maca + 2, y2 - espacamento_maca + 2,
                          al_map_rgb(130, 0, 0), 4);
      } else if (mapa[y][x] == 4) { 
        al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(200, 200, 0));
      }
    }
  }
}

void grid(void) {
  for (int y = 0; y < ALTURA; y++) {
    for (int x = 0; x < LARGURA; x++) {
      int x1 = x * TILE;
      int y1 = y * TILE;
      int x2 = x1 + TILE;
      int y2 = y1 + TILE;
      al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(50,50,50), 2);
    }
  }
}
