#include <iostream>
#include <vector>
#include <algorithm>
#include <omp.h>
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <stdlib.h>

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

const char *kernelSource = R"(
__kernel void findPalindromes(__global int *numbers, __global int *results, int N) {
    int id = get_global_id(0);
    int num = numbers[id];

    if (num > N) {
        // Проверяем, является ли num палиндромом
        int is_palindrome = 1;
        int temp = num, reversed = 0;
        while (temp > 0) {
            reversed = reversed * 10 + (temp % 10);
            temp /= 10;
        }
        if (num != reversed) {
            is_palindrome = 0;
        }

        if (is_palindrome) {
            // Проверяем, является ли num произведением двух различных чисел
            for (int j = 1; j * j <= num; j++) {
                if (num % j == 0) {
                    int otherFactor = num / j;
                    if (j != otherFactor) {
                        results[id] = num; // Сохраняем найденный палиндром
                        return;
                    }
                }
            }
        }
    }
}
)";

int findSmallestPalindromeParallel(int N) {
    int smallestPalindrome = -1;
    
    // Здесь предположим, что мы проверяем числа от N + 1 до N + 100000.
    const int range = 100000;
    int *numbers = (int*)malloc(range * sizeof(int));
    int *results = (int*)malloc(range * sizeof(int));
    
    for (int i = 0; i < range; i++) {
        numbers[i] = N + 1 + i;
    }

    // Инициализация OpenCL
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);

    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);

    cl_mem numbersBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, range * sizeof(int), numbers, NULL);
    cl_mem resultsBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, range * sizeof(int), NULL, NULL);

    // Создание и компиляция ядра
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, NULL, NULL);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    
    cl_kernel kernel = clCreateKernel(program, "findPalindromes", NULL);

    // Выполнение ядра
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &numbersBuffer);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &resultsBuffer);
    clSetKernelArg(kernel, 2, sizeof(int), &N);
    
    size_t globalWorkSize = range;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalWorkSize, NULL, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, resultsBuffer, CL_TRUE, 0, range * sizeof(int), results, 0, NULL, NULL);

    // Поиск самого маленького палиндрома среди результатов
    for (int i = 0; i < range; i++) {
        if (results[i] != -1) {
            smallestPalindrome = results[i];
            break;
        }
    }

    // Освобождение ресурсов
    clReleaseMemObject(numbersBuffer);
    clReleaseMemObject(resultsBuffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    
    free(numbers);
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
    for (int i = 4194304; i < 41943040; i += 4194304) {
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
