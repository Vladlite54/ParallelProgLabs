#include <iostream>
#include <vector>
#include <algorithm>
#include <omp.h>
#include <CL/cl.h>
#include <CL/cl_platform.h>

bool isPalindrome(int num) {
    std::string str = std::to_string(num);
    std::string reversedStr = std::string(str.rbegin(), str.rend());
    return str == reversedStr;
}

int findSmallestPalindrome(int N) {
    int smallestPalindrome = -1; // Инициализируем переменную для хранения результата
    for (int i = N + 1; ; ++i) { // Начинаем с N + 1
        if (isPalindrome(i)) { // Проверяем, является ли число палиндромом
            // Теперь проверяем, является ли оно произведением двух различных чисел
            for (int j = 1; j * j <= i; ++j) {
                if (i % j == 0) {
                    int otherFactor = i / j;
                    if (j != otherFactor) { // Убедимся, что числа разные
                        smallestPalindrome = i; // Сохраняем найденный палиндром
                        return smallestPalindrome; // Возвращаем результат
                    }
                }
            }
        }
    }
}

const char* kernelSrc = R"(
__kernel void find_palindromes(__global int* results, int start, int end) {
    int gid = get_global_id(0);
    if (gid >= start && gid < end) {
        int num = gid;
        // Проверка на палиндром
        int reversed = 0, original = num;
        while (num > 0) {
            reversed = reversed * 10 + num % 10;
            num /= 10;
        }
        if (original == reversed) {
            results[gid] = original;
        } else {
            results[gid] = -1;
        }
    }
}
)";

int isProductOfDistinct(int num) {
    for (int j = 1; j * j <= num; ++j) {
        if (num % j == 0) {
            int otherFactor = num / j;
            if (j != otherFactor) {
                return 1; // Это произведение двух разных чисел
            }
        }
    }
    return 0; // Не является
}

int findSmallestPalindromeParallel(int N) {
    // Инициализация OpenCL
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);
    
    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);

    int range = 1000; // Примерный диапазон, который можем проверить параллельно
    int* results = (int*)malloc(sizeof(int) * range);
    int K = N + range;
    
    cl_mem resultsBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * range, NULL, NULL);
    
    // Создание и компиляция Kernel
    cl_program program = clCreateProgramWithSource(context, 1, &kernelSrc, NULL, NULL);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "find_palindromes", NULL);
    
    // Запуск Kernel
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &resultsBuffer);
    clSetKernelArg(kernel, 1, sizeof(int), &N);
    clSetKernelArg(kernel, 2, sizeof(int), &K);

    size_t globalSize = range;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    
    // Считывание результатов
    clEnqueueReadBuffer(queue, resultsBuffer, CL_TRUE, 0, sizeof(int) * range, results, 0, NULL, NULL);
    
    // Поиск самого маленького палиндрома
    int smallestPalindrome = -1;
    for (int i = 0; i < range; i++) {
        if (results[i] != -1 && results[i] > N) {
            if (isProductOfDistinct(results[i])) {
                smallestPalindrome = results[i];
                break;
            }
        }
    }

    // Освобождение ресурсов
    clReleaseMemObject(resultsBuffer);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    free(results);

    return smallestPalindrome;
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
    for (int i = 1000; i < 10000; i += 1000) {
        detectTime(findSmallestPalindrome, i);
        std::cout << "    |    ";
        detectTime(findSmallestPalindromeParallel, i);
        std::cout << std::endl;
    }
}


int main() {
    
    test();
    
    return 0;
}
