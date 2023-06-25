#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <iostream>
#include <sstream>

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const float FISH_RADIUS = 5.0f;
const float FISH_SPEED = 2.5f;
const float BUBBLE_RADIUS = 15.0f;
const float BUBBLE_SPEED = 0.2f; // Reduced bubble speed
const float FOOD_RADIUS = 10.0f;
const int MAX_FOOD_COUNT = 7; // Maximum number of food bubbles on the screen

class Bubble {
public:
    Bubble(float posX, float posY, bool isDeadly)
        : velocity(0.0f, getRandomVelocity()), isDeadly(isDeadly)
    {
        shape.setRadius(BUBBLE_RADIUS);
        shape.setFillColor(sf::Color::Blue);
        shape.setPosition(posX, posY);
    }

    void move() {
        shape.move(velocity);
    }

    bool isOutOfScreen() const {
        return shape.getPosition().y > WINDOW_HEIGHT;
    }

    const sf::FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }

    sf::CircleShape shape;
    sf::Vector2f velocity;
    bool isDeadly;

private:
    float getRandomVelocity() const {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> randomVelocity(0.1f, 0.3f);
        return randomVelocity(gen);
    }
};

class Food {
public:
    Food(float posX, float posY)
        : velocity(0.0f, getRandomVelocity()), isEdible(true)
    {
        shape.setRadius(FOOD_RADIUS);
        shape.setFillColor(sf::Color::Green);
        shape.setPosition(posX, posY);
    }

    void move() {
        shape.move(velocity);
    }

    bool isOutOfScreen() const {
        return shape.getPosition().y > WINDOW_HEIGHT;
    }

    const sf::FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }

    sf::CircleShape shape;
    sf::Vector2f velocity;
    bool isEdible;

private:
    float getRandomVelocity() const {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> randomVelocity(0.1f, 0.3f);
        return randomVelocity(gen);
    }
};

class Game {
public:
    Game()
        : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Bubble Dodge Game"),
        fishTexture(), bubbleClock(), score(0)
    {
        initializeTextures();
        initializeSprites();
        initializeRandomGenerators();
        initializeScoreText();
    }

    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            render();
        }
    }

private:
    void initializeTextures() {
        if (!backgroundTexture.loadFromFile("background.jpeg")) {
            std::cout << "Failed to load background texture!" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (!fishTexture.loadFromFile("fish.png")) {
            std::cout << "Failed to load fish texture!" << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void initializeSprites() {
        background.setTexture(backgroundTexture);
        background.setScale(
            static_cast<float>(WINDOW_WIDTH) / background.getTexture()->getSize().x,
            static_cast<float>(WINDOW_HEIGHT) / background.getTexture()->getSize().y
        );

        fish.setTexture(fishTexture);
        fish.setOrigin(FISH_RADIUS, FISH_RADIUS);
        fish.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    }

    void initializeRandomGenerators() {
        std::random_device rd;
        std::mt19937 gen(rd());
        randomX = std::uniform_real_distribution<float>(0, WINDOW_WIDTH);
    }

    void initializeScoreText() {
        if (!font.loadFromFile("arial.ttf")) {
            std::cout << "Failed to load font!" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        scoreText.setFont(font);
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(10, 10);
    }

    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        sf::Vector2f movement(0.0f, 0.0f);
        fish.move(movement);

        sf::Vector2f fishPosition = fish.getPosition();
        fishPosition.x = std::max(FISH_RADIUS, std::min(fishPosition.x, static_cast<float>(WINDOW_WIDTH) - FISH_RADIUS));
        fishPosition.y = std::max(FISH_RADIUS, std::min(fishPosition.y, static_cast<float>(WINDOW_HEIGHT) - FISH_RADIUS));
        fish.setPosition(fishPosition);
    }

    void update() {
        generateBubbles();
        generateFoodBubbles();

        updateBubbles();
        updateFoodBubbles();

        checkCollisions();

        if (isGameOver())
            endGame();
    }

    void generateBubbles() {
        if (bubbleClock.getElapsedTime().asSeconds() >= 1.0f) {
            bubbles.push_back(Bubble(randomX(gen), -BUBBLE_RADIUS, true));
            bubbleClock.restart();
        }
    }

    void generateFoodBubbles() {
        if (foodBubbles.size() < MAX_FOOD_COUNT) {
            foodBubbles.push_back(Food(randomX(gen), -FOOD_RADIUS));
        }
    }

    void updateBubbles() {
        for (auto& bubble : bubbles) {
            bubble.move();

            if (bubble.isOutOfScreen()) {
                bubble.shape.setPosition(-1000.0f, -1000.0f);
            }
        }
    }

    void updateFoodBubbles() {
        for (auto it = foodBubbles.begin(); it != foodBubbles.end(); ++it) {
            auto& food = *it;
            food.move();

            if (food.isOutOfScreen()) {
                it = foodBubbles.erase(it);
                if (it == foodBubbles.end()) {
                    break;
                }
            }
        }
    }

    void checkCollisions() {
        for (auto& bubble : bubbles) {
            if (fish.getGlobalBounds().intersects(bubble.getBounds()) && bubble.isDeadly) {
                endGame();
                break;
            }
        }

        for (auto it = foodBubbles.begin(); it != foodBubbles.end(); ++it) {
            auto& food = *it;
            if (fish.getGlobalBounds().intersects(food.getBounds()) && food.isEdible) {
                it = foodBubbles.erase(it);
                if (it == foodBubbles.end()) {
                    break;
                }
                score += 10;
                scoreText.setString("Score: " + std::to_string(score));
            }
        }
    }

    bool isGameOver() const {
        for (const auto& food : foodBubbles) {
            if (food.isOutOfScreen()) {
                return true;
            }
        }
        return false;
    }

    void endGame() {
        window.close();
        std::cout << "Game Over! Score: " << score << std::endl;
    }

    void render() {
        window.clear();
        window.draw(background);
        window.draw(fish);

        for (const auto& bubble : bubbles) {
            window.draw(bubble.shape);
        }

        for (const auto& food : foodBubbles) {
            window.draw(food.shape);
        }

        window.draw(scoreText); // Render the score text

        window.display();
    }

    sf::RenderWindow window;
    sf::Texture backgroundTexture;
    sf::Texture fishTexture;
    sf::Sprite background;
    sf::Sprite fish;
    std::vector<Bubble> bubbles;
    std::vector<Food> foodBubbles;
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> randomX;
    sf::Clock bubbleClock;
    int score;
    sf::Font font;
    sf::Text scoreText;
};

int main() {
    Game game;
    game.run();

    return 0;
}
