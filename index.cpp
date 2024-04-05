#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>

struct Cell {
  std::string symbol; 
};

class Object {
  public:
    virtual ~Object() {};
    virtual void update() = 0;
    std::string getSymbol() const {return symbol; }
  protected:
    std::string symbol;
};

class Stone : public Object {
  public:
    Stone () {symbol = "";}
    void update() override {} 
};

class Reef : public Object {
  public:
    Reef () {symbol = "🐚";}
    void update() override {} 
};

struct Ocean final {
  public:
    size_t iterationCounter;

    Ocean(size_t r, size_t c) : rows{r}, cols{c}, data{r * c}, iterationCounter{0}, stagnantCounter{0}{
      srand(static_cast<unsigned int>(time(nullptr)));
      // Fill the ocean with random organisms
      for (size_t i = 0; i < r; i++){
        for (size_t j = 0; j < c; j++){
          data[i * c + j] = generateRandomObject();
        }
      }
    }

    // Neighbourhood generate_neighbourhood(size_t i, size_t j, size_t r) {
    //   /* code */
    // }

  void tick() { 
    for (size_t i = 0; i < rows; i++){
      for (size_t j = 0; j < cols; j++){
        data[i * cols + j]->update();
      }
    }
    iterationCounter++;
  }

  void display(){
    std::cout << "Iteration count: " << iterationCounter << std::endl;
    for (size_t i = 0; i < rows; i++){
      for (size_t j = 0; j < cols; j++)
        std::cout << data[i*cols + j] -> getSymbol() << " ";
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

  private:
    size_t rows, cols;
    std::vector<Object*> data;
    size_t stagnantCounter;

    Object* generateRandomObject(){
      int randNum = rand() % 30; 

        if (randNum < 15)
          return new Stone();
        else
          return new Reef();
        // else if (randNum < 15)
        //   return new Prey();
        // else if (randNum < 15)
        //   return new Predator();
        // else
        //   return new ApexPredator();
    }
};

int main(){
  Ocean ocean(10, 10);

  while (true){
    ocean.display();
    ocean.tick();
    if (ocean.iterationCounter >= 10)
      break;
  }

  return 0;
}