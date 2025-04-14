#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <omp.h>
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

#pragma pack(push, 1) // Ensure struct packing for correct file I/O

// BMP File Header (14 bytes)
typedef struct
{
    unsigned short bfType;      // identifier "BM" (0x4D42)
    unsigned int bfSize;        // File size in bytes
    unsigned short bfReserved1; // Reserved
    unsigned short bfReserved2; // Reserved
    unsigned int bfOffBits;     // Offset to image data
} BMPFileHeader;

// BMP Information Header (40 bytes)
typedef struct
{
    unsigned int biSize;         // Header size
    int biWidth;                 // Width of the image
    int biHeight;                // Height of the image
    unsigned short biPlanes;     // Color planes
    unsigned short biBitCount;   // Bits per pixel
    unsigned int biCompression;  // Compression type
    unsigned int biSizeImage;    // Image size in bytes
    int biXPelsPerMeter;         // X resolution
    int biYPelsPerMeter;         // Y resolution
    unsigned int biClrUsed;      // Colors used
    unsigned int biClrImportant; // Important colors
} BMPInfoHeader;

#pragma pack(pop)

// RGB Pixel structure
typedef struct
{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
} Pixel;

// Image structure
typedef struct
{
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;
    Pixel *pixels; // Linear array of pixels
    size_t width;
    size_t height;
} BMPImage;

BMPImage *loadBMP(const char *filename);
int saveBMP(BMPImage *image, const char *filename);
void freeBMP(BMPImage *image);
BMPImage *createEmptyBMP(int width, int height);

int addBrightness(BMPImage *image, int value);
int averageImages(BMPImage *image1, BMPImage *image2, BMPImage **result);
int convertToGrayscale(BMPImage *image);
int flipImage(BMPImage *image, const char *direction);
int blurImage(BMPImage *image, int radius);
int detectEdges(BMPImage *image, int threshold);

void printUsage();
int parseArgs(int argc, char *argv[]);

// Access pixel at position (x,y)
#define PIXEL_AT(image, x, y) ((image)->pixels + (y) * (image)->width + (x))

// Global variables to store command line arguments
char *inputFiles[2] = {NULL, NULL};
int inputFileCount = 0;
char *outputFile = NULL;
char *operation = NULL;
char *opArgs[5] = {NULL};
int opArgsCount = 0;

// Main function
int main(int argc, char *argv[])
{
    // Parse command line arguments
    if (parseArgs(argc, argv) != 0)
    {
        printUsage();
        return 1;
    }

    // Load the first input image
    BMPImage *image1 = loadBMP(inputFiles[0]);
    if (!image1)
    {
        fprintf(stderr, "Error: Could not load input file %s\n", inputFiles[0]);
        return 1;
    }

    BMPImage *image2 = NULL;
    BMPImage *result = NULL;
    int operationSuccess = 0;

    // Perform the requested operation
    if (strcmp(operation, "add") == 0)
    {
        if (opArgsCount < 1)
        {
            fprintf(stderr, "Error: 'add' operation requires a value\n");
            freeBMP(image1);
            return 1;
        }
        // Convert string to integer (supports both positive and negative values)
        char *endptr;
        int value = strtol(opArgs[0], &endptr, 10);
        // Check for conversion errors
        if (*endptr != '\0')
        {
            fprintf(stderr, "Error: Invalid brightness value '%s'. Must be an integer.\n", opArgs[0]);
            freeBMP(image1);
            return 1;
        }
        if (value == 0 || value > 255)
        {
            fprintf(stderr, "Error: Brightness value must be between 1 and 255\n");
            freeBMP(image1);
            return 1;
        }
        operationSuccess = addBrightness(image1, value);
        result = image1;
    }
    else if (strcmp(operation, "average") == 0)
    {
        if (inputFileCount < 2)
        {
            fprintf(stderr, "Error: 'average' operation requires two input images\n");
            freeBMP(image1);
            return 1;
        }
        image2 = loadBMP(inputFiles[1]);
        if (!image2)
        {
            fprintf(stderr, "Error: Could not load second input file %s\n", inputFiles[1]);
            freeBMP(image1);
            return 1;
        }
        operationSuccess = averageImages(image1, image2, &result);
    }
    else if (strcmp(operation, "grayscale") == 0)
    {
        operationSuccess = convertToGrayscale(image1);
        result = image1;
    }
    else if (strcmp(operation, "flip") == 0)
    {
        if (opArgsCount < 1)
        {
            fprintf(stderr, "Error: 'flip' operation requires a direction (h or v)\n");
            freeBMP(image1);
            return 1;
        }
        operationSuccess = flipImage(image1, opArgs[0]);
        result = image1;
    }
    else if (strcmp(operation, "blur") == 0)
    {
        int radius = 1; // Default radius
        if (opArgsCount >= 1)
        {
            radius = atoi(opArgs[0]);
            if (radius < 1)
                radius = 1;
            if (radius > 10)
                radius = 10; // Limit radius for performance
        }
        operationSuccess = blurImage(image1, radius);
        result = image1;
    }
    else if (strcmp(operation, "sobel") == 0)
    {
        int threshold = 100; // 默认阈值
        if (opArgsCount >= 1)
        {
            threshold = atoi(opArgs[0]);
            if (threshold < 0)
                threshold = 0;
            if (threshold > 255)
                threshold = 255;
        }
        operationSuccess = detectEdges(image1, threshold);
        result = image1;
    }
    else
    {
        fprintf(stderr, "Error: Unknown operation '%s'\n", operation);
        freeBMP(image1);
        if (image2)
            freeBMP(image2);
        return 1;
    }

    // Check if operation was successful
    if (!operationSuccess || !result)
    {
        fprintf(stderr, "Error: Operation failed\n");
        freeBMP(image1);
        if (image2)
            freeBMP(image2);
        return 1;
    }

    // Save the result image
    if (saveBMP(result, outputFile) != 0)
    {
        fprintf(stderr, "Error: Could not save output file %s\n", outputFile);
        freeBMP(image1);
        if (image2)
            freeBMP(image2);
        if (result != image1 && result != image2)
            freeBMP(result);
        return 1;
    }

    printf("Operation '%s' completed successfully. Output saved to %s\n", operation, outputFile);

    // Clean up
    freeBMP(image1);
    if (image2)
        freeBMP(image2);
    if (result != image1 && result != image2)
        freeBMP(result);

    return 0;
}

// Parse command line arguments
int parseArgs(int argc, char *argv[])
{
    int i = 1;

    while (i < argc)
    {
        if (strcmp(argv[i], "-i") == 0)
        {
            if (i + 1 < argc)
            {
                if (inputFileCount < 2)
                {
                    inputFiles[inputFileCount++] = argv[i + 1];
                }
                else
                {
                    fprintf(stderr, "Error: Too many input files\n");
                    return 1;
                }
                i += 2;
            }
            else
            {
                fprintf(stderr, "Error: Missing filename after -i\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            if (i + 1 < argc)
            {
                outputFile = argv[i + 1];
                i += 2;
            }
            else
            {
                fprintf(stderr, "Error: Missing filename after -o\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-op") == 0)
        {
            if (i + 1 < argc)
            {
                operation = argv[i + 1];
                i += 2;

                // Collect operation arguments
                opArgsCount = 0;
                while (i < argc && argv[i][0] != '-' && opArgsCount < 5)
                {
                    opArgs[opArgsCount++] = argv[i++];
                }

                continue; // Skip the increment at the end of the loop
            }
            else
            {
                fprintf(stderr, "Error: Missing operation after -op\n");
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
            return 1;
        }
    }

    // Validate required arguments
    if (inputFileCount == 0)
    {
        fprintf(stderr, "Error: No input file specified\n");
        return 1;
    }

    if (!outputFile)
    {
        fprintf(stderr, "Error: No output file specified\n");
        return 1;
    }

    if (!operation)
    {
        fprintf(stderr, "Error: No operation specified\n");
        return 1;
    }

    return 0;
}

// Print usage information
void printUsage()
{
    printf("\nUsage: ./bmpedit -i input.bmp [-i input2.bmp] -o output.bmp -op operation [args]\n\n");
    printf("Operations:\n");
    printf("  add VALUE        - Adjust brightness by adding VALUE to all pixels\n");
    printf("  average          - Blend two images by averaging their pixel values\n");
    printf("  grayscale        - Convert image to grayscale\n");
    printf("  flip h|v         - Flip image horizontally (h) or vertically (v)\n");
    printf("  blur [RADIUS]    - Apply blur effect with optional radius (default: 1)\n\n");
    printf("  sobel [THRESHOLD] - Apply Sobel edge detection (default threshold: 100)\n");
    printf("Examples:\n");
    printf("  ./bmpedit -i input.bmp -o output.bmp -op add 50\n");
    printf("  ./bmpedit -i input1.bmp -i input2.bmp -o output.bmp -op average\n");
    printf("  ./bmpedit -i input.bmp -o output.bmp -op grayscale\n");
    printf("  ./bmpedit -i input.bmp -o output.bmp -op flip h\n");
    printf("  ./bmpedit -i input.bmp -o output.bmp -op blur 3\n");
    printf("  ./bmpedit -i input.bmp -o output.bmp -op sobel 120\n");
}

// Load a BMP image from file (optimized version with linear memory)
BMPImage *loadBMP(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }

    BMPImage *image = (BMPImage *)malloc(sizeof(BMPImage));
    if (!image)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    // Read file header
    if (fread(&image->fileHeader, sizeof(BMPFileHeader), 1, file) != 1)
    {
        fprintf(stderr, "Error: Could not read BMP file header\n");
        free(image);
        fclose(file);
        return NULL;
    }

    // Check BMP signature
    if (image->fileHeader.bfType != 0x4D42)
    { // "BM" in little endian
        fprintf(stderr, "Error: Not a BMP file (invalid signature)\n");
        free(image);
        fclose(file);
        return NULL;
    }

    // Read info header
    if (fread(&image->infoHeader, sizeof(BMPInfoHeader), 1, file) != 1)
    {
        fprintf(stderr, "Error: Could not read BMP info header\n");
        free(image);
        fclose(file);
        return NULL;
    }

    // Check if it's a 24-bit uncompressed BMP
    if (image->infoHeader.biBitCount != 24 || image->infoHeader.biCompression != 0)
    {
        fprintf(stderr, "Error: Only 24-bit uncompressed BMP files are supported\n");
        free(image);
        fclose(file);
        return NULL;
    }

    // Store image dimensions
    image->width = image->infoHeader.biWidth;
    image->height = image->infoHeader.biHeight;

    // Calculate row padding (rows must be aligned to 4 bytes)
    int padBytes = (4 - (image->width * 3) % 4) % 4;

    // Allocate memory for pixel data (single contiguous block)
    image->pixels = (Pixel *)malloc(image->width * image->height * sizeof(Pixel));
    if (!image->pixels)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(image);
        fclose(file);
        return NULL;
    }

    // Seek to the beginning of pixel data
    fseek(file, image->fileHeader.bfOffBits, SEEK_SET);

    // Temporary buffer for a row including padding
    unsigned char *rowBuffer = (unsigned char *)malloc(image->width * 3 + padBytes);
    if (!rowBuffer)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(image->pixels);
        free(image);
        fclose(file);
        return NULL;
    }

    // Read pixel data row by row (bottom to top)
    for (int i = image->height - 1; i >= 0; i--)
    {
        // Position in the linear array for current row
        Pixel *rowStart = image->pixels + i * image->width;

        // Read the entire row with padding
        if (fread(rowBuffer, 1, image->width * 3 + padBytes, file) != image->width * 3 + padBytes)
        {
            fprintf(stderr, "Error: Could not read pixel data\n");
            free(rowBuffer);
            free(image->pixels);
            free(image);
            fclose(file);
            return NULL;
        }

        // Copy pixel data from buffer (excluding padding)
        for (int j = 0; j < image->width; j++)
        {
            rowStart[j].blue = rowBuffer[j * 3];
            rowStart[j].green = rowBuffer[j * 3 + 1];
            rowStart[j].red = rowBuffer[j * 3 + 2];
        }
    }

    free(rowBuffer);
    fclose(file);
    return image;
}

// Save a BMP image to file (optimized version with linear memory)
int saveBMP(BMPImage *image, const char *filename)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        return 1;
    }

    // Write file header
    if (fwrite(&image->fileHeader, sizeof(BMPFileHeader), 1, file) != 1)
    {
        fprintf(stderr, "Error: Could not write BMP file header\n");
        fclose(file);
        return 1;
    }

    // Write info header
    if (fwrite(&image->infoHeader, sizeof(BMPInfoHeader), 1, file) != 1)
    {
        fprintf(stderr, "Error: Could not write BMP info header\n");
        fclose(file);
        return 1;
    }
    int padBytes = (4 - (image->width * 3) % 4) % 4;
    // Temporary buffer for a row including padding
    unsigned char *rowBuffer = (unsigned char *)malloc(image->width * 3 + padBytes);
    if (!rowBuffer)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return 1;
    }

    // Initialize padding bytes to 0
    for (int i = 0; i < padBytes; i++)
    {
        rowBuffer[image->width * 3 + i] = 0;
    }

    // Seek to the beginning of pixel data
    fseek(file, image->fileHeader.bfOffBits, SEEK_SET);

    // Write pixel data row by row (bottom to top)
    for (int i = image->height - 1; i >= 0; i--)
    {
        // Position in the linear array for current row
        Pixel *rowStart = image->pixels + i * image->width;

        // Copy pixel data to buffer
        for (int j = 0; j < image->width; j++)
        {
            rowBuffer[j * 3] = rowStart[j].blue;
            rowBuffer[j * 3 + 1] = rowStart[j].green;
            rowBuffer[j * 3 + 2] = rowStart[j].red;
        }

        // Write the entire row with padding
        if (fwrite(rowBuffer, 1, image->width * 3 + padBytes, file) != image->width * 3 + padBytes)
        {
            fprintf(stderr, "Error: Could not write pixel data\n");
            free(rowBuffer);
            fclose(file);
            return 1;
        }
    }

    free(rowBuffer);
    fclose(file);
    return 0;
}

// Free memory allocated for BMP image
void freeBMP(BMPImage *image)
{
    if (!image)
        return;
    if (image->pixels)
    {
        free(image->pixels);
    }
    free(image);
}

// Create an empty BMP image with specified dimensions
BMPImage *createEmptyBMP(int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        fprintf(stderr, "Error: Invalid image dimensions\n");
        return NULL;
    }

    BMPImage *image = (BMPImage *)malloc(sizeof(BMPImage));
    if (!image)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    // Calculate row padding (rows must be aligned to 4 bytes)
    int padBytes = (4 - (width * 3) % 4) % 4;
    // Calculate file size
    int headerSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    int pixelDataSize = (width * 3 + padBytes) * height;
    int fileSize = headerSize + pixelDataSize;

    // Initialize file header
    image->fileHeader.bfType = 0x4D42; // "BM" in little endian
    image->fileHeader.bfSize = fileSize;
    image->fileHeader.bfReserved1 = 0;
    image->fileHeader.bfReserved2 = 0;
    image->fileHeader.bfOffBits = headerSize;

    // Initialize info header
    image->infoHeader.biSize = sizeof(BMPInfoHeader);
    image->infoHeader.biWidth = width;
    image->infoHeader.biHeight = height;
    image->infoHeader.biPlanes = 1;
    image->infoHeader.biBitCount = 24;
    image->infoHeader.biCompression = 0;
    image->infoHeader.biSizeImage = pixelDataSize;
    image->infoHeader.biXPelsPerMeter = 2835; // 72 DPI
    image->infoHeader.biYPelsPerMeter = 2835; // 72 DPI
    image->infoHeader.biClrUsed = 0;
    image->infoHeader.biClrImportant = 0;

    // Store image dimensions
    image->width = width;
    image->height = height;

    // Allocate memory for pixel data (single contiguous block)
    image->pixels = (Pixel *)malloc(width * height * sizeof(Pixel));
    if (!image->pixels)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(image);
        return NULL;
    }

    // Initialize pixels to black
    memset(image->pixels, 0, width * height * sizeof(Pixel));

    return image;
}

// Add a value to all pixel components (brightness adjustment)
int addBrightness(BMPImage *image, int value)
{
    if (!image || !image->pixels)
    {
        return 0;
    }

    int totalPixels = image->width * image->height;

#pragma omp parallel for
    for (int i = 0; i < totalPixels; i++)
    {
        // Apply the value to each color channel with clamping
        int newRed = image->pixels[i].red + value;
        int newGreen = image->pixels[i].green + value;
        int newBlue = image->pixels[i].blue + value;

        // Clamp values to 0-255 range
        image->pixels[i].red = (newRed < 0) ? 0 : (newRed > 255) ? 255
                                                                 : newRed;
        image->pixels[i].green = (newGreen < 0) ? 0 : (newGreen > 255) ? 255
                                                                       : newGreen;
        image->pixels[i].blue = (newBlue < 0) ? 0 : (newBlue > 255) ? 255
                                                                    : newBlue;
    }

    return 1;
}

// Average two images (blending)
int averageImages(BMPImage *image1, BMPImage *image2, BMPImage **result)
{
    if (!image1 || !image2 || !image1->pixels || !image2->pixels)
    {
        return 0;
    }

    // Check if the images have the same dimensions
    if (image1->width != image2->width || image1->height != image2->height)
    {
        fprintf(stderr, "Error: Images must have the same dimensions for averaging\n");
        return 0;
    }

    // Create a new image for the result
    *result = createEmptyBMP(image1->width, image1->height);
    if (!*result)
    {
        return 0;
    }

    // Average the pixel values
    int totalPixels = image1->width * image1->height;

#pragma omp parallel for
    for (int i = 0; i < totalPixels; i++)
    {
        (*result)->pixels[i].red = (image1->pixels[i].red + image2->pixels[i].red) / 2;
        (*result)->pixels[i].green = (image1->pixels[i].green + image2->pixels[i].green) / 2;
        (*result)->pixels[i].blue = (image1->pixels[i].blue + image2->pixels[i].blue) / 2;
    }

    return 1;
}

// Convert image to grayscale
int convertToGrayscale(BMPImage *image)
{
    if (!image || !image->pixels)
    {
        return 0;
    }

    int totalPixels = image->width * image->height;

#pragma omp parallel for
    for (int i = 0; i < totalPixels; i++)
    {
        // Use weighted method (luminance): 0.299R + 0.587G + 0.114B
        unsigned char gray = (unsigned char)(0.299 * image->pixels[i].red +
                                             0.587 * image->pixels[i].green +
                                             0.114 * image->pixels[i].blue);

        // Set all channels to the same value
        image->pixels[i].red = gray;
        image->pixels[i].green = gray;
        image->pixels[i].blue = gray;
    }

    return 1;
}

// Flip image horizontally or vertically
int flipImage(BMPImage *image, const char *direction)
{
    if (!image || !image->pixels || !direction)
    {
        return 0;
    }

    if (direction[0] == 'h' || direction[0] == 'H')
    {
        // Horizontal flip
        Pixel temp;
#pragma omp parallel for private(temp)
        for (int i = 0; i < image->height; i++)
        {
            for (int j = 0; j < image->width / 2; j++)
            {
                // Calculate indices for the pixels to swap
                int left = i * image->width + j;
                int right = i * image->width + (image->width - 1 - j);

                // Swap pixels
                temp = image->pixels[left];
                image->pixels[left] = image->pixels[right];
                image->pixels[right] = temp;
            }
        }
    }
    else if (direction[0] == 'v' || direction[0] == 'V')
    {
        // Vertical flip
        Pixel temp;
#pragma omp parallel for private(temp)
        for (int i = 0; i < image->height / 2; i++)
        {
            for (int j = 0; j < image->width; j++)
            {
                // Calculate indices for the pixels to swap
                int top = i * image->width + j;
                int bottom = (image->height - 1 - i) * image->width + j;

                // Swap pixels
                temp = image->pixels[top];
                image->pixels[top] = image->pixels[bottom];
                image->pixels[bottom] = temp;
            }
        }
    }
    else
    {
        fprintf(stderr, "Error: Invalid flip direction. Use 'h' for horizontal or 'v' for vertical.\n");
        return 0;
    }

    return 1;
}

// 修复黑边问题的模糊函数
int blurImage(BMPImage *image, int radius)
{
    if (!image || !image->pixels || radius < 1)
    {
        fprintf(stderr, "Error: Invalid image or radius for blur.\n");
        return 0;
    }

    int width = (int)image->width;
    int height = (int)image->height;

    // 分配临时缓冲区
    Pixel *tempPixels = (Pixel *)malloc(width * height * sizeof(Pixel));
    if (!tempPixels)
    {
        fprintf(stderr, "Error: Memory allocation failed for blur temp buffer.\n");
        return 0;
    }

    TimePoint start, end;
    double time_taken;
    get_time(&start);
#pragma omp parallel for schedule(static)
    for (int i = 0; i < height; i++)
    {
        int index_temp = i * width;
        for (int j = 0; j < width; j++)
        {
            long long sumRed = 0, sumGreen = 0, sumBlue = 0;
            int count = 0;
            for (int kj = -radius; kj <= radius; kj++)
            {
                int col = j + kj;
                if (col < 0)
                    col = -col;
                if (col >= width)
                    col = 2 * width - col - 1;
                Pixel *pixel = &image->pixels[i * width + col];
                sumRed += pixel->red;
                sumGreen += pixel->green;
                sumBlue += pixel->blue;
                count++;
            }
            tempPixels[index_temp + j].red = (unsigned char)(sumRed / count);
            tempPixels[index_temp + j].green = (unsigned char)(sumGreen / count);
            tempPixels[index_temp + j].blue = (unsigned char)(sumBlue / count);
        }
    }

#pragma omp parallel for schedule(static)
    for (int i = 0; i < height; i++)
    {
        int index_temp = i * width;
        for (int j = 0; j < width; j++)
        {
            long long sumRed = 0, sumGreen = 0, sumBlue = 0;
            int count = 0;
            for (int ki = -radius; ki <= radius; ki++)
            {
                int row = i + ki;
                if (row < 0)
                    row = -row;
                if (row >= height)
                    row = 2 * height - row - 1;
                Pixel *pixel = &tempPixels[row * width + j];
                sumRed += pixel->red;
                sumGreen += pixel->green;
                sumBlue += pixel->blue;
                count++;
            }
            image->pixels[index_temp + j].red = (unsigned char)(sumRed / count);
            image->pixels[index_temp + j].green = (unsigned char)(sumGreen / count);
            image->pixels[index_temp + j].blue = (unsigned char)(sumBlue / count);
        }
    }

    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Blur operation took %f seconds\n", time_taken);
    free(tempPixels);
    return 1;
}

int detectEdges(BMPImage *image, int threshold)
{
    if (!image || !image->pixels)
    {
        fprintf(stderr, "Error: Invalid image for edge detection.\n");
        return 0;
    }

    int width = (int)image->width;
    int height = (int)image->height;

    Pixel *tempPixels = (Pixel *)malloc(width * height * sizeof(Pixel));
    if (!tempPixels)
    {
        fprintf(stderr, "Error: Memory allocation failed for edge detection buffer.\n");
        return 0;
    }

    TimePoint start, end;
    double time_taken;
    get_time(&start);

#pragma omp parallel for schedule(static)
    for (int i = 1; i < height - 1; i++)
    {
        Pixel *prevRow = &image->pixels[(i - 1) * width];
        Pixel *currRow = &image->pixels[i * width];
        Pixel *nextRow = &image->pixels[(i + 1) * width];
        Pixel *destRow = &tempPixels[i * width];

        for (int j = 1; j < width - 1; j++)
        {
            int gx_r = (prevRow[j + 1].red + 2 * currRow[j + 1].red + nextRow[j + 1].red) -
                       (prevRow[j - 1].red + 2 * currRow[j - 1].red + nextRow[j - 1].red);
            int gx_g = (prevRow[j + 1].green + 2 * currRow[j + 1].green + nextRow[j + 1].green) -
                       (prevRow[j - 1].green + 2 * currRow[j - 1].green + nextRow[j - 1].green);
            int gx_b = (prevRow[j + 1].blue + 2 * currRow[j + 1].blue + nextRow[j + 1].blue) -
                       (prevRow[j - 1].blue + 2 * currRow[j - 1].blue + nextRow[j - 1].blue);
            int gy_r = (nextRow[j - 1].red + 2 * nextRow[j].red + nextRow[j + 1].red) -
                       (prevRow[j - 1].red + 2 * prevRow[j].red + prevRow[j + 1].red);
            int gy_g = (nextRow[j - 1].green + 2 * nextRow[j].green + nextRow[j + 1].green) -
                       (prevRow[j - 1].green + 2 * prevRow[j].green + prevRow[j + 1].green);
            int gy_b = (nextRow[j - 1].blue + 2 * nextRow[j].blue + nextRow[j + 1].blue) -
                       (prevRow[j - 1].blue + 2 * prevRow[j].blue + prevRow[j + 1].blue);

            int mag_r = abs(gx_r) + abs(gy_r);
            int mag_g = abs(gx_g) + abs(gy_g);
            int mag_b = abs(gx_b) + abs(gy_b);

            int magnitude = mag_r;
            if (mag_g > magnitude)
                magnitude = mag_g;
            if (mag_b > magnitude)
                magnitude = mag_b;

            unsigned char edge_val = (magnitude > threshold) ? 0 : 255;
            destRow[j].red = edge_val;
            destRow[j].green = edge_val;
            destRow[j].blue = edge_val;
        }
    }

#pragma omp parallel for
    for (int j = 0; j < width; j++)
    {
        tempPixels[j].red = tempPixels[j].green = tempPixels[j].blue = 0;
    }
#pragma omp parallel for
    for (int j = 0; j < width; j++)
    {
        tempPixels[(height - 1) * width + j].red = 0;
        tempPixels[(height - 1) * width + j].green = 0;
        tempPixels[(height - 1) * width + j].blue = 0;
    }
#pragma omp parallel for
    for (int i = 1; i < height - 1; i++)
    {
        // 左边界
        tempPixels[i * width].red = 0;
        tempPixels[i * width].green = 0;
        tempPixels[i * width].blue = 0;
        // 右边界
        tempPixels[i * width + width - 1].red = 0;
        tempPixels[i * width + width - 1].green = 0;
        tempPixels[i * width + width - 1].blue = 0;
    }

    memcpy(image->pixels, tempPixels, width * height * sizeof(Pixel));

    get_time(&end);
    time_taken = time_diff_seconds(start, end);
    printf("Edge detection took %f seconds\n", time_taken);

    free(tempPixels);
    return 1;
}