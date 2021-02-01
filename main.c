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
    char   nome[21];
    char   id[4];
    int    tipo;
    char   rotulo[21];
    float  vMin;
    float  vMax;
} c_produto;

void delay(int milissegundos);
void wclrscr(WINDOW* win);
int supports_full_hd(const char * const monitor);
c_terminal readConfTerminal(const char *str);
void printReport(time_t time_ref);
c_produto readLstProduto(const char *str, int i);
const char * formatMoney(const char *money);

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
    int key;            //Input de Usuário nas seleções de Menu
    time_t tm;          //Variável para receber o valor de Tempo no formato time_t
    struct tm ts;       //Struct para separar os elementos do valor de tempo (Dia, Mês, Hora, etc.)
    char tbuf[30];      //Array para receber os valores de tempo formatados da maneira desejada

    /* Valores de Venda */
    double valor_da_venda;
    int    parcelas;
    char   svalor_da_venda[16] = {0};
    char   cartao[20] = {0};
    char   sparcelas[21] = {0};
    int    parc;
    int    debt;

    /* Variáveis para seleção do Produto */
    int    prod_id;


    /* Variáveis para parsing de JSON */
    c_terminal cfTerminal = {0};
    c_produto  lstProduto[5];
    int nprodutos = 3;

    /* Carregando Configurações do Terminal salvas em 'terminal.json' */
    cfTerminal = readConfTerminal("terminal.json");

    /* Carregando Configurações dos Produtos salvas em 'produtos.json' */
    for (int x = 0; x < nprodutos; x++)
    {
        lstProduto[x] = readLstProduto("produtos.json", x*12);
    }

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
BEGIN:
        werase(term);

        memset(cartao, 0, sizeof cartao);
        memset(svalor_da_venda, 0, sizeof svalor_da_venda);
        memset(sparcelas, 0, sizeof sparcelas);

        tm = time(NULL);
        ts = *localtime(&tm);
        strftime(tbuf, sizeof(tbuf), "%d/%m %I:%M", &ts);
        //strftime(tbuf2, sizeof(tbuf2), "%Y%m%d%H%M%s", &ts);

        /* Menu Principal */
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

        wmove(term, 3,0);
        wprintw(term, "     Tecle ENTER\n     para vender\n\n1-ESTORNO     2-RELAT");
        wmove(term,6,10);
        wrefresh(term);

        /*       Tabela de Caracteres Utilizáveis         *
         * ENTER - 10   '0' - 48    '1' - 49     '2' - 50 *
         *   '3' - 51   '4' - 52    '5' - 53     '6' - 54 *
         *   '7' - 55   '8' - 56    '9' - 57    BKSP - 08 */

        /* Máquina de Estados do Menu 1 */
        while(1)
        {
            key = wgetch(term);
            wrefresh(term);
            delay(100);

            if(key == 10 || key == 8 || (key > 47 && key < 58)) //Limita entrada somente aos caracteres válidos
            {
                wclrscr(term);

                switch(key)
                {
                    case 10:
                        parc = 0;
                        wprintw(term, "   ESCOLHA A VENDA   ");
                        wprintw(term, "1-CREDITO A VISTA    ");
                        wprintw(term, "2-CREDITO PARCELADO  ");
                        wprintw(term, "3-DEBITO             ");
                        wmove(term, 6, 0);

                        //Máquina de Estados do Menu Vendas
                        while(1)
                        {
                            key = wgetch(term);
                            wrefresh(term);
                            delay(100);

                            if(key == 10 || key == 8 || (key > 47 && key < 58)) //Limita entrada somente aos caracteres válidos
                            {
                                switch(key)
                                {
                                    case 8:
                                        goto BEGIN;
                                        break;
                                    case 49:
                                        wmove(term,1,21);
                                        parc = 0;
                                        debt = 0;
                                        prod_id = 1;
                                        goto VENDA1;
                                        break;
                                    case 50:
                                        wmove(term,2,21);
                                        parc = 1;
                                        debt = 0;
                                        prod_id = 2;
                                        goto VENDA1;
                                        break;
                                    case 51:
                                        wmove(term,3,21);
                                        parc = 0;
                                        debt = 1;
                                        prod_id = 3;
                                        goto VENDA1;
                                        break;
                                    default:
                                        break;
                                }

                            }
                        }

 VENDA1:
                        /* Valor da Venda */
                        wclrscr(term);
                        wprintw(term, " <ROTULO DO PRODUTO> ");
                        wmove(term, 2, 0);
                        wprintw(term, "           VALOR (R$)");
                        wmove(term, 4, 0);
                        wrefresh(term);

                        int idx = 0;

                        while(1)
                        {
                            int q = wgetch(term);
                            wrefresh(term);

                            if(idx < 10)
                            {
                                if(q < 58 && q > 47)
                                {
                                    svalor_da_venda[idx] = q;
                                    wmove(term, 4, 0);
                                    valor_da_venda = strtol(svalor_da_venda,NULL,10);
                                    wprintw(term, "%.2f", valor_da_venda/100);
                                    idx++;
                                }
                            }

                            if (q == 10)
                            {
                                //wprintw(term, "%.2f %.2f",lstProduto[prod_id - 1].vMin,lstProduto[prod_id - 1].vMax);
                                //wrefresh(term);
                                //delay(2000);
                                if(!(lstProduto[prod_id - 1].vMax == 0 || lstProduto[prod_id - 1].vMax == 0))
                                {
                                    if(valor_da_venda/100 < lstProduto[prod_id - 1].vMin)
                                    {
                                        wclrscr(term);
                                        memset(svalor_da_venda, 0, sizeof svalor_da_venda);
                                        valor_da_venda = 0;
                                        idx = 0;
                                        wmove(term,1,0);
                                        wprintw(term, "        ERRO:        ");
                                        wmove(term,3,0);
                                        wprintw(term, "  VALOR MENOR QUE O  ");
                                        wprintw(term, "      MINIMO DE      ");
                                        char zbuf[16];
                                        sprintf(zbuf, "%.f", lstProduto[prod_id -1].vMin);
                                        int l = (15 - strlen(zbuf))/2;
                                        wmove(term, 5, l);
                                        wprintw(term, "R$ %.2f  ", lstProduto[prod_id -1].vMin);
                                        wrefresh(term);
                                        wgetch(term);
                                        wrefresh(term);
                                        goto VENDA1;
                                    }

                                    /*else if(valor_da_venda/100 > lstProduto[prod_id - 1].vMax)
                                    {
                                        wclrscr(term);
                                        memset(svalor_da_venda, 0, sizeof svalor_da_venda);
                                        valor_da_venda = 0;
                                        idx = 0;
                                        wmove(term,1,0);
                                        wprintw(term, "        ERRO:        ");
                                        wmove(term,3,0);
                                        wprintw(term, "  VALOR MAIOR QUE O  ");
                                        wprintw(term, "      MAXIMO DE      ");
                                        char zbuf[16];
                                        sprintf(zbuf, "%.f", lstProduto[prod_id -1].vMax);
                                        int l = (15 - strlen(zbuf))/2;
                                        wmove(term, 5, l);
                                        wprintw(term, "R$ %.2f  ", lstProduto[prod_id -1].vMax);
                                        wrefresh(term);
                                        wgetch(term);
                                        wrefresh(term);
                                        goto VENDA1;
                                    }*/
                                }
                                if(parc == 0)
                                {
                                    goto CARD;
                                }
                                else
                                {
                                    break;
                                }
                            }
                            else if(q == 8)
                            {
                                goto BEGIN;
                            }
                        }

VENDA2:
                        /* Número de Parcelas */
                        wclrscr(term);
                        wprintw(term, " <ROTULO DO PRODUTO> ");
                        wmove(term, 2, 0);
                        wprintw(term, "             PARCELAS");
                        wmove(term, 4, 0);
                        wrefresh(term);

                        idx = 0;

                        while(1)
                        {
                            int q = wgetch(term);
                            wrefresh(term);
                            if(q < 58 && q > 47)
                            {
                                sparcelas[idx] = q;
                                sparcelas[idx+1] = 'x';
                                wmove(term, 4, 0);
                                wprintw(term, "%s", sparcelas);
                                idx++;
                            }
                            else if(q == 10)
                            {
                                if(strlen(sparcelas)!=0)
                                {
                                    if(strcmp(sparcelas, "0x"))
                                    {
                                       break;
                                    }
                                    else
                                    {
                                       wclrscr(term);
                                       memset(sparcelas, 0, sizeof sparcelas);
                                       idx = 0;
                                       wmove(term,1,0);
                                       wprintw(term, "        ERRO:        ");
                                       wmove(term,3,0);
                                       wprintw(term, "   NUMERO INCORRETO  ");
                                       wprintw(term, "     DE PARCELAS     ");
                                       wrefresh(term);
                                       wgetch(term);
                                       wrefresh(term);
                                       goto VENDA2;
                                    }

                                }

                            }
                            else if(q == 8)
                            {
                                goto BEGIN;
                            }
                        }

 CARD:
                        /* Número do Cartão */
                        wclrscr(term);
                        wprintw(term, " <ROTULO DO PRODUTO> ");
                        wmove(term, 2, 0);
                        wprintw(term, "               CARTAO");
                        wmove(term, 4, 0);
                        wrefresh(term);

                        idx = 0;

                        while(1)
                        {
                            int q = wgetch(term);
                            wrefresh(term);

                            if(idx < 19)
                            {
                                if(q < 58 && q > 47)
                                {
                                    cartao[idx] = q;
                                    wmove(term, 4, 0);
                                    wprintw(term, "%s", cartao);
                                    idx++;
                                }
                            }
                            if(q == 10)
                            {
                                if(strlen(cartao)<11)
                                {
                                      wclrscr(term);
                                      memset(sparcelas, 0, sizeof sparcelas);
                                      idx = 0;
                                      wmove(term,1,0);
                                      wprintw(term, "        ERRO:        ");
                                      wmove(term,3,0);
                                      wprintw(term, "   CARTAO INVALIDO   ");
                                      wrefresh(term);
                                      wgetch(term);
                                      wrefresh(term);
                                      goto CARD;
                                }
                                else
                                {
                                    break;
                                }
                            }
                            else if(q == 8)
                            {
                                goto BEGIN;
                            }

                        }

CVEND:
                        /*Confirmação de Venda */
                        wclrscr(term);
                        wprintw(term, " <ROTULO DO PRODUTO> ");
                        wprintw(term, "   Confirma Venda?   ");
                        int w = (21 - strlen(cartao))/2;
                        wmove(term, 3, w);
                        wprintw(term, "%c%c%c%c", cartao[0],
                                                  cartao[1],
                                                  cartao[2],
                                                  cartao[3]);
                        int e = strlen(cartao) - 8;
                        for(int p = 0; p < e; p++)
                        {
                            waddch(term, '*');
                        }
                        wprintw(term, "%c%c%c%c", cartao[strlen(cartao)-4],
                                                  cartao[strlen(cartao)-3],
                                                  cartao[strlen(cartao)-2],
                                                  cartao[strlen(cartao)-1]);
                        wmove(term,4,0);
                        wprintw(term,"R$");
                        int z = 17 - strlen(svalor_da_venda);
                        wmove(term, 4, z);
                        wprintw(term,"%.2f", valor_da_venda/100);
                        wmove(term,6,0);
                        wprintw(term, "NAO               SIM");
                        wmove(term,6,10);
                        wrefresh(term);
                        while(1)
                        {
                            int q = wgetch(term);
                            wrefresh(term);
                            if (q == 10)
                            {
                                break;
                            }
                            else if(q == 8)
                            {
                                wclrscr(term);
                                memset(sparcelas, 0, sizeof sparcelas);
                                idx = 0;
                                wmove(term,1,0);
                                wprintw(term, "        ERRO:        ");
                                wmove(term,3,0);
                                wprintw(term, " OPERACAO CANCELADA  ");
                                wrefresh(term);
                                wgetch(term);
                                wrefresh(term);
                                goto BEGIN;
                            }
                        }

                        /* Salvar Venda no Repositório de Vendas */

                        //Função de Salvar Venda
                        while(1)
                        {
                            wclrscr(term);
                            memset(sparcelas, 0, sizeof sparcelas);
                            idx = 0;
                            wmove(term,3,0);
                            wprintw(term, "   VENDA CONCLUIDA   ");
                            wmove(term,6,10);
                            wrefresh(term);
                            delay(2000);
                            goto BEGIN;
                        }

                        break;
                    case 49:
                        wprintw(term, "       ESTORNO       ");
                        wprintw(term, "1-HH/mm R$ 999.999,99");
                        wprintw(term, "2-HH/mm R$ 999.999,99");
                        wprintw(term, "3-HH/mm R$ 999.999,99");
                        wprintw(term, "4-HH/mm R$ 999.999,99");
                        wprintw(term, "5-HH/mm R$ 999.999,99");

                        //Máquina de Estados do Menu Estorno
                        while(1)
                        {
                            key = wgetch(term);
                            wrefresh(term);
                            delay(100);

                            if(key == 10 || key == 8 || (key > 47 && key < 58)) //Limita entrada somente aos caracteres válidos
                            {
                                switch(key)
                                {
                                    case 8:
                                        goto BEGIN;
                                        break;
                                    case 49:
                                        wmove(term,1,21);
                                        break;
                                    case 50:
                                        wmove(term,2,21);
                                        break;
                                    case 51:
                                        wmove(term,3,21);
                                        break;
                                    case 52:
                                        wmove(term,4,21);
                                        break;
                                    case 53:
                                        wmove(term,5,21);
                                        break;
                                    default:
                                        break;
                                }

                            }
                        }

                        /*Confirmação de Estorno */
                        wclrscr(term);
                        wprintw(term, " <ROTULO DO PRODUTO> ");
                        wprintw(term, "  Confirma Estorno?  ");
                        wmove(term, 3, 0);
                        wprintw(term, "  1234********3456   ");
                        wprintw(term, "RS   9.999.999.999,99");
                        wmove(term,6,0);
                        wprintw(term, "NAO               SIM");
                        wmove(term,6,10);
                        wrefresh(term);
                        while(1)
                        {
                            int q = wgetch(term);
                            wrefresh(term);
                            if (q == 10)
                            {
                                //CONFIRMA
                            }
                            else if(q == 8)
                            {
                                goto BEGIN;
                            }
                        }
                        break;
                    case 50:
                        wprintw(term, "      RELATORIO      ");
                        wprintw(term, "                     ");
                        wprintw(term, "     Tecle ENTER     ");
                        wprintw(term, "    para imprimir    ");
                        wprintw(term, "\n\n");
                        wprintw(term, "<-VOLTAR             ");

                        //Máquina de Estados do Menu Relatório
                        while(1)
                        {
                            key = wgetch(term);
                            wrefresh(term);
                            delay(100);

                            if(key == 10 || key == 8 || (key > 47 && key < 58)) //Limita entrada somente aos caracteres válidos
                            {
                                switch(key)
                                {
                                    case 8:
                                        goto BEGIN;
                                        break;
                                    case 10:
                                        wmove(term,2,0);
                                        wprintw(term, "                                          ");
                                        wmove(term,2,0);
                                        wprintw(term, "    IMPRIMINDO...    ");
                                        wrefresh(term);
                                        /* Interrompe o modo TUI para utilizar o FILE-C */
                                        endwin();
                                        /* Função de Impressão de relatório */
                                        printReport(tm);  //FUNCAO DE IMPRESSAO
                                        /* Retoma o modo TUI de onde parou */
                                        reset_prog_mode();
                                        wmove(term,2,0);
                                        wrefresh(term);
                                        wprintw(term, "    IMPRESSAO        ");
                                        wprintw(term, "        CONCLUIDA    ");
                                        wrefresh(term);
                                        break;
                                    default:
                                        break;
                                }

                            }
                        }
                        break;
                    case 8:
                        goto BEGIN;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    //wclrscr(term);
    endwin();
    delscreen(term);

    //printf("THIS IS A TEST!");
    //while(1);
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

    strncpy(&rett.identificacao, JSON_STRING + t[4].start, t[4].end - t[4].start);
    //printf("- Identificação: %s\n", rett.identificacao);
    strncpy(&rett.endereco, JSON_STRING + t[6].start, t[6].end - t[6].start);
    //printf("- Endereço: %s\n", rett.endereco);
    strncpy(&rett.cnpj, JSON_STRING + t[8].start, t[8].end - t[8].start);
    //printf("- CNPJ: %s\n", rett.cnpj);
    strncpy(&rett.razaoSocial, JSON_STRING + t[10].start, t[10].end - t[10].start);
    //printf("- Razão Social: %s\n", rett.razaoSocial);
    strncpy(&rett.rodapeVenda, JSON_STRING + t[12].start, t[12].end - t[12].start);
    //printf("- Rodapé Venda: %s\n", rett.rodapeVenda);
    strncpy(&rett.rodapeEstorno, JSON_STRING + t[14].start, t[14].end - t[14].start);
    //printf("- Rodapé Estorno: %s\n", rett.rodapeEstorno);

    return rett;
}

void printReport(time_t time_ref)
{
    FILE * fp;
    int i;
    /* open the file for writing*/
    char buf[32];
    clock_t t1 = clock();
    time_t t = time(NULL);
    sprintf(buf, "/reports/report%lu.txt", t);
    fp = fopen (buf,"w");

    /* write 10 lines of text into the file stream*/
    for(i = 0; i < 10;i++){
            fprintf (fp, "%s\n",buf);
        }
    fclose (fp);
}

c_produto readLstProduto(const char *str, int i)
{
    FILE *fp;
    char JSON_STRING[2048];
    c_produto rett;

    //Carregando configurações dos produtos financeiros (produto.json)
    fp = fopen(str, "r");
    fread(JSON_STRING, 2048, 1, fp);
    fclose(fp);

    jsmn_parser p;
    jsmntok_t t[128];

    jsmn_init(&p);
    int r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, 128);

    strncpy(&rett.nome, JSON_STRING + t[3+i].start, t[3+i].end - t[3+i].start);
    //printf("- Nome: %s\n", rett.nome);
    strncpy(&rett.id, JSON_STRING + t[6+i].start, t[6+i].end - t[6+i].start);
    //printf("- ID: %s\n", rett.id);
    //char sbuf[32];
    //strncpy(sbuf, JSON_STRING + t[8+i].start, t[8+i].end - t[8+i].start);
    //rett.tipo = strtol(sbuf,NULL,10);
    //printf("- Tipo: %d\n", rett.tipo);
    strncpy(&rett.rotulo, JSON_STRING + t[10+i].start, t[10+i].end - t[10+i].start);
    //printf("- Rotulo: %s\n", rett.rotulo);
    //memcpy(sbuf, 0, sizeof sbuf);
    //strncpy(sbuf, JSON_STRING + t[12+i].start, t[12+i].end - t[12+i].start);
    //rett.vMin = strtod(sbuf,NULL);
    //printf("- vMin: %.2f\n", rett.vMin);
    //memcpy(sbuf, 0, sizeof sbuf);
    //strncpy(sbuf, JSON_STRING + t[14+i].start, t[14+i].end - t[14+i].start);
    //rett.vMax = strtod(sbuf,NULL);
    //printf("- vMax: %.2f\n", rett.vMax);

    //return rett;
}

const char * formatMoney(const char * money)
{
    char buf[16];
    if(strlen(money)==1)
    {
        sprintf(buf, "0,0%s",money);
    }
    else if(strlen(money)==2)
    {
        sprintf(buf, "0,%s",money);
    }
    else
    {
        sprintf(buf, "0,00",money);
    }
    //
    return buf;
}
