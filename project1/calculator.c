#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <ctype.h>

// 定义通用数字结构体
typedef struct
{
    char *digits;       // 存储每位数字（0-9），MSB在前
    long long digit;    // 存储总位数
    long long exponent; // 存储指数 10^exponent, 用于处理小数点位置, 0表示整数
    bool sign;          // 存储符号位 0: 正数 1: 负数
} General_Num;

void help();
int general_logic(int argc, char **argv);

int parse_integer(const char *num, General_Num *integer);
int print_num(const General_Num *num);
int print_sci(const General_Num *num);
void examine_number(General_Num *num);
int parse_precision(const char *token);

int is_valid_int(const char *token);
int is_valid_decimal(const char *token);
int is_valid_scientific(const char *token);

int is_valid_sqrt(const char *token, General_Num *result);
int sqrt_of_number(General_Num num, General_Num *result);

int is_valid_exp(const char *token, General_Num *result);
int exp_of_number(General_Num num, General_Num *result);

int convert_int(const char *token, General_Num *num);
int convert_decimal(const char *token, General_Num *num);
int convert_scientific(const char *token, General_Num *num);

void destroy_number(General_Num *num);

int sum_of_two_integer(General_Num num1, General_Num num2, General_Num *result);
int sub_of_two_integer(General_Num num1, General_Num num2, General_Num *result);

int sum_of_two_number(General_Num num1, General_Num num2, General_Num *result);
int sub_of_two_number(General_Num num1, General_Num num2, General_Num *result);

int multiply_of_two_number(General_Num num1, General_Num num2, General_Num *result);
int divide_of_two_number(General_Num num1, General_Num num2, General_Num *result);

int compare_abs(General_Num a, General_Num b);
int compare_abs_exp(General_Num a, General_Num b);
int precision_install(General_Num *a);
int precision = 10; //default

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("Enter expressions (e.g., 2 + 3). Type 'quit' to exit.\n");
        char line[1024];//处理超长输入
        while (1)
        {
            printf("> ");
            if (!fgets(line, sizeof(line), stdin))
                break;
            line[strcspn(line, "\n")] = '\0';
            if (strcmp(line, "quit") == 0)
                break;
            if (strcmp(line, "exit") == 0)
                break; // also exit
            if (strcmp(line, "help") == 0)
            {
                help();
                continue;
            }
            char *tokens[5] = {NULL};
            char *token = strtok(line, " ");
            int count = 0;
            while (token && count < 5)
            {
                tokens[count++] = token;
                token = strtok(NULL, " ");
            }
            general_logic(count, tokens);
        }
    }
    else
    { // 从命令行参数读取
        general_logic(argc - 1, argv + 1);
    }
    return 0;
}

void help()
{
    printf("\n");
    printf("Expression: (Number) (Operator) (Number) or (function)\n");
    printf("Number can be a function like sqrt(2), exp(1)\n");
    printf("Supported operators: +, -, x, /\n");
    printf("Supported functions: sqrt(x), exp(x)\n");
    printf("to set precision for infinite number, use precision(integer), default 10.\n");
    printf("\n");
}

int general_logic(int argc, char *tokens[])
{
    General_Num num1 = {NULL, 0, 0, 0};
    General_Num num2 = {NULL, 0, 0, 0};
    General_Num result = {NULL, 0, 0, 0};
    bool is_scientific1 = false;
    bool is_scientific2 = false;
    bool is_not_convert = false;
    if (argc == 1){
        // First try precision setting
        if (parse_precision(tokens[0])) {
            return 1;
        }
        General_Num result = {NULL, 0, 0, 0};
        int function_success = 0;
        // Try each function in sequence
        function_success = is_valid_sqrt(tokens[0], &result) || 
                           is_valid_exp(tokens[0], &result);
        if (function_success) {
        printf("%s = ", tokens[0]);
        examine_number(&result);
        print_num(&result);
        return 1;
        }
        printf(" --Invalid input, try like 'sqrt(2)'\n");
        return 0;
    } else if (argc == 2){
        printf(" --Invalid input, try like '2 + 3'\n");
    } else if (argc == 3){
        // 处理第一个token
    if (tokens[0] != NULL)
    {
        if (is_valid_int(tokens[0]))
        {
            if (!convert_int(tokens[0], &num1))
            {
                printf(" --Invalid token or memory error!\n");
            }
        }
        else if (is_valid_decimal(tokens[0]))
        {
            if (!convert_decimal(tokens[0], &num1))
            {
                printf(" --Invalid token or memory error!\n");
            }
        }
        else if (is_valid_scientific(tokens[0]))
        {
            if (!convert_scientific(tokens[0], &num1))
            {
                printf(" --Invalid token or memory error!\n");
            } else is_scientific1 = true;
        } else if (is_valid_sqrt(tokens[0],&num1)){
        } else if (is_valid_exp(tokens[0], &num1)){
        } else is_not_convert = true;
    }
    // 处理第二个token
    char operator = '\0';
    if (tokens[1] != NULL && strlen(tokens[1]) == 1)
    {
        operator= tokens[1][0];
    }
    else {
        printf(" --Invalid operator, try like '2 + 3'\n");
        return 0;
    }
    // 处理第三个token
    if (tokens[2] != NULL)
    {
        if (is_valid_int(tokens[2]))
        {
            // integer;
            if (!convert_int(tokens[2], &num2))
            {
                printf(" --Invalid token or memory error!\n");
            }
        }
        else if (is_valid_decimal(tokens[2]))
        {
            // decimal number

            if (!convert_decimal(tokens[2], &num2))
            {
                printf(" --Invalid token or memory error!\n");
            }
        }
        else if (is_valid_scientific(tokens[2]))
        {
            // scientific number
            if (!convert_scientific(tokens[2], &num2))
            {
                printf(" --Invalid token or memory error!\n");
            } else is_scientific2 = true;
        }
        else if (is_valid_sqrt(tokens[2],&num2)){
        } else if (is_valid_exp(tokens[2], &num2)){
        } else is_not_convert = true;
    }
    // 开始计算
    if (is_not_convert){
        printf(" --Invalid number, try like '2 + 3'\n");
        return 0;
    }

    if (operator== '+')
    {
        if (!sum_of_two_number(num1, num2, &result))
        {
            printf(" --Memory error!\n");
        }
        else
        {
            printf("%s %s %s = ",tokens[0],tokens[1],tokens[2]);
            if (is_scientific1 && is_scientific2) {
                print_sci(&result);
            } else print_num(&result);
        }
    }else if (operator== '-')
    {
        if (!sub_of_two_number(num1, num2, &result))
        {
            printf(" --Memory error!\n");
        }
        else
        {
            printf("%s %s %s = ",tokens[0],tokens[1],tokens[2]);
            if (is_scientific1 && is_scientific2) {
                print_sci(&result);
            } else print_num(&result);
        }
    }else if (operator== 'x' | operator== '*') // *也可以吧
    { 
        if (!multiply_of_two_number(num1, num2, &result))
        {
            printf(" --Memory error!\n");
        }
        else
        {
            printf("%s %s %s = ",tokens[0],tokens[1],tokens[2]);
            if (is_scientific1 && is_scientific2) {
                print_sci(&result);
            } else print_num(&result);
        }
    }else if (operator== '/')
    {
        int feedback = divide_of_two_number(num1, num2, &result);
        if (feedback == 0){
            printf(" --Memory error!\n");
        } else if (feedback == -1){
            printf(" --A number cannot be divided by zero.\n");
        }else
        {
            printf("%s %s %s = ",tokens[0],tokens[1],tokens[2]);
            if (is_scientific1 && is_scientific2) {
                print_sci(&result);
            } else print_num(&result);
        }
    }else printf(" --Invalid operator, try like '2 + 3'\n");

    } else if (argc >= 4) {
        printf(" --Invalid input, too many tokens!\n");
        return 0;
    } 
    destroy_number(&num1); // 手动释放内存
    destroy_number(&num2);
    destroy_number(&result);
    return 0;
}

int print_num(const General_Num *num)
{
    if (num == NULL)
        return 0;
    if (num->digits == NULL)
        return 0;
    if (num->digit == 0)
        return 0;
    if (num->sign == 1)
        printf("-");
    if (num->digit + num->exponent <= 0)
    {
        // 输出小数
        printf("0.");
        for (int i = num->digit + num->exponent; i < 0; i++)
        {
            printf("0");
        }
    }
    for (int i = 0; i < num->digit; i++)
    {
        printf("%d", num->digits[i]);
        if (i == (num->digit + num->exponent - 1) & i != num->digit - 1)
            printf(".");
    }
    if (num->exponent > 0)
    {
        for (int i = 0; i < num->exponent; i++)
        {
            printf("0");
        }
    }
    // printf("\n");
    // printf("digit: %lld, ", num->digit);
    // printf("exponent: %lld, ", num->exponent);
    // printf("sign: %d, ", num->sign);
    // for (int i = 0; i < num->digit; i++)
    // {
    //   printf("[%d]", num->digits[i]);
    // }
    printf("\n");
    return 1;
}

int print_sci(const General_Num *num){
    if (num == NULL)
        return 0;
    if (num->digits == NULL)
        return 0;
    if (num->digit == 0)
        return 0;
    if (num->sign == 1)
        printf("-");
    if (num->digit == 1){
        printf("%de%d\n", num->digits[0],num->exponent);
    } else {
        printf("%d.", num->digits[0]);
        for (int i = 1; i < num->digit; i++){
            printf("%d", num->digits[i]);
            if (i == precision - 1){
                break;
            }
        }
        printf("e%d\n", num->exponent + num->digit - 1);
    }
    return 1;
}

void examine_number(General_Num *num)
{
    if (num == NULL)
        printf("num is NULL\n");
    if (num->digits == NULL)
        printf("num->digits is NULL\n");
    if (num->digit == 0)
        printf("num->digit is 0\n");
}

int parse_precision(const char *token) {
    if (token == NULL)
        return 0;
    
    const char *prefix = "precision(";
    size_t prefix_len = strlen(prefix);
    size_t token_len = strlen(token);
    
    // token 长度须大于 prefix 和右括号
    if (token_len <= prefix_len + 1)
        return 0;
    
    // 检查前缀是否匹配
    if (strncmp(token, prefix, prefix_len) != 0)
        return 0;
    
    // 检查最后一个字符是否为 ')'
    if (token[token_len - 1] != ')')
        return 0;
    
    // 取出括号内的数字部分
    size_t number_len = token_len - prefix_len - 1;
    char *number_str = malloc(number_len + 1);
    if (number_str == NULL)
        return 0;
    
    strncpy_s(number_str, number_len + 1, token + prefix_len, number_len);
    number_str[number_len] = '\0';
    
    // 检查每个字符是否都是数字
    for (size_t i = 0; i < number_len; i++) {
        if (!isdigit((unsigned char)number_str[i])) {
            printf("ERROR: Precision must be a positive integer!\n");
            free(number_str);
            return 1;
        }
    }
    if (number_len == 0)
    {
        // 0位precision不可被接受！
        printf("ERROR: Precision must be a positive integer!\n");
        return 1;
    } else if (number_len > 4){
        //10000以上位数的precision不可被接受！因为无法分配足够内存。
        printf("ERROR: Precision cannot be larger than 9999!\n");
        return 1;
    }
    
    int num = atoi(number_str);
    if (num == 0 || num == 1) {
        printf("ERROR: Precision should in range [2,9999]\n");
        free(number_str);
        return 1;
    }
    printf("* ");
    if (num >= 1000){
        printf("Warning: too large precision may cause memory error.\n");
    } else if (num >= 51){
        printf("Warning: large precision may cause slow calculation.\n");
    }
    printf("Precision set to %d.\n", num);
    precision = num;
    free(number_str);
    return 1;
}


int is_valid_sqrt(const char *token, General_Num *result) {
    if (token == NULL)
        return 0;
    
    const char *prefix = "sqrt(";
    size_t prefix_len = strlen(prefix);
    size_t token_len = strlen(token);
    
    // token 长度须大于 prefix 和右括号
    if (token_len <= prefix_len + 1)
        return 0;
    
    // 检查前缀是否匹配
    if (strncmp(token, prefix, prefix_len) != 0)
        return 0;
    
    // 检查最后一个字符是否为 ')'
    if (token[token_len - 1] != ')')
        return 0;
    // (这里已经默认匹配成功了)取出括号内的数字部分
    size_t number_len = token_len - prefix_len - 1;
    char *number_str = malloc(number_len + 1);
    if (number_str == NULL){
        return 0;
    }
    
    strncpy_s(number_str, number_len + 1, token + prefix_len, number_len);
    number_str[number_len] = '\0';
    
    // 检查每个字符是否都是数字
    if (is_valid_int(number_str) || is_valid_decimal(number_str) || is_valid_scientific(number_str)){
        General_Num num = {NULL, 0, 0, 0};
        if (is_valid_int(number_str)){
            convert_int(number_str, &num);
        } else if (is_valid_decimal(number_str)){
            convert_decimal(number_str, &num);
        } else if (is_valid_scientific(number_str)){
            convert_scientific(number_str, &num);
        }
        if (num.sign == 1){
            printf(" Cannot sqrt a negative number!\n");
            free(number_str);
            return 0;
        }
        if (!sqrt_of_number(num, result)){
            printf("Memory error!\n");
        }
        destroy_number(&num);
        free(number_str);
        return 1;
    }else {
        free(number_str);
        return 0;
    }
}

int sqrt_of_number(General_Num num, General_Num *result){
    if (num.digits == NULL || num.digit == 0){
        return 0;
    }
    if (num.digit == 1 && num.digits[0] == 0){
        result->digit = 1;
        result->digits = malloc(1 * sizeof(char));
        if (result->digits == NULL)
            return 0;
        result->digits[0] = 0;
        result->exponent = 0;
        result->sign = 0;
        return 1;
    }
    // 牛顿迭代
    General_Num current_solution = {NULL, 2, 0, 0};
    General_Num next_solution = {NULL, 0, 0, 0};
    General_Num halfofnum = {NULL, 0, 0, 0};
    General_Num temp = {NULL, 0, 0, 0};
    General_Num temp2 = {NULL, 0, 0, 0};
    General_Num temp3 = {NULL, 0, 0, 0};
    General_Num number_1p5 = {NULL, 2, -1, 0}; // 就是一个单纯的‘1.5’
    General_Num number_0p5 = {NULL, 1, -1, 0}; // 就是一个单纯的‘0.5’
    General_Num number_1 = {NULL, 1, 0, 0}; // 就是一个单纯的‘1’
    current_solution.digits = malloc(current_solution.digit * sizeof(char));
    number_1p5.digits = malloc(number_1p5.digit * sizeof(char));
    number_0p5.digits = malloc(number_0p5.digit * sizeof(char));
    number_1.digits = malloc(number_1.digit * sizeof(char));
    int exp = -(num.exponent+num.digit);
    //printf("%d",-(num.exponent+num.digit));
    if (exp % 2 == 1){ 
        current_solution.exponent = exp/2 - 2;
        current_solution.digits[0] = 3;
        current_solution.digits[1] = 1;
    }else {
        current_solution.exponent = exp/2 - 2;
        current_solution.digits[0] = 1;
        current_solution.digits[1] = 0;
        if (num.digits[0] >= 3) current_solution.digits[0] +=2;
    }
   // printf("START WITH:");
   // print_num(&current_solution);
    number_0p5.digits[0] = 5;
    number_1p5.digits[0] = 1;
    number_1p5.digits[1] = 5;
    number_1.digits[0] = 1;
    multiply_of_two_number(num, number_0p5, &halfofnum);
    
    int loops = (int)(log2(precision)/log2(2) + 10);
   // printf("loops:%d\n",loops);
    for (int i = 0; i < loops; i++){
        //temp = current_solution * current_solution
        multiply_of_two_number(current_solution, current_solution, &temp);
        //temp2 = temp * halfofnum
        multiply_of_two_number(temp, halfofnum, &temp2);
        //temp3 = 1.5 - temp2
        sub_of_two_number(number_1p5, temp2, &next_solution);
        //next_solution = temp3 * current_solution
        multiply_of_two_number(next_solution, current_solution, &next_solution);
        if (next_solution.digit > (int)(2.5 * precision))
        {
            //printf("CUT\n"); //保证不爆
            int new_digit = (next_solution.digit + 1) / 3; // 向上取整
            if (new_digit > 0) {
                char *new_digits = malloc(new_digit * sizeof(char));
                memcpy_s(new_digits, new_digit * sizeof(char), next_solution.digits, new_digit * sizeof(char));
                free(next_solution.digits);
                next_solution.digits = new_digits;
                next_solution.exponent += next_solution.digit - new_digit;
                next_solution.digit = new_digit;  
            }
           // print_num(&next_solution);
        } 
        //next_solution = current_solution
        destroy_number(&current_solution);
        current_solution = next_solution;
        //printf("current_solution:");
        //print_num(&current_solution);
    }
    //result = 1 / current_solution
    divide_of_two_number(number_1, current_solution, result);
    result->sign = 0;
    destroy_number(&current_solution);
    destroy_number(&halfofnum);
    destroy_number(&temp);
    destroy_number(&temp2);
    destroy_number(&temp3);
    destroy_number(&number_1p5);
    destroy_number(&number_0p5);
    destroy_number(&number_1);
    return 1;
}

int is_valid_exp(const char *token, General_Num *result) {
    if (token == NULL)
        return 0;
    const char *prefix = "exp(";
    size_t prefix_len = strlen(prefix);
    size_t token_len = strlen(token);
    if (token_len <= prefix_len + 1)
        return 0;
    if (strncmp(token, prefix, prefix_len) != 0)
        return 0;
    if (token[token_len - 1] != ')')
        return 0;
    size_t number_len = token_len - prefix_len - 1;
    char *number_str = malloc(number_len + 1);
    if (number_str == NULL){
        return 0;
    }
    strncpy_s(number_str, number_len + 1, token + prefix_len, number_len);
    number_str[number_len] = '\0';
    // 检查每个字符是否都是数字
    if (is_valid_int(number_str) || is_valid_decimal(number_str) || is_valid_scientific(number_str)){
        General_Num num = {NULL, 0, 0, 0};
        if (is_valid_int(number_str)){
            convert_int(number_str, &num);
        } else if (is_valid_decimal(number_str)){
            convert_decimal(number_str, &num);
        } else if (is_valid_scientific(number_str)){
            convert_scientific(number_str, &num);
        }
        if (!exp_of_number(num, result)){
            printf("Memory error!\n");
        }
        destroy_number(&num);
        free(number_str);
        return 1;
    }else {
        free(number_str);
        return 0;
    }
}

int exp_of_number(General_Num num, General_Num *result){
    int least_recur = 7;
    if (num.digits == NULL || num.digit == 0){
        return 0;
    }
    if (num.exponent + num.digit == 2){
        if (num.digits[0]>= 3) {
            least_recur = 95;
        } else least_recur = 50;
    }
    if (num.exponent + num.digit >= 3){
        least_recur = 200;
    }
    //计算误差范围
    int signofnum = num.sign;
    num.sign = 0;
    long long digit_of_final = (long long)(pow(10,(num.exponent + num.digit - 1)) * 0.4342944819);
    General_Num accurancy = {NULL, 1, digit_of_final - precision - 1, 0};
    accurancy.digits = malloc(accurancy.digit * sizeof(char));
    result->digit = 1; // result起始是‘1’
    result->digits = malloc(sizeof(char));
    General_Num x_exp = {NULL, 1, 0, 0};
    General_Num k_JC = {NULL, 1, 0, 0}; 
    General_Num temp = {NULL, 0, 0, 0};
    General_Num temp0 = {NULL, 0, 0, 0};
    General_Num temp1 = {NULL, 0, 0, 0};
    General_Num temp2 = {NULL, 0, 0, 0};
    General_Num temp3 = {NULL, 0, 0, 0};
    x_exp.digits = malloc(sizeof(char));
    k_JC.digits = malloc(sizeof(char));
    accurancy.digits[0] = 1;
    result->digits[0] = 1;
    x_exp.digits[0] = 1;
    k_JC.digits[0] = 1;
    int count = 1;
    //泰勒展开计算exp
    int store_precision = precision;
    precision = 20;
    while(1){
        //temp = x^count
        multiply_of_two_number(x_exp,num,&temp);
        destroy_number(&x_exp);
        x_exp = temp;
        //获取阶乘 K_jc = count!; temp0存储 count，temp1存储结果 然后赋值给kjc
        int temp = count;
        int len = 0;
        while (temp > 0) {
            len++;
            temp /= 10;
        }
        temp = count;
        char str_digits[len + 1];
        for (int i = len - 1; i >= 0; i--) {
            str_digits[i] = '0' + (temp%10);  // Convert digit to ASCII
            temp /= 10;
        }
        str_digits[len] = '\0';  // Null terminate
        convert_int(str_digits, &temp0);
        multiply_of_two_number(k_JC, temp0, &temp1);
        destroy_number(&k_JC);
        k_JC = temp1;
        if (k_JC.digit > 30){
            int temp_digit = (k_JC.digit + 1) / 2;
            char *temp_digits = malloc(temp_digit * sizeof(char));
            memcpy_s(temp_digits, temp_digit * sizeof(char), k_JC.digits, temp_digit * sizeof(char));
            free(k_JC.digits);
            k_JC.digits = temp_digits;
            k_JC.exponent += k_JC.digit - temp_digit;
            k_JC.digit = temp_digit;
        }
        count = count + 1;
        // temp2 = x.exp / k_JC
        divide_of_two_number(x_exp,k_JC,&temp2);
        // temp3 = result + temp2
        sum_of_two_number(*result,temp2,&temp3);
        // result = temp3
        destroy_number(result);
        *result = temp3;
        if (count>= least_recur & compare_abs_exp(accurancy,temp2) == 1){
            break;
        }
    }
   // printf("count:%d\n",count);
    precision = store_precision;
    General_Num number_1 = {NULL, 1, 0, 0};
    number_1.digits = malloc(number_1.digit * sizeof(char));
    number_1.digits[0] = 1;
    destroy_number(&temp0);
    destroy_number(&temp1);
    destroy_number(&temp2);
    destroy_number(&x_exp);
    destroy_number(&accurancy);
    if (signofnum == 1){
        if (!divide_of_two_number(number_1,*result,result)){
            destroy_number(&number_1);
            return 0;
        }
    }else{
        if (!divide_of_two_number(*result,number_1,result)){
            destroy_number(&number_1);
            return 0;
        }
    }
    destroy_number(&number_1);
    return 1;
}

int is_valid_int(const char *token)
{
    if (token == NULL || *token == '\0')
    {
        return 0; // 处理空指针或空字符串
    }
    size_t start = 0;
    // 检查首字符是否为负号
    if (token[0] == '-')
    {
        start = 1;
        // 如果只有负号，或负号后没有其他字符，则不合法
        if (token[start] == '\0')
        {
            return 0;
        }
    }
    if (token[start] == '0' && token[start + 1] != '\0')
    {
        return 0; // 存在前导零且长度>1，如"012"或"-012"，不合法
    }
    // 检查剩余字符是否均为数字
    for (size_t i = start; token[i] != '\0'; ++i)
    {
        if (!isdigit((unsigned char)token[i]))
        {
            return 0; // 发现非数字字符
        }
    }
    return 1; // 字符串符合整数格式
}

int is_valid_decimal(const char *token)
{
    if (token == NULL || *token == '\0')
    {
        return 0; // 处理空指针或空字符串
    }

    size_t start = 0;
    // 检查首字符是否为负号
    if (token[0] == '-')
    {
        start = 1;
        // 负号后没有其他字符则不合法
        if (token[start] == '\0')
        {
            return 0;
        }
    }

    size_t dot_count = 0;
    size_t dot_pos = (size_t)-1; // 初始化为无效位置

    // 遍历检查小数点和字符合法性
    for (size_t i = start; token[i] != '\0'; ++i)
    {
        if (token[i] == '.')
        {
            if (dot_count > 0)
            {
                return 0; // 多个小数点
            }
            dot_count++;
            dot_pos = i;
        }
        else if (!isdigit((unsigned char)token[i]))
        {
            return 0; // 非数字字符
        }
    }

    // 必须有且仅有一个小数点
    if (dot_count != 1)
    {
        return 0;
    }

    // 检查整数部分和小数部分长度
    size_t integer_len = dot_pos - start;
    size_t decimal_len = strlen(token) - (dot_pos + 1);

    if (integer_len == 0 || decimal_len == 0)
    {
        return 0; // 整数或小数部分为空
    }

    // 检查整数部分前导零
    if (integer_len > 1 && token[start] == '0')
    {
        return 0; // 前导零
    }

    return 1; // 格式合法
}

int is_valid_scientific(const char *token)
{
    if (token == NULL || *token == '\0')
    {
        return 0; // 处理空指针或空字符串
    }

    // 查找 'e' 或 'E' 的位置
    char *e_pos = strchr(token, 'e');
    if (e_pos == NULL)
    {
        e_pos = strchr(token, 'E');
    }
    if (e_pos == NULL)
    {
        return 0; // 没有找到 'e' 或 'E'
    }

    // 分割字符串为两部分
    size_t base_len = e_pos - token;
    char base[base_len + 1];
    strncpy_s(base, sizeof(base), token, base_len);
    base[base_len] = '\0';

    char *exponent = e_pos + 1;

    // 判断基数部分是否是有效的小数或整数
    if (!is_valid_int(base) && !is_valid_decimal(base))
    {
        return 0;
    }

    // 判断指数部分是否是有效的整数
    if (!is_valid_int(exponent))
    {
        return 0;
    }
    return 1; // 符合科学计数法格式
}

int convert_int(const char *token, General_Num *num)
{
    if (token == NULL || num == NULL)
        return 0;
    bool isNegative = false;
    int len = strlen(token);
    if (len == 0)
        return 0; // 空字符串无效
    if (token[0] == '-')
    {
        len--; // 负号不计入总位数
        isNegative = true;
    }
    // 分配内存
    char *digits = malloc(len * sizeof(char));
    if (digits == NULL)
        return 0; // 内存不足

    // 逐字符验证并转换
    for (int i = 0 + isNegative; i < len + isNegative; i++)
    {
        if (!isdigit(token[i]))
        {
            free(digits); // 发现非数字字符，释放内存
            return 0;
        }
        digits[i - isNegative] = token[i] - '0'; // 字符转数字
    }

    // 赋值到结构体
    num->digits = digits;
    num->digit = len;
    num->exponent = 0;
    num->sign = isNegative;
    return 1;
}

int convert_decimal(const char *token, General_Num *num)
{
    // 已经保证输入是合法的小数
    if (token == NULL || num == NULL)
        return 0;
    bool isNegative = false;
    int len = strlen(token);
    len--; // 小数点不计入总位数
    if (token[0] == '-')
    {
        len--; // 负号不计入总位数
        isNegative = true;
    }

    if (len == 0)
        return 0; // 空字符串无效
    // 分配内存
    char *digits = malloc(len * sizeof(char));
    if (digits == NULL)
        return 0; // 内存不足

    // 逐字符验证并转换
    int dot_pos = -1;
    bool dot = false;
    for (int i = 0 + isNegative; i < len + isNegative + 1; i++)
    {
        if (token[i] == '.')
        {
            dot_pos = i - isNegative;
            dot = true;
            continue;
        }
        if (!isdigit(token[i]))
        {
            free(digits); // 发现非数字字符，释放内存
            return 0;
        }
        digits[i - isNegative - dot] = token[i] - '0'; // 字符转数字
    }
    // 赋值到结构体
    num->digits = digits;
    num->digit = len;
    num->exponent = dot_pos - len;
    num->sign = isNegative;
    // 处理前置0
    if (num->digits == NULL || num->digit == 0) {
        return 0; // Ensure digits array is not NULL and digit is not zero
    }
    int pointer = 0;
    int count = 0;
    while (true)
    {
        if (num->digits[pointer] == 0)
        {
            count++;
            pointer++;
        }
        else
            break;
        if (pointer == num->digit)
        {
            // 处理全为0的情况
            num->digits = realloc(num->digits, 1 * sizeof(char));
            if (num->digits == NULL)
                return 0; // 内存分配失败
            num->digits[0] = 0;
            num->digit = 1;
            num->exponent = 0;
            num->sign = 0;
            return 1;
        }
    }
    if (count != 0)
    { // 检测到有，处理前置0
        num->digit -= count;
        memmove_s(num->digits, num->digit * sizeof(char), num->digits + count, num->digit * sizeof(char));
        num->digits = realloc(num->digits, num->digit * sizeof(char));
    }
    //print_num(num);
    return 1;
}

int convert_scientific(const char *token, General_Num *num)
{
    if (token == NULL || num == NULL)
        return 0;
    if (*token == '\0')
        return 0; // 空字符串直接返回

    // 查找 'e' 或 'E' 的位置
    const char *e_pos = strchr(token, 'e');
    if (e_pos == NULL)
        e_pos = strchr(token, 'E');
    if (e_pos == NULL)
        return 0; // 无指数部分

    // 分割基数和指数
    int base_len = e_pos - token;
    if (base_len == 0)
        return 0;            // 基数部分为空
    char base[base_len + 1]; // 增加1以容纳终止符
    strncpy_s(base, sizeof(base), token, base_len);
    base[base_len] = '\0';
    // 转换基数部分

    if (is_valid_int(base))
    {
        if (!convert_int(base, num))
            return 0;
    }
    else if (is_valid_decimal(base))
    {
        if (!convert_decimal(base, num))
            return 0;
    }
    else
        return 0;

    // 处理指数部分
    const char *exponent = e_pos + 1;
    if (*exponent == '\0')
        return 0; // 指数部分为空

    General_Num exp_num = {NULL, 1, 0, 0};
    if (!convert_int(exponent, &exp_num))
    {
        return 0;
    }
    // 计算指数数值（考虑符号）
    long long exp_value = 0;
    for (int i = 0; i < exp_num.digit; i++)
    {
        exp_value = exp_value * 10 + exp_num.digits[i];
    }
    if (exp_num.sign)
        exp_value = -exp_value;
    num->exponent += exp_value;
    destroy_number(&exp_num);
    examine_number(num);
    return 1;
}

void destroy_number(General_Num *num)
{
    if (num != NULL)
    {
        free(num->digits);
        num->digits = NULL; // 避免野指针
        num->digit = 0;
        num->exponent = 0;
        num->sign = 0;
    }
}
int compare_abs(General_Num a, General_Num b)
{
    if (a.digit > b.digit)
        return 1;
    if (a.digit < b.digit)
        return -1;
    for (int i = 0; i < a.digit; i++)
    {
        if (a.digits[i] > b.digits[i])
            return 1;
        if (a.digits[i] < b.digits[i])
            return -1;
    }
    return 0;
}

int compare_abs_exp(General_Num a, General_Num b)
{
    if (a.digit + a.exponent > b.digit + b.exponent){
        return 1;
    }else return -1;
}
// sum of two integer 和 sub of two integer 只处理传入num内的digits，不处理exponent
// 当且仅当返回值为0的时候，函数执行失败
int sum_of_two_integer(General_Num num1, General_Num num2, General_Num *result)
{
    if (result == NULL)
        return 0;
    if (num1.digit == 0 || num2.digit == 0)
        return 0;
    if (num1.digits == NULL || num2.digits == NULL)
        return 0;
    // 如果两个数都是正数或者负数，则可以调用
    bool flagOfSign = num1.sign == 1 && num2.sign == 1; // 两个数都是负数,则flagOfSign = 1。
    if (num1.sign == 0 && num2.sign == 0 || flagOfSign)
    {
        if (num1.digit < num2.digit)
        { // 保证num1的位数不小于num2
            General_Num temp = num1;
            num1 = num2;
            num2 = temp;
        }
        result->digit = num1.digit + 1;
        result->digits = calloc(result->digit, sizeof(char));
        if (result->digits == NULL)
            return 0;
        memset(result->digits, 0, result->digit * sizeof(char));
        if (result->digits == NULL)
        {
            return 0;
        }
        result->exponent = 0;
        result->sign = flagOfSign;
        result->digits[0] = 0;

        for (int i = 0; i < num1.digit; i++)
        {
            result->digits[i + 1] = num1.digits[i];
        }
        for (int i = num2.digit - 1; i >= 0; i--)
        {
            result->digits[num1.digit - num2.digit + i + 1] += num2.digits[i];
            if (result->digits[num1.digit - num2.digit + i + 1] >= 10)
            {
                result->digits[num1.digit - num2.digit + i + 1] -= 10;
                result->digits[num1.digit - num2.digit + i]++;
            }
        }
        for (int i = num1.digit - 1; i > 0; i--)
        {
            if (result->digits[i] >= 10)
            {
                result->digits[i] -= 10;
                result->digits[i - 1]++;
            }
        }
        if (result->digits != NULL && result->digits[0] == 0)
        {
            // 处理前置0
            result->digit--;
            memmove_s(result->digits, result->digit * sizeof(char), result->digits + 1, result->digit * sizeof(char));
            result->digits = realloc(result->digits, result->digit * sizeof(char));
        }
        return 1;
    }
    else
    {
        // 调用减法函数
        if (num1.sign == 1)
        { // -100 + 200 可以理解为 200 - 100
            num1.sign = 0;
            return sub_of_two_integer(num2, num1, result);
        }
        else
        { // 100 + (-200) 可以理解为 100 - 200
            num2.sign = 0;
            return sub_of_two_integer(num1, num2, result);
        }
    }
}

int sub_of_two_integer(General_Num num1, General_Num num2, General_Num *result)
{
    if (result == NULL)
        return 0;
    if (num1.digit == 0 || num2.digit == 0)
        return 0;
    if (num1.digits == NULL || num2.digits == NULL)
        return 0;

    // 符号不同，转换为加法
    if (num1.sign == 1 && num2.sign == 0)
    { // -a - b = -a + -b
        num2.sign = 1;
        return sum_of_two_integer(num1, num2, result);
    }
    else if (num1.sign == 0 && num2.sign == 1)
    { // a - (-b) = a + b
        num2.sign = 0;
        return sum_of_two_integer(num1, num2, result);
    }

    // 符号相同，比较绝对值大小
    int cmp = compare_abs(num1, num2);
    if (cmp == 0)
    { // 相同数相减，结果为0
        result->digit = 1;
        result->digits = malloc(1 * sizeof(char));
        if (result->digits == NULL)
            return 0;
        result->digits[0] = 0;
        result->exponent = 0;
        result->sign = 0;
        return 2; // 标记
    }

    General_Num larger, smaller;
    int result_sign;

    if (cmp > 0)
    { // num1更大
        larger = num1;
        smaller = num2;
        result_sign = (num1.sign) ? 1 : 0; // Both negatives: result is negative if |num1| > |num2|
    }
    else
    { // num2更大
        larger = num2;
        smaller = num1;
        result_sign = (num1.sign) ? 0 : 1; // Both negatives: result is positive if |num2| > |num1|
    }
    result->digit = larger.digit;
    result->digits = malloc(result->digit * sizeof(char));
    if (result->digits == NULL)
        return 0;
    for (int i = 0; i < result->digit; i++)
    {
        result->digits[i] = larger.digits[i];
    }
    for (int i = 0; i < smaller.digit; i++)
    {
        int result_index = result->digit - i - 1;
        int smaller_index = smaller.digit - i - 1;
        if (result_index < 0 || smaller_index < 0)
            continue; // Ensure indices are within bounds

        if (result->digits[result_index] < smaller.digits[smaller_index])
        {
            result->digits[result_index] += 10 - smaller.digits[smaller_index];
            if (result_index - 1 >= 0)
                result->digits[result_index - 1]--;
        }
        else
        {
            result->digits[result_index] -= smaller.digits[smaller_index];
        }
    }
    for (int i = result->digit - 1; i > 0; i--)
    {
        if (result->digits[i] < 0)
        {
            result->digits[i] += 10;
            result->digits[i - 1]--;
        }
    }
    // 移除前导0
    int leading_zeros = 0;
    while (leading_zeros < result->digit && result->digits[leading_zeros] == 0)
    {
        leading_zeros++;
    }

    if (leading_zeros > 0)
    {
        result->digit -= leading_zeros;
        memmove_s(result->digits, result->digit * sizeof(char), result->digits + leading_zeros, result->digit * sizeof(char));
        char *new_digits = realloc(result->digits, result->digit * sizeof(char));
        if (new_digits == NULL)
        {
            free(result->digits);
            return 0; // 内存不足
        }
        result->digits = new_digits;
    }
    // 如果是0，则
    if (result->digit == 0)
    {
        printf("result->digit == 0\n");
        result->digit = 1;
        result->digits = realloc(result->digits, 1 * sizeof(char));
        result->digits[0] = 0;
        result->sign = 0;
    }
    else
    {
        result->sign = result_sign;
    }
    return 1;
}

int sum_of_two_number(General_Num num1, General_Num num2, General_Num *result)
{
    if (result == NULL)
        return 0;
    if (num1.digit == 0 || num2.digit == 0)
        return 0;
    if (num1.digits == NULL || num2.digits == NULL)
        return 0;
    if (num1.sign == 0 && num2.sign == 1)
    { // a + -b = a - b;
        num1.sign = 1;
        return sub_of_two_number(num1, num2, result);
    }
    else if (num1.sign == 1 && num2.sign == 0)
    { // -a + b = b - a;
        num1.sign = 0;
        return sub_of_two_number(num2, num1, result);
    }

    if (num1.exponent < num2.exponent)
    { // 保证num1的指数大于num2
        General_Num temp = num2;
        num2 = num1;
        num1 = temp;
    }
    // 1234 * 10^-2 + 5678 * 10^-3 = (12340 + 5678) * 10^-3;^^^^

    General_Num enlarged_num1 = {NULL, num1.digit + (num1.exponent - num2.exponent), num2.exponent, num1.sign};
    enlarged_num1.digits = malloc(enlarged_num1.digit * sizeof(char));
    if (enlarged_num1.digits == NULL)
        return 0; // 分配内存失败

    for (int i = 0; i < num1.digit; i++)
    {
        enlarged_num1.digits[i] = num1.digits[i];
    }
    for (int i = num1.digit; i < enlarged_num1.digit; i++)
    {
        enlarged_num1.digits[i] = 0;
    }

    sum_of_two_integer(enlarged_num1, num2, result); // digits等效，直接相加

    destroy_number(&enlarged_num1); // 回收内存
    result->exponent = num2.exponent;
    // 处理后置0
    int pointer = result->digit - 1;
    int count = 0;
    while (true)
    {
        if (result->digits[pointer] == 0)
        {
            count++;
            pointer--;
        }
        else
            break;
        if (pointer == -1)
        {
            // 处理全为0的情况
            result->digits = realloc(result->digits, 1 * sizeof(char));
            if (result->digits == NULL)
                return 0; // 内存分配失败
            result->digits[0] = 0;
            result->digit = 1;
            result->exponent = 0;
            result->sign = 0;
            return 1;
        }
    }

    if (count == 0)
        return 1;

    result->digit -= count;
    result->exponent += count;
    char *temp1 = realloc(result->digits, result->digit * sizeof(char));
    if (temp1 == NULL)
        return 0; // 内存分配失败
    result->digits = temp1;
    return 1;
}

int sub_of_two_number(General_Num num1, General_Num num2, General_Num *result)
{

    if (result == NULL)
        return 0;
    if (num1.digit == 0 || num2.digit == 0)
        return 0;
    if (num1.digits == NULL || num2.digits == NULL)
        return 0;
    if (num1.sign == 1 && num2.sign == 0)
    { // -a - b = -a + -b
        num2.sign = 1;
        return sum_of_two_number(num1, num2, result);
    }
    else if (num1.sign == 0 && num2.sign == 1)
    { // a - (-b) = a + b
        num2.sign = 0;
        return sum_of_two_number(num1, num2, result);
    }
    if (num1.exponent == num2.exponent)
    {
        // 1234 * 10^-2 - 5678 * 10^-2 = ( 1234 - 5678 )* 10^-2
        int exponent = num1.exponent;
        if (sub_of_two_integer(num1, num2, result) == 2)
        { // 相等,赋值为0
            result->digit = 1;
            result->digits = realloc(result->digits, 1 * sizeof(char));
            if (result->digits == NULL)
                return 0; // 内存分配失败
            result->digits[0] = 0;
            result->exponent = 0;
            result->sign = 0;
        }
        else
            result->exponent = exponent;
        return 1;
    }
    bool if_switched = false; // 标记是否交换了num1和num2，如果交换了，最后就要变号
    if (num1.exponent < num2.exponent)
    { // 保证num1的指数大于num2
        General_Num temp = num2;
        num2 = num1;
        num1 = temp;
        if_switched = true; // a - b = -(b - a)
    }
    // 把num1的数组放大 123 * 10^-2 - 5678 * 10^-3 = (1230 - 5678) * 10^-3;
    General_Num enlarged_num1 = {NULL, num1.digit + num1.exponent - num2.exponent, num2.exponent, num1.sign};
    enlarged_num1.digits = malloc(enlarged_num1.digit * sizeof(char));
    if (enlarged_num1.digits == NULL)
        return 0; // 分配内存失败

    for (int i = 0; i < num1.digit; i++)
    {
        enlarged_num1.digits[i] = num1.digits[i];
    }
    for (int i = num1.digit; i < enlarged_num1.digit; i++)
    {
        enlarged_num1.digits[i] = 0;
    }
    sub_of_two_integer(enlarged_num1, num2, result); // digits等效，直接相减
    destroy_number(&enlarged_num1);                  // 回收内存
    result->exponent = num2.exponent;
    if (if_switched)
    {
        if (result->sign == 1)
        {
            result->sign = 0;
        }
        else
            result->sign = 1;
    }
    // 处理前置0
    int pointer = 0;
    int count = 0;
    while (true)
    {
        if (result->digits[pointer] == 0)
        {
            count++;
            pointer++;
        }
        else
            break;
        if (pointer == 0)
        {
            // 处理全为0的情况
            result->digits = realloc(result->digits, 1 * sizeof(char));
            if (result->digits == NULL)
                return 0; // 内存分配失败
            result->digits[0] = 0;
            result->digit = 1;
            result->exponent = 0;
            result->sign = 0;
            return 1;
        }
    }
    if (count != 0)
    { // 检测到有，处理前置0
        result->digit -= count;
        memmove_s(result->digits, result->digit * sizeof(char), result->digits + count, result->digit * sizeof(char));
        result->digits = realloc(result->digits, result->digit * sizeof(char));
    }
    return 1;
}

// 使用O(n^2)相乘 
int multiply_of_two_number(General_Num num1, General_Num num2, General_Num *result)
{
    if (result == NULL)
        return 0;
    if (num1.digit == 0 || num2.digit == 0)
        return 0;
    if (num1.digits == NULL || num2.digits == NULL)
        return 0;

    result->sign = num1.sign != num2.sign;
    result->exponent = num1.exponent + num2.exponent;
    result->digit = num1.digit + num2.digit;
    result->digits = malloc(result->digit * sizeof(char));

    int *cal = malloc(result->digit * sizeof(int)); // char数组会溢出，建立临时数组参与运算


    for (int i = 0; i < result->digit; i++)
    {
        cal[i] = 0;
    }

    for (int i = 0; i < num1.digit; i++)
    {
        for (int j = 0; j < num2.digit; j++)
        {
            cal[i + j] += num1.digits[num1.digit - i - 1] * num2.digits[num2.digit - j - 1];
            // printf("cal[%d]+=%d\n",i+j,num1.digits[num1.digit - i - 1] * num2.digits[num2.digit - j - 1]);
        }
    }

    // 处理进位
    for (int i = 0; i < result->digit - 1; i++)
    {
        if (cal[i] >= 10)
        {
            cal[i + 1] += cal[i] / 10;
            cal[i] %= 10;
        }
    }
    for (int i = 0; i < result->digit; i++)
    {
        result->digits[i] = cal[result->digit - i - 1];
    }
    // 处理前置0
    if (result->digits[0] == 0)
    {
        memmove_s(result->digits, result->digit * sizeof(char), result->digits + 1, (result->digit - 1) * sizeof(char));
        result->digit--;
        result->digits = realloc(result->digits, result->digit * sizeof(char));
    }
    free(cal);
    
    return 1;
}

// 0内存错误 -1除数为0
int divide_of_two_number(General_Num num1, General_Num num2, General_Num *result)
{
    if (result == NULL)
        return 0;
    if (num1.digit == 0 || num2.digit == 0)
        return 0;
    if (num1.digits == NULL || num2.digits == NULL)
        return 0;

    int sign = num1.sign != num2.sign;
    num1.sign = 0;
    num2.sign = 0;
    result->exponent = num1.exponent - num2.exponent;
    for (int i = 0; i < num2.digit; i++)
    {
        if (num2.digits[i] != 0)
            break;
        if (i == num2.digit - 1)
            return -1; // 除数为0
    }
    // 求num2的逆元，牛顿迭代法初始化
    General_Num inverse_num2 = {NULL, 1, 0, 0};
    General_Num next_solution = {NULL, 0, 0, 0};
    General_Num temp = {NULL, 0, 0, 0};
    General_Num temp2 = {NULL, 0, 0, 0};
    General_Num number_2 = {NULL, 1, 0, 0}; // 就是一个单纯的'2'
    inverse_num2.digits = malloc(inverse_num2.digit * sizeof(char));
    number_2.digits = malloc(number_2.digit * sizeof(char));
    if (inverse_num2.digits == NULL || number_2.digits == NULL) {
        // Free any successfully allocated memory
        free(inverse_num2.digits);
        free(number_2.digits);
        return 0;
    }
    
    inverse_num2.digits[0] = 1;
    inverse_num2.exponent = -num2.exponent - num2.digit;
    number_2.digits[0] = 2;
    int loops = (int)ceil(log(precision) / log(2) + 4);
    //printf("start loops:%d\n", loops);
    //print_num(&inverse_num2);

    for (int i = 0; i < loops; i++)
    {
        // temp = num2*inverse_num2
        if (!multiply_of_two_number(inverse_num2, num2, &temp)) {
            destroy_number(&inverse_num2);
            destroy_number(&number_2);
            return 0;
        }
        // temp2 = 2 - temp
        if (!sub_of_two_number(number_2, temp, &temp2)) {
            destroy_number(&inverse_num2);
            destroy_number(&number_2);
            destroy_number(&temp);
            return 0;
        }
        // result = temp2 * inverse_num2
        if (!multiply_of_two_number(temp2, inverse_num2, &next_solution)) {
            destroy_number(&inverse_num2);
            destroy_number(&number_2);
            destroy_number(&temp);
            destroy_number(&temp2);
            return 0;
        }
        // 将result赋值给inverse_num2
        if (i > loops - 4)
        {
            int new_digit = (next_solution.digit + 1) / 2; // 向上取整
            if (new_digit > 0) {
                char *new_digits = malloc(new_digit * sizeof(char));
                if (new_digits == NULL) {
                    destroy_number(&inverse_num2);
                    destroy_number(&number_2);
                    destroy_number(&temp);
                    destroy_number(&temp2);
                    destroy_number(&next_solution);
                    return 0;
                }
                memcpy_s(new_digits, new_digit * sizeof(char), next_solution.digits, new_digit * sizeof(char));
                free(next_solution.digits);
                next_solution.digits = new_digits;
                next_solution.exponent += next_solution.digit - new_digit;
                next_solution.digit = new_digit;  
            }
        }
        
        destroy_number(&temp);
        destroy_number(&temp2);
        destroy_number(&inverse_num2);
        inverse_num2 = next_solution;
        // next_solution.digits is now owned by inverse_num2, so don't free it
        next_solution.digits = NULL;
    }
    
    // 求完逆元，进行相乘
    if (!multiply_of_two_number(num1, inverse_num2, result)) {
        destroy_number(&inverse_num2);
        destroy_number(&number_2);
        return 0;
    }
    
    int count = result->digit;
    precision_install(result);
    result->exponent += count - result->digit;
    result->sign = sign;
    
    // Clean up all remaining memory
    destroy_number(&inverse_num2);
    destroy_number(&number_2);
    
    return 1;
}

int precision_install(General_Num *a)
{
    if (a->digit <= precision)
        return 1; // 当前精度已经小于指定精度
    if (a->digits[precision] < 5)
    { // 指定进位数小于5，直接截取返回
        char *temp = malloc(precision * sizeof(char));
        memcpy_s(temp, precision * sizeof(char), a->digits, precision * sizeof(char));
        free(a->digits);
        a->digits = temp;
        a->digit = precision;
        return 1;
    }

    int copy_length = precision + 1;
    char *temp_for_precision = malloc((precision + 1) * sizeof(char));
    if (temp_for_precision == NULL)
        return 0;
    memcpy_s(temp_for_precision, (precision + 1) * sizeof(char), a->digits, copy_length * sizeof(char));
    free(a->digits);
    a->digits = temp_for_precision;
    a->digit = copy_length;

    if (a->digits[precision] >= 5)
    {
        a->digits[precision - 1] += 1;
        a->digits[precision] = 0;
    }
    else
        return 1; // 不需要四舍五入

    for (int i = precision - 1; i >= 0; i--)
    {
        a->digits[i + 1] = a->digits[i];
    }
    a->digits[0] = 0;
    for (int i = precision; i > 0; i--)
    {
        if (a->digits[i] == 10)
        {
            a->digits[i] = 0;
            a->digits[i - 1]++;
        }
    }
    a->digit--; // 删除接位
    if (a->digits[0] == 0)
    {
        // 无进位，所以删除第一个
        memmove_s(a->digits, a->digit * sizeof(char), a->digits + 1, a->digit * sizeof(char));
        char *new_ptr = realloc(a->digits, a->digit * sizeof(char));
        if (new_ptr != NULL)
        {
            a->digits = new_ptr;
        }
    }
    else
    {
        a->exponent++; // 进位保存，需要乘10，删除最后一个
        char *new_ptr = realloc(a->digits, a->digit * sizeof(char));
        if (new_ptr != NULL)
        {
            a->digits = new_ptr;
        }
    }
    return 1;
}