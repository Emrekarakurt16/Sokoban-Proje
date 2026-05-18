#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600
#define TILE_SIZE 60
#define MAP_ROWS 10
#define MAP_COLS 10
#define TOTAL_LEVELS 3
#define MAX_UNDO 50

// --- OYUN DURUMLARI (STATES) ---
typedef enum {
    STATE_MENU,
    STATE_CONTROLS,
    STATE_PLAY
} GameState;

GameState currentState = STATE_MENU;
int selectedOption = 0; // 0: Basla, 1: Komutlar, 2: Cikis

// --- YENİ DÜZELTTİĞİN HARİTA MATRİSLERİ HAVUZU ---
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
    // --- BÖLÜM 2 (Senin son düzelttiğin hali) ---
    {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1, 0, 0, 0, 4, 1},
        {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 1, 1, 1},
        {1, 1, 1, 0, 0, 0, 0, 0, 0, 1},
        {1, 4, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    },
    // --- BÖLÜM 3 (Senin son kusursuzlaştırdığın sürüm) ---
    {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 4, 0, 0, 0, 0, 0, 0, 4, 1},
        {1, 0, 1, 0, 0, 1, 0, 0, 0, 1},
        {1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 0, 1, 1, 1, 1, 0, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 0, 0, 0, 1},
        {1, 4, 1, 0, 0, 1, 0, 4, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    }
};

int map[MAP_ROWS][MAP_COLS];
int initialMap[MAP_ROWS][MAP_COLS];
int currentLevel = 0;
int playerRow, playerCol;
int hasWon = 0;

// --- UNDO STACK YAPISI ---
typedef struct {
    int mapState[MAP_ROWS][MAP_COLS];
    int pRow, pCol;
} UndoStep;

UndoStep undoStack[MAX_UNDO];
int undoTop = -1;

void saveState() {
    if (undoTop < MAX_UNDO - 1) {
        undoTop++;
        for (int r = 0; r < MAP_ROWS; r++)
            for (int c = 0; c < MAP_COLS; c++)
                undoStack[undoTop].mapState[r][c] = map[r][c];
        undoStack[undoTop].pRow = playerRow;
        undoStack[undoTop].pCol = playerCol;
    }
}

void undoMove() {
    if (undoTop >= 0) {
        for (int r = 0; r < MAP_ROWS; r++)
            for (int c = 0; c < MAP_COLS; c++)
                map[r][c] = undoStack[undoTop].mapState[r][c];
        playerRow = undoStack[undoTop].pRow;
        playerCol = undoStack[undoTop].pCol;
        undoTop--;
    }
}

// --- SDL TEXTURE TANIMLAMALARI ---
SDL_Texture *playerTex = NULL;
SDL_Texture *boxTex = NULL;
SDL_Texture *wallTex = NULL;
SDL_Texture *targetTex = NULL;
SDL_Texture *menuTex = NULL;
SDL_Texture *controlsTex = NULL;

SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Surface* loadedSurface = SDL_LoadBMP(path);
    if (!loadedSurface) return NULL;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    return texture;
}

void loadLevel(int levelNum) {
    if (levelNum >= TOTAL_LEVELS) { hasWon = 1; return; }
    undoTop = -1; // Yeni bölüme geçince yığını temizle

    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            initialMap[r][c] = initialLevels[levelNum][r][c];
            map[r][c] = initialLevels[levelNum][r][c];
        }
    }

    // Karakter ve Kutuların Başlangıç Noktaları (Matris değişikliklerine uygun yerleşimler)
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
        map[2][7] = 3;
        map[6][3] = 3;
        map[6][7] = 3;
    }
}

int checkWin() {
    for (int r = 0; r < MAP_ROWS; r++)
        for (int c = 0; c < MAP_COLS; c++)
            if (initialMap[r][c] == 4 && map[r][c] != 3) return 0;
    return 1;
}

void movePlayer(int dRow, int dCol) {
    if (hasWon) return;
    int nextRow = playerRow + dRow, nextCol = playerCol + dCol;
    int moved = 0;

    if (map[nextRow][nextCol] == 0 || map[nextRow][nextCol] == 4) {
        saveState();
        map[playerRow][playerCol] = (initialMap[playerRow][playerCol] == 4) ? 4 : 0;
        map[nextRow][nextCol] = 2;
        playerRow = nextRow; playerCol = nextCol; moved = 1;
    } else if (map[nextRow][nextCol] == 3) {
        int boxNextR = nextRow + dRow, boxNextC = nextCol + dCol;
        if (map[boxNextR][boxNextC] == 0 || map[boxNextR][boxNextC] == 4) {
            saveState();
            map[boxNextR][boxNextC] = 3;
            map[playerRow][playerCol] = (initialMap[playerRow][playerCol] == 4) ? 4 : 0;
            map[nextRow][nextCol] = 2;
            playerRow = nextRow; playerCol = nextCol; moved = 1;
        }
    }
    if (moved && checkWin()) { currentLevel++; loadLevel(currentLevel); }
}

int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Sokoban Projesi - Emre", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Dosya Yollarını Tanımlama
    playerTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\player.bmp", renderer);
    boxTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\box.bmp", renderer);
    wallTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\wall.bmp", renderer);
    targetTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\target.bmp", renderer);
    menuTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\menu.bmp", renderer);
    controlsTex = loadTexture("C:\\Users\\Emrek\\OneDrive\\Desktop\\Sokoban Proje\\controls.bmp", renderer);

    loadLevel(currentLevel);
    int isRunning = 1; SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) isRunning = 0;
            else if (event.type == SDL_KEYDOWN) {
                // --- MENÜ KONTROLLERİ ---
                if (currentState == STATE_MENU) {
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        if (selectedOption == 0) currentState = STATE_PLAY;
                        else if (selectedOption == 1) currentState = STATE_CONTROLS;
                        else isRunning = 0; // Çıkış
                    }
                    else if (event.key.keysym.sym == SDLK_UP) selectedOption = (selectedOption - 1 + 3) % 3;
                    else if (event.key.keysym.sym == SDLK_DOWN) selectedOption = (selectedOption + 1) % 3;
                }
                // --- KOMUTLAR EKRANI KONTROLLERİ ---
                else if (currentState == STATE_CONTROLS) {
                    if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_RETURN) currentState = STATE_MENU;
                }
                // --- OYUN İÇİ KONTROLLER ---
                else if (currentState == STATE_PLAY) {
                    switch (event.key.keysym.sym) {
                        case SDLK_UP: case SDLK_w: movePlayer(-1, 0); break;
                        case SDLK_DOWN: case SDLK_s: movePlayer(1, 0); break;
                        case SDLK_LEFT: case SDLK_a: movePlayer(0, -1); break;
                        case SDLK_RIGHT: case SDLK_d: movePlayer(0, 1); break;
                        case SDLK_r: loadLevel(currentLevel); break;
                        case SDLK_z: undoMove(); break;
                        case SDLK_ESCAPE: currentState = STATE_MENU; break;
                    }
                }
            }
        }

        SDL_RenderClear(renderer);

        // --- DRAW 1: MENU ---
        if (currentState == STATE_MENU) {
            if (menuTex) SDL_RenderCopy(renderer, menuTex, NULL, NULL);

            // Yeşil Seçim İmleci - Çizdiğin okların tam üstüne oturacak şekilde ayarlandı
            SDL_SetRenderDrawColor(renderer, 34, 197, 94, 255);
            SDL_Rect cursor = { 135, 142 + (selectedOption * 123), 15, 15 };
            SDL_RenderFillRect(renderer, &cursor);
        }
        // --- DRAW 2: CONTROLS ---
        else if (currentState == STATE_CONTROLS) {
            if (controlsTex) SDL_RenderCopy(renderer, controlsTex, NULL, NULL);
        }
        // --- DRAW 3: PLAY ---
        else if (currentState == STATE_PLAY) {
            if (hasWon) SDL_SetRenderDrawColor(renderer, 20, 100, 20, 255);
            else SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
            SDL_RenderClear(renderer);

            for (int r = 0; r < MAP_ROWS; r++) {
                for (int c = 0; c < MAP_COLS; c++) {
                    SDL_Rect tile = { c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                    if (initialMap[r][c] == 4) SDL_RenderCopy(renderer, targetTex, NULL, &tile);
                    if (map[r][c] == 1) SDL_RenderCopy(renderer, wallTex, NULL, &tile);
                    else if (map[r][c] == 2) SDL_RenderCopy(renderer, playerTex, NULL, &tile);
                    else if (map[r][c] == 3) {
                        if (initialMap[r][c] == 4) SDL_SetTextureColorMod(boxTex, 150, 255, 150);
                        else SDL_SetTextureColorMod(boxTex, 255, 255, 255);
                        SDL_RenderCopy(renderer, boxTex, NULL, &tile);
                    }
                }
            }
        }
        SDL_RenderPresent(renderer);
    }

    // Temizlik
    SDL_DestroyTexture(playerTex); SDL_DestroyTexture(boxTex);
    SDL_DestroyTexture(wallTex); SDL_DestroyTexture(targetTex);
    SDL_DestroyTexture(menuTex); SDL_DestroyTexture(controlsTex);
    SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
