#include <stdio.h>
#include <curses.h>

typedef struct {
   char major;
   char middle;
   char minor;
} version;
version v = {0, 1, 0};

int main()
{
    initscr();
    //printf("POS App Sim v%d.%d.%d", v.major, v.middle, v.minor);
    //printf("\nSelecione:\n1 - Venda\n2 - Administrativas");

    printw("POS App Sim v%d.%d.%d", v.major, v.middle, v.minor);
    refresh();
    getch();

    endwin();
    return 0;
}
