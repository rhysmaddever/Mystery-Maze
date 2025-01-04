#include <SFML/Graphics.hpp>
#include <iostream>
#include <stack>
#include <vector>
#include <set>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <string>
#include <algorithm>
#include <fstream>
#include <iostream>

//#define DEFINE_FIELD(fieldname, value_t, obis, field_t, field_args) \
//  struct fieldname : field_t<fieldname, ##field_args> { \
//    value_t fieldname; \
//    bool fieldname ## _present = false; \
//    static constexpr ObisId id = obis; \
//    static constexpr char name_progmem[] DSMR_PROGMEM = #fieldname; \
//    static constexpr const __FlashStringHelper *name() { \
//      return reinterpret_cast<const __FlashStringHelper*>(&name_progmem); \
//    } \
//    value_t& val() { return fieldname; } \
//    bool& present() { return fieldname ## _present; } \
//  }


// Global game timer
sf::Clock gameTimer;
float timeLimit = 120.0f;  // 2 minutes in seconds

// Constants for maze dimensions and tile size
int height = 21;     // Maze width
int width = 21;    // Maze height
int tile_size = 32; // Tile size in pixels
int level = 1;
//GameState loadedState;

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
struct AdditionQuestion {
    int num1;
    int num2;
    int correctAnswer;

    std::string toString() const {
        return "What is " + std::to_string(num1) + " + " + std::to_string(num2) + "? ";
    }
};
struct GameState {
    int playerX;
    int playerY;
    int level;
    // Add other relevant game state variables
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
AdditionQuestion generateRandomAdditionQuestion();
void readLevelAndTimer(std::ifstream& infile);
void writeLevelAndTimer(std::ofstream& outfile);
void saveGame(const GameState& gameState, const std::string& filename);
bool loadGame(GameState& gameState, const std::string& filename);
void saveGame();
void loadGame();
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
    sf::RenderWindow window(sf::VideoMode(width * tile_size, height * tile_size), "Mystery Maze Game");

    // Rectangle shapes for drawing maze tiles, player, enemy, exit, and purple blocks
    sf::RectangleShape wall(sf::Vector2f(tile_size, tile_size));
    wall.setFillColor(sf::Color::Blue);

    sf::RectangleShape emptySpace(sf::Vector2f(tile_size, tile_size));
    emptySpace.setFillColor(sf::Color::Black);

    sf::RectangleShape playerShape(sf::Vector2f(tile_size, tile_size));
    playerShape.setFillColor(sf::Color::Green);

    sf::RectangleShape enemyShape(sf::Vector2f(tile_size, tile_size));
    enemyShape.setFillColor(sf::Color::Red);

    sf::RectangleShape exitShape(sf::Vector2f(tile_size, tile_size));
    exitShape.setFillColor(sf::Color::Yellow);

    sf::RectangleShape purpleBlockShape(sf::Vector2f(tile_size, tile_size));
    purpleBlockShape.setFillColor(sf::Color::Magenta);

    // Load font
    sf::Font font;

    // First attempt: Absolute path
    if (font.loadFromFile("C:/Users/Rhys/Documents/GitHub/Mystery-Maze/Release/assets/arial.ttf")) {
        std::cout << "Font loaded from absolute path!" << std::endl;
    }
    // Second attempt: Relative path
    else if (font.loadFromFile("MysteryMaze_redistribute/assets/arial.ttf")) {
        std::cout << "Font loaded from relative path!" << std::endl;
    }
    // If both fail, log an error and exit
    else {
        std::cerr << "Error: Failed to load font" << std::endl;
        return 1;  // Exit the game
    }

    // Create timer text
    sf::Text timerText;
    timerText.setFont(font);
    timerText.setCharacterSize(20);  // Set an appropriate font size
    timerText.setFillColor(sf::Color::White);

    // Position the timer text slightly from the top-right corner
    // Get the width of the window and adjust for timer text width
    // Calculate position for the top-right corner of the window
    // Calculate text bounds to prevent cutoff
    // Position the timer text slightly from the top-right corner
    timerText.setPosition(width * tile_size - 200, 12); // Initial placement

    // Main game loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::J) {
                    saveGame();
                    std::cout << "Game saved!" << std::endl;
                }
                else if (event.key.code == sf::Keyboard::L) {
                    loadGame();
                    std::cout << "Game loaded." << std::endl;
                }
            }
            if (event.type == sf::Event::Closed) {
                // Calculate and display elapsed time when the user closes the window
                float elapsedTime = gameTimer.getElapsedTime().asSeconds();
                int minutes = static_cast<int>(elapsedTime) / 60;
                int seconds = static_cast<int>(elapsedTime) % 60;
                std::cout << "Game exited! Elapsed time: " << minutes << " minutes and "
                    << seconds << " seconds." << std::endl;

                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Num3) {
                    // User pressed '3' to exit the game
                    float elapsedTime = gameTimer.getElapsedTime().asSeconds();
                    int minutes = static_cast<int>(elapsedTime) / 60;
                    int seconds = static_cast<int>(elapsedTime) % 60;
                    std::cout << "Game exited! Elapsed time: " << minutes << " minutes and "
                        << seconds << " seconds." << std::endl << "You reached level " << level << '\n' << "Your player position is: " << playerX << ' ' << playerY; //player x is across (width)
                    // player y position is down (rows)

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

            // Display elapsed time before exiting
            float elapsedTime = gameTimer.getElapsedTime().asSeconds();
            int minutes = static_cast<int>(elapsedTime) / 60;
            int seconds = static_cast<int>(elapsedTime) % 60;
            std::cout << "Elapsed time: " << minutes << " minutes and "
                << seconds << " seconds." << std::endl;

            window.close();
        }

        // Slow down enemy movement
        if (enemy.moveClock.getElapsedTime().asSeconds() > 0.5f) {
            enemy.move();
            enemy.moveClock.restart();
        }

        sf::Time elapsedTime = gameTimer.getElapsedTime();
        float remainingTime = timeLimit - elapsedTime.asSeconds();

        if (remainingTime <= 0.0f) {
            std::cout << "Time's up! Game Over!" << std::endl;

            // Display elapsed time before exiting
            float elapsedTime = gameTimer.getElapsedTime().asSeconds();
            int minutes = static_cast<int>(elapsedTime) / 60;
            int seconds = static_cast<int>(elapsedTime) % 60;
            std::cout << "Elapsed time: " << minutes << " minutes and "
                << seconds << " seconds." << std::endl;

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

    // Reading from file
    //std::ifstream infile("game_data.txt");

    //if (infile.is_open()) {
    //    readLevelAndTimer(infile);
    //    infile.close();
    //}
    //else {
    //    std::cerr << "Unable to open file" << std::endl;
    //    return 1;
    //}

    //// Writing to file
    //std::ofstream outfile("game_data.txt");

    //if (outfile.is_open()) {
    //    writeLevelAndTimer(outfile);
    //    outfile.close();
    //}
    //else {
    //    std::cerr << "Unable to open file for writing" << std::endl;
    //    return 1;
    //}

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
                wall.setPosition(j * tile_size, i * tile_size);
                window.draw(wall);
            }
            else if (maze[i][j] == ' ') {
                emptySpace.setPosition(j * tile_size, i * tile_size);
                window.draw(emptySpace);
            }
            else if (maze[i][j] == 'E') {
                exitShape.setPosition(j * tile_size, i * tile_size);
                window.draw(exitShape);
            }
        }
    }

    // Draw the player and enemy
    playerShape.setPosition(playerX * tile_size, playerY * tile_size);
    window.draw(playerShape);

    enemyShape.setPosition(enemy.x * tile_size, enemy.y * tile_size);
    window.draw(enemyShape);

    // Draw purple blocks
    for (const auto& block : purpleBlocks) {
        purpleBlockShape.setPosition(block.first * tile_size, block.second * tile_size);
        window.draw(purpleBlockShape);
    }

    if (powerUpActive) {
        sf::RectangleShape powerUpShape(sf::Vector2f(tile_size, tile_size));
        powerUpShape.setFillColor(sf::Color::Cyan);  // Cyan for power-up
        powerUpShape.setPosition(powerUpX * tile_size, powerUpY * tile_size);
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
    std::cout << "Press j to save game state" << '\n';
    std::cout << "Press l to load game" << '\n';
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
            // Display instructions
            std::cout << "Instructions:\n";
            std::cout << "Welcome to the maze game! Here are the controls and objectives:\n\n";

            std::cout << "Controls:\n";
            std::cout << "  Use the 'W', 'A', 'S', 'D' keys to move your character:\n";
            std::cout << "    - 'W' to move up\n";
            std::cout << "    - 'A' to move left\n";
            std::cout << "    - 'S' to move down\n";
            std::cout << "    - 'D' to move right\n";

            std::cout << "\nObjective:\n";
            std::cout << "  - You need to reach the yellow block at the bottom-right corner of the maze to exit.\n";
            std::cout << "  - Avoid the red enemy blocks as they will cause you to lose.\n";
            std::cout << "  - Collect the blue (cyan) power-up blocks for extra abilities.\n";
            std::cout << "  - The purple blocks are obstacles that will block your path.\n";

            std::cout << "\nTime Limit:\n";
            std::cout << "  - You have 2 minutes to complete each level!\n";

            std::cout << "\nGood luck! Stay sharp, and remember to avoid the enemy and reach the exit!\n";

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
            AdditionQuestion question = generateRandomAdditionQuestion();

            int attempts = 3; // Player gets three attempts
            bool passed = false;

            while (attempts > 0) {
                std::cout << "Solve the puzzle to pass: " << question.toString() << std::endl;
                int answer;
                std::cin >> answer;

                if (answer == question.correctAnswer) {
                    std::cout << "Correct! The purple block disappears." << std::endl;
                    maze[y][x] = ' ';
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
void collectPowerUp() {
    // Randomize the effect
    int effect = rand() % 3;  // 0 = freeze enemy, 1 = extra time, 2 = teleport player

    // Declare validTeleport before the switch statement
    bool validTeleport = false;

    switch (effect) {

    case 0:  // Extra time
        std::cout << "Power-Up: Time extended by 30 seconds!" << std::endl;
        gameTimer.restart();  // Restart the game timer with adjusted remaining time
        timeLimit += 30;     // Add 30 seconds to the time limit
        break;

    case 1:  // Teleport player
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
    std::cout << "Congratulations! You've completed Level 1.\n \n" << std::endl;
    char choice;
    bool validTeleport = false;
    while (true) {
        std::cout << "Do you want to continue to level 2? y for yes, n for no\n \n" << std::endl;
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
    // Increment the level
    level++;

    // Increase maze dimensions proportionally
    int increaseAmount = 4;
    height += increaseAmount;
    width += increaseAmount;

    // Recalculate tile size based on the new dimensions
    tile_size = std::min(850 / width, 650 / height);  // Adjust these values as needed

    // Resize the window
    sf::RenderWindow window(sf::VideoMode(width * tile_size, height * tile_size), "Mystery Maze Game");

    // Reset maze
    maze = std::vector<std::vector<char>>(height, std::vector<char>(width, '#'));

    // Reset player position to top-left corner
    playerX = 1;
    playerY = 1;

    // Reset exit position to bottom-right corner of the new maze
    exitX = width - 2;
    exitY = height - 2;

    // Reinitialize and regenerate the maze
    initializeMaze();
    generateMaze(1, 1);

    // Place purple blocks for the new level
    purpleBlocks.clear();  // Clear old blocks
    placePurpleBlocks();

    // Place power-up in a valid location
    placePowerUp();

    // Reset enemy position
    int enemyStartX = width - 3;
    int enemyStartY = height - 3;

    while (maze[enemyStartY][enemyStartX] == '#' || (enemyStartX == playerX && enemyStartY == playerY) || isTooCloseToPlayer(enemyStartX, enemyStartY)) {
        enemyStartX = rand() % width;
        enemyStartY = rand() % height;
    }
    // Create a new enemy at the new position
    Enemy enemy(enemyStartX, enemyStartY);

    // Reset the game timer for the new level
    gameTimer.restart();
    timeLimit = 120.0f;  // Reset time limit if desired
}

AdditionQuestion generateRandomAdditionQuestion() {
    static const std::vector<int> numbers = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    static const std::vector<int> maxSum = { 5, 10, 15, 20 };

    int num1 = numbers[rand() % numbers.size()];
    int num2 = numbers[rand() % numbers.size()];

    // Ensure the sum doesn't exceed the maximum possible value (e.g., 100)
    if (num1 + num2 > 100) {
        return generateRandomAdditionQuestion();
    }

    AdditionQuestion question;
    question.num1 = num1;
    question.num2 = num2;
    question.correctAnswer = num1 + num2;

    return question;
}

//idk if these do anything bruh
void readLevelAndTimer(std::ifstream& infile) {
    int level;
    float timer;

    infile >> level >> timer;

    // Use the values
    std::cout << "Level: " << level << std::endl;
    std::cout << "Time remaining: " << timer << std::endl;
}
// Function to write level and timer to file
void writeLevelAndTimer(std::ofstream& outfile) {
    int level = 1; // Example value

    outfile << "You are on level" << level << " " << std::endl;
}

void saveGame(const GameState& gameState, const std::string& filename) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << gameState.playerX << " ";
        outfile << gameState.playerY << " ";
        outfile << gameState.level << " ";
        // Save other relevant data here
        outfile.close();
    }
    else {
        std::cerr << "Unable to open file for writing" << std::endl;
    }
}
void saveGame() {
    GameState gameState;
    gameState.playerX = playerX;
    gameState.playerY = playerY;
    gameState.level = level;

    // Save other relevant game state variables

    saveGame(gameState, "game_state.dat");
}


bool loadGame(GameState& gameState, const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile) {
        return false; // Unable to open file
    }

    infile >> gameState.playerX >> gameState.playerY >> gameState.level;
    // Load other relevant data here
    infile.close();
    return true;
}

void loadGame() {
    GameState gameState;
    if (loadGame(gameState, "game_state.dat")) {
        playerX = gameState.playerX;
        playerY = gameState.playerY;
        level = gameState.level;

        // Load other relevant game state variables
    }
    else {
        std::cout << "Failed to load game state" << std::endl;
    }
}



//TODO - add scoring system, add save and load game
//Fix bug of when saving game state and then re-playing the game, if the saved game position of the player is not in the new maze's path, the player will be spawed outside of the maze path
