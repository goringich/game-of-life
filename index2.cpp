#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <cmath>

enum class Direction { NORTH, SOUTH, EAST, WEST };

class Ocean;
class Object;

class Action {
public:
    virtual ~Action() {}
    virtual void execute(Ocean& ocean, int x, int y) = 0;
};

class Object {
public:
    virtual ~Object() {}
    virtual char getSymbol() const = 0;
    virtual void update(Ocean& ocean, int x, int y) = 0;
};

class Stone : public Object {
    int lifetime;
public:
    Stone() : lifetime(rand() % 5 + 5) {}
    virtual ~Stone() {}

    char getSymbol() const override {
        return '#';
    }

    void update(Ocean& ocean, int x, int y) override;
};

class Reef : public Object {
    int lifetime;
public:
    Reef() : lifetime(rand() % 10 + 10) {}
    virtual ~Reef() {}

    char getSymbol() const override {
        return '@';
    }

    void update(Ocean& ocean, int x, int y) override;
};

class Prey : public Object {
    int age;
    int maturityAge;
    int maxAge;
    bool isAdult;
public:
    Prey() : age(0), maturityAge(rand() % 5 + 5), maxAge(maturityAge + rand() % 10 + 10), isAdult(false) {}
    virtual ~Prey() {}

    char getSymbol() const override {
        return 'P';
    }

    void update(Ocean& ocean, int x, int y) override;
};

class Predator : public Object {
    int hunger;
    int maxHunger;
public:
    Predator() : hunger(0), maxHunger(rand() % 5 + 5) {}
    virtual ~Predator() {}

    char getSymbol() const override {
        return 'D';
    }

    void update(Ocean& ocean, int x, int y) override;
};

class Ocean {
public:
    int width, height;
    std::vector<std::vector<std::unique_ptr<Object>>> grid;

    Ocean(int w, int h) : width(w), height(h), grid(h, std::vector<std::unique_ptr<Object>>(w)) {
        srand(static_cast<unsigned int>(time(nullptr)));
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                int randVal = rand() % 100;
                if (randVal < 5) {
                    grid[i][j] = std::make_unique<Stone>();
                } else if (randVal < 10) {
                    grid[i][j] = std::make_unique<Reef>();
                } else if (randVal < 30) {
                    grid[i][j] = std::make_unique<Prey>();
                } else if (randVal < 40) {
                    grid[i][j] = std::make_unique<Predator>();
                }
            }
        }
    }

    void simulate() {
        for (int step = 0; step < 100; ++step) {
            system("clear");
            display();
            update();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void display() {
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                if (grid[i][j]) {
                    std::cout << grid[i][j]->getSymbol() << ' ';
                } else {
                    std::cout << ". ";
                }
            }
            std::cout << '\n';
        }
    }

    void update() {
        auto newGrid = grid;
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                if (grid[i][j]) {
                    grid[i][j]->update(*this, i, j);
                }
            }
        }
        grid = std::move(newGrid);
    }

    // Helper function to move an object within the grid, wrapping around edges
    void moveObject(int oldX, int oldY, int newX, int newY) {
        newX = (newX + height) % height;
        newY = (newY + width) % width;
        grid[newX][newY] = std::move(grid[oldX][oldY]);
    }

    // Add other necessary helper methods as needed
};

void Stone::update(Ocean& ocean, int x, int y) {
    if (--lifetime <= 0) {
        ocean.grid[x][y] = std::make_unique<Reef>();
    }
}

void Reef::update(Ocean& ocean, int x, int y) {
    if (--lifetime <= 0) {
        ocean.grid[x][y] = std::make_unique<Stone>();
    }
}

void Prey::update(Ocean& ocean, int x, int y) {
    if (++age >= maxAge) {
        ocean.grid[x][y].reset();
    } else {
        // Move randomly
        int dx = rand() % 3 - 1;
        int dy = rand() % 3 - 1;
        int nx = (x + dx + ocean.height) % ocean.height;
        int ny = (y + dy + ocean.width) % ocean.width;
        if (!ocean.grid[nx][ny]) {
            ocean.moveObject(x, y, nx, ny);
        }
    }
}

void Predator::update(Ocean& ocean, int x, int y) {
    if (++hunger > maxHunger) {
        ocean.grid[x][y].reset();
    } else {
        // Move towards nearest prey or randomly
        int dx = rand() % 3 - 1;
        int dy = rand() % 3 - 1;
        int nx = (x + dx + ocean.height) % ocean.height;
        int ny = (y + dy + ocean.width) % ocean.width;
        if (ocean.grid[nx][ny] && dynamic_cast<Prey*>(ocean.grid[nx][ny].get())) {
            ocean.grid[x][y].reset(); // Predator eats the prey
            ocean.moveObject(x, y, nx, ny);
            hunger = 0; // Reset hunger
        } else if (!ocean.grid[nx][ny]) {
            ocean.moveObject(x, y, nx, ny);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <width> <height>\n";
        return 1;
    }

    int width = std::stoi(argv[1]);
    int height = std::stoi(argv[2]);

    Ocean ocean(width, height);
    ocean.simulate();

    return 0;
}
