#include <SFML/Graphics.hpp>
#include <iostream>
#include <stack>
#include <vector>
#include <set>
#include <ctime>
#include <cstdlib>
#include <cmath>

// Constants for maze dimensions and tile size
const int WIDTH = 29;     // Maze width
const int HEIGHT = 15;    // Maze height
const int TILE_SIZE = 32; // Tile size in pixels

// Directions for maze carving (up, right, down, left)
const std::vector<std::pair<int, int>> DIRECTIONS = {
    {0, -1},  // Up
    {1, 0},   // Right
    {0, 1},   // Down
    {-1, 0}   // Left
};

// Maze grid represented as a 2D character array
char maze[HEIGHT][WIDTH];

// Player and exit positions
int playerX = 1, playerY = 1;
int exitX = WIDTH - 2, exitY = HEIGHT - 2;

// Enemy class to manage enemy movement and state
class Enemy {
public:
    int x, y;
    std::stack<std::pair<int, int>> moveHistory; // For backtracking enemy's movement
    std::set<std::pair<int, int>> visited;      // Tracks cells visited by the enemy
    sf::Clock moveClock;  // Clock to control the enemy's movement speed

    Enemy(int startX, int startY) : x(startX), y(startY) {
        visited.insert({ x, y }); // Mark the enemy's start position as visited
    }

    void move(); // Function for enemy movement
};

// Function declarations for various tasks
void initializeMaze();
void generateMaze(int startX, int startY);
void drawMaze(sf::RenderWindow& window, sf::RectangleShape& wall, sf::RectangleShape& emptySpace, sf::RectangleShape& playerShape, sf::RectangleShape& enemyShape, sf::RectangleShape& exitShape, Enemy& enemy);
void movePlayer(char direction);
bool isExitReached();
bool isWalkable(int x, int y);
void showMenu();
bool startGame();
bool isTooCloseToPlayer(int enemyX, int enemyY);

int main() {
    if (!startGame()) {
        return 0;
    }

    srand(static_cast<unsigned int>(time(0))); // Initialize random seed for maze generation
    initializeMaze();
    generateMaze(1, 1); // Start maze generation from position (1, 1)

    // Set initial enemy position
    int enemyStartX = WIDTH - 3;
    int enemyStartY = HEIGHT - 3;

    // Ensure the enemy does not spawn too close to the player
    while (maze[enemyStartY][enemyStartX] == '#' || (enemyStartX == playerX && enemyStartY == playerY) || isTooCloseToPlayer(enemyStartX, enemyStartY)) {
        enemyStartX = rand() % WIDTH;
        enemyStartY = rand() % HEIGHT;
    }

    // Create enemy object
    Enemy enemy(enemyStartX, enemyStartY);

    // SFML window setup
    sf::RenderWindow window(sf::VideoMode(WIDTH * TILE_SIZE, HEIGHT * TILE_SIZE), "Mystery Maze Game");

    // Rectangle shapes for drawing maze tiles, player, enemy, and exit
    sf::RectangleShape wall(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    wall.setFillColor(sf::Color::Blue);

    sf::RectangleShape emptySpace(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    emptySpace.setFillColor(sf::Color::Black);

    sf::RectangleShape playerShape(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    playerShape.setFillColor(sf::Color::Green);

    sf::RectangleShape enemyShape(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    enemyShape.setFillColor(sf::Color::Red);

    sf::RectangleShape exitShape(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    exitShape.setFillColor(sf::Color::Yellow);

    // Main game loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::W) {
                    movePlayer('W'); // Move player up
                }
                else if (event.key.code == sf::Keyboard::S) {
                    movePlayer('S'); // Move player down
                }
                else if (event.key.code == sf::Keyboard::A) {
                    movePlayer('A'); // Move player left
                }
                else if (event.key.code == sf::Keyboard::D) {
                    movePlayer('D'); // Move player right
                }
            }
        }

        // Check if the player reached the exit
        if (isExitReached()) {
            std::cout << "Congratulations! You've found the exit!" << std::endl;
            window.close();
        }

        // Check if the enemy caught the player
        if (enemy.x == playerX && enemy.y == playerY) {
            std::cout << "Game Over! The enemy caught you!" << std::endl;
            window.close();
        }

        // Slow down enemy by checking elapsed time
        if (enemy.moveClock.getElapsedTime().asSeconds() > 0.5f) {  // 0.5 seconds delay between moves
            enemy.move(); // Move enemy
            enemy.moveClock.restart();  // Reset the clock after each move
        }

        // Clear window and redraw maze
        window.clear(sf::Color::Black);
        drawMaze(window, wall, emptySpace, playerShape, enemyShape, exitShape, enemy);
        window.display();
    }

    return 0;
}

// Initialize the maze with walls ('#')
void initializeMaze() {
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            maze[i][j] = '#'; // Initialize all cells as walls
        }
    }
}

// Recursive backtracking maze generation
void generateMaze(int startX, int startY) {
    std::stack<std::pair<int, int>> cellStack;  // Stack to track path
    maze[startY][startX] = ' ';  // Mark starting cell as empty space
    cellStack.push({ startX, startY });

    // Generate maze by visiting cells randomly
    while (!cellStack.empty()) {
        int x = cellStack.top().first;
        int y = cellStack.top().second;

        // Randomize direction order
        std::vector<int> directions = { 0, 1, 2, 3 };
        std::random_shuffle(directions.begin(), directions.end());

        bool moved = false;
        for (int dir : directions) {
            int nx = x + DIRECTIONS[dir].first * 2;  // Calculate next x
            int ny = y + DIRECTIONS[dir].second * 2; // Calculate next y

            // Check if the next cell is within bounds and unvisited
            if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT && maze[ny][nx] == '#') {
                maze[ny][nx] = ' ';  // Mark new cell as empty
                maze[y + DIRECTIONS[dir].second][x + DIRECTIONS[dir].first] = ' ';  // Remove wall between cells
                cellStack.push({ nx, ny });  // Push to stack for further exploration
                moved = true;
                break;
            }
        }

        if (!moved) {
            cellStack.pop();  // Backtrack if no valid moves are available
        }
    }

    maze[exitY][exitX] = 'E'; // Place the exit at the bottom right
}

// Move player in the specified direction
void movePlayer(char direction) {
    if (direction == 'W' && isWalkable(playerX, playerY - 1)) playerY--;
    else if (direction == 'S' && isWalkable(playerX, playerY + 1)) playerY++;
    else if (direction == 'A' && isWalkable(playerX - 1, playerY)) playerX--;
    else if (direction == 'D' && isWalkable(playerX + 1, playerY)) playerX++;
}

// Draw the maze, player, enemy, and exit on the window
void drawMaze(sf::RenderWindow& window, sf::RectangleShape& wall, sf::RectangleShape& emptySpace, sf::RectangleShape& playerShape, sf::RectangleShape& enemyShape, sf::RectangleShape& exitShape, Enemy& enemy) {
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            sf::RectangleShape* tile = &emptySpace; // Default to empty space

            if (maze[i][j] == '#') {
                tile = &wall; // Wall
            }
            else if (i == playerY && j == playerX) {
                tile = &playerShape; // Player
            }
            else if (i == enemy.y && j == enemy.x) {
                tile = &enemyShape; // Enemy
            }
            else if (maze[i][j] == 'E') {
                tile = &exitShape; // Exit
            }

            tile->setPosition(j * TILE_SIZE, i * TILE_SIZE); // Set tile position
            window.draw(*tile); // Draw tile
        }
    }
}

// Check if the player has reached the exit
bool isExitReached() {
    return playerX == exitX && playerY == exitY;
}

// Check if the position is walkable (either empty space or exit)
bool isWalkable(int x, int y) {
    return maze[y][x] == ' ' || maze[y][x] == 'E';
}

// Show the game menu options
void showMenu() {
    std::cout << "1. Start Game\n2. Instructions\n3. Quit\n";
}

// Start the game by selecting a menu option
bool startGame() {
    int choice;
    showMenu();
    std::cin >> choice;

    if (choice == 1) {
        return true;
    }
    else if (choice == 3) {
        return false;
    }

    std::cout << "Invalid choice. Try again.\n";
    return startGame();
}

// Check if the enemy is too close to the player (less than 5 tiles away)
bool isTooCloseToPlayer(int enemyX, int enemyY) {
    return std::abs(enemyX - playerX) < 5 && std::abs(enemyY - playerY) < 5;
}

// Enemy movement logic, including backtracking
void Enemy::move() {
    for (auto dir : DIRECTIONS) {
        int nx = x + dir.first;
        int ny = y + dir.second;

        if (isWalkable(nx, ny) && visited.find({ nx, ny }) == visited.end()) {
            moveHistory.push({ x, y }); // Record current position for backtracking
            x = nx;
            y = ny;
            visited.insert({ x, y });
            return;
        }
    }

    // If no unvisited move is available, backtrack
    if (!moveHistory.empty()) {
        auto prev = moveHistory.top();
        moveHistory.pop();
        x = prev.first;
        y = prev.second;
    }
}
