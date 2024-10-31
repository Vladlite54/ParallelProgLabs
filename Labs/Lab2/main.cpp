#include <iostream>
#include <vector>
#include <algorithm>
#include <omp.h>

bool isPalindrome(int num) {
    std::string str = std::to_string(num);
    std::string reversedStr = std::string(str.rbegin(), str.rend());
    return str == reversedStr;
}

int findSmallestPalindrome(int N) {
    int num = N + 1;
    
    while (true) {
        if (isPalindrome(num)) {
            // Проверяем, является ли палиндром произведением двух различных чисел
            for (int i = 1; i < num; ++i) {
                if (num % i == 0) {
                    int j = num / i;
                    if (i != j) {
                        return num; // нашли палиндром, который является произведением двух различных чисел
                    }
                }
            }
        }
        num++;
    }
}

void detectTime(int (*target)(int), int N) {
    double start = omp_get_wtime();
    int result = (*target)(N);
    std::cout << "Palindrome: " << result;
    double end = omp_get_wtime();
    double duration = end - start;
    std::cout << " Time: " << duration;
}

void test() {
    for (int i = 4194304; i < 41943040; i += 4194304) {
        detectTime(findSmallestPalindrome, i);
        //std::cout << "    |    ";
        // detectTime(findLargestPalindromeParallel, i);
        std::cout << std::endl;
    }
}


int main() {
    
    test();

    return 0;
}
