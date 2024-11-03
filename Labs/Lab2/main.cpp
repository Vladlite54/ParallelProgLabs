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
    int smallestPalindrome = -1; // �������������� ���������� ��� �������� ����������
    for (int i = N + 1; ; ++i) { // �������� � N + 1
        if (isPalindrome(i)) { // ���������, �������� �� ����� �����������
            // ������ ���������, �������� �� ��� ������������� ���� ��������� �����
            for (int j = 1; j * j <= i; ++j) {
                if (i % j == 0) {
                    int otherFactor = i / j;
                    if (j != otherFactor) { // ��������, ��� ����� ������
                        smallestPalindrome = i; // ��������� ��������� ���������
                        return smallestPalindrome; // ���������� ���������
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
        // �������� �� ���������
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
                return 1; // ��� ������������ ���� ������ �����
            }
        }
    }
    return 0; // �� ��������
}

int findSmallestPalindromeParallel(int N) {
    // ������������� OpenCL
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);
    
    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);

    int range = 1000; // ��������� ��������, ������� ����� ��������� �����������
    int* results = (int*)malloc(sizeof(int) * range);
    int K = N + range;
    
    cl_mem resultsBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * range, NULL, NULL);
    
    // �������� � ���������� Kernel
    cl_program program = clCreateProgramWithSource(context, 1, &kernelSrc, NULL, NULL);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "find_palindromes", NULL);
    
    // ������ Kernel
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &resultsBuffer);
    clSetKernelArg(kernel, 1, sizeof(int), &N);
    clSetKernelArg(kernel, 2, sizeof(int), &K);

    size_t globalSize = range;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    
    // ���������� �����������
    clEnqueueReadBuffer(queue, resultsBuffer, CL_TRUE, 0, sizeof(int) * range, results, 0, NULL, NULL);
    
    // ����� ������ ���������� ����������
    int smallestPalindrome = -1;
    for (int i = 0; i < range; i++) {
        if (results[i] != -1 && results[i] > N) {
            if (isProductOfDistinct(results[i])) {
                smallestPalindrome = results[i];
                break;
            }
        }
    }

    // ������������ ��������
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
