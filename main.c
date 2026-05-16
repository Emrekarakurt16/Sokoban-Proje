#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TILE_SIZE 40
#define MAP_ROWS 10
#define MAP_COLS 10

// Orijinal harita şablonu
int initialMap[MAP_ROWS][MAP_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 4, 0, 1},
    {1, 1, 1, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 4, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// Anlık oyun haritası
int map[MAP_ROWS][MAP_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 2, 0, 0, 0, 3, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 4, 0, 1},
    {1, 1, 1, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 4, 0, 0, 0, 3, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

int playerRow = 2;
int playerCol = 2;
int hasWon = 0;

// Resim Dokuları için değişkenler
SDL_Texture* playerTex = NULL;
SDL_Texture* boxTex = NULL;
SDL_Texture* wallTex = NULL;
SDL_Texture* targetTex = NULL;

// BMP resimlerini yükleyen yardımcı fonksiyon
SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Surface* loadedSurface = SDL_LoadBMP(path);
    if (loadedSurface == NULL) {
        printf("Resim yuklenemedi: %s! SDL Hatasi: %s\n", path, SDL_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    return texture;
}

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

void movePlayer(int dRow, int dCol) {
    if (hasWon) return;

    int nextRow = playerRow + dRow;
    int nextCol = playerCol + dCol;

    if (map[nextRow][nextCol] == 0 || map[nextRow][nextCol] == 4) {
        map[playerRow][playerCol] = (initialMap[playerRow][playerCol] == 4) ? 4 : 0;
        map[nextRow][nextCol] = 2;
        playerRow = nextRow;
        playerCol = nextCol;
    }
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

void resetLevel() {
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            map[r][c] = initialMap[r][c];
        }
    }
    map[2][2] = 2;
    map[2][6] = 3;
    map[7][6] = 3;
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

    // --- RESIMLERI TAM DOSYA YOLUYLA YÜKLE ---
    playerTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\player.bmp", renderer);
    boxTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\box.bmp", renderer);
    wallTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\wall.bmp", renderer);
    targetTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\target.bmp", renderer);

    int isRunning = 1;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                isRunning = 0;
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:    case SDLK_w: movePlayer(-1, 0); break;
                    case SDLK_DOWN:  case SDLK_s: movePlayer(1, 0);  break;
                    case SDLK_LEFT:  case SDLK_a: movePlayer(0, -1); break;
                    case SDLK_RIGHT: case SDLK_d: movePlayer(0, 1);  break;
                    case SDLK_r: resetLevel(); break;
                }
            }
        }

        if (hasWon) {
            SDL_SetRenderDrawColor(renderer, 20, 100, 20, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        }
        SDL_RenderClear(renderer);

        // Çizim Döngüsü
        for (int r = 0; r < MAP_ROWS; r++) {
            for (int c = 0; c < MAP_COLS; c++) {
                SDL_Rect tile = { c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE };

                // 1. Hedef Çizimi (Arka Plan)
                if (initialMap[r][c] == 4 && targetTex != NULL) {
                    SDL_RenderCopy(renderer, targetTex, NULL, &tile);
                }

                // 2. Nesne Çizimi (Ön Plan)
                if (map[r][c] == 1 && wallTex != NULL) {
                    SDL_RenderCopy(renderer, wallTex, NULL, &tile);
                }
                else if (map[r][c] == 2 && playerTex != NULL) {
                    SDL_RenderCopy(renderer, playerTex, NULL, &tile);
                }
                else if (map[r][c] == 3 && boxTex != NULL) {
                    if (initialMap[r][c] == 4) {
                        SDL_SetTextureColorMod(boxTex, 150, 255, 150); // Hedefteki kutuyu yeşillendir
                    } else {
                        SDL_SetTextureColorMod(boxTex, 255, 255, 255);
                    }
                    SDL_RenderCopy(renderer, boxTex, NULL, &tile);
                }
            }
        }

        SDL_RenderPresent(renderer);
    }

    // Temizlik
    SDL_DestroyTexture(playerTex);
    SDL_DestroyTexture(boxTex);
    SDL_DestroyTexture(wallTex);
    SDL_DestroyTexture(targetTex);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
