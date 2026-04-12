#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ALTURA 21
#define LARGURA 21
#define TILE 20

typedef enum { CIMA, BAIXO, ESQUERDA, DIREITA } Direcao;
typedef enum { MENU, JOGO, SAIR } Estado;

typedef struct {
  int x;
  int y;
} Corpo;

typedef struct {
  int x;
  int y;
  int pontuacao;
  int tamanho;
  Corpo *cauda;
} Cabeca;

typedef struct {
  int x;
  int y;
  int pontos;
} Maca;

// typedef struct {
//   int x, y;
//   int largura, altura;
//   ALLEGRO_COLOR cor;
//   char texto[32];
// } Botao;

void inicializar(Cabeca *cobra, Maca *maca);
void desenhar(int **mapa);
void logica(Cabeca *cobra, Maca *maca, Estado *estado, Direcao *dir);
void preencherMapa(Cabeca *cobra, Maca *maca, int **mapa);

int main(void) {
  al_init();                  // Liga o Allegro
  al_install_keyboard();      // Liga o suporte ao teclado
  al_init_primitives_addon(); // Liga o módulo de formas geométricas

  // Define o fps que o jogo roda-ra
  ALLEGRO_TIMER *timer = al_create_timer(1.0 / 10.0); // 10 FPS
  al_start_timer(timer);

  // Cria a janela do jogo (Display)
  ALLEGRO_DISPLAY *display = al_create_display(420, 420);
  srand(time(0));

  // Cria a fila de eventos
  ALLEGRO_EVENT_QUEUE *fila_eventos = al_create_event_queue();
  al_register_event_source(fila_eventos, al_get_keyboard_event_source());
  al_register_event_source(fila_eventos, al_get_timer_event_source(timer));
  al_register_event_source(fila_eventos, al_get_display_event_source(display));
  al_register_event_source(fila_eventos, al_get_mouse_event_source());

  // inicializacao de alguns valores
  int **mapa = (int **)calloc(ALTURA, sizeof(int *));
  for (int i = 0; i < LARGURA; i++) {
    mapa[i] = (int *)calloc(LARGURA, sizeof(int));
  }

  Maca maca;
  Cabeca cobra;
  Direcao ndirecao = 0;
  Estado estado = MENU;

  // Botao b_jogar;
  // b_jogar.x = 3;
  // b_jogar.y = 10;
  // b_jogar.altura = 3;
  // b_jogar.largura = 6;
  // b_jogar.cor = al_map_rgb(100, 150, 0);
  // strco

  inicializar(&cobra, &maca);

  while (estado != SAIR) {
    ALLEGRO_EVENT evento;
    al_wait_for_event(fila_eventos, &evento);

    // if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
    //
    // }

    if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
      if (evento.keyboard.keycode == ALLEGRO_KEY_UP && ndirecao != BAIXO) {
        ndirecao = CIMA;
      } else if (evento.keyboard.keycode == ALLEGRO_KEY_DOWN &&
                 ndirecao != CIMA) {
        ndirecao = BAIXO;
      } else if (evento.keyboard.keycode == ALLEGRO_KEY_LEFT &&
                 ndirecao != DIREITA) {
        ndirecao = ESQUERDA;
      } else if (evento.keyboard.keycode == ALLEGRO_KEY_RIGHT &&
                 ndirecao != ESQUERDA) {
        ndirecao = DIREITA;
      }
    }

    if (evento.type == ALLEGRO_EVENT_TIMER) {
      logica(&cobra, &maca, &estado, &ndirecao);
      preencherMapa(&cobra, &maca, mapa);
      desenhar(mapa);
    }
  }

  for (int i = 0; i < LARGURA; i++) {
    free(mapa[i]);
  }
  free(mapa);
  free(cobra.cauda);

  return 0;
}

void logica(Cabeca *cobra, Maca *maca, Estado *estado, Direcao *dir) {

  // i começa no último gomo (tamanho - 1)
  // i vai até 1 (para o gomo i pegar a posição do i-1)
  for (int i = cobra->tamanho - 1; i > 0; i--) {
    cobra->cauda[i].x = cobra->cauda[i - 1].x;
    cobra->cauda[i].y = cobra->cauda[i - 1].y;
  }

  // Depois que todos "andaram" para a frente, o gomo zero pega a cabeça
  cobra->cauda[0].x = cobra->x;
  cobra->cauda[0].y = cobra->y;

  if (*dir == CIMA) {
    cobra->y -= 1;
  } else if (*dir == BAIXO) {
    cobra->y += 1;
  } else if (*dir == ESQUERDA) {
    cobra->x -= 1;
  } else if (*dir == DIREITA) {
    cobra->x += 1;
  }

  if (cobra->x == maca->x && cobra->y == maca->y) {
    cobra->tamanho += 1;
    cobra->pontuacao += maca->pontos;

    cobra->cauda = realloc(cobra->cauda, cobra->tamanho * sizeof(Corpo));
    cobra->cauda[cobra->tamanho - 1] = cobra->cauda[cobra->tamanho - 2];

    // Altera a posicao da maca
    int visivel;
    do {
      visivel = 1;
      maca->x = rand() % (LARGURA - 2) + 1;
      maca->y = rand() % (ALTURA - 2) + 1;

      // verifica se a maca nao apareceu na cabeça
      if (cobra->x == maca->x && cobra->y == maca->y) {
        visivel = 0;
      }
      // ou dentro do corpo
      for (int i = 0; i < cobra->tamanho; i++) {
        if (cobra->cauda[i].x == maca->x && cobra->cauda[i].y == maca->y) {
          visivel = 0;
        }
      }
    } while (visivel == 0);
  }

  if (cobra->y == 0 || cobra->y == ALTURA - 1 || cobra->x == 0 ||
      cobra->x == LARGURA - 1) {
    *estado = MENU;
  }

  for (int k = 0; k < cobra->tamanho; k++) {
    if (cobra->x == cobra->cauda[k].x && cobra->y == cobra->cauda[k].y) {
      *estado = MENU;
    }
  }
}

void desenhar(int **mapa) {
  al_clear_to_color(al_map_rgb(0, 0, 0)); // Limpa a tela do Allegro

  for (int y = 0; y < ALTURA; y++) {
    for (int x = 0; x < LARGURA; x++) {

      // Coordenadas de pixels baseadas na posição da matriz
      int x1 = x * TILE;
      int y1 = y * TILE;
      int x2 = x1 + TILE;
      int y2 = y1 + TILE;

      if (mapa[y][x] == 1) { // Cabeça
        al_draw_filled_rectangle(x1 + 1, y1 + 1, x2 - 1, y2 - 1,
                                 al_map_rgb(0, 150, 0));
      } else if (mapa[y][x] == 2) { // Cauda
        al_draw_filled_rectangle(x1 + 1, y1 + 1, x2 - 1, y2 - 1,
                                 al_map_rgb(0, 255, 0));
      } else if (mapa[y][x] == 3) { // Maçã
        al_draw_filled_rectangle(x1 + 3, y1 + 3, x2 - 3, y2 - 3,
                                 al_map_rgb(255, 0, 0));
      } else if (mapa[y][x] == 4) { // Parede
        al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(200, 200, 0));
      }
    }
  }
  al_flip_display();
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

      for (int k = 0; k < cobra->tamanho; k++) {
        if (y == cobra->cauda[k].y && x == cobra->cauda[k].x) {
          mapa[y][x] = 2;
          break;
        }
      }

      if (y == cobra->y && x == cobra->x) {
        mapa[y][x] = 1;
      }
    }
  }
}

void inicializar(Cabeca *cobra, Maca *maca) {
  // 1. Configuração da Cabeça
  cobra->x = LARGURA / 2;
  cobra->y = ALTURA / 2;
  cobra->pontuacao = 0;
  cobra->tamanho = 5;

  // 2. Alocação e Posicionamento da Cauda
  cobra->cauda = (Corpo *)malloc(cobra->tamanho * sizeof(Corpo));
  for (int i = 0; i < cobra->tamanho; i++) {
    // A cauda começa esticada para a direita (x+1, x+2...)
    cobra->cauda[i].x = cobra->x + (i + 1);
    cobra->cauda[i].y = cobra->y;
  }

  // 3. Configuração da Maçã (Aleatória)
  // rand() % 18 + 1 garante que a maçã não nasça na parede (0 ou 19)
  maca->x = rand() % (LARGURA - 2) + 1;
  maca->y = rand() % (ALTURA - 2) + 1;
  maca->pontos = 1;
}
