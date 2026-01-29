#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*--------------------------------------
  定数
--------------------------------------*/
#define HEIGHT 40
#define WIDTH  70
#define OVERCROWD 6   /* 周囲セルが6以上なら過密とみなす */


/*--------------------------------------
  関数宣言
--------------------------------------*/
void my_init_cells(const int height, const int width, int cell[height][width], FILE* fp);

void my_print_cells(FILE* fp, int gen, const int height, const int width, int cell[height][width]);

void my_update_cells(const int height, const int width, int cell[height][width]);

int my_count_adjacent_cells(const int height, const int width, int cell[height][width], int y, int x);

void my_save_cells_lif(int gen, const int height, const int width,int cell[height][width]);


/*--------------------------------------
  main
--------------------------------------*/
int main(int argc, char* argv[]) {
    int cell[HEIGHT][WIDTH];
    FILE* fp = NULL;

    if (argc > 2) {
        fprintf(stderr, "usage: %s [filename for init]\n", argv[0]);
        return 1;
    }

    if (argc == 2) {
        fp = fopen(argv[1], "r");
        if (!fp) {
            perror("fopen");
            return 1;
        }
    }

    my_init_cells(HEIGHT, WIDTH, cell, fp);

    if (fp) fclose(fp);

    int gen = 0;

    my_print_cells(stdout, gen, HEIGHT, WIDTH, cell);
    sleep(1);
    printf("\x1b[%dA", HEIGHT + 3);

    while (1) {
        gen++;
        my_update_cells(HEIGHT, WIDTH, cell);
        my_save_cells_lif(gen, HEIGHT, WIDTH, cell);
        my_print_cells(stdout, gen, HEIGHT, WIDTH, cell);
        sleep(1);
        printf("\x1b[%dA", HEIGHT + 3);
    }
}

/*--------------------------------------
  周囲セル数カウント
--------------------------------------*/
int my_count_adjacent_cells(const int height, const int width, int cell[height][width], int y, int x) {

    int count = 0;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dy == 0 && dx == 0) continue;

            int ny = y + dy;
            int nx = x + dx;

            if (ny < 0 || ny >= height) continue;
            if (nx < 0 || nx >= width)  continue;

            count += cell[ny][nx];
        }
    }
    return count;
}

/*--------------------------------------
  初期化
--------------------------------------*/
void my_init_cells(const int height, const int width, int cell[height][width], FILE* fp) {

    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            cell[y][x] = 0;

    // default
    if (fp == NULL) {
        int points[][2] = {
            {20, 30},
            {20, 32},
            {22, 30},
            {22, 31},
            {23, 31}
        };
        int n = sizeof(points) / sizeof(points[0]);
        for (int i = 0; i < n; i++)
            cell[points[i][0]][points[i][1]] = 1;
        return;
    }

    char buf[256];
    int px = 0, py = 0;
    int x, y;

    while (fgets(buf, sizeof(buf), fp)) {

        // 原点指定
        if (sscanf(buf, "#P %d %d", &px, &py) == 2) {
            continue;
        }

        // コメント行は無視
        if (buf[0] == '#') continue;

        // セル座標
        if (sscanf(buf, "%d %d", &x, &y) == 2) {
            int xx = x + px;
            int yy = y + py;

            if (yy >= 0 && yy < height &&
                xx >= 0 && xx < width) {
                cell[yy][xx] = 1;
            }
        }
    }
}
/*--------------------------------------
  表示
--------------------------------------*/
void my_print_cells(FILE* fp, int gen, const int height, const int width, int cell[height][width]) {

    fprintf(fp, "generateion = %d\n", gen);

    fprintf(fp, "+");
    for (int x = 0; x < width; x++) fprintf(fp, "-");
    fprintf(fp, "+\n");

    for (int y = 0; y < height; y++) {
        fprintf(fp, "|");
        for (int x = 0; x < width; x++) {
            if (cell[y][x]) {
                fprintf(fp, "\x1b[31m#\x1b[0m");
            } else {
                fprintf(fp, " ");
            }
        }
        fprintf(fp, "|\n");
    }

    fprintf(fp, "+");
    for (int x = 0; x < width; x++) fprintf(fp, "-");
    fprintf(fp, "+\n");
}

/*--------------------------------------
  世代更新
--------------------------------------*/
void my_update_cells(const int height, const int width, int cell[height][width]) {

    int next[height][width];

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int n = my_count_adjacent_cells(height, width, cell, y, x);
            if (cell[y][x]) {
                /* 過密の場合：爆発して死亡 */
                if (n >= OVERCROWD) {
                    next[y][x] = 0;
                } else {
                    next[y][x] = (n == 2 || n == 3);
                }

            } else {
                next[y][x] = (n == 3);
            }
        }
    }

    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            cell[y][x] = next[y][x];
}
/*--------------------------------------
  Life1.06形式で保存
--------------------------------------*/
void my_save_cells_lif(int gen, const int height, const int width,
                       int cell[height][width]) {

    if (gen >= 10000) return;
    if (gen % 100 != 0) return;

    char filename[32];
    sprintf(filename, "gen%04d.lif", gen);

    FILE *fp = fopen(filename, "w");
    if (!fp) return;

    fprintf(fp, "#Life 1.06\n");

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (cell[y][x]) {
                fprintf(fp, "%d %d\n", x, y);
            }
        }
    }

    fclose(fp);
}