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

void clearScreen() {
#ifdef _WIN32
    COORD topLeft = {0, 0};
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y * 2, topLeft, &written);
    FillConsoleOutputAttribute(console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE, screen.dwSize.X * screen.dwSize.Y * 2, topLeft, &written);
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

class Object {
  public:
    bool isMoved = false;
    virtual ~Object() {};
    virtual void update() = 0;
    virtual std::string getSymbol() const = 0;
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
    Reef () : turnsToStone(std::rand() % 7) {symbol = u8"🍙";}
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
    bool isAdult;
    int turnsToReproduce;
    std::string symbol;
  public:
    Prey () : age(0), maxAge(10), isAdult(false), turnsToReproduce(7), symbol(u8"∞" ) /* symbol(u8"><>" ) */{}
    void update() override {
      age++;
      if (age >= 7)
        isAdult = true;
      if (age == 20){
        symbol = u8"X";
        return;
      } else if (age == 22){
        symbol = u8" ";
        return;
      }
      
      if (isAdult)
        turnsToReproduce == 0 ? turnsToReproduce = 10 : turnsToReproduce--;

      if (isAdult && turnsToReproduce == 0)
        turnsToReproduce = 10;
    }

    std::string getSymbol() const override { return symbol; }
};


struct Ocean final {
  public:
    size_t iterationCounter;

    Ocean(size_t r, size_t c) : rows{r}, cols{c}, data{r * c}, iterationCounter{0}, stagnantCounter{0}{
      srand(static_cast<unsigned int>(time(nullptr)));
      // Fill the ocean with random organisms
      for (size_t i = 0; i < r; i++){
        for (size_t j = 0; j < c; j++)
          data[i * c + j] = generateRandomObject();
      }
    }

  void tick() { 
    for (size_t i = 0; i < rows; i++) {
      for (size_t j = 0; j < cols; j++) {
        if (data[i * cols + j]->getSymbol() == u8"∞" && !data[i * cols + j]->isMoved) {
          bool moved = false;
          while (!moved) {
            int direction = rand() % 4; // 0: up, 1: down, 2: left, 3: right
            int dirX = 0, dirY = 0;
            if (direction == 0) dirY = -1; // Move up
            else if (direction == 1) dirY = 1; // Move down
            else if (direction == 2) dirX = -1; // Move left
            else dirX = 1; // Move right

            size_t ni = i + dirX;
            size_t nj = j + dirY;

            if (ni >= 0 && ni < rows && nj >= 0 && nj < cols && data[ni * cols + nj]->getSymbol() == " ") {
              std::swap(data[i * cols + j], data[ni * cols + nj]);
              data[ni * cols + nj]->isMoved = true;
              moved = true;
            }
          }
        }
      }
    }

    // Reset isMoved flag after each tick
    for (size_t i = 0; i < rows; i++) {
      for (size_t j = 0; j < cols; j++) {
        if (data[i * cols + j]->getSymbol() == u8"∞") 
          data[i * cols + j]->isMoved = false;
      }
    }

    
    for (size_t i = 0; i < rows; i++){
      for (size_t j = 0; j < cols; j++) {
        data[i * cols + j]->update();
        // createNewPreyIfPossible(i, j); // Check and create new Prey if possible
      }
    }

    iterationCounter++;
  }

  void display(){
    clearScreen(); // Clear the screena 
    std::cout << "Iteration count: " << iterationCounter << std::endl;
    for (size_t i = 0; i < rows; i++){
      for (size_t j = 0; j < cols; j++){
        std::cout << data[i*cols + j]->getSymbol() << " ";
      }
      std::cout << std::endl;
      //  << std::endl;
    }
    // std::this_thread::sleep_for(std::chrono::seconds(2)); // Delay for 2 seconds
    Sleep(2000);
    std::cout << std::endl;
  }

  private:
    size_t rows, cols;
    std::vector<Object*> data;
    size_t stagnantCounter;

    Object* generateRandomObject(){
      int randNum = rand() % 100; 

        if (randNum < 70)
          return new Empty();
        else if (randNum >= 70 && randNum < 80)
          return new Stone();
        else if (randNum >= 80 && randNum < 90)
          return new Reef();
        else 
          return new Prey();
    }

    void createNewPreyIfPossible(size_t i, size_t j) {
      bool maturePreyAdjacent = false;
      bool emptyCellAdjacent = false;
      for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
          size_t ni = i + x;
          size_t nj = j + y;
          if (ni >= 0 && ni < rows && nj >= 0 && nj < cols && abs(x)+abs(y)==1) {
            // if (data[ni * cols + nj]->getSymbol() == u8"\U0001F990") 
            if (data[ni * cols + nj]->getSymbol() == u8"∞") 
              maturePreyAdjacent = true;
            else if (data[ni * cols + nj]->getSymbol() == " ")
              emptyCellAdjacent = true;
          }
        }
      }

      if (maturePreyAdjacent && emptyCellAdjacent) {
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

    void movePrey() {
      for (size_t i = 0; i < rows; i++) {
        bool ch = false;
        for (size_t j = 0; j < cols; j++) {
          // if (data[i * cols + j]->getSymbol() == u8"\U0001F990") { // If Prey
          if (data[i * cols + j]->getSymbol() == u8"∞") {
            int dirX = rand() % 3 - 1; // -1, 0, 1
            int dirY = rand() % 3 - 1; // -1, 0, 1
            size_t ni = i + dirX;
            size_t nj = j + dirY;
            if (ni >= 0 && ni < rows && nj >= 0 && nj < cols && data[ni * cols + nj]->getSymbol() == " ") {
              std::swap(data[i * cols + j], data[ni * cols + nj]);
              ch = true;
              break;
            }
          }
        }
        if (ch)
          break;
      }
    }
};

int main(){
  SetConsoleOutputCP(CP_UTF8);
  std::string str = u8"\u263A";
  std::cout << "Reef symbol: " << "🪸" << std::endl; // Unicode smiley face

  Ocean ocean(10, 10);

  while (true){
    ocean.display();
    ocean.tick();
    if (ocean.iterationCounter >= 50)
      break;
  }

  return 0;
}
