#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <typeinfo>
#include <map>

#ifdef _WIN32
  #include <windows.h>
  // Cross-platform sleep function in milliseconds
  void sleepMs(int ms) {
    Sleep(ms);
  }
#else
  #include <unistd.h>
  // Cross-platform sleep function in milliseconds
  void sleepMs(int ms) {
    usleep(ms * 1000);
  }
#endif

/**
 * Clears the console screen.
 * On Windows, we use FillConsoleOutputCharacter.
 * On other systems, we simply call system("clear").
 */
void clearScreen() {
#ifdef _WIN32
  COORD topLeft = {0, 0};
  HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO screen;
  DWORD written;
  GetConsoleScreenBufferInfo(console, &screen);
  FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y,
    topLeft, &written);
  FillConsoleOutputAttribute(console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
    screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
  SetConsoleCursorPosition(console, topLeft);
#else
  std::system("clear");
#endif
}

/**
 * Base class for all objects in the ocean.
 * Each object:
 *  - Has a display symbol
 *  - A tick() method to update its internal state
 *  - Can be "alive" or not
 *  - Has coordinates in the field
 */
struct Object {
  virtual ~Object() = default;
  virtual std::string getSymbol() const = 0;
  virtual void tick() = 0;
  virtual bool isAlive() const { return true; }
  size_t x = 0;
  size_t y = 0;
  bool movedThisTurn = false;
};

/**
 * Represents an empty cell. We'll display it as "  " (two spaces).
 */
struct Empty : public Object {
  std::string getSymbol() const override {
    return "  ";
  }
  void tick() override {}
};

/**
 * Stone (S). Eventually transforms into a Reef (R).
 */
struct Stone : public Object {
  int turnsToReef;

  Stone() {
    // Extended duration so we can see more transformations
    turnsToReef = 150 + rand() % 50;
  }

  std::string getSymbol() const override {
    return "S ";
  }

  void tick() override {
    if (turnsToReef > 0) {
      turnsToReef--;
    }
  }

  bool isReadyToTransform() const {
    return (turnsToReef <= 0);
  }
};

/**
 * Reef (R). Transforms back into Stone (S).
 */
struct Reef : public Object {
  int turnsToStone;
  
  Reef() {
    turnsToStone = 300 + rand() % 50;
  }

  std::string getSymbol() const override {
    return "R ";
  }

  void tick() override {
    if (turnsToStone > 0) {
      turnsToStone--;
    }
  }

  bool isReadyToTransform() const {
    return (turnsToStone <= 0);
  }
};

/**
 * Prey (~). Flees from predators and can reproduce.
 */
struct Prey : public Object {
  int age;
  int maxAge;
  bool adult;
  int reproduceCountdown;

  Prey() {
    // Longer lifespan
    maxAge = 800 + rand() % 200;
    age = 0;
    adult = false;
    // Delayed reproduction
    reproduceCountdown = 80 + rand() % 20;
  }

  std::string getSymbol() const override {
    return "~ ";
  }

  void tick() override {
    age++;
    if (age > (maxAge / 3)) {
      adult = true;
    }
    if (age > maxAge) {
      adult = false;
    }
    if (reproduceCountdown > 0) {
      reproduceCountdown--;
    }
  }

  bool isAlive() const override {
    return (age <= maxAge);
  }

  bool canReproduce() const {
    return (adult && reproduceCountdown == 0);
  }

  void resetReproduce() {
    reproduceCountdown = 80 + rand() % 20;
  }
};

/**
 * Predator (P). Hunts Prey (~). Avoids Apex (A). Has hunger; dies if too hungry.
 */
struct Predator : public Object {
  int age;
  int maxAge;
  bool adult;
  int hunger;
  int hungerLimit;
  int reproduceCountdown;

  Predator() {
    maxAge = 1000 + rand() % 200;
    age = 0;
    adult = false;
    hunger = 0;
    hungerLimit = 50;
    reproduceCountdown = 120 + rand() % 30;
  }

  std::string getSymbol() const override {
    return "P ";
  }

  void tick() override {
    age++;
    hunger++;
    if (age > (maxAge / 3)) {
      adult = true;
    }
    if (age > maxAge) {
      adult = false;
    }
    if (reproduceCountdown > 0) {
      reproduceCountdown--;
    }
  }

  bool isAlive() const override {
    if (age > maxAge) return false;
    if (hunger > hungerLimit) return false;
    return true;
  }

  bool isHungry() const {
    return (hunger > 10);
  }

  void feed() {
    hunger = 0;
  }

  bool canReproduce() const {
    return (adult && reproduceCountdown == 0 && !isHungry());
  }

  void resetReproduce() {
    reproduceCountdown = 120 + rand() % 30;
  }
};

/**
 * Apex predator (A). Can eat both Prey and Predator if hungry enough.
 */
struct ApexPredator : public Object {
  int age;
  int maxAge;
  bool adult;
  int hunger;
  int hungerLimit;
  int evolveHungerThreshold;
  int speed;
  int reproduceCountdown;

  ApexPredator() {
    maxAge = 1200 + rand() % 300;
    age = 0;
    adult = false;
    hunger = 0;
    hungerLimit = 60;
    evolveHungerThreshold = 15;
    speed = 1;
    reproduceCountdown = 200 + rand() % 50;
  }

  std::string getSymbol() const override {
    return "A ";
  }

  void tick() override {
    age++;
    hunger++;
    if (age > (maxAge / 4)) {
      adult = true;
    }
    if (age > maxAge) {
      adult = false;
    }
    // Increase speed if hunger crosses thresholds
    if (hunger > evolveHungerThreshold && speed == 1) {
      speed = 2;
    }
    if (hunger > (evolveHungerThreshold + 20) && speed == 2) {
      speed = 3;
    }
    if (reproduceCountdown > 0) {
      reproduceCountdown--;
    }
  }

  bool isAlive() const override {
    if (age > maxAge) return false;
    if (hunger > hungerLimit) return false;
    return true;
  }

  bool canEatPredator() const {
    return (speed == 3);
  }

  bool isHungry() const {
    return (hunger > 10);
  }

  void feed() {
    hunger = 0;
    speed = 1;
  }

  bool canReproduce() const {
    return (adult && reproduceCountdown == 0 && !isHungry());
  }

  void resetReproduce() {
    reproduceCountdown = 200 + rand() % 50;
  }
};

/**
 * Base Action class used by objects to modify the ocean state.
 */
struct Action {
  Action(std::shared_ptr<Object> obj) : obj(obj) {}
  virtual ~Action() = default;

  bool operator()(std::vector<std::shared_ptr<Object>>& field, size_t rows, size_t cols) {
    return apply(field, rows, cols);
  }

protected:
  virtual bool apply(std::vector<std::shared_ptr<Object>>& field, size_t rows, size_t cols) = 0;
  std::shared_ptr<Object> obj;
};

/**
 * NoAction: an object does nothing this turn.
 */
struct NoAction : public Action {
  NoAction(std::shared_ptr<Object> obj) : Action(obj) {}
private:
  bool apply(std::vector<std::shared_ptr<Object>>&, size_t, size_t) override {
    return true;
  }
};

/**
 * MoveAction: moves an object by (dx, dy) if the target is empty.
 */
struct MoveAction : public Action {
  int dx, dy;

  MoveAction(std::shared_ptr<Object> obj, int dx, int dy)
    : Action(obj), dx(dx), dy(dy) {}

private:
  bool apply(std::vector<std::shared_ptr<Object>>& field, size_t rows, size_t cols) override {
    size_t newX = (obj->x + dx + rows) % rows;
    size_t newY = (obj->y + dy + cols) % cols;
    size_t oldIdx = obj->x * cols + obj->y;
    size_t newIdx = newX * cols + newY;

    if (field[newIdx]->getSymbol() == "  ") {
      auto tmp = field[newIdx];
      field[newIdx] = field[oldIdx];
      field[oldIdx] = tmp;
      obj->x = newX;
      obj->y = newY;
      obj->movedThisTurn = true;
      return true;
    }
    return false;
  }
};

/**
 * EatAction: used by predators to eat adjacent prey and move into that cell.
 */
struct EatAction : public Action {
  int dx, dy;

  EatAction(std::shared_ptr<Object> obj, int dx, int dy)
    : Action(obj), dx(dx), dy(dy) {}

private:
  bool apply(std::vector<std::shared_ptr<Object>>& field, size_t rows, size_t cols) override {
    size_t newX = (obj->x + dx + rows) % rows;
    size_t newY = (obj->y + dy + cols) % cols;
    size_t oldIdx = obj->x * cols + obj->y;
    size_t newIdx = newX * cols + newY;

    std::string sym = field[newIdx]->getSymbol();
    // We treat Stone (S ) or Reef (R ) or "  " as non-edible
    if (sym != "  " && sym != "S " && sym != "R ") {
      field[newIdx] = std::make_shared<Empty>();
      auto tmp = field[newIdx];
      field[newIdx] = field[oldIdx];
      field[oldIdx] = tmp;
      obj->x = newX;
      obj->y = newY;
      obj->movedThisTurn = true;
      return true;
    }
    return false;
  }
};

/**
 * StormAction: a new type of action that simulates a small storm event
 * clearing everything in a certain radius around the given center.
 */
struct StormAction : public Action {
  int centerX, centerY;
  int radius;

  StormAction(std::shared_ptr<Object> obj, int cx, int cy, int r)
    : Action(obj), centerX(cx), centerY(cy), radius(r) {}

private:
  bool apply(std::vector<std::shared_ptr<Object>>& field, size_t rows, size_t cols) override {
    // For demonstration: remove (turn into Empty) everything in a circle or square region
    for (int dx = -radius; dx <= radius; ++dx) {
      for (int dy = -radius; dy <= radius; ++dy) {
        // Square or circular check
        // Let's do a simple square for simplicity
        int nx = (centerX + dx + rows) % rows;
        int ny = (centerY + dy + cols) % cols;
        size_t idx = nx * cols + ny;
        field[idx] = std::make_shared<Empty>();
      }
    }
    return true;
  }
};

/**
 * ActionWrapper is a helper that allows us to store actions in a vector.
 */
struct ActionWrapper {
  std::shared_ptr<Action> action;
  ActionWrapper(std::shared_ptr<Action> a) : action(a) {}
};

/**
 * The Ocean class: manages the grid of objects and the main simulation loop.
 */
class Ocean {
public:
  Ocean(size_t r, size_t c)
    : rows(r), cols(c), iterationCount(0), noChangeCounter(0)
  {
    field.resize(rows * cols);
    for (size_t i = 0; i < rows; ++i) {
      for (size_t j = 0; j < cols; ++j) {
        auto obj = randomObject();
        obj->x = i;
        obj->y = j;
        field[i * cols + j] = obj;
      }
    }
  }

  /**
   * Main simulation loop:
   *  - Clear the screen
   *  - Print stats
   *  - Display the ocean
   *  - Let each object tick and perform actions
   *  - Sometimes trigger a "storm" event at random
   *  - Stop if no changes happen for a while or we exceed a large iteration count
   */
  void run() {
    bool running = true;
    while (running) {
      clearScreen();
      printStats();
      std::cout << "\n";
      display();

      auto oldState = copyState();

      for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
          size_t idx = i * cols + j;
          auto& obj = field[idx];

          // Replace dead object with Empty
          if (!obj->isAlive()) {
            field[idx] = std::make_shared<Empty>();
            continue;
          }
          if (!obj->movedThisTurn) {
            obj->tick();
            auto actions = decideActions(obj);
            for (auto& aw : actions) {
              aw.action->operator()(field, rows, cols);
            }
          }
        }
      }

      // Occasionally trigger a random storm
      if ((rand() % 1000) < 3) { 
        int cx = rand() % rows;
        int cy = rand() % cols;
        int rad = 1 + rand() % 3; 
        // We'll attach StormAction to a dummy object
        auto dummy = std::make_shared<Empty>();
        auto storm = std::make_shared<StormAction>(dummy, cx, cy, rad);
        storm->operator()(field, rows, cols);
        std::cout << ">>> Storm occurred around (" << cx << ", " << cy 
                  << ") with radius " << rad << "!\n";
      }

      for (auto& o : field) {
        o->movedThisTurn = false;
      }

      iterationCount++;

      if (!isChanged(oldState, field)) {
        noChangeCounter++;
      } else {
        noChangeCounter = 0;
      }

      if (noChangeCounter > 150 || iterationCount > 5000) {
        running = false;
      }

      sleepMs(120);
    }

    // Final display
    clearScreen();
    printStats();
    std::cout << "\n";
    display();
    std::cout << "\nSimulation ended. Press Enter to exit.\n";
    std::cin.get();
  }

private:
  size_t rows;
  size_t cols;
  std::vector<std::shared_ptr<Object>> field;
  size_t iterationCount;
  size_t noChangeCounter;

  /**
   * Creates a random object (Empty, Stone, Reef, Prey, Predator, ApexPredator)
   * with adjusted probabilities.
   */
  std::shared_ptr<Object> randomObject() {
    int r = rand() % 100;
    if (r < 40) {
      return std::make_shared<Empty>();
    } else if (r < 50) {
      return std::make_shared<Stone>();
    } else if (r < 60) {
      return std::make_shared<Reef>();
    } else if (r < 80) {
      return std::make_shared<Prey>();
    } else if (r < 95) {
      return std::make_shared<Predator>();
    } else {
      return std::make_shared<ApexPredator>();
    }
  }

  /**
   * Renders the ocean to the console.
   */
  void display() {
    std::cout << "Iteration: " << iterationCount
              << "  (No change counter: " << noChangeCounter << ") \n";
    for (size_t i = 0; i < rows; ++i) {
      for (size_t j = 0; j < cols; ++j) {
        std::cout << field[i * cols + j]->getSymbol();
      }
      std::cout << "\n";
    }
    std::cout << std::flush;
  }

  /**
   * Prints how many of each type of object are present in the ocean.
   */
  void printStats() {
    std::map<std::string,int> counts;
    for (auto& obj : field) {
      counts[obj->getSymbol()]++;
    }

    auto nameOf = [&](const std::string& sym) {
      if (sym == "  ")  return "Empty";
      if (sym == "S ")  return "Stone";
      if (sym == "R ")  return "Reef";
      if (sym == "~ ")  return "Prey";
      if (sym == "P ")  return "Predator";
      if (sym == "A ")  return "ApexPredator";
      return "Unknown";
    };

    std::cout << "----- Ocean Statistics -----\n";
    for (auto& kv : counts) {
      std::cout << nameOf(kv.first) << ": " << kv.second << "\n";
    }
  }

  /**
   * Decides what actions an object should take based on its type
   * and local environment.
   */
  std::vector<ActionWrapper> decideActions(std::shared_ptr<Object> obj) {
    std::vector<ActionWrapper> actions;
    if (obj->getSymbol() == "  " || !obj->isAlive()) {
      actions.emplace_back(std::make_shared<NoAction>(obj));
      return actions;
    }

    // Prey
    if (auto prey = std::dynamic_pointer_cast<Prey>(obj)) {
      auto neighbours = getNeighbours(obj->x, obj->y, 1);
      bool dangerNearby = false;
      std::pair<int,int> runDir = {0,0};
      for (auto& n : neighbours) {
        auto s = n->getSymbol();
        if (s == "P " || s == "A ") {
          dangerNearby = true;
          runDir = getOppositeDirection(obj->x, obj->y, n->x, n->y);
          break;
        }
      }
      if (dangerNearby) {
        actions.emplace_back(std::make_shared<MoveAction>(obj, runDir.first, runDir.second));
      } else {
        if (prey->canReproduce()) {
          for (auto& n : neighbours) {
            if (n->getSymbol() == "  ") {
              size_t idx = n->x * cols + n->y;
              auto baby = std::make_shared<Prey>();
              baby->x = n->x;
              baby->y = n->y;
              field[idx] = baby;
              prey->resetReproduce();
              break;
            }
          }
        }
        auto d = randomDirection(1);
        actions.emplace_back(std::make_shared<MoveAction>(obj, d.first, d.second));
      }
    }
    // Predator
    else if (auto predator = std::dynamic_pointer_cast<Predator>(obj)) {
      auto neighbours = getNeighbours(obj->x, obj->y, 1);
      bool apexNearby = false;
      std::pair<int,int> runDir = {0,0};
      for (auto& n : neighbours) {
        if (n->getSymbol() == "A ") {
          apexNearby = true;
          runDir = getOppositeDirection(obj->x, obj->y, n->x, n->y);
          break;
        }
      }
      if (apexNearby) {
        actions.emplace_back(std::make_shared<MoveAction>(obj, runDir.first, runDir.second));
      } else {
        bool ate = false;
        for (auto& n : neighbours) {
          if (n->getSymbol() == "~ ") {
            auto dir = getDirection(obj->x, obj->y, n->x, n->y);
            actions.emplace_back(std::make_shared<EatAction>(obj, dir.first, dir.second));
            predator->feed();
            ate = true;
            break;
          }
        }
        if (!ate) {
          if (predator->isHungry()) {
            auto d = randomDirection(2);
            actions.emplace_back(std::make_shared<MoveAction>(obj, d.first, d.second));
          } else {
            auto d = randomDirection(1);
            actions.emplace_back(std::make_shared<MoveAction>(obj, d.first, d.second));
          }
        }
        if (predator->canReproduce()) {
          for (auto& n : neighbours) {
            if (n->getSymbol() == "  ") {
              size_t idx = n->x * cols + n->y;
              auto baby = std::make_shared<Predator>();
              baby->x = n->x;
              baby->y = n->y;
              field[idx] = baby;
              predator->resetReproduce();
              break;
            }
          }
        }
      }
    }
    // ApexPredator
    else if (auto apex = std::dynamic_pointer_cast<ApexPredator>(obj)) {
      auto neighbours = getNeighbours(obj->x, obj->y, apex->speed);
      bool ate = false;
      for (auto& n : neighbours) {
        if (n->getSymbol() == "~ ") {
          auto dir = getDirection(obj->x, obj->y, n->x, n->y);
          actions.emplace_back(std::make_shared<EatAction>(obj, dir.first, dir.second));
          apex->feed();
          ate = true;
          break;
        }
      }
      // If still hungry, can eat Predator if speed=3
      if (!ate && apex->canEatPredator()) {
        for (auto& n : neighbours) {
          if (n->getSymbol() == "P ") {
            auto dir = getDirection(obj->x, obj->y, n->x, n->y);
            actions.emplace_back(std::make_shared<EatAction>(obj, dir.first, dir.second));
            apex->feed();
            ate = true;
            break;
          }
        }
      }
      if (!ate) {
        auto d = randomDirection(apex->speed);
        actions.emplace_back(std::make_shared<MoveAction>(obj, d.first, d.second));
      }
      if (apex->canReproduce()) {
        for (auto& n : neighbours) {
          if (n->getSymbol() == "  ") {
            size_t idx = n->x * cols + n->y;
            auto baby = std::make_shared<ApexPredator>();
            baby->x = n->x;
            baby->y = n->y;
            field[idx] = baby;
            apex->resetReproduce();
            break;
          }
        }
      }
    }
    // Stone
    else if (auto stone = std::dynamic_pointer_cast<Stone>(obj)) {
      stone->tick();
      if (stone->isReadyToTransform()) {
        size_t idx = obj->x * cols + obj->y;
        auto reef = std::make_shared<Reef>();
        reef->x = obj->x;
        reef->y = obj->y;
        field[idx] = reef;
      }
      actions.emplace_back(std::make_shared<NoAction>(obj));
    }
    // Reef
    else if (auto reef = std::dynamic_pointer_cast<Reef>(obj)) {
      reef->tick();
      if (reef->isReadyToTransform()) {
        size_t idx = obj->x * cols + obj->y;
        auto st = std::make_shared<Stone>();
        st->x = obj->x;
        st->y = obj->y;
        field[idx] = st;
      }
      actions.emplace_back(std::make_shared<NoAction>(obj));
    }
    else {
      actions.emplace_back(std::make_shared<NoAction>(obj));
    }

    return actions;
  }

  /**
   * Returns neighbors in a square radius (toroidal wrapping).
   */
  std::vector<std::shared_ptr<Object>> getNeighbours(size_t x, size_t y, int range) {
    std::vector<std::shared_ptr<Object>> neighbours;
    for (int dx = -range; dx <= range; ++dx) {
      for (int dy = -range; dy <= range; ++dy) {
        if (dx == 0 && dy == 0) continue;
        size_t nx = (x + dx + rows) % rows;
        size_t ny = (y + dy + cols) % cols;
        neighbours.push_back(field[nx * cols + ny]);
      }
    }
    return neighbours;
  }

  /**
   * Returns a random direction (dx, dy) with the given step size.
   */
  std::pair<int,int> randomDirection(int step) {
    int d = rand() % 4;
    int dx = 0, dy = 0;
    if (d == 0) dx = -step;
    else if (d == 1) dx = step;
    else if (d == 2) dy = -step;
    else dy = step;
    return {dx, dy};
  }

  /**
   * Returns direction from (x1,y1) to (x2,y2).
   */
  std::pair<int,int> getDirection(size_t x1, size_t y1, size_t x2, size_t y2) {
    int dx = 0, dy = 0;
    if (x2 < x1) dx = -1;
    else if (x2 > x1) dx = 1;
    if (y2 < y1) dy = -1;
    else if (y2 > y1) dy = 1;
    return {dx, dy};
  }

  /**
   * Returns the opposite direction from (x1,y1) to (x2,y2).
   * Useful for running away from threats.
   */
  std::pair<int,int> getOppositeDirection(size_t x1, size_t y1, size_t x2, size_t y2) {
    int dx = 0, dy = 0;
    if (x2 < x1) dx = 1;
    else if (x2 > x1) dx = -1;
    if (y2 < y1) dy = 1;
    else if (y2 > y1) dy = -1;
    return {dx, dy};
  }

  /**
   * Creates a copy of the entire field (vector of shared_ptr).
   */
  std::vector<std::shared_ptr<Object>> copyState() {
    return field;
  }

  /**
   * Checks if the ocean changed compared to the old state.
   * We look at pointer equality and symbol differences.
   */
  bool isChanged(const std::vector<std::shared_ptr<Object>>& oldS,
                 const std::vector<std::shared_ptr<Object>>& newS) {
    if (oldS.size() != newS.size()) return true;
    for (size_t i = 0; i < oldS.size(); ++i) {
      if (oldS[i].get() != newS[i].get()) return true;
      if (oldS[i]->getSymbol() != newS[i]->getSymbol()) return true;
    }
    return false;
  }
};

// ------------------ main ------------------

int main(int argc, char* argv[]) {
  srand(static_cast<unsigned>(time(nullptr)));
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8); // For UTF-8 characters
#endif

  size_t N = 30; 
  size_t M = 40;
  if (argc >= 3) {
    N = std::atoi(argv[1]);
    M = std::atoi(argv[2]);
  }

  Ocean ocean(N, M);
  ocean.run();

  return 0;
}
