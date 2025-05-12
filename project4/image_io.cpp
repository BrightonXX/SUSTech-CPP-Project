// image_io.cpp
#include "image.h"
#include <fstream>
#include <vector>
#include <iostream>

#pragma pack(push, 1) // 设置为1字节对齐

// BMP 文件头
struct BMPFileHeader
{
    uint16_t bfType;      // 文件类型标识，必须为 0x4D42 ('BM')
    uint32_t bfSize;      // 整个文件的大小（字节）
    uint16_t bfReserved1; // 保留，必须为0
    uint16_t bfReserved2; // 保留，必须为0
    uint32_t bfOffBits;   // 从文件头到实际像素数据的偏移量（字节）
};

// BMP 信息头
struct BMPInfoHeader
{
    uint32_t biSize;         // 此结构体的大小 (40字节)
    int32_t biWidth;         // 图像宽度（像素）
    int32_t biHeight;        // 图像高度（像素）。如果为正，图像是底向顶存储。如果为负，顶向底。
    uint16_t biPlanes;       // 色彩平面数，必须为1
    uint16_t biBitCount;     // 每像素位数 (我们处理24位，即每个像素3字节)
    uint32_t biCompression;  // 压缩类型 (0表示不压缩 BI_RGB)
    uint32_t biSizeImage;    // 实际像素数据的大小（字节）。对于未压缩图像，可以设为0。
    int32_t biXPelsPerMeter; // 水平分辨率（像素/米），可忽略
    int32_t biYPelsPerMeter; // 垂直分辨率（像素/米），可忽略
    uint32_t biClrUsed;      // 调色板中颜色数，对于24位图像为0
    uint32_t biClrImportant; // 重要颜色数，对于24位图像为0
};

#pragma pack(pop) // 恢复之前的对齐设置

static_assert(sizeof(BMPFileHeader) == 14, "BMPFileHeader size must be 14 bytes");
static_assert(sizeof(BMPInfoHeader) == 40, "BMPInfoHeader size must be 40 bytes");

namespace ILib
{

    ImageU8C3 loadImageFromFile(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        BMPFileHeader fileHeader;
        BMPInfoHeader infoHeader;

        // 1. 读取文件头
        file.read(reinterpret_cast<char *>(&fileHeader), sizeof(BMPFileHeader));
        if (!file)
        {
            throw std::runtime_error("Failed to read BMP file header from: " + filename);
        }

        // 2. 验证文件类型
        if (fileHeader.bfType != 0x4D42)
        { // 'BM' 的小端表示
            throw std::runtime_error("Not a BMP file (invalid signature): " + filename);
        }

        // 3. 读取信息头
        file.read(reinterpret_cast<char *>(&infoHeader), sizeof(BMPInfoHeader));
        if (!file)
        {
            throw std::runtime_error("Failed to read BMP info header from: " + filename);
        }

        // 4. 验证我们支持的 BMP 类型
        if (infoHeader.biBitCount != 24)
        {
            throw std::runtime_error("Unsupported BMP: bit count is not 24. Received: " + std::to_string(infoHeader.biBitCount));
        }
        if (infoHeader.biCompression != 0)
        {
            throw std::runtime_error("Unsupported BMP: compression is not 0 (uncompressed). Received: " + std::to_string(infoHeader.biCompression));
        }
        if (infoHeader.biWidth <= 0)
        { // biHeight 可以为负
            throw std::runtime_error("Unsupported BMP: invalid width " + std::to_string(infoHeader.biWidth));
        }

        // 5. 获取图像尺寸
        size_t width = static_cast<size_t>(infoHeader.biWidth);
        size_t height = static_cast<size_t>(std::abs(infoHeader.biHeight)); // 高度取绝对值
        bool top_down_dib = (infoHeader.biHeight < 0);                      // 检查图像是否是顶行优先存储

        // 6. 创建 Image 对象
        ImageU8C3 image(width, height, ColorSpace::BGR);
        if (image.empty())
        { // 检查内存分配是否成功
            throw std::runtime_error("Failed to create image object for: " + filename);
        }

        // 7. 计算 BMP 文件中每行的实际字节数 (考虑4字节对齐的 padding)
        size_t bmp_row_stride_bytes = ((width * 3) + 3) & ~3u; // 确保是4的倍数

        // 8. 读取像素数据
        file.seekg(fileHeader.bfOffBits, std::ios::beg); // 跳转到像素数据起始位置
        if (!file)
        {
            throw std::runtime_error("Failed to seek to pixel data in: " + filename);
        }

        size_t image_row_bytes = image.width() * sizeof(BGRPixelU8); // Image 中一行的有效数据字节

        std::vector<unsigned char> bmp_row_buffer(bmp_row_stride_bytes);

        for (size_t y = 0; y < height; ++y)
        {
            size_t dest_y = top_down_dib ? y : (height - 1 - y);
            file.read(reinterpret_cast<char *>(bmp_row_buffer.data()), bmp_row_stride_bytes);
            if (!file)
            {
                throw std::runtime_error("Failed to read pixel row " + std::to_string(y) + " from: " + filename);
            }
            std::memcpy(image.ptr(dest_y), bmp_row_buffer.data(), image_row_bytes);
        }
        return image;
    }

    int saveImageToFile(const ImageU8C3 &image, const std::string &filename)
    {
        if (image.empty())
        {
            std::cerr << "Error: Cannot save an empty image to " << filename << std::endl;
            return -1;
        }
        if (image.getColorSpace() != ColorSpace::BGR)
        {
            std::cerr << "Warning: Saving image with color space other than BGR as BMP. "
                      << "Colors might be incorrect if not BGR. Current: "
                      << static_cast<int>(image.getColorSpace()) << std::endl;
        }

        std::ofstream file(filename, std::ios::binary | std::ios::trunc);
        if (!file)
        {
            std::cerr << "Error: Cannot open file for writing: " << filename << std::endl;
            return -1;
        }

        size_t width = image.width();
        size_t height = image.height();

        BMPInfoHeader infoHeader = {};
        infoHeader.biSize = sizeof(BMPInfoHeader);
        infoHeader.biWidth = static_cast<int32_t>(width);
        infoHeader.biHeight = static_cast<int32_t>(height);
        infoHeader.biPlanes = 1;
        infoHeader.biBitCount = 24;
        infoHeader.biCompression = 0;

        size_t bmp_row_stride_bytes = ((width * 3) + 3) & ~3u;
        infoHeader.biSizeImage = static_cast<uint32_t>(bmp_row_stride_bytes * height);

        BMPFileHeader fileHeader = {};
        fileHeader.bfType = 0x4D42;                                           // 'BM'
        fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader); // 偏移到像素数据
        fileHeader.bfSize = fileHeader.bfOffBits + infoHeader.biSizeImage;    // 文件总大小
        fileHeader.bfReserved1 = 0;
        fileHeader.bfReserved2 = 0;

        file.write(reinterpret_cast<const char *>(&fileHeader), sizeof(BMPFileHeader));
        if (!file)
        {
            std::cerr << "Error: Failed to write BMP file header to " << filename << std::endl;
            return -1;
        }
        file.write(reinterpret_cast<const char *>(&infoHeader), sizeof(BMPInfoHeader));
        if (!file)
        {
            std::cerr << "Error: Failed to write BMP info header to " << filename << std::endl;
            return -1;
        }

        size_t image_row_bytes = image.width() * sizeof(BGRPixelU8);
        std::vector<unsigned char> padding_bytes(bmp_row_stride_bytes - image_row_bytes, 0);

        for (size_t y = 0; y < height; ++y)
        {
            const BGRPixelU8 *image_row_ptr = image.ptr(height - 1 - y);

            file.write(reinterpret_cast<const char *>(image_row_ptr), image_row_bytes);
            if (!padding_bytes.empty())
            {
                file.write(reinterpret_cast<const char *>(padding_bytes.data()), padding_bytes.size());
            }
            if (!file)
            {
                std::cerr << "Error: Failed to write pixel row " << y << " to " << filename << std::endl;
                return -1;
            }
        }

        if (!file.good())
        {
            std::cerr << "Error: An error occurred during writing to " << filename << std::endl;
            return -1;
        }

        return 0;
    }
    int saveImageToFile(const ImageU8C1& gray_image, const std::string& filename) {
        if (gray_image.empty()) {
            std::cerr << "Error: Cannot save an empty grayscale image to " << filename << std::endl;
            return -1;
        }
    
        // 1. 创建一个临时的 BGR U8C3 图像，尺寸与灰度图相同
        ImageU8C3 temp_bgr_image(gray_image.width(), gray_image.height(), ColorSpace::BGR);
        if (temp_bgr_image.empty()) {
             std::cerr << "Error: Failed to create temporary BGR image for saving." << std::endl;
             return -1;
        }
    
        // 2. 将灰度图像素复制到临时 BGR 图像的三个通道
        #pragma omp parallel for // 可以并行处理行
        for (size_t y = 0; y < gray_image.height(); ++y) {
            const GrayscalePixelU8* gray_row = gray_image.ptr(y); // ptr 返回 Pixel<uchar, 1>*
            BGRPixelU8* bgr_row = temp_bgr_image.ptr(y);         // ptr 返回 Pixel<uchar, 3>*
    
            for (size_t x = 0; x < gray_image.width(); ++x) {
                uchar gray_value = gray_row[x][0]; // 获取灰度值
                bgr_row[x][0] = gray_value; // B = Gray
                bgr_row[x][1] = gray_value; // G = Gray
                bgr_row[x][2] = gray_value; // R = Gray
            }
        }
    
        // 3. 调用已有的保存 BGR 图像的函数来保存这个临时图像
        return saveImageToFile(temp_bgr_image, filename);
    }
} // namespace ILib