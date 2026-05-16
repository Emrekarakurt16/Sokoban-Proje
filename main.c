#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TILE_SIZE 40
#define MAP_ROWS 10
#define MAP_COLS 10

// Orijinal harita şablonu (Kutuların hedeflere gelip gelmediğini kontrol etmek için)
int initialMap[MAP_ROWS][MAP_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 4, 0, 1}, // 4: Hedef
    {1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
    {1, 0, 4, 0, 1, 0, 0, 0, 0, 1}, // 4: Hedef
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// Anlık oyun haritası
int map[MAP_ROWS][MAP_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 2, 0, 1, 0, 3, 0, 0, 1}, // 2: Oyuncu, 3: Kutu
    {1, 0, 0, 0, 0, 0, 0, 4, 0, 1},
    {1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
    {1, 0, 4, 0, 1, 0, 0, 3, 0, 1}, // 3: Kutu
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

int playerRow = 2;
int playerCol = 2;

void movePlayer(int dRow, int dCol) {
    int nextRow = playerRow + dRow;
    int nextCol = playerCol + dCol;

    // 1. Durum: Gitmek istediğimiz yer boş yol (0) veya hedef noktası (4) ise direkt yürü
    if (map[nextRow][nextCol] == 0 || map[nextRow][nextCol] == 4) {
        map[playerRow][playerCol] = (initialMap[playerRow][playerCol] == 4) ? 4 : 0;
        map[nextRow][nextCol] = 2;
        playerRow = nextRow;
        playerCol = nextCol;
    }
    // 2. Durum: Gitmek istediğimiz yerde KUTU (3) varsa
    else if (map[nextRow][nextCol] == 3) {
        int boxNextRow = nextRow + dRow;
        int boxNextCol = nextCol + dCol;

        // Kutunun arkası boş yol (0) veya hedef (4) ise kutuyu itebiliriz!
        if (map[boxNextRow][boxNextCol] == 0 || map[boxNextRow][boxNextCol] == 4) {
            // Önce kutuyu yeni yerine taşı
            map[boxNextRow][boxNextCol] = 3;

            // Oyuncunun eski yerini ayarla (Eğer hedef üzerindeyse hedef kalsın, yoksa boş yol olsun)
            map[playerRow][playerCol] = (initialMap[playerRow][playerCol] == 4) ? 4 : 0;

            // Oyuncuyu kutunun eski yerine taşı
            map[nextRow][nextCol] = 2;

            // Koordinatları güncelle
            playerRow = nextRow;
            playerCol = nextCol;
        }
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
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:    case SDLK_w: movePlayer(-1, 0); break;
                    case SDLK_DOWN:  case SDLK_s: movePlayer(1, 0);  break;
                    case SDLK_LEFT:  case SDLK_a: movePlayer(0, -1); break;
                    case SDLK_RIGHT: case SDLK_d: movePlayer(0, 1);  break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        // Çizim Aşaması
        for (int r = 0; r < MAP_ROWS; r++) {
            for (int c = 0; c < MAP_COLS; c++) {
                SDL_Rect tile = { c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE };

                // Arka plandaki yeşil hedefleri her halükarda çiz (Kutunun veya oyuncunun altında kalabilirler)
                if (initialMap[r][c] == 4) {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 100, 255);
                    SDL_Rect targetRect = { c * TILE_SIZE + 15, r * TILE_SIZE + 15, 10, 10 };
                    SDL_RenderFillRect(renderer, &targetRect);
                }

                if (map[r][c] == 1) { // Duvar
                    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                    SDL_RenderFillRect(renderer, &tile);
                }
                else if (map[r][c] == 2) { // Oyuncu
                    SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255);
                    SDL_RenderFillRect(renderer, &tile);
                }
                else if (map[r][c] == 3) { // Kutu
                    // Eğer kutu bir hedefin üzerindeyse rengini değiştir (Görsel geri bildirim)
                    if (initialMap[r][c] == 4) {
                        SDL_SetRenderDrawColor(renderer, 50, 200, 50, 255); // Hedefteki kutu -> Yeşilimsi
                    } else {
                        SDL_SetRenderDrawColor(renderer, 200, 150, 50, 255); // Normal kutu -> Kahverengi
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
