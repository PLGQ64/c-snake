#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/color.h>
#include <allegro5/drawing.h>
#include <allegro5/events.h>
#include <allegro5/keycodes.h>
#include <allegro5/timer.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Definições de tamanho do tabuleiro e dos blocos (pixels)
#define ALTURA 24
#define LARGURA 24
#define TILE 40

// Enums para organizar as direções e as telas (estados) do jogo
typedef enum { CIMA, BAIXO, ESQUERDA, DIREITA } Direcao;
typedef enum { MENU, JOGO, SAIR } Estado;

// Struct para cada segmento (gomo) do corpo da cobra
typedef struct {
  int x;
  int y;
} Corpo;

// Struct que controla a cabeça, pontuação e o ponteiro para o corpo
typedef struct {
  int x;
  int y;
  int pontuacao;
  int tamanho;
  Corpo *cauda;
} Cabeca;

// Struct para a posição e valor de pontos da maçã
typedef struct {
  int x;
  int y;
  int pontos;
} Maca;

// Struct para renderização e detecção de cliques em botões
typedef struct {
  int x, y;
  int largura, altura;
  ALLEGRO_COLOR cor;
  char texto[32];
} Botao;

// Funções do jogo
void inicializar(Cabeca *cobra, Maca *maca, Direcao *dir);
void desenhar(int **mapa);
void logica(Cabeca *cobra, Maca *maca, Estado *estado, Direcao *dir);
void preencherMapa(Cabeca *cobra, Maca *maca, int **mapa);
void desenharBotao(Botao *botao, ALLEGRO_FONT *fonte);
bool clickBotao(Botao botao, int mouse_x, int mouse_y);
void desenharPontuacao(Cabeca *c, ALLEGRO_FONT *fonte);
void ordenar_pontuacao(int nova_pontuacao, int *board);
void ler_pontuacao(int *board);
void salvar_pontuacao(int *board);
void apagar_pontuacao(int p, int *board);
void desenhar_scoreboard(int *board, ALLEGRO_FONT *font);
void grid();


int main(void) {
  al_init();                  // Liga o Allegro
  al_install_keyboard();      // Liga o suporte ao teclado
  al_init_primitives_addon(); // Liga o módulo de formas geométricas
  al_init_font_addon();       // Liga o módulo de fontes básicas
  al_init_ttf_addon();        // Liga o suporte a fontes TTF
  al_install_mouse();         // Liga o suporte ao mouse

  ALLEGRO_FONT *fonte = al_load_font("BigBlueTerm437NerdFontMono-Regular.ttf",
                                     20, 0); // Cria a fonte padrão do Allegro

  // Define o fps que o jogo roda-ra (10 fps)
  ALLEGRO_TIMER *timer = al_create_timer(1.0 / 10.0);
  al_start_timer(timer);

  // Cria a janela do jogo (Display)
  ALLEGRO_DISPLAY *display = al_create_display(LARGURA * TILE, ALTURA * TILE);
  srand(time(0));

  // Cria a fila de eventos
  ALLEGRO_EVENT_QUEUE *fila_eventos = al_create_event_queue();
  // de onde os eventos da lista vem ou o que ela deve escutar
  al_register_event_source(fila_eventos, al_get_keyboard_event_source()); // teclado
  al_register_event_source(fila_eventos, al_get_timer_event_source(timer)); // timer
  al_register_event_source(fila_eventos, al_get_display_event_source(display)); // monitor
  al_register_event_source(fila_eventos, al_get_mouse_event_source()); // mouse

  // Alocação dinâmica da matriz bidimensional do mapa
  int **mapa = (int **)calloc(ALTURA, sizeof(int *));
  for (int i = 0; i < ALTURA; i++) {
    mapa[i] = (int *)calloc(LARGURA, sizeof(int));
  }

  // Declaração das variáveis de estado e controle do jogo
  Maca maca;
  Cabeca cobra;
  Direcao ndirecao = ESQUERDA;
  Estado estado = MENU;
  bool flag_waitlogic = false; // Evita que o jogador mude de direção duas vezes no mesmo frame
  bool flag_inicializada = false;
  bool flag_salvar = false;
  bool flag_apagar_pontuacao = false;
  int board[6] = {0, 0, 0, 0, 0, 0};

  // Configuração visual e de posicionamento do Botão "Jogar"
  Botao b_jogar;
  b_jogar.x = 3;
  b_jogar.y = 4;
  b_jogar.altura = 3;
  b_jogar.largura = 6;
  b_jogar.cor = al_map_rgb(100, 150, 0);
  strcpy(b_jogar.texto, "Jogar");

  // Configuração visual e de posicionamento do Botão "Sair"
  Botao b_sair;
  b_sair.x = 3;
  b_sair.y = 8;
  b_sair.altura = 3;
  b_sair.largura = 6;
  b_sair.cor = al_map_rgb(120, 0, 0);
  strcpy(b_sair.texto, "Sair");

  // Configuração visual e de posicionamento do Botão "Apagar Pontuação"
  Botao b_apagar_pontuacao;
  b_apagar_pontuacao.x = 3;
  b_apagar_pontuacao.y = 12;
  b_apagar_pontuacao.altura = 3;
  b_apagar_pontuacao.largura = 6;
  b_apagar_pontuacao.cor = al_map_rgb(180, 180, 0);
  strcpy(b_apagar_pontuacao.texto, "Apagar pontuacao");

  // bool draw = true;

  // Reset inicial dos dados da cobra e maçã
  inicializar(&cobra, &maca, &ndirecao);
  ler_pontuacao(board);

  int pos = 0;
  // Loop principal do jogo (roda enquanto o estado não for SAIR)
  while (estado != SAIR) {
    // para aonde os eventos vao ser enviados
    ALLEGRO_EVENT evento;
    al_wait_for_event(fila_eventos, &evento); // Aguarda acontecer alguma ação

    // verifica se a pessoa apertou dentro do botao
    if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
      int mx = evento.mouse.x;
      int my = evento.mouse.y;
      if (clickBotao(b_jogar, mx, my)) {
        estado = JOGO;
      } else if (clickBotao(b_sair, mx, my)) {
        estado = SAIR;
      } else if (clickBotao(b_apagar_pontuacao, mx, my)){
        flag_apagar_pontuacao = true;
      }
    }

    // Captura as teclas direcionais, impedindo a cobra de voltar para trás
    // diretamente
    if (evento.type == ALLEGRO_EVENT_KEY_DOWN && flag_waitlogic == true) {
      if (evento.keyboard.keycode == ALLEGRO_KEY_UP && ndirecao != BAIXO) {
        ndirecao = CIMA;
      } else if (evento.keyboard.keycode == ALLEGRO_KEY_DOWN && ndirecao != CIMA) {
        ndirecao = BAIXO;
      } else if (evento.keyboard.keycode == ALLEGRO_KEY_LEFT && ndirecao != DIREITA) {
        ndirecao = ESQUERDA;
      } else if (evento.keyboard.keycode == ALLEGRO_KEY_RIGHT && ndirecao != ESQUERDA) {
        ndirecao = DIREITA;
      }
      flag_waitlogic = false; // Bloqueia novos inputs até o próximo ciclo do timer
    }

    // recebe o input do teclado e atualiza a variavel pos
    if (flag_apagar_pontuacao == true &&
        evento.type == ALLEGRO_EVENT_KEY_DOWN) {
      if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
        flag_apagar_pontuacao = false;
      } else if (evento.keyboard.keycode == ALLEGRO_KEY_UP && pos > 0) {
        pos--;
      } else if (evento.keyboard.keycode == ALLEGRO_KEY_DOWN && pos < 5) {
        pos++;
      } else if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER) {
        apagar_pontuacao(pos, board);
      }
    }

    // Atualização física e renderização (acionado a cada ciclo do Timer)
    if (evento.type == ALLEGRO_EVENT_TIMER) {
      if (estado == JOGO) {

        // Se entrou no jogo agora, inicializa os objetos
        if (flag_inicializada == false) {
          inicializar(&cobra, &maca, &ndirecao);
          flag_inicializada = true;
        }
        logica(&cobra, &maca, &estado, &ndirecao); // Processa colisões e movimentos
        preencherMapa(&cobra, &maca, mapa);       // Atualiza a matriz do mapa
        al_clear_to_color(al_map_rgb(0, 0, 0));  // pinta a tela toda de preto
        grid();                                 // desenha a grade no fundo preto
        desenhar(mapa);                        // Desenha o mapa na tela
        desenharPontuacao(&cobra, fonte);     // Desenha o placar
        al_flip_display();                   // Atualiza a tela do monitor
        flag_waitlogic = true;              // Libera a leitura de novos inputs
        flag_salvar = true;

      } else if (estado == MENU){
        if (flag_salvar == true) {
          ordenar_pontuacao(cobra.pontuacao, board);
          salvar_pontuacao(board);
          flag_salvar = false;
        }

        // Renderização do Menu Principal
        al_clear_to_color(al_map_rgb(0, 0, 0));
        grid();
        desenharBotao(&b_jogar, fonte);
        desenharBotao(&b_sair, fonte);
        desenharBotao(&b_apagar_pontuacao, fonte);
        desenhar_scoreboard(board, fonte);

        // desenha o quadradinho amarelo quando selecionando a pontuacao para
        // apgar
        if (flag_apagar_pontuacao == true) {
          al_draw_rectangle(11 * TILE, (4 + pos) * TILE, 13 * TILE,
                            (5 + pos) * TILE, al_map_rgb(180, 180, 0), 5);
        }

        al_flip_display();
        flag_inicializada = false; // Garante reinicialização das variaveis ao voltar pro jogo
        flag_salvar = false;
      }
    }
  }


  // Liberação da memória alocada dinamicamente antes de fechar o programa
  for (int i = 0; i < ALTURA; i++) {
    free(mapa[i]);
  }
  free(mapa);
  free(cobra.cauda);

  return 0;
}

// Carrega/le o placar do arquivo binário
void ler_pontuacao(int *board) {
  FILE *file = fopen("pontuacao.bin", "rb");
  if (file == NULL) {
    file = fopen("pontuacao.bin", "wb");
    fwrite(board, sizeof(int), 6, file);
    fclose(file);
    file = fopen("pontuacao.bin", "rb");
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

// Processa toda a física do jogo: movimento, crescimento e colisões
void logica(Cabeca *cobra, Maca *maca, Estado *estado, Direcao *dir) {

  // Move os gomos do corpo para a posição do gomo anterior (efeito dominó)
  // i começa no último gomo (tamanho - 1)
  // i vai até 1 (para o gomo i pegar a posição do i-1)
  for (int i = cobra->tamanho - 1; i > 0; i--) {
    cobra->cauda[i].x = cobra->cauda[i - 1].x;
    cobra->cauda[i].y = cobra->cauda[i - 1].y;
  }

  // Depois que todos os gomos "andaram" para a frente, o gomo zero pega a mesma
  // posicao da cabeça
  cobra->cauda[0].x = cobra->x;
  cobra->cauda[0].y = cobra->y;

  // move a cabeça
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
    // aumenta o tamanho
    cobra->tamanho += 1;
    // aumenta o pontuacao
    cobra->pontuacao += maca->pontos;

    // aumenta o tamanho do corpo da cobra
    cobra->cauda = realloc(cobra->cauda, cobra->tamanho * sizeof(Corpo));
    // inicializa o novo gomo com a mesma posicao do ultimo gomo
    cobra->cauda[cobra->tamanho - 1] = cobra->cauda[cobra->tamanho - 2];

    // Sorteia nova posição para a maçã garantindo que não nasça em cima da
    // cobra
    bool visivel;
    do {
      visivel = true;
      maca->x = rand() % (LARGURA - 2) + 1;
      maca->y = rand() % (ALTURA - 2) + 1;

      // verifica se a maca nao apareceu na cabeça
      if (cobra->x == maca->x && cobra->y == maca->y) {
        visivel = false;
        continue;
      }
      // ou dentro do corpo
      for (int i = false; i < cobra->tamanho; i++) {
        if (cobra->cauda[i].x == maca->x && cobra->cauda[i].y == maca->y) {
          visivel = false;
          continue;
        }
      }
    } while (visivel == false);
  }

  // verifica se a cobra bateu em uma parede
  if (cobra->y == 0 || cobra->y == ALTURA - 1 || cobra->x == 0 ||
      cobra->x == LARGURA - 1) {
    *estado = MENU;
  }

  // verifica se a cobra bateu no proprio corpo
  for (int k = 0; k < cobra->tamanho; k++) {
    if (cobra->x == cobra->cauda[k].x && cobra->y == cobra->cauda[k].y) {
      *estado = MENU;
    }
  }

  if (cobra->tamanho == 483) {
    *estado = MENU;
  }
}

// Desenha o placar atual de pontos no canto superior esquerdo da tela
void desenharPontuacao(Cabeca *c, ALLEGRO_FONT *fonte) {
  al_draw_textf(fonte, al_map_rgb(255, 255, 255), (1 * TILE) / 2 - 5,
                (1 * TILE) / 2 - 5, 0, "%d", c->pontuacao);
}

// Função matemática que checa se as coordenadas do mouse estão dentro do
// retângulo do botão
bool clickBotao(Botao botao, int mouse_x, int mouse_y) {
  if (mouse_x >= botao.x * TILE &&
      mouse_x <= (botao.x * TILE + botao.largura * TILE) &&
      mouse_y >= botao.y * TILE &&
      mouse_y <= (botao.y * TILE + botao.altura * TILE)) {
    return true; // Sucesso!
  }
  return false; // Clicou fora do botão
}
// Renderiza o retângulo do botão e centraliza o texto dentro dele
void desenharBotao(Botao *botao, ALLEGRO_FONT *fonte) {
  al_draw_filled_rectangle(botao->x * TILE, botao->y * TILE,
                           botao->x * TILE + botao->largura * TILE,
                           botao->y * TILE + botao->altura * TILE, botao->cor);
  al_draw_text(fonte, al_map_rgb(255, 255, 255),
               botao->x * TILE + (botao->largura * TILE / 2),
               botao->y * TILE + (botao->altura * TILE / 2),
               ALLEGRO_ALIGN_CENTRE, botao->texto);
}

// Varre a matriz 'mapa' e renderiza os gráficos adequados para cada ID (Cobra,
// Maçã, Parede)
void desenhar(int **mapa) {
  for (int y = 0; y < ALTURA; y++) {
    for (int x = 0; x < LARGURA; x++) {

      // Coordenadas de pixels baseadas na posição da matriz
      int x1 = x * TILE;
      int y1 = y * TILE;
      int x2 = x1 + TILE;
      int y2 = y1 + TILE;

      // deixa um espaco para separar a cabeca e o da maca deixa ela menor
      int espacamento_gomo = (TILE * 31 / 32);
      int espacamento_maca = (TILE * 8 / 32);

      if (mapa[y][x] == 1) { // Desenha Cabeça (Verde Escuro)
        al_draw_filled_rectangle(x1 + espacamento_gomo, y1 + espacamento_gomo,
                                 x2 - espacamento_gomo, y2 - espacamento_gomo,
                                 al_map_rgb(0, 150, 0));
        al_draw_rectangle(x1 + espacamento_gomo - 2, y1 + espacamento_gomo - 2,
                          x2 - espacamento_gomo + 2, y2 - espacamento_gomo + 2,
                          al_map_rgb(0, 100, 0), 4);
      } else if (mapa[y][x] == 2) { // Desenha Cauda/Corpo (Verde Claro)
        al_draw_filled_rectangle(x1 + espacamento_gomo, y1 + espacamento_gomo,
                                 x2 - espacamento_gomo, y2 - espacamento_gomo,
                                 al_map_rgb(0, 255, 0));
        al_draw_rectangle(x1 + espacamento_gomo - 2, y1 + espacamento_gomo - 2,
                          x2 - espacamento_gomo + 2, y2 - espacamento_gomo + 2,
                          al_map_rgb(0, 150, 0), 4);
      } else if (mapa[y][x] == 3) { // Desenha Maçã (Vermelho)
        al_draw_filled_rectangle(x1 + espacamento_maca, y1 + espacamento_maca,
                                 x2 - espacamento_maca, y2 - espacamento_maca,
                                 al_map_rgb(255, 0, 0));
        al_draw_rectangle(x1 + espacamento_maca - 2, y1 + espacamento_maca - 2,
                          x2 - espacamento_maca + 2, y2 - espacamento_maca + 2,
                          al_map_rgb(130, 0, 0), 4);
      } else if (mapa[y][x] == 4) { // Desenha Parede/Borda (Amarelo)
        al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(200, 200, 0));
      }
    }
  }
}

// Atualiza os IDs da matriz do mapa baseado nas posições atuais da Cobra, Maçã
// e das Paredes
void preencherMapa(Cabeca *cobra, Maca *maca, int **mapa) {
  for (int y = 0; y < ALTURA; y++) {
    for (int x = 0; x < LARGURA; x++) {

      // Define ID 4 se for borda da matriz, senão limpa com 0 (vazio)
      if (y == 0 || y == ALTURA - 1 || x == 0 || x == LARGURA - 1) {
        mapa[y][x] = 4;
      } else {
        mapa[y][x] = 0;
      }

      // Aplica ID 3 na posição da Maçã
      if (y == maca->y && x == maca->x) {
        mapa[y][x] = 3;
      }

      // Aplica ID 2 em cada posição que contém parte do corpo da cobra
      for (int k = 0; k < cobra->tamanho; k++) {
        if (y == cobra->cauda[k].y && x == cobra->cauda[k].x) {
          mapa[y][x] = 2;
          break;
        }
      }

      // Aplica ID 1 na posição exata da Cabeça (sobrepõe o corpo se necessário)
      if (y == cobra->y && x == cobra->x) {
        mapa[y][x] = 1;
      }
    }
  }
}
// Configura o estado inicial padrão de jogo ao iniciar ou reiniciar a partida
void inicializar(Cabeca *cobra, Maca *maca, Direcao *dir) {
  // 1. Configuração da Cabeça
  *dir = ESQUERDA;
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

// desenhar grade no fundo preto do jogo
void grid() {
  for (int y = 0; y < ALTURA; y++) {
    for (int x = 0; x < LARGURA; x++) {
      // Coordenadas de pixels baseadas na posição da matriz
      int x1 = x * TILE;
      int y1 = y * TILE;
      int x2 = x1 + TILE;
      int y2 = y1 + TILE;
      al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(50,50,50), 2);
    }
  }
}
