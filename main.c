#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TILE_SIZE 40
#define MAP_ROWS 10
#define MAP_COLS 10

int map[MAP_ROWS][MAP_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 2, 0, 1, 0, 3, 0, 0, 1}, // 2: Oyuncu
    {1, 0, 0, 0, 0, 0, 0, 4, 0, 1},
    {1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
    {1, 0, 4, 0, 1, 0, 0, 3, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// Oyuncunun mevcut konumunu tutan değişkenler (Matristeki başlangıç yeri: satır 2, sütun 2)
int playerRow = 2;
int playerCol = 2;

// Oyuncuyu hareket ettiren fonksiyon
void movePlayer(int dRow, int dCol) {
    int nextRow = playerRow + dRow;
    int nextCol = playerCol + dCol;

    // Gitmek istediğimiz yer duvar (1) DEĞİLSE hareket et (Kutuları bir sonraki adımda iteceğiz)
    if (map[nextRow][nextCol] != 1) {
        // Eski konumu boş yola (0) çevir
        map[playerRow][playerCol] = 0;

        // Yeni konumu oyuncu (2) yap
        map[nextRow][nextCol] = 2;

        // Oyuncunun konum değişkenlerini güncelle
        playerRow = nextRow;
        playerCol = nextCol;
    }
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
            // Klavye tuşuna basıldığında
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:    // Yukarı Ok Tuşu
                    case SDLK_w:     // Veya W Tuşu
                        movePlayer(-1, 0); // Satırı 1 azalt
                        break;
                    case SDLK_DOWN:  // Aşağı Ok Tuşu
                    case SDLK_s:     // Veya S Tuşu
                        movePlayer(1, 0);  // Satırı 1 artır
                        break;
                    case SDLK_LEFT:  // Sol Ok Tuşu
                    case SDLK_a:     // Veya A Tuşu
                        movePlayer(0, -1); // Sütunu 1 azalt
                        break;
                    case SDLK_RIGHT: // Sağ Ok Tuşu
                    case SDLK_d:     // Veya D Tuşu
                        movePlayer(0, 1);  // Sütunu 1 artır
                        break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        // Haritayı çizdirme
        for (int r = 0; r < MAP_ROWS; r++) {
            for (int c = 0; c < MAP_COLS; c++) {
                SDL_Rect tile = { c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE };

                if (map[r][c] == 1) {
                    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                    SDL_RenderFillRect(renderer, &tile);
                }
                else if (map[r][c] == 2) {
                    SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255);
                    SDL_RenderFillRect(renderer, &tile);
                }
                else if (map[r][c] == 3) {
                    SDL_SetRenderDrawColor(renderer, 200, 150, 50, 255);
                    SDL_RenderFillRect(renderer, &tile);
                }
                else if (map[r][c] == 4) {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 100, 255);
                    SDL_Rect targetRect = { c * TILE_SIZE + 15, r * TILE_SIZE + 15, 10, 10 };
                    SDL_RenderFillRect(renderer, &targetRect);
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
