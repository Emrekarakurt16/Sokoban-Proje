#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TILE_SIZE 40
#define MAP_ROWS 10
#define MAP_COLS 10

// Her iki kutunun da rahatça hareket edebileceği genişletilmiş harita şablonu
int initialMap[MAP_ROWS][MAP_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 4, 0, 1}, // Sağ üstteki hedef (Değişmedi)
    {1, 1, 1, 0, 0, 0, 0, 1, 0, 1}, // Ortadaki sıkışıklığa sebep olan duvarlar temizlendi
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 4, 0, 0, 0, 0, 0, 0, 1}, // Sol alttaki hedef (Değişmedi)
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

int map[MAP_ROWS][MAP_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 2, 0, 0, 0, 3, 0, 0, 1}, // Oyuncu (2,2) ve Üstteki Kutu (2,6)
    {1, 0, 0, 0, 0, 0, 0, 4, 0, 1},
    {1, 1, 1, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 4, 0, 0, 0, 3, 0, 0, 1}, // Alttaki Kutu (7,6) - Önü arkası tamamen açık
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

int playerRow = 2;
int playerCol = 2;
int hasWon = 0;

// Kazanma durumunu kontrol eden fonksiyon
int checkWin() {
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            if (initialMap[r][c] == 4 && map[r][c] != 3) {
                return 0;
            }
        }
    }
    return 1;
}

// Oyuncuyu ve kutuları hareket ettiren ana fonksiyon
void movePlayer(int dRow, int dCol) {
    if (hasWon) return;

    int nextRow = playerRow + dRow;
    int nextCol = playerCol + dCol;

    // Boş yol veya hedefe ilerleme
    if (map[nextRow][nextCol] == 0 || map[nextRow][nextCol] == 4) {
        map[playerRow][playerCol] = (initialMap[playerRow][playerCol] == 4) ? 4 : 0;
        map[nextRow][nextCol] = 2;
        playerRow = nextRow;
        playerCol = nextCol;
    }
    // Kutu itme mantığı
    else if (map[nextRow][nextCol] == 3) {
        int boxNextRow = nextRow + dRow;
        int boxNextCol = nextCol + dCol;

        if (map[boxNextRow][boxNextCol] == 0 || map[boxNextRow][boxNextCol] == 4) {
            map[boxNextRow][boxNextCol] = 3;
            map[playerRow][playerCol] = (initialMap[playerRow][playerCol] == 4) ? 4 : 0;
            map[nextRow][nextCol] = 2;
            playerRow = nextRow;
            playerCol = nextCol;
        }
    }

    if (checkWin()) {
        hasWon = 1;
        printf("TEBRIKLER! Bolumu basariyla tamamladiniz!\n");
    }
}

// Bölümü sıfırlama (Reset) fonksiyonu
void resetLevel() {
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            map[r][c] = initialMap[r][c];
        }
    }
    // Başlangıç konumlarını elle yerleştiriyoruz
    map[2][2] = 2; // Oyuncu
    map[2][6] = 3; // 1. Kutu
    map[7][6] = 3; // 2. Kutu
    playerRow = 2;
    playerCol = 2;
    hasWon = 0;
    printf("Bolum sifirlandi!\n");
}

int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL baslatilamadi! Hata: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Sokoban Projesi - Emre",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        printf("Pencere olusturulamadi! Hata: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer olusturulamadi! Hata: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int isRunning = 1;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                isRunning = 0;
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    // Hareket Tuşları
                    case SDLK_UP:    case SDLK_w: movePlayer(-1, 0); break;
                    case SDLK_DOWN:  case SDLK_s: movePlayer(1, 0);  break;
                    case SDLK_LEFT:  case SDLK_a: movePlayer(0, -1); break;
                    case SDLK_RIGHT: case SDLK_d: movePlayer(0, 1);  break;

                    // Reset Tuşu
                    case SDLK_r:
                        resetLevel();
                        break;
                }
            }
        }

        // Ekran Arka Plan Rengi
        if (hasWon) {
            SDL_SetRenderDrawColor(renderer, 20, 100, 20, 255); // Kazanma Rengi (Koyu Yeşil)
        } else {
            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);  // Normal Renk (Koyu Gri)
        }
        SDL_RenderClear(renderer);

        // Çizim Döngüsü
        for (int r = 0; r < MAP_ROWS; r++) {
            for (int c = 0; c < MAP_COLS; c++) {
                SDL_Rect tile = { c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE };

                // Hedef noktalarını arka planda hep çiz
                if (initialMap[r][c] == 4) {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 100, 255);
                    SDL_Rect targetRect = { c * TILE_SIZE + 15, r * TILE_SIZE + 15, 10, 10 };
                    SDL_RenderFillRect(renderer, &targetRect);
                }

                if (map[r][c] == 1) { // Duvar -> Beyaz/Gri
                    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                    SDL_RenderFillRect(renderer, &tile);
                }
                else if (map[r][c] == 2) { // Oyuncu -> Mavi
                    SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255);
                    SDL_RenderFillRect(renderer, &tile);
                }
                else if (map[r][c] == 3) { // Kutu
                    if (initialMap[r][c] == 4) {
                        SDL_SetRenderDrawColor(renderer, 50, 200, 50, 255); // Hedefteki Kutu -> Parlak Yeşil
                    } else {
                        SDL_SetRenderDrawColor(renderer, 200, 150, 50, 255); // Normal Kutu -> Kahverengi
                    }
                    SDL_RenderFillRect(renderer, &tile);
                }
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
