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

// void clearScreen() {
// #ifdef _WIN32
//     COORD topLeft = {0, 0};
//     HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
//     CONSOLE_SCREEN_BUFFER_INFO screen;
//     DWORD written;

//     GetConsoleScreenBufferInfo(console, &screen);
//     FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
//     FillConsoleOutputAttribute(console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE, screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
//     SetConsoleCursorPosition(console, topLeft);
// #else
//     std::system("clear");
// #endif
// }


void setcur(int x1, int y1) {
    COORD coord;
    coord.X = x1;
    coord.Y = y1;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

class Object {
  public:
    bool isMoved = false;
    bool isAdult = false; 
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
    Prey () : age(0), maxAge(10), turnsToReproduce(7), symbol(u8"∞" ) /* symbol(u8"><>" ) */{}
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
      
      if (isAdult)
        turnsToReproduce == 0 ? turnsToReproduce = 10 : turnsToReproduce--;

      if (isAdult && turnsToReproduce == 0)
        turnsToReproduce = 10;
    }

    std::string getSymbol() const override { return symbol; }
};

// class Predator : public Object {
//   private:
//     int age;
//     int maxAge;
//     bool isAdult;
//     bool isHangry;
//     int turnsToReproduce;
//     std::string symbol;
//   public:
//     Predator () : age(0), maxAge(7), isAdult(false), isHangry(false), 
// }


struct Ocean final {
  public:
    int start_emptyCount = 0, start_stoneCount = 0, start_reefCount = 0, start_preyCount = 0;
    size_t iterationCounter;

    Ocean(size_t r, size_t c) : rows{r}, cols{c}, data{r * c}, iterationCounter{0}, stagnantCounter{0}{
      srand(static_cast<unsigned int>(time(nullptr)));
      // Fill the ocean with random organisms
      for (size_t i = 0; i < r; i++){
        for (size_t j = 0; j < c; j++)
          data[i * c + j] = generateRandomObject();
      }
    }

    ~Ocean() {
      for (auto& obj : data) {
          delete obj;
      }
    }

  void tick() { 
    // for (size_t i = 0; i < rows; i++) {
    //   for (size_t j = 0; j < cols; j++) {
    //     if (data[i * cols + j]->getSymbol() == u8"∞" && !data[i * cols + j]->isMoved) {
    //       bool moved = false;
    //       while (!moved) {
    //         int direction = rand() % 4; // 0: up, 1: down, 2: left, 3: right
    //         int dirX = 0, dirY = 0;
    //         if (direction == 0) dirY = -1; // Move up
    //         else if (direction == 1) dirY = 1; // Move down
    //         else if (direction == 2) dirX = -1; // Move left
    //         else dirX = 1; // Move right

    //         size_t ni = i + dirX;
    //         size_t nj = j + dirY;

    //         if (ni < 0)
    //           ni = rows - 1; //=rows - 1 = 29
    //         else if (ni >= rows)
    //           ni = 0;
    //         if (nj < 0)
    //           nj = cols - 1;
    //         else if (nj >= cols)
    //           nj = 0;

    //         if (ni >= 0 && ni < rows && nj >= 0 && nj < cols && data[ni * cols + nj]->getSymbol() == " ") {
    //           std::swap(data[i * cols + j], data[ni * cols + nj]);
    //           data[ni * cols + nj]->isMoved = true;
    //           moved = true;
    //         }
    //       }
    //     }
    //   }
    // }

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
        createNewPreyIfPossible(i, j);
      }
    }

    iterationCounter++;
  }

  void display(){
    // clearScreen(); // Clear the screena 
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
    Sleep(1000);
    std::cout << std::endl;
  }

  private:
    size_t rows, cols;
    std::vector<Object*> data;
    size_t stagnantCounter;

    // std::vector<std::unique_ptr<Object>> data;


    /* ----------------- GENERATION -------------------- */
    Object* generateRandomObject(){
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
  // std::string str = u8"\u263A";
  // std::cout << "Reef symbol: " << "🪸" << std::endl; // Unicode smiley face

  Ocean ocean(10, 10);

  while (true){
    setcur(0, 0);
    ocean.display();
    ocean.tick();
    if (ocean.iterationCounter >= 50)
      break;
  }

  return 0;
}
