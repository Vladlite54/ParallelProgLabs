#include <iostream>
#include <omp.h>

bool isPalindrome(int num) {
    int reversed = 0;
    int original = num;

    while (num > 0) {
        reversed = reversed * 10 + (num % 10);
        num /= 10;
    }

    return original == reversed;
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

    #pragma omp parallel for reduction(max:largestPalindrome)
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

void detectTime(int (*target)(int), int N) {
    double start = omp_get_wtime();
    int result = (*target)(N);
    std::cout << "Palindrome: " << result;
    double end = omp_get_wtime();
    double duration = end - start;
    std::cout << " Time: " << duration;
}

void test() {
    for (int i = 10000000; i < 100000000; i += 10000000) {
        detectTime(findLargestPalindrome, i);
        std::cout << "    |    ";
        detectTime(findLargestPalindromeParallel, i);
        std::cout << std::endl;
    }
}

int main() {

    test();

    return 0;
}