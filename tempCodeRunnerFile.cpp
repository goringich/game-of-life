#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <codecvt>
#include <locale>
#include <thread>
#include <chrono> 
#include <windows.h>
#include <conio.h>
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
    COORD coord;
    coord.X = x1;
    coord.Y = y1;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Define the Action struct
struct Action {
    Object* obj;

    Action(Object* obj)
        : obj(obj) {}

    virtual ~Action() = default;

    bool operator()(Ocean& ocean) { return apply(ocean); }

protected:
    virtual bool apply(Ocean& ocean) = 0;
};

// Define the MoveAction struct
struct MoveAction : public Action {
    int newX, newY;

    MoveAction(Object* obj, int newX, int newY)
        : Action(obj), newX(newX), newY(newY) {} // Constructor

private:
    bool apply(Ocean& ocean) override {
        size_t rows = ocean.getRows();
        size_t cols = ocean.getCols();
        std::vector<Object*>& data = ocean.getData();

        // Check if the new position is within the ocean bounds
        if (newX >= 0 && newX < rows && newY >= 0 && newY < cols) {
            // Check if the new position is empty
            if (data[newX * cols + newY]->getSymbol() == " ") {
                // Move the object to the new position
                data[newX * cols + newY] = obj;
                obj->x = newX; // Update the object's x position
                obj->y = newY; // Update the object's y position
                obj->isMoved = true; // Set the isMoved flag
                // Clear the old position
                data[obj->x * cols + obj->y] = new Empty();
                return true; // Successfully moved the object
            }
        }
        return false; // Failed to move the object
    }
};




// Define the Object class
class Object {
public:
    int x; // Row position
    int y; // Column position
    bool isMoved; // Flag to track if the object has been moved

    Object(int x, int y) : x(x), y(y), isMoved(false) {} // Constructor

    virtual ~Object() {} // Virtual destructor
    virtual void update() = 0; // Pure virtual function to update object state
    virtual std::string getSymbol() const = 0; // Pure virtual function to get object symbol
};


class Empty : public Object {
  public:
    Empty () {symbol = " ";}
    void update() override {}
    std::string getSymbol() const override { return symbol; }
  private:
    std::string symbol;
};

class Stone : public Object {
  private:
    int turnsToReef;
  public:
    Stone () : turnsToReef(std::rand() % 7) {symbol = u8"\u26F0";}
    void update() override {
      if (turnsToReef == 0) {
        symbol = u8"&";
      } else 
        turnsToReef--;
    }
    std::string getSymbol() const override { return symbol; }
  private:
    std::string symbol;
};

class Reef : public Object {
  private:
    int turnsToStone;
  public:
    Reef () : turnsToStone(std::rand() % 7) {symbol = u8"➿";}
    void update() override {
      if (turnsToStone == 0)
        symbol = u8"\u26F0";
      else 
        turnsToStone--;
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
    std::string symbol;
  public:
    Prey () : age(0), maxAge(10), turnsToReproduce(7), symbol(u8"~" ) /* symbol(u8"><>" ) */{}
    void update() override {
      age++;
      if (age >= 3)
        isAdult = true;
      if (age == 20){
        symbol = u8"X";
        return;
      } else if (age == 22){
        symbol = u8" ";
        return;
      }
      
      if (isAdult){
        turnsToReproduce == 0 ? turnsToReproduce = 10 : turnsToReproduce--;
        symbol = u8"∞";
      } else {
        symbol = u8"~";
      }
      if (isAdult && turnsToReproduce == 0)
        turnsToReproduce = 10;
    }

    std::string getSymbol() const override { return symbol; }
};

class Predator : public Object {
  private:
    int hungerLevel;
    bool isHungry;
    int turnsToReproduce;
    bool isAdult;

  public:
    Predator() : Object(rand() % 10, rand() % 10), hungerLevel(0), isHungry(false), turnsToReproduce(rand() % 5 + 5), isAdult(false) {
      symbol = u8"🦈";
    }

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



struct Ocean final {
  public:
    int start_emptyCount = 0, start_stoneCount = 0, start_reefCount = 0, start_preyCount = 0;
    size_t iterationCounter;
    using Cell = Object*;

    Ocean(size_t r, size_t c) : rows{r}, cols{c}, data{r * c}, iterationCounter{0}, stagnantCounter{0}{
      srand(static_cast<unsigned int>(time(nullptr)));
      // Fill the ocean with random organisms
      for (size_t i = 0; i < r; i++){
        for (size_t j = 0; j < c; j++)
          data[i * c + j] = generate_neighbourhood(i, j);
      }
    }

    ~Ocean() {
      for (auto& obj : data) {
        delete obj;
      }
    }

  void tick() { 
    // Reset isMoved flag after each tick
    for (size_t i = 0; i < rows; i++) {
      for (size_t j = 0; j < cols; j++) {
        data[i * cols + j]->update();
        if (data[i * cols + j]->isMoved == false){
          move_prey(i, j);
        }
      }
    }

    for (size_t i = 0; i < rows; i++) {
      for (size_t j = 0; j < cols; j++) {
        data[i * cols + j]->isMoved = false;
      }
    }

    iterationCounter++;
  }

  void display(){
    // setcur(-6, 0);
    clearScreen(); // Clear the screena 
    std::cout << "Iteration count: " << iterationCounter << std::endl;
    std::cout << "emptyCount: " << start_emptyCount << std::endl;
    std::cout << "stoneCount: " << start_stoneCount << std::endl;
    std::cout << "reefCount: " << start_reefCount << std::endl;
    std::cout << "preyCount: " << start_preyCount << std::endl;
    for (size_t i = 0; i < rows; i++){
      for (size_t j = 0; j < cols; j++){
        std::cout << data[i*cols + j]->getSymbol() << " ";
      }
      std::cout << std::endl;
    }
    Sleep(3000);
    std::cout << std::endl;
  }

  size_t getRows() const { return rows; }
  size_t getCols() const { return cols; }
  std::vector<Cell>& getData() { return data; }

  private:
    size_t rows, cols;
    std::vector<Object*> data;
    size_t stagnantCounter;

    // std::vector<std::unique_ptr<Object>> data;


    /* ----------------- GENERATION -------------------- */
    Cell generate_neighbourhood(int x, int y) {
      int randNum = rand() % 100; 
      Object* newObj;
      if (randNum < 70){
        newObj = new Empty();
        ++start_emptyCount;
      } else if (randNum >= 70 && randNum < 80){
        newObj = new Stone();
        ++start_stoneCount;
      }
      else if (randNum >= 80 && randNum < 97){
        newObj = new Reef();
        ++start_reefCount;
      } else {
        newObj = new Prey();
        ++start_preyCount;
      }
      return newObj;
    }
    /* ----------------- GENERATION -------------------- */



    void createNewPreyIfPossible(size_t i, size_t j) {
      bool maturePreyAdjacent = false;
      bool emptyCellAdjacent = false;
      for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
          size_t ni = i + x;
          size_t nj = j + y;
          if (ni >= 0 && ni < rows && nj >= 0 && nj < cols && std::abs(static_cast<int>(nj)) != std::abs(static_cast<int>(ni))) {
            if (data[ni * cols + nj]->getSymbol() == u8"∞" && data[ni * cols + nj]->isAdult == true) 
              maturePreyAdjacent = true;
            else if (data[ni * cols + nj]->getSymbol() == " ")
              emptyCellAdjacent = true;
          }
        }
      }

      if (maturePreyAdjacent && emptyCellAdjacent && data[i * cols + j]->isAdult == true) {
        for (int x = -1; x <= 1; ++x) {
          for (int y = -1; y <= 1; ++y) {
            size_t ni = i + x;
            size_t nj = j + y;
            if (ni >= 0 && ni < rows && nj >= 0 && nj < cols && data[ni * cols + nj]->getSymbol() == " ") {
              data[ni * cols + nj] = new Prey();
              return;
            }
          }
        }
      }
    }

    void move_prey(int i, int j){
      if (typeid(*data[i * cols + j]).name() == typeid(Prey).name()) {
        if (!data[i * cols + j]->isMoved) {
            bool moved = false;
            while (!moved) {
                int direction = rand() % 4; // 0: up, 1: down, 2: left, 3: right
                int dirX = 0, dirY = 0;
                if (direction == 0) dirY = -1; // Move up
                else if (direction == 1) dirY = 1; // Move down
                else if (direction == 2) dirX = -1; // Move left
                else dirX = 1; // Move right

                size_t ni = (i + dirX + rows) % rows; // Ensure wrapping around borders
                size_t nj = (j + dirY + cols) % cols;

                if (data[ni * cols + nj]->getSymbol() == " " && abs(dirX) + abs(dirY) == 1) {
                  std::swap(data[i * cols + j], data[ni * cols + nj]);
                  data[i * cols + j]->isMoved = true;
                  moved = true;
                }
            }
        }

        //shift on start
        if (i * cols + j + 1 == rows * cols - 1 && typeid(*data[0]).name() == "Empty") {
          std::swap(data[0], data[i * cols + j]);
          data[0]->isMoved = 1;
          return;
        }
        if (i * cols + j + 1 >= rows * cols)
          j = 0;
        else if (i * cols + j + 2 >= rows * cols)
          j = 0;
        //shifting
        if (typeid(*data[i * cols + j + 1]).name() == "Empty") {
          std::swap(data[i * cols + j + 1], data[i * cols + j]);
          if (i * cols + j + 1 < rows * cols)
            data[i * cols + j + 1]->isMoved = 1;
          return;
        }
      }
    }



};

int main(){
  SetConsoleOutputCP(CP_UTF8);
  // std::string str = u8"\u263A";
  // std::cout << "Reef symbol: " << "🪸" << std::endl; // Unicode smiley face

  Ocean ocean(10, 10);

  system("cls");
  while (true){
    ocean.display();
    ocean.tick();
    if (ocean.iterationCounter >= 50)
      break;
  }

  return 0;
}
