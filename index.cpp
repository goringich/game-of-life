#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <codecvt>
#include <locale>
#include <thread>
#include <chrono> 


// std::wstring utf8_to_wstring(const std::string& str) {
//     std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
//     return converter.from_bytes(str);
// }

void clearScreen() {
  #ifdef _WIN32
    std::system("cls");
  #else
    // Assume POSIX
    std::system("clear");
  #endif
}

struct Cell {
  std::wstring symbol; 
};

class Object {
  public:
    virtual ~Object() {};
    virtual void update() = 0;
    std::string getSymbol() const {return symbol; }
  protected:
    std::string symbol;
};

class Empty : public Object {
  public:
    Empty () {symbol = u8" ";}
    void update() override {}
};

class Stone : public Object {
  private:
    int turnsToReef;
  public:
    Stone () : turnsToReef(std::rand() % 3+5) {symbol = u8"\u26F0";}
    void update() override {
      if (turnsToReef == 0) {
        // delete this;//🪸
        this->symbol = u8"🪸";
        return;
      } else 
        turnsToReef--;
    }
};

class Reef : public Object {
  private:
    int turnsToStone;
  public:
    Reef () : turnsToStone(std::rand() % 3+5) {symbol = u8"🪸";}
    void update() override {
      if (turnsToStone == 0) {
        // delete this;
        this->symbol = u8"\u26F0";
        return;
      } else 
        turnsToStone--;
    }
  
};

class Prey : public Object {
  private:
    int age;
    int maxAge;
    bool isAdult;
    int turnsToReproduce;
  public:
    Prey () : age(0), maxAge(10), isAdult(false), turnsToReproduce(7) {symbol = u8"\U0001F990";}
    void update() override {
      age++;
      if (age >= 7)
        isAdult = true;
      if (age == 10){
        symbol = u8"💀";
        return;
      } else if (age == 12){
        symbol = u8" ";
        return;
      }
      
      if (isAdult)
        turnsToReproduce == 0 ? turnsToReproduce = 10 : turnsToReproduce--;
    }
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

    // Neighbourhood generate_neighbourhood(size_t i, size_t j, size_t r) {
    //   /* code */
    // }

  void tick() { 
    for (size_t i = 0; i < rows; i++){
      for (size_t j = 0; j < cols; j++)
        data[i * cols + j]->update();
    }
    iterationCounter++;
  }

  void display(){
    std::cout << "Iteration count: " << iterationCounter << std::endl;
    for (size_t i = 0; i < rows; i++){
      for (size_t j = 0; j < cols; j++){
        // if(std::cout.fail())
        //   std::cout << "hello world!";
        std::cout << data[i*cols + j] -> getSymbol() << " ";
      }
      std::cout << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Delay for 1 second
    clearScreen(); // Clear the screen
    std::cout << std::endl;
  }

  private:
    size_t rows, cols;
    std::vector<Object*> data;
    size_t stagnantCounter;

    Object* generateRandomObject(){
      int randNum = rand() % 100; 

        if (randNum < 30)
          return new Empty();
        else if (randNum >= 30 && randNum < 50)
          return new Stone();
        else if (randNum >= 50 && randNum < 80)
          return new Reef();
        else 
          return new Prey();
        // else if (randNum < 15)
        //   return new Prey();
        // else if (randNum < 15)
        //   return new Predator();
        // else
        //   return new ApexPredator();
    }
};

int main(){
  std::string str = u8"\u263A";
  std::cout << "Reef symbol: " << str << std::endl; // Unicode smiley face


  Ocean ocean(10, 10);

  while (true){
    ocean.display();
    ocean.tick();
    if (ocean.iterationCounter >= 20)
      break;
  }

  return 0;
}