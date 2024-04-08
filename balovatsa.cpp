#include <iostream>
#include <cmath>

int main() {
    double epsilon = 30; // Заданный порог
    double prev = 1.0; // Первое число
    double next = 10.0; // Второе число
    int n = 1; // Начальная степень

    while (true) {
        double diff = next - prev; // Вычисляем разницу между числами
        double percent_diff = (diff / prev) * 100.0; // Находим разницу в процентах

        // Если разница в процентах меньше заданного порога, завершаем цикл
        if (percent_diff < epsilon) {
            break;
        }

        // Переходим к следующей степени
        prev = next;
        next *= 10;
        n++;
    }

    std::cout << "Число 10 в степени " << n << " равно " << prev << std::endl;

    return 0;
}
