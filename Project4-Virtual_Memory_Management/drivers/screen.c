#include <screen.h>
#include <pgtable.h>
#define SCREEN_WIDTH    80
#define SCREEN_HEIGHT   50

int screen_cursor_x[NR_CPUS];
int screen_cursor_y[NR_CPUS];

/* screen buffer */
char new_screen[SCREEN_HEIGHT * SCREEN_WIDTH] = {0};
char old_screen[SCREEN_HEIGHT * SCREEN_WIDTH] = {0};

/* cursor position */
void vt100_move_cursor(int x, int y)
{
    // \033[y;xH
    disable_preempt();
    printk("%c[%d;%dH", 27, y, x);
    current_running[cpu_id]->cursor_x = x;
    current_running[cpu_id]->cursor_y = y;
    enable_preempt();
}

/* clear screen */
static void vt100_clear()
{
    // \033[2J
    printk("%c[2J", 27);
}

/* hidden cursor */
static void vt100_hidden_cursor()
{
    // \033[?25l
    printk("%c[?25l", 27);
}

void init_screen(void)
{
    vt100_hidden_cursor();
    vt100_clear();
}

/* scroll screen range(line1, line2) */
void screen_scroll(int line1, int line2)
{
    int i, j;

    for (i = line1 - 1; i < line2; i++){
        for (j = 0; j < SCREEN_WIDTH; j++){
            old_screen[i * SCREEN_WIDTH + j] = 0;
        }
    }

    for (i = line1 - 1; i < line2; i++){
        for (j = 0; j < SCREEN_WIDTH; j++){
            if (i == line2 - 1){
                new_screen[i * SCREEN_WIDTH + j] = ' ';
            }
            else{
                new_screen[i * SCREEN_WIDTH + j] = new_screen[(i + 1) * SCREEN_WIDTH + j];
            }
        }
    }
}

/* clear screen range(line1, line2) */
void screen_clear(int line1, int line2)
{
    int i, j;
    for (i = line1 - 1; i < line2; i++){
        for (j = 0; j < SCREEN_WIDTH; j++){
            new_screen[i * SCREEN_WIDTH + j] = ' ';
        }
    }
    screen_cursor_x[cpu_id] = 1;
    screen_cursor_y[cpu_id] = line1;
    screen_reflush();
}

void screen_move_cursor(int x, int y)
{
    screen_cursor_x[cpu_id] = x;
    screen_cursor_y[cpu_id] = y;
    current_running[cpu_id]->cursor_x = screen_cursor_x[cpu_id];
    current_running[cpu_id]->cursor_y = screen_cursor_y[cpu_id];
}

/* write a char */
static void screen_write_ch(char ch)
{
    if (ch == '\n'){
        screen_cursor_x[cpu_id] = 1;
        screen_cursor_y[cpu_id]++;
    }
    else{
        new_screen[(screen_cursor_y[cpu_id] - 1) * SCREEN_WIDTH + (screen_cursor_x[cpu_id] - 1)] = ch;
        screen_cursor_x[cpu_id]++;
    }
    current_running[cpu_id]->cursor_x = screen_cursor_x[cpu_id];
    current_running[cpu_id]->cursor_y = screen_cursor_y[cpu_id];
}

void screen_write(char *buff)
{
    buff = get_kva_of(buff, current_running[cpu_id]->pgdir);
    int i = 0;
    int l = strlen(buff);

    for (i = 0; i < l; i++){
        screen_write_ch(buff[i]);
    }
}

/*
 * This function is used to print the serial port when the clock
 * interrupt is triggered. However, we need to pay attention to
 * the fact that in order to speed up printing, we only refresh
 * the characters that have been modified since this time.
 */
void screen_reflush(void)
{
    int i, j;

    /* here to reflush screen buffer to serial port */
    for (i = 0; i < SCREEN_HEIGHT; i++){
        for (j = 0; j < SCREEN_WIDTH; j++){
            /* We only print the data of the modified location. */
            if (new_screen[i * SCREEN_WIDTH + j] != old_screen[i * SCREEN_WIDTH + j]){
                vt100_move_cursor(j + 1, i + 1);
                port_write_ch(new_screen[i * SCREEN_WIDTH + j]);
                old_screen[i * SCREEN_WIDTH + j] = new_screen[i * SCREEN_WIDTH + j];
            }
        }
    }

    /* recover cursor position */
    vt100_move_cursor(screen_cursor_x[cpu_id], screen_cursor_y[cpu_id]);
}