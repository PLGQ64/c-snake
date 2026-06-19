#include "jogo.h"
#include "ui.h"
#include "utils.h"

#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Os tipos (Direcao, Estado, Cabeca, Maca, Corpo, Botao) e as constantes
// de tamanho (ALTURA, LARGURA, TILE) vêm de utils.h. A lógica pura do
// jogo (movimento, colisão, maçã) vem de jogo.h, e tudo relacionado a
// desenho/interface/persistência de pontuação vem de ui.h.


int main(void) {
  al_init();                  // Liga o Allegro
  al_install_keyboard();      // Liga o suporte ao teclado
  al_init_primitives_addon(); // Liga o módulo de formas geométricas
  al_init_font_addon();       // Liga o módulo de fontes básicas
  al_init_ttf_addon();        // Liga o suporte a fontes TTF
  al_install_mouse();         // Liga o suporte ao mouse

  ALLEGRO_FONT *fonte = al_load_font("BigBlueTerm437NerdFontMono-Regular.ttf",
                                     20, 0); // Cria a fonte padrão do Allegro
  if (fonte == NULL) {
    fprintf(stderr, "Erro: não foi possível carregar a fonte "
                     "BigBlueTerm437NerdFontMono-Regular.ttf. Verifique se o "
                     "arquivo está na mesma pasta do executável.\n");
    return 1;
  }

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
          liberar_lista(&cobra);
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
  liberar_lista(&cobra);

  return 0;
}
