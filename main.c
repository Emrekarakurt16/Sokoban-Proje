#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TILE_SIZE 40
#define MAP_ROWS 10
#define MAP_COLS 10
#define TOTAL_LEVELS 3
#define MAX_UNDO 50 // Maksimum geri alınabilecek hamle sayısı

// Orijinal Bölüm Şablonları Havuzu
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
    // --- BÖLÜM 3 (Senin kusursuzlaştırdığın sürüm) ---
    {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 4, 0, 0, 0, 0, 0, 0, 4, 1},
        {1, 0, 1, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 0, 1, 1, 1, 1, 0, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 0, 0, 0, 1},
        {1, 4, 1, 0, 0, 0, 0, 0, 4, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    }
};

int map[MAP_ROWS][MAP_COLS];
int initialMap[MAP_ROWS][MAP_COLS];

int currentLevel = 0;
int playerRow;
int playerCol;
int hasWon = 0;

// --- UNDO (GERİ ALMA) STACK YAPISI ---
typedef struct {
    int mapState[MAP_ROWS][MAP_COLS];
    int pRow;
    int pCol;
} UndoStep;

UndoStep undoStack[MAX_UNDO];
int undoTop = -1; // Stack'in en üstünü gösteren işaretçi (Boşken -1)

// Hamleyi Stack'e kaydeden fonksiyon (Push)
void saveState() {
    if (undoTop < MAX_UNDO - 1) {
        undoTop++;
        for (int r = 0; r < MAP_ROWS; r++) {
            for (int c = 0; c < MAP_COLS; c++) {
                undoStack[undoTop].mapState[r][c] = map[r][c];
            }
        }
        undoStack[undoTop].pRow = playerRow;
        undoStack[undoTop].pCol = playerCol;
    } else {
        // Stack dolduysa en eski hamleyi silip kaydırabiliriz ama pratiklik için 50 hamle sınırı koyduk.
    }
}

// Son hamleyi geri yükleyen fonksiyon (Pop)
void undoMove() {
    if (undoTop >= 0) {
        for (int r = 0; r < MAP_ROWS; r++) {
            for (int c = 0; c < MAP_COLS; c++) {
                map[r][c] = undoStack[undoTop].mapState[r][c];
            }
        }
        playerRow = undoStack[undoTop].pRow;
        playerCol = undoStack[undoTop].pCol;
        undoTop--;
        printf("Hamle geri alindi. Kalan Geri Alma Hakki: %d\n", undoTop + 1);
    } else {
        printf("Geri alinacak hamle yok!\n");
    }
}

// Stack'i tamamen temizleyen fonksiyon (Bölüm değiştiğinde veya resetlendiğinde çağrılır)
void clearUndoStack() {
    undoTop = -1;
}

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

void loadLevel(int levelNum) {
    if (levelNum >= TOTAL_LEVELS) {
        hasWon = 1;
        printf("TEBRIKLER! Tum oyunu bitirdiniz!\n");
        return;
    }

    clearUndoStack(); // Bölüm yüklenirken eski hamle geçmişini temizle

    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            initialMap[r][c] = initialLevels[levelNum][r][c];
            map[r][c] = initialLevels[levelNum][r][c];
        }
    }

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
        map[4][4] = 2; playerRow = 4; playerCol = 4;

        map[2][3] = 3;
        map[2][5] = 3;
        map[6][3] = 3;
        map[6][5] = 3;
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
    int moved = 0; // Oyuncunun gerçekten hareket edip etmediğini kontrol eder

    if (map[nextRow][nextCol] == 0 || map[nextRow][nextCol] == 4) {
        saveState(); // Hareket gerçekleşmeden ÖNCE mevcut durumu kaydet
        map[playerRow][playerCol] = (initialMap[playerRow][playerCol] == 4) ? 4 : 0;
        map[nextRow][nextCol] = 2;
        playerRow = nextRow;
        playerCol = nextCol;
        moved = 1;
    }
    else if (map[nextRow][nextCol] == 3) {
        int boxNextRow = nextRow + dRow;
        int boxNextCol = nextCol + dCol;

        if (map[boxNextRow][boxNextCol] == 0 || map[boxNextRow][boxNextCol] == 4) {
            saveState(); // Kutu itilmeden ÖNCE mevcut durumu kaydet
            map[boxNextRow][boxNextCol] = 3;
            map[playerRow][playerCol] = (initialMap[playerRow][playerCol] == 4) ? 4 : 0;
            map[nextRow][nextCol] = 2;
            playerRow = nextRow;
            playerCol = nextCol;
            moved = 1;
        }
    }

    if (moved && checkWin()) {
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
        printf("Pencere olusturulamadi!\n");
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

    playerTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\player.bmp", renderer);
    boxTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\box.bmp", renderer);
    wallTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\wall.bmp", renderer);
    targetTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\target.bmp", renderer);

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
                    case SDLK_z: undoMove(); break; // Z tuşuna basınca hamleyi geri al
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
