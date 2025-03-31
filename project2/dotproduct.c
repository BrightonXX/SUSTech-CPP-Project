#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32 // Calculate Time in Windows
#include <Windows.h>
typedef LARGE_INTEGER TimePoint;

void get_time(TimePoint *t)
{
    if (!QueryPerformanceCounter(t))
    {
        // 处理错误：无法获取计数器
        exit(1);
    }
}

double time_diff_seconds(TimePoint start, TimePoint end)
{
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq) || freq.QuadPart == 0)
    {
        // 处理错误：无效频率
        exit(1);
    }
    return (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;
}
#else
#include <time.h>
#include <stdio.h>
typedef struct timespec TimePoint;

void get_time(TimePoint *t)
{
    if (clock_gettime(CLOCK_MONOTONIC, t) == -1)
    {
        perror("clock_gettime");
        exit(1);
    }
}

double time_diff_seconds(TimePoint start, TimePoint end)
{
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1000000000.0;
}
#endif

int check_and_parse_size(const char *line, int *size);

int xor_test(int size);
int int_dot_product(int size);
int float_dot_product(int size);
int double_dot_product(int size);
int signed_char_dot_product(int size);
int short_dot_product(int size);

unsigned int xorshift_state = 1;
unsigned long long xorshift64_state = 1;

unsigned int xorshift32()
{
    xorshift_state ^= xorshift_state << 13;
    xorshift_state ^= xorshift_state >> 17;
    xorshift_state ^= xorshift_state << 5;
    return xorshift_state;
}
unsigned long long xorshift64()
{
    xorshift64_state ^= xorshift64_state << 13;
    xorshift64_state ^= xorshift64_state >> 7;
    xorshift64_state ^= xorshift64_state << 17;
    return xorshift64_state;
}
int main()
{
    char line[100];
    char line2[2];
    int size;
    int type;
    printf("Enter the size of the array: \n> ");
    while (1)
    {
        if (!fgets(line, sizeof(line), stdin))
        {
            printf("An Error occured\n");
            continue;
        }
        if (check_and_parse_size(line, &size))
            break;
        printf("Invalid input. Please enter a positive integer: \n");
    }
    printf("%d\n", size);
    printf("Enter the type of the array: \n");
    printf("0.XOR Test; 1. int; 2. float; 3. double; 4.signed char; 5.short\n");
    printf("> ");
    while (1)
    {
        if (!fgets(line2, sizeof(line2), stdin))
        {
            printf("An Error occured\n");
            continue;
        }
        if (check_and_parse_size(line2, &type))
        {
            if (type >= 0 && type <= 5)
                break;
        }
        printf("Invalid input, please enter a integer in [1,5]\n");
        printf("> ");
    }
    printf("type:%d\n", type);
    for (int i = 0; i < 1; i++) // when doing experiment, we can easily adjust the loop.
    {
        printf("Test %d\n", i);
        switch (type)
        {
        case 0:
            xor_test(size);
            break;
        case 1:
            int_dot_product(size);
            break;
        case 2:
            float_dot_product(size);
            break;
        case 3:
            double_dot_product(size);
            break;
        case 4:
            signed_char_dot_product(size);
            break;
        case 5:
            short_dot_product(size);
            break;
        default:
            break;
        }
    }

    return 0;
}

int check_and_parse_size(const char *line, int *size)
{
    if (line == NULL || size == NULL)
        return 0;

    // Skip whitespace and check if all remaining characters are digits
    int i = 0;

    // Skip leading whitespace
    while (isspace((unsigned char)line[i]))
        i++;

    // Check if there are any digits
    if (line[i] == '\0' || !isdigit((unsigned char)line[i]))
        return 0;

    // Parse the number
    int a = 0;
    while (isdigit((unsigned char)line[i]))
    {
        a = a * 10 + line[i] - '0';
        i++;
    }

    // Check if remaining characters are whitespace
    while (line[i] != '\0')
    {
        if (!isspace((unsigned char)line[i]))
        {
            printf("Invalid input: non-digit character found\n");
            return 0;
        }
        i++;
    }

    // Check if input is a positive number
    if (a < 0)
    {
        printf("Invalid input: size must be positive\n");
        return 0;
    }

    *size = a;
    return 1;
}
int xor_test(int size)
{
    int *a = (int *)malloc(size * sizeof(int));
    int *b = (int *)malloc(size * sizeof(int));
    if (a == NULL || b == NULL)
    {
        if (a != NULL) free(a);
        if (b != NULL) free(b);
        printf("Memory allocation failed, try smaller number or spare more memory\n");
        return 1;
    }
    long long sum = 0;
    TimePoint start, end;
    double time_taken;
    get_time(&start);
    for (int i = 0; i < size; i++)
    {
        a[i] = rand();
        b[i] = rand();
    }
    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Generate Random number using rand(): %.6f seconds\n", time_taken);
    get_time(&start);
    xorshift_state = (unsigned int)time(NULL);
    for (int i = 0; i < size; i++)
    {
        a[i] = xorshift32();
        b[i] = xorshift32();
    }
    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Generate Random number using XORshift: %.6f seconds\n", time_taken);
    get_time(&start);
    for (int i = 0; i < size; i++)
    {
        sum += a[i] * b[i];
    }
    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Dot product: %.6f seconds\n", time_taken);
    printf("Dot product of two arrays: %lld\n", sum);
    free(a);
    free(b);
    return 0;
}

int int_dot_product(int size)
{
    int *a = (int *)malloc(size * sizeof(int));
    int *b = (int *)malloc(size * sizeof(int));
    if (a == NULL || b == NULL)
    {
        if (a != NULL) free(a);
        if (b != NULL) free(b);
        printf("Memory allocation failed, try smaller number or spare more memory\n");
        return 1;
    }
    long long sum = 0;
    TimePoint start, end;
    double time_taken;
    get_time(&start);
    xorshift_state = (unsigned int)time(NULL);
    for (int i = 0; i < size; i++)
    {
        a[i] = xorshift32();
        b[i] = xorshift32();
    }
    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Generate Random number using XORshift: %.6f seconds\n", time_taken);
    get_time(&start);
    for (int i = 0; i < size; i++)
    {
        sum += a[i] * b[i];
    }
    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Dot product: %.6f seconds\n", time_taken);
    printf("Dot product of two arrays: %lld\n", sum);
    free(a);
    free(b);
    return 0;
}

int float_dot_product(int size)
{
    float *a = (float *)malloc(size * sizeof(float));
    float *b = (float *)malloc(size * sizeof(float));
    if (a == NULL || b == NULL)
    {
        if (a != NULL) free(a);
        if (b != NULL) free(b);
        printf("Memory allocation failed, try smaller number or spare more memory\n");
        return 1;
    }
    float sum = 0;
    TimePoint start, end;
    double time_taken;
    get_time(&start);
    xorshift_state = (unsigned int)time(NULL);
    for (int i = 0; i < size; i++)
    {
        a[i] = (float)xorshift32();
        b[i] = (float)xorshift32();
    }
    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Generate using XORshift: %.6f seconds\n", time_taken);
    get_time(&start);
    for (int i = 0; i < size; i++)
    {
        sum += a[i] * b[i];
    }
    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Dot product: %.6f seconds\n", time_taken);
    printf("Dot product of two arrays: %f\n", sum);
    free(a);
    free(b);
    return 0;
}

int double_dot_product(int size)
{
    double *a = (double *)malloc(size * sizeof(double));
    double *b = (double *)malloc(size * sizeof(double));
    if (a == NULL || b == NULL)
    {
        if (a != NULL) free(a);
        if (b != NULL) free(b);
        printf("Memory allocation failed, try smaller number or spare more memory\n");
        return 1;
    }
    double sum = 0;
    TimePoint start, end;
    double time_taken;
    get_time(&start);
    xorshift64_state = (unsigned long long)time(NULL);
    for (int i = 0; i < size; i++)
    {
        a[i] = (double)xorshift64();
        b[i] = (double)xorshift64();
    }
    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Generate using XORshift: %.6f seconds\n", time_taken);
    get_time(&start);
    for (int i = 0; i < size; i++)
    {
        sum += a[i] * b[i];
    }
    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Dot product: %.6f seconds\n", time_taken);
    printf("Dot product of two arrays: %f\n", sum);
    free(a);
    free(b);
    return 0;
}

int signed_char_dot_product(int size)
{
    signed char *a = (signed char *)malloc(size * sizeof(signed char));
    signed char *b = (signed char *)malloc(size * sizeof(signed char));
    if (a == NULL || b == NULL)
    {
        if (a != NULL) free(a);
        if (b != NULL) free(b);
        printf("Memory allocation failed, try smaller number or spare more memory\n");
        return 1;
    }
    long long sum = 0;
    TimePoint start, end;
    double time_taken;
    get_time(&start);
    xorshift_state = (unsigned int)time(NULL);
    for (int i = 0; i < size; i++)
    {
        a[i] = (signed char)xorshift32();
        b[i] = (signed char)xorshift32();
    }
    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Generate using XORshift: %.6f seconds\n", time_taken);
    get_time(&start);
    for (int i = 0; i < size; i++)
    {
        sum += a[i] * b[i];
    }
    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Dot product: %.6f seconds\n", time_taken);
    printf("Dot product of two arrays: %lld\n", sum);
    free(a);
    free(b);
    return 0;
}

int short_dot_product(int size)
{
    short *a = (short *)malloc(size * sizeof(short));
    short *b = (short *)malloc(size * sizeof(short));
    if (a == NULL || b == NULL)
    {
        if (a != NULL) free(a);
        if (b != NULL) free(b);
        printf("Memory allocation failed, try smaller number or spare more memory\n");
        return 1;
    }
    long long sum = 0;
    TimePoint start, end;
    double time_taken;
    get_time(&start);
    xorshift_state = (unsigned int)time(NULL);
    for (int i = 0; i < size; i++)
    {
        a[i] = (short)xorshift32();
        b[i] = (short)xorshift32();
    }
    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Generate using XORshift: %.6f seconds\n", time_taken);
    get_time(&start);
    for (int i = 0; i < size; i++)
    {
        sum += a[i] * b[i];
    }
    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Dot product: %.6f seconds\n", time_taken);
    printf("Dot product of two arrays: %lld\n", sum);
    free(a);
    free(b);
    return 0;
}
