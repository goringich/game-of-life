#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <Windows.h>
#include <string>
#include <codecvt>
#include <locale>
#include <thread>
#include <chrono>
 
//функция для обновления экрана без просадки FPS (моя фишка)
void setcur(int x1, int y1) {
    COORD coord;
    coord.X = x1;
    coord.Y = y1;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}


class Object {
public:
    std::string symbol;
    virtual ~Object() {};
    virtual void update() = 0;
    virtual std::string getSymbol() const = 0;
    int getFlag = 0;
    int not_eat = 0;
    int speed = 1;
};

class Empty : public Object {
private:
    int flag;
public:
    Empty() : flag(0) { symbol = " "; }
    void update() override {}

    std::string getSymbol() const override { return symbol; }
};

class Stone : public Object {
private:
    int to_reef;
    int flag;
public:
    Stone() : to_reef(std::rand() % 3 + 5), flag(0) { symbol = "*"; }

    void update() override {
        if (to_reef == 0) {
            symbol = "&";
        }
        else
            to_reef--;
    }
    std::string getSymbol() const override { return symbol; }
};

class Reef : public Object {
private:
    int to_stone;
    int flag;
public:
    Reef() : to_stone(std::rand() % 3 + 5), flag(0) { symbol = "&"; }
    void update() override {
        if (to_stone == 0)
            symbol = "*";
        else
            to_stone--;
    }
    std::string getSymbol() const override { return symbol; }
};

class Prey : public Object {
private:
    int age;
    std::string symbol; // Declare symbol member variable
public:
    Prey() : age(0), symbol("~") {}
    void update() override {
        age++;
        if (age == 10) {
            symbol = ">";
            return;
        }
        else if (age == 20) {
            symbol = " ";
            return;
        }
    }
    std::string getSymbol() const override { return symbol; }
};

class ApexPredator : public Object {
private:
    int age;
    int not_eat;
    int speed;
    std::string symbol; // Declare symbol member variable
public:
    ApexPredator() : age(0), symbol("?"), not_eat(0), speed(1) {}
    void update() override {
        age++;
        if (age == 15) {
            symbol = "$";
            return;
        }
        else if (age == 35) {
            symbol = " ";
            return;
        }
    }
    std::string getSymbol() const override { return symbol; }
};


struct Ocean final {
public:
    using Cell = Object*;
    size_t iteration_Counter;

    Ocean(size_t r, size_t c) : rows{ r }, cols{ c }, data{ r * c }, iteration_Counter{ 1 } {
        //делаем уникальность чисел
        srand(static_cast<unsigned int>(time(nullptr)));
        //Наполните океан случайными организмами
        for (size_t i = 0; i < r; i++) {
            for (size_t j = 0; j < c; j++)
                data[i * c + j] = generate_neighbourhood(i, j);
        }
    }

    void tick() {
        for (size_t i = 0; i < rows; i++) {
            for (size_t j = 0; j < cols; j++) {
                data[i * cols + j]->update();
                if (data[i * cols + j]->getFlag == 0) {
                    move_prey(i, j);
                    move_apex(i,j);
                }
                createNewPreyIfPossible(i, j);
            }
        }

        for (size_t i = 0; i < rows; i++) {
            for (size_t j = 0; j < cols; j++) {
                data[i * cols + j]->getFlag = 0;
            }
        }


        iteration_Counter++;
    }

    void display() {
        setcur(0, 0);
        std::cout << "Iteration = " << iteration_Counter << std::endl;
        for (size_t i = 0; i < rows; i++) {
            for (size_t j = 0; j < cols; j++) {
                std::cout << data[i * cols + j]->getSymbol() << " ";
            }
            std::cout << std::endl;
        }
        Sleep(500);
        std::cout << std::endl;
    }

private:
    size_t rows, cols;
    std::vector<Cell> data;

    Cell generate_neighbourhood(int x, int y) {
        int randNum = rand() % 100;

        if (randNum < 35 || randNum > 49)
            return new Empty();
        else if (randNum >= 35 && randNum < 38)
            return new Stone();
        else if (randNum >= 39 && randNum < 43)
            return new Reef();
        else if (randNum >= 44 && randNum < 49)
            return new Prey();
        else 
            return new ApexPredator();
    }

    void createNewPreyIfPossible(size_t i, size_t j) {
        if (data[i * cols + j]->getSymbol() == ">") {
            bool maturePreyAdjacent = false; //зрелая добыча Рядом
            bool emptyCellAdjacent = false; // пустая ячейка Рядом
            bool emptyPredatorAdjacent = false; //нет рядом хищника
            int par_x, par_y; //координаты второй половинки
            int create_x, create_y; //координаты роддома
            for (int x = -1; x <= 1; x++) {
                for (int y = -1; y <= 1; y++) {
                    size_t _x = i + x;
                    size_t _y = j + y;
                    if (_x >= 0 && _x < rows && _y >= 0 && _y < cols) {
                        //рыба еще не размножалась
                        if (data[_x * cols + _y]->getSymbol() == ">") {
                            maturePreyAdjacent = true;
                            par_x = _x;
                            par_y = _y;
                        }
                        //рядом есть место родить
                        else if (data[_x * cols + _y]->getSymbol() == " ") {
                            emptyCellAdjacent = true;
                            create_x = _x;
                            create_y = _y;
                        }
                        //рядом нет хищника
                        else if (data[_x * cols + _y]->getSymbol() == "$" || data[_x * cols + _y]->getSymbol() == "@") {
                            emptyPredatorAdjacent = true;
                        }
                    }
                }
            }

            if (maturePreyAdjacent && emptyCellAdjacent && !emptyPredatorAdjacent) {
                int start = 1;
                int end = 100;
                int x = rand() % (end - start + 1) + start;
                if (x%4 == 0)
                    data[create_x * cols + create_y] = new Prey();
                return;               
            }
        }
    }

    void move_prey(size_t i, size_t j) {
        if (data[i * cols + j]->getSymbol() == ">" || data[i * cols + j]->getSymbol() == "~") {
            //проверка на хищника
            for (int x = -1; x <= 1; ++x) {
                for (int y = -1; y <= 1; ++y) {
                    size_t _x = i + x;
                    size_t _y = j + y;
                    if (_x >= 0 && _x < rows && _y >= 0 && _y < cols) {
                        //если хищник , то даём дёру
                        if (data[_x * cols + _y]->getSymbol() == "$") {
                            if (data[i * cols + j - 1]->getSymbol() == " " && (i * cols + j - 1) < rows * cols) {
                                std::swap(data[i * cols + j - 1], data[i * cols + j]);
                                data[i * cols + j - 1]->getFlag = 1;
                                return;
                            }
                        }
                    }
                }
            }

            //сдвигаем в начало
            if (i * cols + j + 1 == rows * cols - 1 && data[0]->getSymbol() == " ") {
                std::swap(data[0], data[i * cols + j]);
                data[0]->getFlag = 1;
                return;
            }
            if (i * cols + j + 1 >= rows * cols)
                j = 0;
            else  if (i * cols + j + 2 >= rows * cols)
                j = 0;
            //передвигаем рыбку
            if (data[i * cols + j + 1]->getSymbol() == " ") {
                std::swap(data[i * cols + j + 1], data[i * cols + j]);
                if (i * cols + j + 1 < rows * cols) {
                    data[i * cols + j + 1]->getFlag = 1;
                }
                return;
            }
            //перепрыгиваем через препятствие
            else if (data[i * cols + j + 1]->getSymbol() == "*" || data[i * cols + j + 1]->getSymbol() == "&" || data[i * cols + j + 1]->getSymbol() == ">" || data[i * cols + j + 1]->getSymbol() == "?") {
                if (data[i * cols + j + 2]->getSymbol() == " ") {
                    std::swap(data[i * cols + j + 2], data[i * cols + j]);
                    data[i * cols + j + 2]->getFlag = 1;
                    return;
                }
            }
        }

    }


    //оставь надежду всяк сюда входящий
    bool reef_safe(size_t i, size_t j) {
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                size_t _x = i + x;
                size_t _y = j + y;
                if (_x >= 0 && _x < rows && _y >= 0 && _y < cols) {
                    if (data[_x * cols + _y]->getSymbol() == "&")
                        return true;
                }
            }
        }
        return false; //yoy can eat 
    }
    

    void move_apex(size_t i, size_t j) {
        //если взрослая особь
        if (data[i * cols + j]->getSymbol() == "$") {
            if (i * cols + j + 1 >= rows * cols)
                j = 0;
            //проверка на начилие поблизости еды
            for (int x = -1 * data[i * cols + j]->speed; x <= 1 * data[i * cols + j]->speed; ++x) {
                for (int y = -1 * data[i * cols + j]->speed; y <= 1 * data[i * cols + j]->speed; ++y) {            
                    size_t _x = i + x;
                    size_t _y = j + y;
                    if (_x >= 0 && _x < rows && _y >= 0 && _y < cols) {
                        if (data[_x * cols + _y]->getSymbol() == "~" || data[_x * cols + _y]->getSymbol() == ">") {
                            //нет рифов рядом с едой 
                            if (reef_safe(_x, _y) == false) {
                                data[_x * cols + _y] = new Empty(); //уничтожение рыбы
                                std::swap(data[_x * cols + _y], data[i * cols + j]);
                                data[_x * cols + _y]->getFlag = 1;
                                return;
                            }
                        }
                    }
                }
            }
                           
            //еды нет
            data[i * cols + j]->not_eat += 1;
            if (data[i * cols + j]->not_eat == 3) {
                data[i * cols + j]->speed = 3; //от голода выросла скорость
            }
            //умер от когода
            if (data[i * cols + j]->not_eat == 6) {
                data[i * cols + j] = new Empty();
            }
            //если еды нет то рандомно встаем на позицию
            int chet = 1;
            while (chet < 10) {
                int start = -1;
                int end = 1;
                int x = rand() % (end - start + 1) + start;
                int y = rand() % (end - start + 1) + start;
                if (data[x * cols + y]->getSymbol() == " " && x >= 0 && x < rows && y >= 0 && y < cols) {
                    std::swap(data[x * cols + y], data[i * cols + j]);
                    data[x * cols + y]->getFlag = 1;
                    return;
                } 
                chet += 1;
            }
        }
    }
};

int main() {
    setlocale(LC_ALL, "RU");
    int n, m;

    std::cout << "Введите размер для океана" << std::endl;
    std::cout << "Рекомендую размер океана 30 на 30" << std::endl;
    std::cout << "И обязательно сделайте полноэкранный режим для лучшего просмотра" << std::endl;
    std::cin >> n >> m;
    Ocean ocean(n, m);

    //старт игры
    system("cls");
    while (true) {
        ocean.tick();
        ocean.display();
        if (ocean.iteration_Counter >= 40)
            break;
    }

    return 0;
}



// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.





// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.




// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
