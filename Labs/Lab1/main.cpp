#include <iostream>
#include <algorithm>
#include <chrono>
#include <omp.h>

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

int main() {

    auto start = std::chrono::high_resolution_clock::now();
    int result = findLargestPalindrome(60000000);
    std::cout << "Palindrome: " << result << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = end - start;
    std::cout << duration.count() << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    int result2 = findLargestPalindromeParallel(60000000);
    std::cout << "Palindrome: " << result2 << std::endl;
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration2 = end2 - start2;
    std::cout << duration2.count() << std::endl;


    return 0;
}