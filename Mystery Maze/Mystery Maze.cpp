#include <SFML/Graphics.hpp>
#include <iostream>
#include <stack>
#include <vector>
#include <set>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <string>

// Global game timer
sf::Clock gameTimer;
float timeLimit = 120.0f;  // 2 minutes in seconds

// Constants for maze dimensions and tile size
int height = 19;     // Maze width
int width = 15;    // Maze height, i think the width and height has to be an odd number for it to work
const int TILE_SIZE = 32; // Tile size in pixels
int level = 1;

// Directions for maze carving (up, right, down, left)
const std::vector<std::pair<int, int>> DIRECTIONS = {
    {0, -1},  // Up
    {1, 0},   // Right
    {0, 1},   // Down
    {-1, 0}   // Left
};

// Maze grid represented as a 2D character array
std::vector<std::vector<char>> maze(height, std::vector<char>(width, '#'));


// Player and exit positions
int playerX = 1, playerY = 1;
int exitX = width - 2, exitY = height - 2;

// Obstacle positions (purple blocks)
std::vector<std::pair<int, int>> purpleBlocks;

// Power-up position
int powerUpX = -1, powerUpY = -1;
bool powerUpActive = false;  // Whether the power-up is active
sf::Clock powerUpClock;      // Timer for power-up effects


class Enemy {
public:
    int x, y;
    sf::Clock moveClock; // Controls movement speed
    sf::Clock powerUpClock; // Tracks power-up freeze duration
    std::set<std::pair<int, int>> visited; // Tracks visited cells
    std::stack<std::pair<int, int>> backtrackStack; // For DFS backtracking

    Enemy(int startX, int startY) : x(startX), y(startY) {
        visited.insert({ x, y });
        backtrackStack.push({ x, y });
        srand(static_cast<unsigned>(time(0))); // Seed random number generator
    }

    void move();
};


// Function declarations
void initializeMaze();
void generateMaze(int startX, int startY);
void placePurpleBlocks();
void drawMaze(sf::RenderWindow& window, sf::RectangleShape& wall, sf::RectangleShape& emptySpace, sf::RectangleShape& playerShape, sf::RectangleShape& enemyShape, sf::RectangleShape& exitShape, sf::RectangleShape& purpleBlockShape, Enemy& enemy, sf::Text& timerText);
void movePlayer(char direction);
bool isExitReached();
bool isWalkable(int x, int y);
void showMenu();
bool startGame();
bool isTooCloseToPlayer(int enemyX, int enemyY);
bool checkPurpleBlockInteraction(int x, int y);
void updateTimerText(sf::Text& timerText);
void placePowerUp();
void collectPowerUp();
void showPostLevelMenu();
void prepareNextLevel();


bool levelCompleted = false;

int main() {
    if (!startGame()) {
        return 0;
    }


    srand(static_cast<unsigned int>(time(0))); // Initialize random seed for maze generation
    initializeMaze();
    generateMaze(1, 1); // Start maze generation from position (1, 1)

    // Place purple blocks
    placePurpleBlocks();

    placePowerUp();


    // Set initial enemy position
    int enemyStartX = width - 3;
    int enemyStartY = height - 3;
    while (maze[enemyStartY][enemyStartX] == '#' || (enemyStartX == playerX && enemyStartY == playerY) || isTooCloseToPlayer(enemyStartX, enemyStartY)) {
        enemyStartX = rand() % width;
        enemyStartY = rand() % height;
    }
    Enemy enemy(enemyStartX, enemyStartY);

    enemy.moveClock.restart(); // Reset move clock as soon as the enemy is created

    // SFML window setup
    sf::RenderWindow window(sf::VideoMode(width * TILE_SIZE, height * TILE_SIZE), "Mystery Maze Game");

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
    exitShape.setFillColor(sf::Color::Yellow);

    sf::RectangleShape purpleBlockShape(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    purpleBlockShape.setFillColor(sf::Color::Magenta);

    // Load font
    sf::Font font;
    if (!font.loadFromFile("C:/Users/Rhys/Documents/GitHub/Mystery-Maze/arial.ttf")) {
        std::cerr << "Error loading font!" << std::endl;
        return 1;  // Exit the game if font can't be loaded
    }

    // Create timer text
    sf::Text timerText;
    timerText.setFont(font);
    timerText.setCharacterSize(24);  // Set an appropriate font size
    timerText.setFillColor(sf::Color::White);

    // Position the timer text slightly from the top-right corner
    timerText.setPosition(width * TILE_SIZE - 150, 10); // Initial placement


    // Main game loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
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
            showPostLevelMenu();
            prepareNextLevel();
        }

        // Check if the enemy caught the player
        if (enemy.x == playerX && enemy.y == playerY) {
            std::cout << "Game Over! The enemy caught you!" << std::endl;
            window.close();
        }

        // Slow down enemy movement
        if (enemy.moveClock.getElapsedTime().asSeconds() > 0.5f) {
            enemy.move();
            enemy.moveClock.restart();
        }

        // Inside the game loop (in main)
        if (powerUpClock.getElapsedTime().asSeconds() >= 5.0f) {
            // Reset enemy freeze state after the duration
            powerUpClock.restart();  // Restart or stop freezing the enemy
        }

        // Inside the main game loop
        sf::Time elapsedTime = gameTimer.getElapsedTime();
        float remainingTime = timeLimit - elapsedTime.asSeconds();

        if (remainingTime <= 0.0f) {
            std::cout << "Time's up! Game Over!" << std::endl;
            window.close();
        }

        // Update the timer and display it
        updateTimerText(timerText);

        // Clear window and redraw maze
        window.clear(sf::Color::Black);
        drawMaze(window, wall, emptySpace, playerShape, enemyShape, exitShape, purpleBlockShape, enemy, timerText);
        window.display();

        if (levelCompleted) {
            // Handle post-level menu here
            showPostLevelMenu();
            levelCompleted = false;
        }
    }

    return 0;
}

// Initialize the maze with walls ('#')
void initializeMaze() {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            maze[i][j] = '#'; // Initialize all cells as walls
        }
    }
}

void generateMaze(int startX, int startY) {
    std::stack<std::pair<int, int>> cellStack;
    maze[startY][startX] = ' ';
    cellStack.push({ startX, startY });

    while (!cellStack.empty()) {
        int x = cellStack.top().first;
        int y = cellStack.top().second;
        std::vector<int> directions = { 0, 1, 2, 3 };
        std::random_shuffle(directions.begin(), directions.end());

        bool moved = false;
        for (int dir : directions) {
            int nx = x + DIRECTIONS[dir].first * 2;
            int ny = y + DIRECTIONS[dir].second * 2;

            if (nx >= 0 && nx < width && ny >= 0 && ny < height && maze[ny][nx] == '#') {
                maze[ny][nx] = ' ';
                maze[y + DIRECTIONS[dir].second][x + DIRECTIONS[dir].first] = ' ';
                cellStack.push({ nx, ny });
                moved = true;
                break;
            }
        }

        if (!moved) {
            cellStack.pop();
        }
    }

    maze[exitY][exitX] = 'E';
}

// Function to place exactly two purple blocks randomly on the maze
void placePurpleBlocks() {
    while (purpleBlocks.size() < 2) { // Limit to 2 blocks
        int x = rand() % width;
        int y = rand() % height;

        // Ensure the block is placed on a walkable cell and not overlapping existing blocks
        if (maze[y][x] == ' ' && std::find(purpleBlocks.begin(), purpleBlocks.end(), std::make_pair(x, y)) == purpleBlocks.end()) {
            purpleBlocks.push_back({ x, y });
            maze[y][x] = 'P'; // Mark the block in the maze
        }
    }
}

// Function to place the power-up in the maze at a random walkable position
void placePowerUp() {
    while (true) {
        int x = rand() % width;
        int y = rand() % height;

        // Ensure the power-up is on a walkable tile, not overlapping purple blocks, exit, or the player
        if (maze[y][x] == ' ' && !(x == playerX && y == playerY) &&
            !(x == exitX && y == exitY) &&
            std::find(purpleBlocks.begin(), purpleBlocks.end(), std::make_pair(x, y)) == purpleBlocks.end()) {
            powerUpX = x;
            powerUpY = y;
            powerUpActive = true;  // Activate the power-up
            break;
        }
    }
}



// Function to draw the maze and game objects on the screen
void drawMaze(sf::RenderWindow& window, sf::RectangleShape& wall, sf::RectangleShape& emptySpace, sf::RectangleShape& playerShape, sf::RectangleShape& enemyShape, sf::RectangleShape& exitShape, sf::RectangleShape& purpleBlockShape, Enemy& enemy, sf::Text& timerText) {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (maze[i][j] == '#') {
                wall.setPosition(j * TILE_SIZE, i * TILE_SIZE);
                window.draw(wall);
            }
            else if (maze[i][j] == ' ') {
                emptySpace.setPosition(j * TILE_SIZE, i * TILE_SIZE);
                window.draw(emptySpace);
            }
            else if (maze[i][j] == 'E') {
                exitShape.setPosition(j * TILE_SIZE, i * TILE_SIZE);
                window.draw(exitShape);
            }
        }
    }

    // Draw the player and enemy
    playerShape.setPosition(playerX * TILE_SIZE, playerY * TILE_SIZE);
    window.draw(playerShape);

    enemyShape.setPosition(enemy.x * TILE_SIZE, enemy.y * TILE_SIZE);
    window.draw(enemyShape);

    // Draw purple blocks
    for (const auto& block : purpleBlocks) {
        purpleBlockShape.setPosition(block.first * TILE_SIZE, block.second * TILE_SIZE);
        window.draw(purpleBlockShape);
    }

    if (powerUpActive) {
        sf::RectangleShape powerUpShape(sf::Vector2f(TILE_SIZE, TILE_SIZE));
        powerUpShape.setFillColor(sf::Color::Cyan);  // Cyan for power-up
        powerUpShape.setPosition(powerUpX * TILE_SIZE, powerUpY * TILE_SIZE);
        window.draw(powerUpShape);
    }


    // Draw timer text
    window.draw(timerText);
}

// Function to update the timer text
void updateTimerText(sf::Text& timerText) {
    float elapsedTime = gameTimer.getElapsedTime().asSeconds();
    float remainingTime = timeLimit - elapsedTime;  // Use variable time limit
    if (remainingTime < 0) {
        remainingTime = 0;
    }
    int minutes = static_cast<int>(remainingTime) / 60;
    int seconds = static_cast<int>(remainingTime) % 60;

    timerText.setString("Time Remaining: " + std::to_string(minutes) + ":" +
        (seconds < 10 ? "0" : "") + std::to_string(seconds));
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

    // Check if the player collected the power-up
    if (powerUpActive && newX == powerUpX && newY == powerUpY) {
        collectPowerUp();
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
    bool validChoice = false;

    // Show the menu until a valid option is entered
    while (!validChoice) {
        showMenu();
        std::cin >> choice;

        // Check if the input is valid (either '1', '2', or '3')
        if (choice == '1') {
            validChoice = true;  // Valid choice to start the game
            return true;
        }
        else if (choice == '2') {
            validChoice = true;  // Option for instructions or other
            // Optionally, you can display instructions here
            std::cout << "Instructions:\n";
            std::cout << "Use WASD to move. Avoid the enemy and solve puzzles!\n";
            return true;  // Returning true to proceed with game start (or any other function)
        }
        else if (choice == '3') {
            validChoice = true;  // Exit the game
            return false;  // Exiting the game
        }
        else {
            std::cout << "Invalid choice. Please enter 1, 2, or 3." << std::endl;
        }
    }
}


// Check if the enemy is too close to the player
bool isTooCloseToPlayer(int enemyX, int enemyY) {
    return std::abs(enemyX - playerX) < 2 && std::abs(enemyY - playerY) < 2;
}

// Function to check purple block interaction
bool checkPurpleBlockInteraction(int x, int y) {
    for (const auto& block : purpleBlocks) {
        if (block.first == x && block.second == y) {
            int attempts = 3; // Player gets three attempts
            int answer;
            bool passed = false;

            while (attempts > 0) {
                std::cout << "Solve the puzzle to pass: What is 5 + 3? ";
                std::cin >> answer;

                if (answer == 8) {
                    std::cout << "Correct! The purple block disappears." << std::endl;
                    maze[y][x] = ' '; // Make the purple block disappear
                    purpleBlocks.erase(std::remove(purpleBlocks.begin(), purpleBlocks.end(), block), purpleBlocks.end());
                    passed = true;
                    break;
                }
                else {
                    attempts--;
                    if (attempts > 0) {
                        std::cout << "Incorrect! You have " << attempts << " attempt(s) remaining." << std::endl;
                    }
                    else {
                        std::cout << "Incorrect! You have no attempts left. Game Over!" << std::endl;
                        exit(0); // End the game
                    }
                }
            }

            return !passed; // Return true if the block is still blocking
        }
    }
    return false; // No interaction with a purple block
}

// Function to collect the power-up and apply a random effect
// Function to collect the power-up and apply a random effect
void collectPowerUp() {
    // Randomize the effect
    int effect = rand() % 3;  // 0 = freeze enemy, 1 = extra time, 2 = teleport player

    // Declare validTeleport before the switch statement
    bool validTeleport = false;

    switch (effect) {
    case 0:  // Freeze enemy for 5 seconds
        std::cout << "Power-Up: Enemy frozen for 5 seconds!" << std::endl;
        powerUpClock.restart();  // Restart the power-up timer
        break;

    case 1:  // Extra time
        std::cout << "Power-Up: Time extended by 30 seconds!" << std::endl;
        gameTimer.restart();  // Restart the game timer with adjusted remaining time
        timeLimit += 30;     // Add 30 seconds to the time limit
        break;

    case 2:  // Teleport player
        std::cout << "Power-Up: Teleporting to a new position!" << std::endl;

        while (!validTeleport) {
            int newX = rand() % width;
            int newY = rand() % height;

            // Ensure the teleport position is walkable and not near the enemy
            if (isWalkable(newX, newY) && !isTooCloseToPlayer(newX, newY)) {
                playerX = newX;
                playerY = newY;
                validTeleport = true;
            }
        }
        break;

    default:
        std::cerr << "Unknown power-up effect!" << std::endl;
        break;
    }

    // Deactivate the power-up
    powerUpActive = false;
    powerUpX = -1;
    powerUpY = -1;
}




void Enemy::move() {
    // If the power-up effect is active, freeze the enemy
    if (powerUpClock.getElapsedTime().asSeconds() < 5.0f) {
        return; // Skip movement while power-up is active
    }

    std::vector<std::pair<int, int>> neighbors;

    // Check all possible neighbors
    for (const auto& dir : DIRECTIONS) {
        int nx = x + dir.first;
        int ny = y + dir.second;

        // Add valid, unvisited neighbors
        if (isWalkable(nx, ny) && visited.find({ nx, ny }) == visited.end()) {
            neighbors.push_back({ nx, ny });
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
        visited.insert({ x, y });
        backtrackStack.push({ x, y });
    }
    else if (!backtrackStack.empty()) {
        // Backtrack if no unvisited neighbors are found
        backtrackStack.pop(); // Remove the current position
        if (!backtrackStack.empty()) {
            x = backtrackStack.top().first;
            y = backtrackStack.top().second;
        }
    }
}

void showPostLevelMenu() {
    std::cout << "Congratulations! You've completed Level 1." << std::endl;
    std::cout << "Would you like to continue to Level 2?. y for yes, n for no" << std::endl;
    char choice;
    while (true) {
        std::cin >> choice;
        if (choice == 'Y' || choice == 'y') {
            std::cout << "Continuing to Level 2..." << std::endl;
            break;
        }
        else if (choice == 'N' || choice == 'n') {
            std::cout << "Exiting game..." << std::endl;
            
            break;
        }
        else {
            std::cout << "Invalid input. Please enter Y for Yes or N for No." << std::endl;
        }
    }
}

void prepareNextLevel() {
    level++; // Increment level

    // Adjust time limit for harder levels
    timeLimit = 120.0f - (level * 10);  // For example, 10 seconds less per level

    // Regenerate the maze
    initializeMaze();
    generateMaze(1, 1); // Starting point of maze

    // Optionally, add more enemies based on the level number
    int numberOfEnemies = level; // For example, 1 enemy on level 1, 2 on level 2, etc.
    for (int i = 0; i < numberOfEnemies; ++i) {
        int enemyStartX = rand() % width;
        int enemyStartY = rand() % height;
        while (maze[enemyStartY][enemyStartX] == '#' || (enemyStartX == playerX && enemyStartY == playerY)) {
            enemyStartX = rand() % width;
            enemyStartY = rand() % height;
        }
        Enemy enemy(enemyStartX, enemyStartY);
        // Setup the enemy for the level
    }

    // Place the exit, purple blocks, and power-ups
    placePurpleBlocks();
    placePowerUp();

    // Set the new exit position
    exitX = width - 2;
    exitY = height - 2;

    // Optionally, make power-ups more rare or introduce new ones
}





//TODO - Fix user input for menu, make clock add 30 seconds and not reset it, add levels, add scoring system, add save and load game
//TODO UPDATED - new level works but player doesn't spawn at top left of maze, fix position of timer, 
