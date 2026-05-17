#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TILE_SIZE 40
#define MAP_ROWS 10
#define MAP_COLS 10
#define TOTAL_LEVELS 3 // Toplam bölüm sayısı

// Orijinal Bölüm Şablonları Havuzu (3 Boyutlu Dizi)
int initialLevels[TOTAL_LEVELS][MAP_ROWS][MAP_COLS] = {
    // --- BÖLÜM 1 ---
    {
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
    },
    // --- BÖLÜM 2 (Senin düzenlediğin hali) ---
    {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1, 0, 0, 0, 4, 1},
        {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1, 1, 1},
        {1, 1, 1, 0, 0, 0, 0, 0, 0, 1},
        {1, 4, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    },
    // --- BÖLÜM 3 (Yeni Zorlu Final Bölümü - 4 Kutulu) ---
    // --- BÖLÜM 3 (Zor ama %100 Çözülebilir Final Haritası) ---
    {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 4, 0, 0, 0, 0, 0, 0, 4, 1}, // Üst köşelerde hedefler
        {1, 0, 1, 0, 0, 0, 1, 0, 0, 1}, // İç duvarlar hafif açıldı
        {1, 0, 1, 1, 1, 0, 1, 0 , 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // Ortada geniş manevra alanı
        {1, 1, 0, 1, 1, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 1, 1, 0, 1, 0, 1},
        {1, 4, 1, 0, 0, 0, 0, 0, 4, 1}, // Alt köşelerde hedefler
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    }
};

// Aktif olarak oynanan harita ve hedef şablonu
int map[MAP_ROWS][MAP_COLS];
int initialMap[MAP_ROWS][MAP_COLS];

int currentLevel = 0;
int playerRow;
int playerCol;
int hasWon = 0;

SDL_Texture* playerTex = NULL;
SDL_Texture* boxTex = NULL;
SDL_Texture* wallTex = NULL;
SDL_Texture* targetTex = NULL;

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

// Belirtilen bölümü hafızaya yükleyen fonksiyon
void loadLevel(int levelNum) {
    if (levelNum >= TOTAL_LEVELS) {
        hasWon = 1; // Tüm bölümler bittiyse oyunu bitir
        printf("TEBRIKLER! Tum oyunu bitirdiniz!\n");
        return;
    }

    // Haritayı havuzdan kopyala
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            initialMap[r][c] = initialLevels[levelNum][r][c];
            map[r][c] = initialLevels[levelNum][r][c];
        }
    }

    // Her bölümün başlangıç elemanlarını (Oyuncu ve Kutuları) yerleştiriyoruz
    if (levelNum == 0) {
        map[2][2] = 2; playerRow = 2; playerCol = 2;
        map[2][6] = 3;
        map[7][6] = 3;
    }
    else if (levelNum == 1) {
        map[1][2] = 2; playerRow = 1; playerCol = 2;
        map[2][3] = 3;
        map[5][5] = 3;
    }
    else if (levelNum == 2) {
        map[4][2] = 2; playerRow = 4; playerCol = 2; // Oyuncu başlangıç yeri

        // 4 Adet Kutunun Başlangıç Konumları
        map[3][3] = 3; // 1. Kutu
        map[4][6] = 3; // 2. Kutu
        map[6][4] = 3; // 3. Kutu
        map[7][7] = 3; // 4. Kutu
    }

    printf("%d. Bolum Yuklendi!\n", levelNum + 1);
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
        currentLevel++;
        loadLevel(currentLevel);
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
        printf("Pencere olusturulamadi! Hata: %s\n", SCREEN_WIDTH);
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

    // Resimleri tam dosya yoluyla yükle
    playerTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\player.bmp", renderer);
    boxTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\box.bmp", renderer);
    wallTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\wall.bmp", renderer);
    targetTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\target.bmp", renderer);

    // İlk bölümü başlat
    loadLevel(currentLevel);

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
                    case SDLK_r: loadLevel(currentLevel); break;
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

                if (initialMap[r][c] == 4 && targetTex != NULL) {
                    SDL_RenderCopy(renderer, targetTex, NULL, &tile);
                }

                if (map[r][c] == 1 && wallTex != NULL) {
                    SDL_RenderCopy(renderer, wallTex, NULL, &tile);
                }
                else if (map[r][c] == 2 && playerTex != NULL) {
                    SDL_RenderCopy(renderer, playerTex, NULL, &tile);
                }
                else if (map[r][c] == 3 && boxTex != NULL) {
                    if (initialMap[r][c] == 4) {
                        SDL_SetTextureColorMod(boxTex, 150, 255, 150);
                    } else {
                        SDL_SetTextureColorMod(boxTex, 255, 255, 255);
                    }
                    SDL_RenderCopy(renderer, boxTex, NULL, &tile);
                }
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(playerTex);
    SDL_DestroyTexture(boxTex);
    SDL_DestroyTexture(wallTex);
    SDL_DestroyTexture(targetTex);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
