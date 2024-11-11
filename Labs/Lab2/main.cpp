#include <iostream>
#include <omp.h>
#include <CL/cl.h>
#include <CL/cl_platform.h>


bool isPalindrome(int num) {
    std::string str = std::to_string(num);
    std::string reversedStr = std::string(str.rbegin(), str.rend());
    return str == reversedStr;
}

long long findSmallestPalindrome(long long N) {
    double start, end, duration;

    start = omp_get_wtime();
    int smallestPalindrome = -1; 
    for (int i = N + 1; ; ++i) { 
        if (isPalindrome(i)) { 
            
            for (int j = 1; j * j <= i; ++j) {
                if (i % j == 0) {
                    int otherFactor = i / j;
                    if (j != otherFactor) { 
                        smallestPalindrome = i; 
                        end = omp_get_wtime();
                        duration = end - start;
                        std::cout << "Palindrome: " << smallestPalindrome;
                        std::cout << " Time: " << duration;
                        return smallestPalindrome; 
                    }
                }
            }
        }
    }
}

long long findSmallestPalindromeParallel(long long N) {
    cl_platform_id platformId = NULL;
    cl_device_id deviceId = NULL;
    cl_context context = NULL;
    cl_command_queue commandQueue = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_mem numbersBuffer = NULL;
    cl_mem resultsBuffer = NULL;
    size_t globalWorkSize;
    cl_int ret;

    
    ret = clGetPlatformIDs(1, &platformId, NULL);
    ret = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_ALL, 1, &deviceId, NULL);
    context = clCreateContext(NULL, 1, &deviceId, NULL, NULL, &ret);
    commandQueue = clCreateCommandQueue(context, deviceId, 0, &ret);


    
    const char *sourceCode = 
    "__kernel void isPalindrome(__global const unsigned long *numbers, __global int *results, const unsigned long numCount) {"
    "   size_t gid = get_global_id(0);"
    "   if (gid < numCount) {"
    "       unsigned long num = numbers[gid];"
    "       unsigned long reversedNum = 0;"
    "       unsigned long tempNum = num;"
    "       while (tempNum > 0) {"
    "           reversedNum = reversedNum * 10 + tempNum % 10;"
    "           tempNum /= 10;"
    "       }"
    "       results[gid] = (num == reversedNum);"
    "   }"
    "}";

    program = clCreateProgramWithSource(context, 1, (const char **)&sourceCode, NULL, &ret);
    ret = clBuildProgram(program, 1, &deviceId, NULL, NULL, NULL);
    kernel = clCreateKernel(program, "isPalindrome", &ret);

    unsigned long numCandidates = 10000; 
    unsigned long *numbers = (unsigned long *)malloc(numCandidates * sizeof(unsigned long));
    int *results = (int *)malloc(numCandidates * sizeof(int));
    globalWorkSize = numCandidates;

    
    for (unsigned long i = 0; i < numCandidates; ++i) {
        numbers[i] = N + i + 1; 
    }

    numbersBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, numCandidates * sizeof(unsigned long), numbers, &ret);
    resultsBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, numCandidates * sizeof(int), NULL, &ret);

    
    ret = clSetKernelArg(kernel, 0, sizeof(numbersBuffer), (void *)&numbersBuffer);
    ret = clSetKernelArg(kernel, 1, sizeof(resultsBuffer), (void *)&resultsBuffer);
    ret = clSetKernelArg(kernel, 2, sizeof(numCandidates), (void *)&numCandidates);

    double start = omp_get_wtime(); 
    ret = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &globalWorkSize, NULL, 0, NULL, NULL);
    clFinish(commandQueue); 
    double end = omp_get_wtime(); 
    double duration = end - start;
    


    ret = clEnqueueReadBuffer(commandQueue, resultsBuffer, CL_TRUE, 0, numCandidates * sizeof(int), results, 0, NULL, NULL);

    unsigned long smallestPalindrome = 0;
    for (unsigned long i = 0; i < numCandidates; ++i) {
      if(results[i]){
        unsigned long num = numbers[i];
        
        for (unsigned long j = 2; j * j <= num; ++j) {
            if (num % j == 0 && num / j != j && num/j !=num) {
                smallestPalindrome = num;
                break;
            }
        }
        if (smallestPalindrome > 0) break;
      }
    }

    
    clReleaseMemObject(numbersBuffer);
    clReleaseMemObject(resultsBuffer);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(context);

    free(numbers);
    free(results);

    std::cout << "Palindrome: " << smallestPalindrome;
    std::cout << " Time: " << duration;
    return smallestPalindrome;
}


void test() {
    for (long long i = 4194304; i < 41943040; i += 4194304) {
        findSmallestPalindrome(i);
        std::cout << "    |    ";
        findSmallestPalindromeParallel(i);
        std::cout << std::endl;
    }
}


int main() {
    
    test();
    
    return 0;
}
