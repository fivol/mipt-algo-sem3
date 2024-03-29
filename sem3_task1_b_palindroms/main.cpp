//B. Палиндромы

//Строка называется палиндромом, если она одинаково читается как слева направо, так и справа налево. Например, «abba» – палиндром, а «roman» – нет.
//
//Для заданной строки s длины n (1 ≤ n ≤ 105) требуется подсчитать число пар (i, j), 1 ≤ i < j ≤ n, таких что подстрока s[i..j] является палиндромом

#include <iostream>
#include "./solution.cpp"


int main() {
    std::string text;
    std::cin >> text;

    std::cout << palindromeSubstringsAmount(text);

    return 0;
}
