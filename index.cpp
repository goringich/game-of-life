#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <codecvt>
#include <locale>
#include <thread>
#include <chrono>
#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif
#include <cmath>

void clearScreen() {
#ifdef _WIN32
  COORD topLeft = {0, 0};
  HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO screen;
  DWORD written;

  GetConsoleScreenBufferInfo(console, &screen);
  FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
  FillConsoleOutputAttribute(console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE, screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
  SetConsoleCursorPosition(console, topLeft);
#else
  std::system("clear");
#endif
}

void setcur(int x1, int y1) {
#ifdef _WIN32
  COORD coord;
  coord.X = x1;
  coord.Y = y1;
  SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#endif
}

class Ocean;

class Object {
public:
  int x; 
  int y; 
  bool isMoved;

  Object(int x, int y) : x(x), y(y), isMoved(false) {}

  virtual ~Object() {}
  virtual void update() = 0; 
  virtual std::string getSymbol() const = 0;
};


struct Action {
  Object* obj;

  Action(Object* obj)
    : obj(obj) {}

  virtual ~Action() = default;

  bool operator()(Ocean& ocean) { return apply(ocean); }

protected:
  virtual bool apply(Ocean& ocean) = 0;
};


struct MoveAction : public Action {
  int newX, newY;

  MoveAction(Object* obj, int newX, int newY)
    : Action(obj), newX(newX), newY(newY) {}

private:
  bool apply(Ocean& ocean) override;
};

class Empty : public Object {
public:
  Empty() : Object(0, 0), symbol(" ") {}
  void update() override {}
  std::string getSymbol() const override { return symbol; }
private:
  std::string symbol;
};

class Stone : public Object {
private:
  int turnsToReef;
public:
  Stone() : Object(0, 0), turnsToReef(std::rand() % 7), symbol(u8"\u26F0") {}
  void update() override {
    if (turnsToReef == 0) {
      symbol = u8"&";
    } else {
      turnsToReef--;
    }
  }
  std::string getSymbol() const override { return symbol; }
private:
  std::string symbol;
};

class Reef : public Object {
private:
  int turnsToStone;
public:
  Reef() : Object(0, 0), turnsToStone(std::rand() % 7), symbol(u8"➿") {}
  void update() override {
    if (turnsToStone == 0) {
      symbol = u8"\u26F0";
    } else {
      turnsToStone--;
    }
  }
  std::string getSymbol() const override { return symbol; }
private:
  std::string symbol;
};

class Prey : public Object {
private:
  int age;
  int maxAge;
  int turnsToReproduce;
  bool isAdult;
  std::string symbol;
public:
  Prey() : Object(0, 0), age(0), maxAge(10), turnsToReproduce(7), isAdult(false), symbol(u8"~") {}
  void update() override {
    age++;
    if (age >= 3) isAdult = true;
    if (age == 20) {
      symbol = u8"X";
      return;
    } else if (age == 22) {
      symbol = u8" ";
      return;
    }

    if (isAdult) {
      turnsToReproduce == 0 ? turnsToReproduce = 10 : turnsToReproduce--;
      symbol = u8"∞";
    } else {
      symbol = u8"~";
    }
    if (isAdult && turnsToReproduce == 0) turnsToReproduce = 10;
  }

  std::string getSymbol() const override { return symbol; }
};

class Predator : public Object {
private:
  int hungerLevel;
  bool isHungry;
  int turnsToReproduce;
  bool isAdult;
  int age;
  std::string symbol;
public:
  Predator() : Object(rand() % 10, rand() % 10), hungerLevel(0), isHungry(false), turnsToReproduce(rand() % 5 + 5), isAdult(false), age(0), symbol(u8"P") {}

  void update() override {
    hungerLevel++;
    if (hungerLevel > 3) isHungry = true;
    if (hungerLevel > 10) {
      symbol = u8"💀"; // Predator dies of hunger
      return;
    }

    age++;
    if (age > 5) isAdult = true;
  }

  std::string getSymbol() const override { return symbol; }
};

class Ocean {
public:
  using Cell = Object*;

  Ocean(size_t r, size_t c) : rows(r), cols(c), data(r * c), iterationCounter(0), stagnantCounter(0) {
    srand(static_cast<unsigned int>(time(nullptr)));
    for (size_t i = 0; i < r; ++i) {
      for (size_t j = 0; j < c; ++j) {
        data[i * c + j] = generate_neighbourhood(i, j);
      }
    }
  }

  ~Ocean() {
    for (auto& obj : data) {
      delete obj;
    }
  }

  void tick() {
    for (size_t i = 0; i < rows; ++i) {
      for (size_t j = 0; j < cols; ++j) {
        data[i * cols + j]->update();
        if (data[i * cols + j]->isMoved == false) {
          move_prey(i, j);
        }
      }
    }

    for (size_t i = 0; i < rows; ++i) {
      for (size_t j = 0; j < cols; ++j) {
        data[i * cols + j]->isMoved = false;
      }
    }

    iterationCounter++;
  }

  void display() {
    clearScreen();
    std::cout << "Iteration count: " << iterationCounter << std::endl;
    for (size_t i = 0; i < rows; ++i) {
      for (size_t j = 0; j < cols; ++j) {
        std::cout << data[i * cols + j]->getSymbol() << " ";
      }
      std::cout << std::endl;
    }
    #ifdef _WIN32
        Sleep(300);
    #else
        usleep(3000);
    #endif
        std::cout << std::endl;
  }

  size_t getRows() const { return rows; }
  size_t getCols() const { return cols; }
  std::vector<Cell>& getData() { return data; }
  size_t getIterationCounter() const { return iterationCounter; }

private:
  size_t rows, cols;
  std::vector<Object*> data;
  size_t stagnantCounter;
  size_t iterationCounter;

    Cell generate_neighbourhood(int x, int y) {
    int randNum = rand() % 100;
    Object* newObj;
    if (randNum < 60) {
      newObj = new Empty();
    } else if (randNum >= 60 && randNum < 70) {
      newObj = new Stone();
    } else if (randNum >= 70 && randNum < 80) { 
      newObj = new Reef();
    } else if (randNum >= 80 && randNum < 90) { 
      newObj = new Predator();
    } else {
      newObj = new Prey();
    }
    return newObj;
  }


  void move_prey(int i, int j) {
    if (typeid(*data[i * cols + j]).name() == typeid(Prey).name()) {
      if (!data[i * cols + j]->isMoved) {
            bool moved = false;
            while (!moved) {
                int direction = rand() % 4;
                int dirX = 0, dirY = 0;
                if (direction == 0) dirY = -1; // вверх
                else if (direction == 1) dirY = 1; // вниз
                else if (direction == 2) dirX = -1; // влево
                else dirX = 1; // вправо

                size_t ni = (i + dirX + rows) % rows;
                size_t nj = (j + dirY + cols) % cols;

                if (data[ni * cols + nj]->getSymbol() == " ") {
                    std::swap(data[i * cols + j], data[ni * cols + nj]);
                    data[i * cols + j]->isMoved = true;
                    moved = true;
                }
            }
        }
    }
}

};

bool MoveAction::apply(Ocean& ocean) {
  size_t rows = ocean.getRows();
  size_t cols = ocean.getCols();
  std::vector<Object*>& data = ocean.getData();

  if (newX >= 0 && newX < rows && newY >= 0 && newY < cols) {
    if (data[newX * cols + newY]->getSymbol() == " ") {
      data[newX * cols + newY] = obj;
      obj->x = newX;
      obj->y = newY;
      obj->isMoved = true;
      data[obj->x * cols + obj->y] = new Empty();
      return true;
    }
  }
  return false;
}

int main() {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif
  Ocean ocean(10, 10);

  while (true) {
    ocean.display();
    ocean.tick();
    if (ocean.getIterationCounter() >= 50)
      break;
  }

  return 0;
}
