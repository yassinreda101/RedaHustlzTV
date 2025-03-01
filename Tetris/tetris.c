#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <string.h>
#include <ctype.h>

// Constants for game dimensions
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define PREVIEW_SIZE 6

// Constants for game controls
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define KEY_DOWN 80
#define KEY_ROTATE 72
#define KEY_QUIT 113 // 'q'
#define KEY_PAUSE 112 // 'p'

// Constants for tetromino types
#define I_TETROMINO 1
#define O_TETROMINO 2
#define T_TETROMINO 3
#define S_TETROMINO 4
#define Z_TETROMINO 5
#define J_TETROMINO 6
#define L_TETROMINO 7

// Maximum number of high scores to track
#define MAX_HIGH_SCORES 10

// Game board - 0 means empty, non-zero values represent tetromino pieces
int board[BOARD_HEIGHT][BOARD_WIDTH] = {0};

// Game variables
int score = 0;
int level = 1;
int lines = 0;
bool gameOver = false;
bool paused = false;
clock_t lastDropTime;
clock_t gameStartTime;
int dropInterval = 1000; // milliseconds

// High score structure
typedef struct {
    char initials[4]; // 3 characters + null terminator
    int score;
    int level;
    int lines;
} HighScore;

HighScore highScores[MAX_HIGH_SCORES];

// Tetromino structure
typedef struct {
    int x, y;         // Position of the tetromino's reference point
    int type;         // Type of tetromino (1-7)
    int rotation;     // Current rotation (0-3)
} Tetromino;

Tetromino currentTetromino;
Tetromino nextTetromino;

// Define shapes for each tetromino type in each rotation
// Each tetromino is defined by 4 blocks relative to a reference point
const int TETROMINO_SHAPES[7][4][4][2] = {
    // I tetromino
    {
        {{0,0}, {-1,0}, {1,0}, {2,0}},  // Rotation 0
        {{0,0}, {0,-1}, {0,1}, {0,2}},  // Rotation 1
        {{0,0}, {-1,0}, {1,0}, {2,0}},  // Rotation 2
        {{0,0}, {0,-1}, {0,1}, {0,2}}   // Rotation 3
    },
    // O tetromino (doesn't change with rotation)
    {
        {{0,0}, {1,0}, {0,1}, {1,1}},  // Rotation 0
        {{0,0}, {1,0}, {0,1}, {1,1}},  // Rotation 1
        {{0,0}, {1,0}, {0,1}, {1,1}},  // Rotation 2
        {{0,0}, {1,0}, {0,1}, {1,1}}   // Rotation 3
    },
    // T tetromino
    {
        {{0,0}, {-1,0}, {1,0}, {0,1}},  // Rotation 0
        {{0,0}, {0,-1}, {0,1}, {-1,0}}, // Rotation 1
        {{0,0}, {-1,0}, {1,0}, {0,-1}}, // Rotation 2
        {{0,0}, {0,-1}, {0,1}, {1,0}}   // Rotation 3
    },
    // S tetromino
    {
        {{0,0}, {-1,0}, {0,1}, {1,1}},  // Rotation 0
        {{0,0}, {0,-1}, {1,0}, {1,1}},  // Rotation 1
        {{0,0}, {-1,0}, {0,1}, {1,1}},  // Rotation 2
        {{0,0}, {0,-1}, {1,0}, {1,1}}   // Rotation 3
    },
    // Z tetromino
    {
        {{0,0}, {1,0}, {0,1}, {-1,1}},  // Rotation 0
        {{0,0}, {0,1}, {1,0}, {1,-1}},  // Rotation 1
        {{0,0}, {1,0}, {0,1}, {-1,1}},  // Rotation 2
        {{0,0}, {0,1}, {1,0}, {1,-1}}   // Rotation 3
    },
    // J tetromino
    {
        {{0,0}, {-1,0}, {1,0}, {-1,1}}, // Rotation 0
        {{0,0}, {0,-1}, {0,1}, {-1,-1}},// Rotation 1
        {{0,0}, {-1,0}, {1,0}, {1,-1}}, // Rotation 2
        {{0,0}, {0,-1}, {0,1}, {1,1}}   // Rotation 3
    },
    // L tetromino
    {
        {{0,0}, {-1,0}, {1,0}, {1,1}},  // Rotation 0
        {{0,0}, {0,-1}, {0,1}, {-1,1}}, // Rotation 1
        {{0,0}, {-1,0}, {1,0}, {-1,-1}},// Rotation 2
        {{0,0}, {0,-1}, {0,1}, {1,-1}}  // Rotation 3
    }
};

// Function prototypes
void generateRandomTetromino(Tetromino *tetromino);
void drawBoard();
void drawInstructions();
bool canMoveTo(int newX, int newY, int newRotation);
void lockTetromino();
int clearLines();
void updateScore(int linesCleared);
void processInput();
void updateGame();
void setCursorPosition(int x, int y);
void hideCursor();
void showCursor();
void setConsoleColor(int color);
void displayTitleScreen();
void displayMainMenu();
void playGame();
void displayHighScores();
void addHighScore(int score, int level, int lines);
bool isHighScore(int score);
void loadHighScores();
void saveHighScores();
void displayLevelSelect();
void displayGameOver();
void displayInstructions();

// Function to set cursor position without clearing screen
void setCursorPosition(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Hide the cursor
void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}

// Show the cursor
void showCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}

// Set console text color
void setConsoleColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// Generate a random tetromino
void generateRandomTetromino(Tetromino *tetromino) {
    tetromino->type = (rand() % 7) + 1; // Random number between 1-7
    tetromino->rotation = 0;
    tetromino->x = BOARD_WIDTH / 2; // Start at the middle of the top
    tetromino->y = 0;
}

// Format time as mm:ss
void formatTime(int seconds, char* buffer) {
    int minutes = seconds / 60;
    seconds = seconds % 60;
    sprintf(buffer, "%d:%02d", minutes, seconds);
}

// Draw the next piece preview in original style
void drawNextPiece() {
    // Get shape of next piece
    const int (*shape)[2] = TETROMINO_SHAPES[nextTetromino.type - 1][0]; // Always use rotation 0
    
    // Clear the next piece area
    for (int y = 0; y < 4; y++) {
        setCursorPosition(3, 12 + y);
        printf("            ");
    }
    
    // Draw the next piece blocks
    for (int i = 0; i < 4; i++) {
        int x = shape[i][0];
        int y = shape[i][1];
        
        // Adjust for piece type (center the piece)
        int offsetX = 9;
        int offsetY = 13;
        
        // Type-specific adjustments
        switch (nextTetromino.type) {
            case I_TETROMINO:
                offsetY = 13;
                break;
            case O_TETROMINO:
                offsetX = 8; 
                break;
            case T_TETROMINO:
            case J_TETROMINO:
            case L_TETROMINO:
                offsetX = 8;
                break;
        }
        
        setCursorPosition(offsetX + x*2, offsetY + y);
        printf("[]");
    }
}

// Draw game instructions on the left side
void drawInstructions() {
    setCursorPosition(0, 7);
    printf("NEXT PIECE:");
    
    setCursorPosition(0, 16);
    printf("CONTROLS:");
    setCursorPosition(0, 17);
    printf("<-/->: Move");
    setCursorPosition(0, 18);
    printf("^: Rotate");
    setCursorPosition(0, 19);
    printf("v: Soft Drop");
    setCursorPosition(0, 20);
    printf("SPACE: Hard Drop");
    setCursorPosition(0, 21);
    printf("P: Pause");
    setCursorPosition(0, 22);
    printf("Q: Quit");
}

// Draw the game board to the console
void drawBoard() {
    // Create a temporary copy of the board to draw the current tetromino
    char displayBoard[BOARD_HEIGHT][BOARD_WIDTH * 2 + 1];
    
    // Initialize with dots for empty spaces
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            displayBoard[y][x*2] = '.';
            displayBoard[y][x*2+1] = ' ';
        }
        displayBoard[y][BOARD_WIDTH*2] = '\0';
    }
    
    // Copy the current board
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x] != 0) {
                displayBoard[y][x*2] = '[';
                displayBoard[y][x*2+1] = ']';
            }
        }
    }
    
    // Draw the current tetromino on the display board
    const int (*shape)[2] = TETROMINO_SHAPES[currentTetromino.type - 1][currentTetromino.rotation];
    for (int i = 0; i < 4; i++) {
        int x = currentTetromino.x + shape[i][0];
        int y = currentTetromino.y + shape[i][1];
        
        if (x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT) {
            if (currentTetromino.type == Z_TETROMINO) {
                // Highlight the currently falling Z piece with red (as in the image)
                displayBoard[y][x*2] = '[';
                displayBoard[y][x*2+1] = ']';
            } else {
                displayBoard[y][x*2] = '[';
                displayBoard[y][x*2+1] = ']';
            }
        }
    }
    
    // Calculate elapsed time
    int elapsedSeconds = (clock() - gameStartTime) / CLOCKS_PER_SEC;
    char timeString[10];
    formatTime(elapsedSeconds, timeString);
    
    // Display game info in the original format
    setCursorPosition(0, 1);
    printf("FULL LINES: %-3d", lines);
    setCursorPosition(0, 2);
    printf("LEVEL: %-3d", level);
    setCursorPosition(0, 3);
    printf("SCORE: %-3d", score);
    setCursorPosition(0, 4);
    printf("TIME: %-5s", timeString);
    
    // Draw the board with classic-style borders
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        setCursorPosition(25, y + 1);
        printf("<!");
        
        // Print the row content
        printf("%s", displayBoard[y]);
        
        printf("!>");
    }
    
    // Draw the bottom decoration (row of stars and zigzag pattern)
    setCursorPosition(25, BOARD_HEIGHT + 1);
    printf("<!");
    for (int i = 0; i < BOARD_WIDTH * 2; i++) {
        printf("*");
    }
    printf("!>");
    
    setCursorPosition(25, BOARD_HEIGHT + 2);
    printf(" ");
    for (int i = 0; i < BOARD_WIDTH; i++) {
        printf("\\/");
    }
    
    // Draw the next piece
    drawNextPiece();
    
    // Draw instructions
    drawInstructions();
}

// Check if a tetromino can move to the specified position
bool canMoveTo(int newX, int newY, int newRotation) {
    const int (*shape)[2] = TETROMINO_SHAPES[currentTetromino.type - 1][newRotation];
    
    for (int i = 0; i < 4; i++) {
        int x = newX + shape[i][0];
        int y = newY + shape[i][1];
        
        // Check boundaries
        if (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT) {
            return false;
        }
        
        // Check collision with locked pieces
        if (y >= 0 && board[y][x] != 0) {
            return false;
        }
    }
    
    return true;
}

// Lock the current tetromino in place
void lockTetromino() {
    const int (*shape)[2] = TETROMINO_SHAPES[currentTetromino.type - 1][currentTetromino.rotation];
    
    for (int i = 0; i < 4; i++) {
        int x = currentTetromino.x + shape[i][0];
        int y = currentTetromino.y + shape[i][1];
        
        if (x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT) {
            board[y][x] = currentTetromino.type;
        }
    }
}

// Clear completed lines and return the number of lines cleared
int clearLines() {
    int linesCleared = 0;
    
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        bool isLineFull = true;
        
        // Check if the line is full
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x] == 0) {
                isLineFull = false;
                break;
            }
        }
        
        // If the line is full, clear it and move everything down
        if (isLineFull) {
            // Clear the line
            for (int x = 0; x < BOARD_WIDTH; x++) {
                board[y][x] = 0;
            }
            
            // Move all lines above down
            for (int moveY = y; moveY > 0; moveY--) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    board[moveY][x] = board[moveY - 1][x];
                    board[moveY - 1][x] = 0;
                }
            }
            
            // Since we moved everything down, we need to check this line again
            y++;
            linesCleared++;
        }
    }
    
    return linesCleared;
}

// Update the score based on the number of lines cleared
void updateScore(int linesCleared) {
    // Original Tetris scoring based on level
    if (linesCleared > 0) {
        // Update lines count
        lines += linesCleared;
        
        // Base points for each line clear type
        int basePoints;
        switch (linesCleared) {
            case 1:
                basePoints = 40;   // Single
                break;
            case 2:
                basePoints = 100;  // Double
                break;
            case 3:
                basePoints = 300;  // Triple
                break;
            case 4:
                basePoints = 1200; // Tetris
                break;
            default:
                basePoints = 0;
        }
        
        // Multiply by level
        score += basePoints * level;
        
        // Check for level up (every 10 lines)
        int newLevel = (lines / 10) + 1;
        if (newLevel > level) {
            level = newLevel;
            
            // Set drop interval based on level (from original Tetris)
            dropInterval = 1000 * (0.8 - ((level - 1) * 0.007));
            if (dropInterval < 100) {
                dropInterval = 100; // Don't go too fast
            }
        }
    }
}

// Process user input - improved for better responsiveness
void processInput() {
    // Process all pending key presses
    while (_kbhit()) {
        int key = _getch();
        
        if (key == KEY_PAUSE) { // P key for pause
            paused = !paused;
            return;
        }
        
        if (paused) {
            return; // Don't process other keys while paused
        }
        
        // Special keys (arrows) in Windows send two bytes
        if (key == 224) {
            key = _getch();
            
            switch (key) {
                case KEY_LEFT:
                    if (canMoveTo(currentTetromino.x - 1, currentTetromino.y, currentTetromino.rotation)) {
                        currentTetromino.x--;
                    }
                    break;
                    
                case KEY_RIGHT:
                    if (canMoveTo(currentTetromino.x + 1, currentTetromino.y, currentTetromino.rotation)) {
                        currentTetromino.x++;
                    }
                    break;
                    
                case KEY_DOWN:
                    if (canMoveTo(currentTetromino.x, currentTetromino.y + 1, currentTetromino.rotation)) {
                        currentTetromino.y++;
                        score += 1; // Small bonus for soft drop
                    }
                    break;
                    
                case KEY_ROTATE:
                    {
                        int newRotation = (currentTetromino.rotation + 1) % 4;
                        if (canMoveTo(currentTetromino.x, currentTetromino.y, newRotation)) {
                            currentTetromino.rotation = newRotation;
                        }
                    }
                    break;
            }
        } else {
            switch (key) {
                case ' ': // Space = Hard drop
                    // Move down until collision
                    while (canMoveTo(currentTetromino.x, currentTetromino.y + 1, currentTetromino.rotation)) {
                        currentTetromino.y++;
                        score += 2; // Bonus for hard drop
                    }
                    break;
                    
                case KEY_QUIT:
                    gameOver = true;
                    break;
            }
        }
    }
}

// Update the game state
void updateGame() {
    if (paused) return; // Don't update game while paused
    
    // Check if it's time to drop the tetromino
    clock_t currentTime = clock();
    double timeDifference = (double)(currentTime - lastDropTime) / CLOCKS_PER_SEC * 1000;
    
    if (timeDifference >= dropInterval) {
        // Try to move the tetromino down
        if (canMoveTo(currentTetromino.x, currentTetromino.y + 1, currentTetromino.rotation)) {
            currentTetromino.y++;
        } else {
            // Can't move down further, lock the tetromino in place
            lockTetromino();
            
            // Check for completed lines
            int linesCleared = clearLines();
            
            // Update the score
            updateScore(linesCleared);
            
            // Get the next tetromino
            currentTetromino = nextTetromino;
            generateRandomTetromino(&nextTetromino);
            
            // Check if the new tetromino can fit
            if (!canMoveTo(currentTetromino.x, currentTetromino.y, currentTetromino.rotation)) {
                gameOver = true;
            }
        }
        
        // Reset the drop timer
        lastDropTime = currentTime;
    }
}

// Display ASCII art TETRIS title
void displayTitleScreen() {
    system("cls");
    
    setCursorPosition(20, 5);
    printf(" #######  ######  #######  ######   ###   ##### ");
    setCursorPosition(20, 6);
    printf("    #     #          #     #     #   #    #     ");
    setCursorPosition(20, 7);
    printf("    #     #####      #     ######    #    ##### ");
    setCursorPosition(20, 8);
    printf("    #     #          #     #   #     #        # ");
    setCursorPosition(20, 9);
    printf("    #     ######     #     #    #   ###   ##### ");
    
    setCursorPosition(30, 14);
    printf("Press any key to continue...");
    
    _getch(); // Wait for key press
}

// Display main menu
void displayMainMenu() {
    int selectedOption = 0;
    const int numOptions = 5;
    bool exitMenu = false;
    
    while (!exitMenu) {
        system("cls");
        
        setCursorPosition(30, 5);
        printf("TETRIS MAIN MENU");
        setCursorPosition(30, 6);
        printf("----------------");
        
        setCursorPosition(30, 8);
        printf("%s1. Start Game", (selectedOption == 0) ? ">" : " ");
        
        setCursorPosition(30, 9);
        printf("%s2. Select Level", (selectedOption == 1) ? ">" : " ");
        
        setCursorPosition(30, 10);
        printf("%s3. High Scores", (selectedOption == 2) ? ">" : " ");
        
        setCursorPosition(30, 11);
        printf("%s4. Instructions", (selectedOption == 3) ? ">" : " ");
        
        setCursorPosition(30, 12);
        printf("%s5. Exit", (selectedOption == 4) ? ">" : " ");
        
        // Get user input
        int key = _getch();
        
        // Handle arrow keys (224 followed by the actual key code)
        if (key == 224) {
            key = _getch();
            
            switch (key) {
                case 72: // Up arrow
                    selectedOption = (selectedOption > 0) ? selectedOption - 1 : numOptions - 1;
                    break;
                    
                case 80: // Down arrow
                    selectedOption = (selectedOption < numOptions - 1) ? selectedOption + 1 : 0;
                    break;
            }
        } else if (key == 13) { // Enter key
            switch (selectedOption) {
                case 0: // Start Game
                    playGame();
                    break;
                    
                case 1: // Select Level
                    displayLevelSelect();
                    break;
                    
                case 2: // High Scores
                    displayHighScores();
                    break;
                    
                case 3: // Instructions
                    displayInstructions();
                    break;
                    
                case 4: // Exit
                    exitMenu = true;
                    break;
            }
        }
    }
}

// Display level selection menu
void displayLevelSelect() {
    int selectedLevel = level - 1; // Default to current level
    const int maxLevel = 10;
    bool exitMenu = false;
    
    while (!exitMenu) {
        system("cls");
        
        setCursorPosition(30, 5);
        printf("SELECT STARTING LEVEL");
        setCursorPosition(30, 6);
        printf("--------------------");
        
        for (int i = 0; i < maxLevel; i++) {
            setCursorPosition(30, 8 + i);
            printf("%sLevel %d", (selectedLevel == i) ? ">" : " ", i + 1);
        }
        
        setCursorPosition(30, 19);
        printf("Use arrow keys to select, Enter to confirm");
        
        // Get user input
        int key = _getch();
        
        // Handle arrow keys (224 followed by the actual key code)
        if (key == 224) {
            key = _getch();
            
            switch (key) {
                case 72: // Up arrow
                    selectedLevel = (selectedLevel > 0) ? selectedLevel - 1 : maxLevel - 1;
                    break;
                    
                case 80: // Down arrow
                    selectedLevel = (selectedLevel < maxLevel - 1) ? selectedLevel + 1 : 0;
                    break;
            }
        } else if (key == 13) { // Enter key
            level = selectedLevel + 1;
            
            // Set drop interval based on level
            dropInterval = 1000 * (0.8 - ((level - 1) * 0.007));
            if (dropInterval < 100) {
                dropInterval = 100; // Don't go too fast
            }
            
            exitMenu = true;
        } else if (key == 27) { // Escape key
            exitMenu = true;
        }
    }
}

// Check if the score qualifies for the high score list
bool isHighScore(int score) {
    for (int i = 0; i < MAX_HIGH_SCORES; i++) {
        if (score > highScores[i].score) {
            return true;
        }
    }
    return false;
}

// Add a new high score
void addHighScore(int score, int level, int lines) {
    // Find the position for the new score
    int position = MAX_HIGH_SCORES - 1;
    
    for (int i = 0; i < MAX_HIGH_SCORES; i++) {
        if (score > highScores[i].score) {
            position = i;
            break;
        }
    }
    
    // Shift down lower scores
    for (int i = MAX_HIGH_SCORES - 1; i > position; i--) {
        highScores[i] = highScores[i - 1];
    }
    
    // Get player initials
    char initials[4] = "AAA";
    int currentPos = 0;
    
    system("cls");
    setCursorPosition(25, 10);
    printf("NEW HIGH SCORE: %d!", score);
    setCursorPosition(25, 12);
    printf("Enter your initials (3 letters):");
    setCursorPosition(25, 14);
    printf("_ _ _");
    
    showCursor();
    
    // Process the input
    while (currentPos < 3) {
        setCursorPosition(25 + currentPos * 2, 14);
        
        int key = _getch();
        
        if (key >= 'A' && key <= 'Z' || key >= 'a' && key <= 'z') {
            initials[currentPos] = toupper(key);
            printf("%c", initials[currentPos]);
            currentPos++;
        } else if (key == 8 && currentPos > 0) { // Backspace
            currentPos--;
            setCursorPosition(25 + currentPos * 2, 14);
            printf("_");
        } else if (key == 13 && currentPos > 0) { // Enter
            break;
        }
    }
    
    hideCursor();
    
    // Fill the rest with spaces if needed
    for (int i = currentPos; i < 3; i++) {
        initials[i] = ' ';
    }
    initials[3] = '\0';
    
    // Insert the new high score
    strcpy(highScores[position].initials, initials);
    highScores[position].score = score;
    highScores[position].level = level;
    highScores[position].lines = lines;
    
    // Save high scores
    saveHighScores();
}

// Load high scores from file
void loadHighScores() {
    FILE *file = fopen("tetris_scores.dat", "rb");
    
    // Initialize with default values if file doesn't exist
    if (file == NULL) {
        for (int i = 0; i < MAX_HIGH_SCORES; i++) {
            strcpy(highScores[i].initials, "---");
            highScores[i].score = 0;
            highScores[i].level = 0;
            highScores[i].lines = 0;
        }
        return;
    }
    
    // Read high scores from file
    fread(highScores, sizeof(HighScore), MAX_HIGH_SCORES, file);
    fclose(file);
}

// Save high scores to file
void saveHighScores() {
    FILE *file = fopen("tetris_scores.dat", "wb");
    
    if (file == NULL) {
        return;
    }
    
    fwrite(highScores, sizeof(HighScore), MAX_HIGH_SCORES, file);
    fclose(file);
}

// Display high scores
void displayHighScores() {
    system("cls");
    
    setCursorPosition(30, 3);
    printf("HIGH SCORES");
    setCursorPosition(30, 4);
    printf("-----------");
    
    setCursorPosition(20, 6);
    printf("RANK  INITIALS  SCORE   LEVEL   LINES");
    setCursorPosition(20, 7);
    printf("----------------------------------------");
    
    for (int i = 0; i < MAX_HIGH_SCORES; i++) {
        setCursorPosition(20, 8 + i);
        printf("%2d.   %3s      %6d   %2d      %3d", 
               i + 1, highScores[i].initials, highScores[i].score, 
               highScores[i].level, highScores[i].lines);
    }
    
    setCursorPosition(20, 19);
    printf("Press any key to return to the main menu...");
    
    _getch();
}

// Display game over screen
void displayGameOver() {
    system("cls");
    
    setCursorPosition(30, 8);
    printf("  GAME OVER  ");
    setCursorPosition(30, 9);
    printf(" ----------- ");
    setCursorPosition(30, 10);
    printf("             ");
    setCursorPosition(30, 11);
    printf(" FINAL SCORE ");
    setCursorPosition(30, 12);
    printf("   %6d    ", score);
    
    setCursorPosition(30, 14);
    printf(" LEVEL: %d    ", level);
    setCursorPosition(30, 15);
    printf(" LINES: %d    ", lines);
    
    setCursorPosition(30, 17);
    printf("Press any key to continue...");
    
    _getch();
    
    // Check for high score
    if (isHighScore(score)) {
        addHighScore(score, level, lines);
    }
    
    // Show high scores
    displayHighScores();
}

// Display instructions
void displayInstructions() {
    system("cls");
    
    setCursorPosition(30, 3);
    printf("HOW TO PLAY TETRIS");
    setCursorPosition(30, 4);
    printf("----------------");
    
    setCursorPosition(20, 6);
    printf("The goal is to complete horizontal lines with the falling blocks.");
    setCursorPosition(20, 7);
    printf("When a line is complete, it disappears and you earn points.");
    
    setCursorPosition(20, 9);
    printf("CONTROLS:");
    setCursorPosition(20, 10);
    printf("<- / ->: Move the tetromino left or right");
    setCursorPosition(20, 11);
    printf("^: Rotate the tetromino");
    setCursorPosition(20, 12);
    printf("v: Soft drop (move down faster)");
    setCursorPosition(20, 13);
    printf("Space: Hard drop (instantly place the tetromino)");
    setCursorPosition(20, 14);
    printf("P: Pause the game");
    setCursorPosition(20, 15);
    printf("Q: Quit the game");
    
    setCursorPosition(20, 17);
    printf("SCORING:");
    setCursorPosition(20, 18);
    printf("1 line cleared: 40 x level points");
    setCursorPosition(20, 19);
    printf("2 lines cleared: 100 x level points");
    setCursorPosition(20, 20);
    printf("3 lines cleared: 300 x level points");
    setCursorPosition(20, 21);
    printf("4 lines cleared: 1200 x level points");
    
    setCursorPosition(20, 23);
    printf("Press any key to return to the main menu...");
    
    _getch();
}

// Main game function
void playGame() {
    // Initialize the game
    score = 0;
    lines = 0;
    gameOver = false;
    paused = false;
    
    // Initialize drop interval based on level
    dropInterval = 1000 * (0.8 - ((level - 1) * 0.007));
    if (dropInterval < 100) {
        dropInterval = 100; // Don't go too fast
    }
    
    // Clear the board
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            board[y][x] = 0;
        }
    }
    
    // Generate initial tetrominoes
    generateRandomTetromino(&currentTetromino);
    generateRandomTetromino(&nextTetromino);
    
    // Set up timing
    lastDropTime = clock();
    gameStartTime = clock();
    
    // Clear screen
    system("cls");
    
    // Main game loop
    while (!gameOver) {
        processInput();
        updateGame();
        drawBoard();
        
        // Small but not blocking delay
        Sleep(10);
    }
    
    // Game over
    displayGameOver();
}

int main() {
    // Set up console
    system("color 0F"); // Black background, white text
    hideCursor();
    
    // Load high scores
    loadHighScores();
    
    // Display title screen
    displayTitleScreen();
    
    // Display main menu
    displayMainMenu();
    
    return 0;
}