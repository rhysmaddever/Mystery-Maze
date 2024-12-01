#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <algorithm> // For std::shuffle
#include <random>    // For random number generators (std::random_device, std::mt19937)
#include <conio.h>   // For _getch()

// Constants for the maze size
const int WIDTH = 30;
const int HEIGHT = 20;
char maze[HEIGHT][WIDTH];

// Directions for moving in the maze (up, right, down, left)
const int DX[] = { -2, 0, 2, 0 };
const int DY[] = { 0, 2, 0, -2 };

// Player position
int playerX = 1, playerY = 1;

// Function declarations
void initializeMaze(char maze[HEIGHT][WIDTH]);
void generateMaze(char maze[HEIGHT][WIDTH], int x, int y);
void printMaze(char maze[HEIGHT][WIDTH]);
void movePlayer(char direction);
void addRandomExit(char maze[HEIGHT][WIDTH]);

struct Enemy {
    int x, y;
    void moveRandomly() {
        // Implement logic for random movement
    }
};

int main() {
    // Initialize the maze
    initializeMaze(maze);
    generateMaze(maze, 1, 1); // Start generating the maze from (1, 1)

    // Add a random exit
    addRandomExit(maze);

    // Place the player in the starting position
    maze[playerY][playerX] = 'P';

    // Game loop
    while (true) {
        printMaze(maze);

        // Get player input
        char input = _getch(); // Wait for key press

        // Clear the current player position
        maze[playerY][playerX] = ' ';

        // Move the player
        movePlayer(toupper(input)); // Convert input to uppercase for consistency

        // Check if player reached the exit
        if (maze[playerY][playerX] == 'E') {
            std::cout << "Congratulations! You found the exit!\n";
            break;
        }

        // Update the player's position
        maze[playerY][playerX] = 'P';
    }

    return 0;
}

void initializeMaze(char maze[HEIGHT][WIDTH]) {
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            maze[i][j] = '|'; // Fill the maze with walls
        }
    }
}

void generateMaze(char maze[HEIGHT][WIDTH], int x, int y) {
    maze[y][x] = ' '; // Mark the current cell as a path

    // Create a shuffled list of directions
    std::vector<int> directions = { 0, 1, 2, 3 };
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(directions.begin(), directions.end(), g);

    for (int i = 0; i < 4; ++i) {
        int dir = directions[i];
        int nx = x + DX[dir];
        int ny = y + DY[dir];

        // Check if the new position is valid
        if (nx >= 1 && nx < WIDTH - 1 && ny >= 1 && ny < HEIGHT - 1 && maze[ny][nx] == '|') {
            maze[ny][nx] = ' '; // Create a path to the new cell
            maze[y + DY[dir] / 2][x + DX[dir] / 2] = ' '; // Carve the wall between cells
            generateMaze(maze, nx, ny); // Recursively generate the maze
        }
    }
}

void printMaze(char maze[HEIGHT][WIDTH]) {
    system("cls"); // Clear the console (Windows-specific)
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            std::cout << maze[i][j];
        }
        std::cout << std::endl;
    }
}

void movePlayer(char direction) {
    // Check the movement direction and ensure it's not a wall
    if (direction == 'W' && maze[playerY - 1][playerX] != '|') playerY--;
    else if (direction == 'S' && maze[playerY + 1][playerX] != '|') playerY++;
    else if (direction == 'A' && maze[playerY][playerX - 1] != '|') playerX--;
    else if (direction == 'D' && maze[playerY][playerX + 1] != '|') playerX++;
}

void addRandomExit(char maze[HEIGHT][WIDTH]) {
    std::vector<std::pair<int, int>> validExits;

    // Check the top and bottom rows
    for (int x = 1; x < WIDTH - 1; ++x) {
        if (maze[1][x] == ' ') validExits.push_back({ x, 0 }); // Top row
        if (maze[HEIGHT - 2][x] == ' ') validExits.push_back({ x, HEIGHT - 1 }); // Bottom row
    }

    // Check the left and right columns
    for (int y = 1; y < HEIGHT - 1; ++y) {
        if (maze[y][1] == ' ') validExits.push_back({ 0, y }); // Left column
        if (maze[y][WIDTH - 2] == ' ') validExits.push_back({ WIDTH - 1, y }); // Right column
    }

    // If no exits are found, return early
    if (validExits.empty()) return;

    // Randomly select an exit from the list
    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<int> dist(0, validExits.size() - 1);
    std::pair<int, int> chosenExit = validExits[dist(g)];

    // Mark the exit in the maze
    int exitX = chosenExit.first;
    int exitY = chosenExit.second;
    maze[exitY][exitX] = 'E';
}
