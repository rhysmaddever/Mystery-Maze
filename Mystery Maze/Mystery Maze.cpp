#include <SFML/Graphics.hpp>
#include <iostream>
#include <stack>
#include <vector>
#include <set>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <string>

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

// Obstacle positions (purple blocks)
std::vector<std::pair<int, int>> purpleBlocks;

// Enemy class to manage enemy movement and state
class Enemy {
public:
    int x, y;
    sf::Clock moveClock; // Controls movement speed
    std::set<std::pair<int, int>> visited; // Tracks visited cells
    std::stack<std::pair<int, int>> backtrackStack; // For DFS backtracking

    Enemy(int startX, int startY) : x(startX), y(startY) {
        // Mark the starting position as visited
        visited.insert({ x, y });
        backtrackStack.push({ x, y });
        srand(static_cast<unsigned>(time(0))); // Seed random number generator
    }

    void move();
};

// Function declarations for various tasks
void initializeMaze();
void generateMaze(int startX, int startY);
void placePurpleBlocks();
void drawMaze(sf::RenderWindow& window, sf::RectangleShape& wall, sf::RectangleShape& emptySpace, sf::RectangleShape& playerShape, sf::RectangleShape& enemyShape, sf::RectangleShape& exitShape, sf::RectangleShape& purpleBlockShape, Enemy& enemy);
void movePlayer(char direction);
bool isExitReached();
bool isWalkable(int x, int y);
void showMenu();
bool startGame();
bool isTooCloseToPlayer(int enemyX, int enemyY);
bool checkPurpleBlockInteraction(int x, int y);

int main() {
    if (!startGame()) {
        return 0;
    }

    srand(static_cast<unsigned int>(time(0))); // Initialize random seed for maze generation
    initializeMaze();
    generateMaze(1, 1); // Start maze generation from position (1, 1)

    // Place the purple blocks in the maze
    placePurpleBlocks();

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

    // Rectangle shapes for drawing maze tiles, player, enemy, exit, and purple blocks
    sf::RectangleShape wall(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    wall.setFillColor(sf::Color::Blue);

    sf::RectangleShape emptySpace(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    emptySpace.setFillColor(sf::Color::Black);

    sf::RectangleShape playerShape(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    playerShape.setFillColor(sf::Color::Green);

    sf::RectangleShape enemyShape(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    enemyShape.setFillColor(sf::Color::Red);

    sf::RectangleShape exitShape(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    exitShape.setFillColor(sf::Color::Yellow);  // Distinct color for the exit block

    sf::RectangleShape purpleBlockShape(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    purpleBlockShape.setFillColor(sf::Color::Magenta);

    // Main game loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                // Check if the user wants to exit by pressing '3'
                if (event.key.code == sf::Keyboard::Num3) {
                    std::cout << "Exiting game..." << std::endl;
                    window.close();
                }

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
            std::cout << "Congratulations! You've reached the exit!" << std::endl;
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

        // Check if the player interacts with a purple block
        if (checkPurpleBlockInteraction(playerX, playerY)) {
            continue;
        }

        // Clear window and redraw maze
        window.clear(sf::Color::Black);
        drawMaze(window, wall, emptySpace, playerShape, enemyShape, exitShape, purpleBlockShape, enemy);
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
                maze[y + DIRECTIONS[dir].second][x + DIRECTIONS[dir].first] = ' '; // Remove wall between cells
                cellStack.push({ nx, ny });
                moved = true;
                break;
            }
        }

        if (!moved) {
            cellStack.pop();  // Backtrack if no direction is possible
        }
    }

    // After generating the maze, place the exit
    maze[exitY][exitX] = 'E';  // Place the exit at the bottom-right corner
}


// Function to place purple blocks randomly on the maze
// Function to place purple blocks randomly on the maze
void placePurpleBlocks() {
    for (int i = 0; i < 2; ++i) {  // Change 5 to 2 to place only 2 purple blocks
        int x, y;
        do {
            x = rand() % WIDTH;
            y = rand() % HEIGHT;
        } while (maze[y][x] != ' ' || (x == playerX && y == playerY)); // Ensure not on player position
        maze[y][x] = 'P'; // Mark the purple block location
        purpleBlocks.push_back({ x, y });
    }
}

// Function to draw the maze and all elements
void drawMaze(sf::RenderWindow& window, sf::RectangleShape& wall, sf::RectangleShape& emptySpace, sf::RectangleShape& playerShape, sf::RectangleShape& enemyShape, sf::RectangleShape& exitShape, sf::RectangleShape& purpleBlockShape, Enemy& enemy) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            if (maze[y][x] == '#') {
                wall.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                window.draw(wall);  // Draw wall
            }
            else if (maze[y][x] == ' ') {
                emptySpace.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                window.draw(emptySpace);  // Draw empty space
            }
            else if (maze[y][x] == 'P') {
                purpleBlockShape.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                window.draw(purpleBlockShape);  // Draw purple block
            }
            else if (maze[y][x] == 'E') {
                exitShape.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                window.draw(exitShape);  // Draw exit
            }
        }
    }

    playerShape.setPosition(playerX * TILE_SIZE, playerY * TILE_SIZE);
    window.draw(playerShape);  // Draw player

    enemyShape.setPosition(enemy.x * TILE_SIZE, enemy.y * TILE_SIZE);
    window.draw(enemyShape);  // Draw enemy
}

// Function to move the player based on key input
void movePlayer(char direction) {
    int newX = playerX;
    int newY = playerY;

    if (direction == 'W') newY -= 1;  // Move up
    else if (direction == 'S') newY += 1;  // Move down
    else if (direction == 'A') newX -= 1;  // Move left
    else if (direction == 'D') newX += 1;  // Move right

    // Check for purple block interaction before moving
    if (maze[newY][newX] == 'P') {
        if (checkPurpleBlockInteraction(newX, newY)) {
            return; // Stop movement if the block interaction fails
        }
    }

    if (isWalkable(newX, newY)) {
        playerX = newX;
        playerY = newY;
    }
}


// Check if a cell is walkable (empty or exit)
bool isWalkable(int x, int y) {
    return maze[y][x] == ' ' || maze[y][x] == 'E';
}

// Check if the player has reached the exit
bool isExitReached() {
    return playerX == exitX && playerY == exitY;
}

// Show the game menu
void showMenu() {
    std::cout << "Welcome to the Mystery Maze Game!" << std::endl;
    std::cout << "Press 1 to Start the Game" << '\n';
    std::cout << "Press 2 to see the instructions" << '\n';
    std::cout << "Press 3 to Exit." << '\n';
}

// Function to start the game or exit based on user input
bool startGame() {
    char choice;
    showMenu();
    std::cin >> choice;
    if (choice == '1') {
        return true;
    }
    else if (choice == '2') {
        std::cout << "There are the instructions: ";
    }
    if (choice == '3') {
        return false;  // Exit the game
    }
}

// Check if the enemy is too close to the player
bool isTooCloseToPlayer(int enemyX, int enemyY) {
    return std::abs(enemyX - playerX) < 2 && std::abs(enemyY - playerY) < 2;
}

// Check for interaction with purple blocks
bool checkPurpleBlockInteraction(int x, int y) {
    for (const auto& block : purpleBlocks) {
        if (block.first == x && block.second == y) {
            int answer;
            std::cout << "Solve the puzzle: 5 + 3 = ";
            std::cin >> answer;

            if (answer == 8) {
                std::cout << "Correct! The purple block disappears." << std::endl;
                maze[y][x] = ' '; // Make the purple block disappear
                purpleBlocks.erase(std::remove(purpleBlocks.begin(), purpleBlocks.end(), block), purpleBlocks.end());
                return false; // No longer blocking
            }
            else {
                std::cout << "Incorrect. You cannot pass the purple block yet." << std::endl;
                return true; // Still blocking
            }
        }
    }
    return false; // No interaction with a purple block
}

void Enemy::move() {
    std::vector<std::pair<int, int>> neighbors;

    // Check all possible neighbors
    for (const auto& dir : DIRECTIONS) {
        int nx = x + dir.first;
        int ny = y + dir.second;

        // Add valid, unvisited neighbors
        if (isWalkable(nx, ny) && visited.find({nx, ny}) == visited.end()) {
            neighbors.push_back({nx, ny});
        }
    }

    if (!neighbors.empty()) {
        // Pick a random unvisited neighbor
        int randomIndex = rand() % neighbors.size();
        int nextX = neighbors[randomIndex].first;
        int nextY = neighbors[randomIndex].second;

        // Move to the chosen neighbor
        x = nextX;
        y = nextY;

        // Mark it as visited and push it to the backtrack stack
        visited.insert({x, y});
        backtrackStack.push({x, y});
    } else if (!backtrackStack.empty()) {
        // Backtrack if no unvisited neighbors are found
        backtrackStack.pop(); // Remove the current position
        if (!backtrackStack.empty()) {
            x = backtrackStack.top().first;
            y = backtrackStack.top().second;
        }
    }
}
