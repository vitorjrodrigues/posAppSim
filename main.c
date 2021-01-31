#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <curses.h>
#include <string.h>
#include "jsmn.h"

typedef struct {
   char major;
   char middle;
   char minor;
} version;
version v = {0, 3, 4};

typedef struct {
    char   identificacao[9];
    char   endereco[128];
    char   cnpj[15];
    char   razaoSocial[128];
    char   rodapeVenda[1024];
    char   rodapeEstorno[1024];
} c_terminal;

typedef struct {
    char   nome[32];
    char   id[2];
    int    tipo;
    char   rotulo[32];
    float  vMin;
    float  vMax;
} c_produto;

void delay(int milissegundos);
void wclrscr(WINDOW* win);
int supports_full_hd(const char * const monitor);
c_terminal readConfTerminal(const char *str);

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

int main()
{
    /* Variáveis Iniciais */
    uint8_t nlines = 7; //Número de Linhas no Terminal Simulado
    uint8_t ncols = 21; //Número de Colunas no Terminal Simulado
    int op;             //Input de Usuário no Menu Principal
    int op2;            //Input de Usuário no Menu Secundário

    /* Variáveis para parsing de JSON */
    c_terminal cfTerminal;
    c_produto  cfProduto;

    /* Carregando Configurações do Terminal salvas em 'terminal.json' */
    cfTerminal = readConfTerminal("terminal.json");

    /* Configurações iniciais do Terminal */
    initscr(); //Inicia a TUI
    noecho();  //Desativa retorno na tela dos caracteres inseridos pelo usuário

    /* Tela Inicial do Simulador */
    printw("POS App Sim v%d.%d.%d\nAperte ENTER p/ cont.", v.major, v.middle, v.minor);
    getch(); //Aguarda inserção de tecla do usuário para iniciar
    refresh();

    /* Cria janela do terminal simulado */
    WINDOW * term = newwin(nlines, ncols, 0, 0);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_YELLOW);
    wbkgd(term, COLOR_PAIR(1));
    keypad(term, TRUE);
    wmove(term,0,0);
    //start_color();

    /* Loop do Simulador */
    while(1)
    {
        werase(term);

        // Menu Principal
        time_t tm;
        tm = time(NULL);
        struct tm ts;
        char tbuf[30];
        ts = *localtime(&tm);
        strftime(tbuf, sizeof(tbuf), "%d/%m %H:%M", &ts);

        wprintw(term, "%c%c%c%c%c%c%c%c  %s",  cfTerminal.identificacao[0],
                                                                cfTerminal.identificacao[1],
                                                                cfTerminal.identificacao[2],
                                                                cfTerminal.identificacao[3],
                                                                cfTerminal.identificacao[4],
                                                                cfTerminal.identificacao[5],
                                                                cfTerminal.identificacao[6],
                                                                cfTerminal.identificacao[7],
                                                                tbuf);
        if(strlen(cfTerminal.razaoSocial)<=21)
        {
            int c = (22-strlen(cfTerminal.razaoSocial))/2;
            wmove(term, 1, c);
            wprintw(term, "%s", cfTerminal.razaoSocial);
        }
        else
        {
            wprintw(term, "%.*s", 21, cfTerminal.razaoSocial);
        }

        wprintw(term, "\n     Tecle ENTER\n     para vender\n\n1-ESTORNO     2-RELAT");
        //wmove(term, nlines - 1,0);
        //wprintw(term,)
        //Menu Principal\nSelecione a Operacao:\n   1 - Venda\n   2 - Administrativa");
        wrefresh(term);
        while(1)
        {
            int k = wgetch(term);
            wmove(term,2,0);
            wprintw(term, "%d",k);
            wrefresh(term);
        }

        wgetch(term);
        wrefresh(term);
        /* Tabela de Caracteres Utilizáveis
         * ENTER - 10   '0' - 48    '1' - 49     '2' - 50
         *   '3' - 51   '4' - 52    '5' - 53     '6' - 54
         *   '7' - 55   '8' - 56    '9' - 57    BKSP - 88




        /*while(1)
        {
            op = wgetch(term);
            wmove(term, 5, 0);
            if(op == '1')
            {
                wprintw(term, "1 - Venda            ");
                wrefresh(term);
                delay(500);
                break;
            }
            else if(op == '2')
            {
                wprintw(term, "2 - Administrativa   ");
                wrefresh(term);
                delay(500);
                break;
            }
            else
            {
                wprintw(term, "ERRO: INVALIDO       ");
                wrefresh(term);
            }
        }

        wclrscr(term);

        if(op == '1')
        {
            wprintw(term, "Menu Venda:\n1 - Credito a Vista\n2 - Credito Parcelado3 - Debito\n0 - Retornar");
            wrefresh(term);
            while(1)
            {
                wmove(term, 5, 0);
                op2 = wgetch(term);
                if(op2 == '1')
                {
                    wprintw(term, "1 - Credito a Vista  ");
                    wrefresh(term);
                    delay(500);
                    break;
                }
                else if(op2 == '2')
                {
                    wprintw(term, "2 - Credito Parcelado");
                    wrefresh(term);
                    delay(500);
                    break;
                }
                else if(op2 == '3')
                {
                    wprintw(term, "3 - Debito           ");
                    wrefresh(term);
                    delay(500);
                    break;
                }
                else if(op2 == '0')
                {
                    wprintw(term, "0 - Retorno          ");
                    wrefresh(term);
                    delay(500);
                    break;
                }
                else
                {
                    wprintw(term, "ERRO: INVALIDO       ");
                    wrefresh(term);
                }
            }

            wclrscr(term);

            if (op2 == '1')
            {
                wprintw(term, "Credito a Vista      ");
                wrefresh(term);
                break;
            }
            else if(op2 == '2')
            {
                wprintw(term, "Credito Parcelado    ");
                wrefresh(term);
            }
            else if(op2 == '3')
            {
                wprintw(term, "3 - Debito           ");
                wrefresh(term);
            }
            else if(op2 == '0')
            {
                wprintw(term, "0 - Retorno          ");
                wrefresh(term);
            }

        }
        else if(op == '2')
        {
            wprintw(term, "Menu Administrativa: ");
            wprintw(term, "1 - Estorno          ");
            wprintw(term, "2 - Relatorio        ");
            wprintw(term, "0 - Retorno          ");
            wrefresh(term);
            while(1)
            {
                wmove(term, 4, 0);
                op2 = wgetch(term);
                if(op2 == '1')
                {
                    wprintw(term, "1 - Estorno          ");
                    wrefresh(term);
                    delay(500);
                    break;
                }
                else if(op2 == '2')
                {
                    wprintw(term, "2 - Relatorio        ");
                    wrefresh(term);
                    delay(500);
                    break;
                }
                else if(op2 == '0')
                {
                    wprintw(term, "0 - Retorno          ");
                    wrefresh(term);
                    delay(500);
                    break;
                }
                else
                {
                    wprintw(term, "ERRO: INVALIDO       ");
                    wrefresh(term);
                }
            }

            wclrscr(term);

            if (op2 == '1')
            {

            }
            else if(op2 == '2')
            {

            }
            else if(op2 == '3')
            {

            }
            else if(op2 == '0')
            {

            }
        }
        //wrefresh(term);
        getch();
        wrefresh(term);
        delay(2000);*/
    }

    //wclrscr(term);
    endwin();

    //return 0;
    printf("THIS IS A TEST!");
    //cJSON *monitor = NULL;
    //int k = supports_full_hd(monitor);
    //printf("\n>>>%d", k);
    while(1);
    return 0;
}

//Atrasa o processo em x milissegundos
void delay(int milissegundos)
{
    clock_t t1 = clock();
    while(clock() < t1 + milissegundos);
}

//Window Clear Screen: Realiza [werase + wrefresh + wmove], limpando os caracteres exibidos...
//                                 ... no terminal e movendo o cursor para o começo da janela.
void wclrscr(WINDOW* win)
{
    werase(win);
    wmove(win, 0, 0);
    wrefresh(win);
}

c_terminal readConfTerminal(const char *str)
{
    FILE *fp;
    char JSON_STRING[2048];
    c_terminal rett;

    //Carregando configurações do terminal (terminal.json)
    fp = fopen(str, "r");
    fread(JSON_STRING, 2048, 1, fp);
    fclose(fp);

    jsmn_parser p;
    jsmntok_t t[128];

    jsmn_init(&p);
    int i;
    int r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, 128);

    if (r < 1 || t[0].type != JSMN_OBJECT) {
        printf("Object expected\n");
    }

    /* Loop over all keys of the root object */
    for (i = 1; i < r; i++) {
        if (jsoneq(JSON_STRING, &t[i], "identificacao") == 0) {
            strncpy(&rett.identificacao, JSON_STRING + t[i + 1].start, t[i + 1].end - t[i + 1].start);
            //printf("- Identificação: %s\n", rett.identificacao);
            i++;
        } else if (jsoneq(JSON_STRING, &t[i], "endereco") == 0) {
            strncpy(&rett.endereco, JSON_STRING + t[i + 1].start, t[i + 1].end - t[i + 1].start);
            //printf("- Endereço: %s\n", rett.endereco);
            i++;
        } else if (jsoneq(JSON_STRING, &t[i], "cnpj") == 0) {
            strncpy(&rett.cnpj, JSON_STRING + t[i + 1].start, t[i + 1].end - t[i + 1].start);
            //printf("- CNPJ: %s\n", rett.cnpj);
            i++;
        } else if (jsoneq(JSON_STRING, &t[i], "razaoSocial") == 0) {
            strncpy(&rett.razaoSocial, JSON_STRING + t[i + 1].start, t[i + 1].end - t[i + 1].start);
            //printf("- Razão Social: %s\n", rett.razaoSocial);
            i++;
        } else if (jsoneq(JSON_STRING, &t[i], "rodapeVenda") == 0) {
            strncpy(&rett.rodapeVenda, JSON_STRING + t[i + 1].start, t[i + 1].end - t[i + 1].start);
            //printf("- Rodapé Venda: %s\n", rett.rodapeVenda);
            i++;
        } else if (jsoneq(JSON_STRING, &t[i], "rodapeEstorno") == 0) {
            strncpy(&rett.rodapeEstorno, JSON_STRING + t[i + 1].start, t[i + 1].end - t[i + 1].start);
            //printf("- Rodapé Estorno: %s\n", rett.rodapeEstorno);
            i++;
        } else {
          //printf("Unexpected key: %.*s\n", t[i].end - t[i].start, JSON_STRING + t[i].start);
        }
    }
    return rett;
}
