#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <iostream>
#include <sstream>
#include <SFML/Audio.hpp>


const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const float FISH_RADIUS = 1.0f;
const float FISH_SPEED = 2.5f;
const float BUBBLE_RADIUS = 20.0f;
const float BUBBLE_SPEED = 5.0f; // Reduced bubble speed
const float FOOD_RADIUS = 10.0f;
const int MAX_FOOD_COUNT = 10; // Maximum number of food bubbles on the screen

using namespace sf;
using namespace std;

class Bubble {
public:
    Bubble(float posX, float posY, bool isDeadly)
        : velocity(0.0f, getRandomVelocity()), isDeadly(isDeadly)
    {
        shape.setRadius(BUBBLE_RADIUS);
        shape.setFillColor(Color::Blue);
        shape.setPosition(posX, posY);
    }

    void move() {
        shape.move(velocity);
    }

    bool isOutOfScreen() const {
        return shape.getPosition().y > WINDOW_HEIGHT;
    }

    const FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }

    CircleShape shape;
    Vector2f velocity;
    bool isDeadly;

private:
    float getRandomVelocity() const {
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<float> randomVelocity(0.1f, 0.5f);
        return randomVelocity(gen);
    }
};

class Food {
public:
    Food(float posX, float posY)
        : velocity(0.0f, getRandomVelocity()), isEdible(true)
    {
        shape.setRadius(FOOD_RADIUS);
        shape.setFillColor(Color::Green);
        shape.setPosition(posX, posY);
    }

    void move() {
        shape.move(velocity);
    }

    bool isOutOfScreen() const {
        return shape.getPosition().y > WINDOW_HEIGHT;
    }

    const FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }

    CircleShape shape;
    Vector2f velocity;
    bool isEdible;

private:
    float getRandomVelocity() const {
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<float> randomVelocity(0.1f, 0.4f);
        return randomVelocity(gen);
    }
};

class Game {
public:
    Game()
        : window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Bubble Dodge Game"),
        fishTexture(), bubbleClock(), score(0), isPlaying(false)
    {
        initializeTextures();
        initializeSprites();
        initializeRandomGenerators();
        initializeScoreText();
        initializePlayButton();       
    }

    void run() {
        while (window.isOpen()) {
            processEvents();
            if (isPlaying) {
                update();
                render();
            }
            else {
                renderPlayButton();
            }
        }
    }

private:
    Music backgroundMusic;
    void initializeTextures() {
        if (!backgroundTexture.loadFromFile("background.jpeg")) {
            cout << "Failed to load background texture!" << endl;
            exit(EXIT_FAILURE);
        }

        if (!fishTexture.loadFromFile("fish.png")) {
            cout << "Failed to load fish texture!" << endl;
            exit(EXIT_FAILURE);
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

        if (!backgroundMusic.openFromFile("bgsound.ogg")) {
            // Handle error: Failed to load the audio file
            cout << "Failed to load the audio file." << endl;
        }

    }

    void initializeRandomGenerators() {
        random_device rd;
        mt19937 gen(rd());
        randomX = uniform_real_distribution<float>(0, WINDOW_WIDTH);
    }

    void initializeScoreText() {
        if (!font.loadFromFile("arial.ttf")) {
            cout << "Failed to load font!" << endl;
            exit(EXIT_FAILURE);
        }

        scoreText.setFont(font);
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(Color::White);
        scoreText.setPosition(10, 10);
    }

    void initializePlayButton() {
        playButton.setSize(Vector2f(200, 50));
        playButton.setPosition(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50); // Adjust vertical position

        playButton.setFillColor(Color::Green);

        playButtonText.setFont(font);
        playButtonText.setCharacterSize(32); // Increase the font size to 32
        playButtonText.setFillColor(Color::White);
        playButtonText.setString("Play");
        playButtonText.setPosition(
            playButton.getPosition().x + (playButton.getSize().x - playButtonText.getLocalBounds().width) / 2,
            playButton.getPosition().y + (playButton.getSize().y - playButtonText.getLocalBounds().height) / 2 - 5
        );

        playButtonText.setStyle(Text::Bold);
    }



    void processEvents() {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
            else if (event.type == Event::MouseButtonPressed) {
                if (event.mouseButton.button == Mouse::Left) {
                    handleMouseClick(event.mouseButton.x, event.mouseButton.y);
                }
            }
        }

        Vector2f movement(0.0f, 0.0f);

        if (Keyboard::isKeyPressed(Keyboard::Left)) {
            movement.x -= FISH_SPEED;
            fish.setScale(1.0f, 1.0f); // Reset the scale when moving left
        }
        if (Keyboard::isKeyPressed(Keyboard::Right)) {
            movement.x += FISH_SPEED;
            fish.setScale(-1.0f, 1.0f); // Flip the sprite vertically when moving right
        }
        if (Keyboard::isKeyPressed(Keyboard::Up))
            movement.y -= FISH_SPEED;
        if (Keyboard::isKeyPressed(Keyboard::Down))
            movement.y += FISH_SPEED;
        if (Keyboard::isKeyPressed(Keyboard::Escape))
            window.close();

        fish.move(movement);

        Vector2f fishPosition = fish.getPosition();
        fishPosition.x = max(FISH_RADIUS, min(fishPosition.x, static_cast<float>(WINDOW_WIDTH) - FISH_RADIUS));
        fishPosition.y = max(FISH_RADIUS, min(fishPosition.y, static_cast<float>(WINDOW_HEIGHT) - FISH_RADIUS));
        fish.setPosition(fishPosition);
    }

    void handleMouseClick(int x, int y) {
        if (!isPlaying && playButton.getGlobalBounds().contains(x, y)) { // Start the game if the play button is clicked
            isPlaying = true;
        }
    }

    void update() {
        generateBubbles();
        generateFoodBubbles();

        updateBubbles();
        updateFoodBubbles();

        checkCollisions();

        if (isGameOver())
            endGame();

        backgroundMusic.play();
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
                scoreText.setString("Score: " + to_string(score));
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
        cout << "Game Over! Score: " << score << endl;
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

        backgroundMusic.play();

        // Set the volume to 50%
        backgroundMusic.setVolume(50.f);

        // Loop the music
        backgroundMusic.setLoop(true);
    }

    void renderPlayButton() {
        window.clear();
        window.draw(background);
        window.draw(playButton);
        window.draw(playButtonText);
        window.display();
    }

    RenderWindow window;
    Texture backgroundTexture;
    Texture fishTexture;
    Sprite background;
    Sprite fish;
    vector<Bubble> bubbles;
    vector<Food> foodBubbles;
    random_device rd;
    mt19937 gen;
    uniform_real_distribution<float> randomX;
    Clock bubbleClock;
    int score;
    Font font;
    Text scoreText;
    RectangleShape playButton;
    Text playButtonText;
    bool isPlaying;
};

int main() {
    Game game;
    game.run();

    return 0;
}