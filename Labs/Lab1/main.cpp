#include <iostream>
#include <algorithm>
#include <chrono>
#include <omp.h>
#include <functional>

bool isPalindrome(int num) {
    std::string str = std::to_string(num);
    return std::equal(str.begin(), str.begin() + str.size() / 2, str.rbegin());
}

int findLargestPalindrome(int N) {
    int largestPalindrome = -1;
    for (int i = 1; i <= N; ++i) {
        int sum = 0;
        for (int j = i; sum < N; ++j) {
            sum += j * j;
            if (isPalindrome(sum) && sum < N) {
                largestPalindrome = std::max(largestPalindrome, sum);
            }
        }
    }
    return largestPalindrome;
}

int findLargestPalindromeParallel(int N) {
    int largestPalindrome = -1;

    #pragma omp parallel for reduction(max: largestPalindrome)
    for (int i = 1; i <= N; ++i) {
        int sum = 0;
        for (int j = i; sum < N; ++j) {
            sum += j * j;
            if (isPalindrome(sum) && sum < N) {

                #pragma omp critical
                largestPalindrome = std::max(largestPalindrome, sum);

            }
        }
    }

    return largestPalindrome;
}

void detectTime(std::function<int(int)> target, int N) {
    auto start = std::chrono::high_resolution_clock::now();
    int result = target(N);
    std::cout << "Palindrome: " << result;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = end - start;
    std::cout << " Time: " << duration.count();
}

void test() {
    for (int i = 1000; i < 10000; i += 1000) {
        detectTime(findLargestPalindrome, i);
        std::cout << " | ";
        detectTime(findLargestPalindromeParallel, i);
        std::cout << std::endl;
    }
}

int main() {

    test();

    return 0;
}