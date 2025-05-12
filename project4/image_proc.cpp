#include "image.h" // 需要 Image 和 Pixel 定义
#include <vector>
#include <cmath>
#include <numeric> 
#include <stdexcept>
#ifdef __AVX2__ // 仅在支持 AVX2 时编译
#include <immintrin.h> 
#endif

#include <immintrin.h>
namespace ILib
{
    // 创建均值滤波核 (Box Filter) - 类型是 float
    Image<Pixel<float, 1>> createBoxKernel(int ksize)
    {
        if (ksize <= 0 || ksize % 2 == 0)
        {
            throw std::invalid_argument("createBoxKernel: ksize must be a positive odd integer.");
        }
        Image<Pixel<float, 1>> kernel(ksize, ksize, ColorSpace::UNKNOWN);
        float value = 1.0f / (ksize * ksize);
        for (int y = 0; y < ksize; ++y)
        {
            Pixel<float, 1> *kernel_row = kernel.ptr(y);
            for (int x = 0; x < ksize; ++x)
            {
                kernel_row[x][0] = value; 
            }
        }
        return kernel;
    }

    // 创建高斯模糊核
    Image<Pixel<float, 1>> createGaussianKernel(int ksize, double sigma)
    {
        if (sigma <= 0) sigma = 0.3 * ((ksize - 1) * 0.5 - 1) + 0.8;
        Image<Pixel<float, 1>> kernel(ksize, ksize, ColorSpace::UNKNOWN);
        double sum = 0.0;
        int center = ksize / 2;
        double sigma_sq_2 = 2.0 * sigma * sigma;
        double pi_sigma_sq_2 = acos(-1.0) * sigma_sq_2; 

        for (int y = -center; y <= center; ++y)
        {
            for (int x = -center; x <= center; ++x)
            {
                double exponent = -(x * x + y * y) / sigma_sq_2;
                double value = std::exp(exponent) / pi_sigma_sq_2;
                kernel.at(y + center, x + center)[0] = static_cast<float>(value); // 访问 data[0]
                sum += value;
            }
        }

        float norm_factor = static_cast<float>(1.0 / sum);
        for (int y = 0; y < ksize; ++y)
        {
            Pixel<float, 1> *kernel_row = kernel.ptr(y);
            for (int x = 0; x < ksize; ++x)
            {
                kernel_row[x][0] *= norm_factor; // 访问 data[0]
            }
        }
        return kernel;
    }

    // 创建拉普拉斯算子核 (4邻域或8邻域简化版)
    Image<Pixel<int, 1>> createLaplacianKernel(int ksize)
    {
        if (ksize == 1)
        {                                                       
            Image<Pixel<int, 1>> kernel(3, 3, ColorSpace::UNKNOWN); 
            // 使用初始化列表的方式赋值更简洁
            kernel.at(0, 0) = {0};
            kernel.at(0, 1) = {1};
            kernel.at(0, 2) = {0};
            kernel.at(1, 0) = {1};
            kernel.at(1, 1) = {-4};
            kernel.at(1, 2) = {1};
            kernel.at(2, 0) = {0};
            kernel.at(2, 1) = {1};
            kernel.at(2, 2) = {0};
            return kernel;
        }
        else
        { 
            Image<Pixel<int, 1>> kernel(3, 3, ColorSpace::UNKNOWN);
            kernel.at(0, 0) = {1};
            kernel.at(0, 1) = {1};
            kernel.at(0, 2) = {1};
            kernel.at(1, 0) = {1};
            kernel.at(1, 1) = {-8};
            kernel.at(1, 2) = {1};
            kernel.at(2, 0) = {1};
            kernel.at(2, 1) = {1};
            kernel.at(2, 2) = {1};
            return kernel;
        }
    }

    // 创建简单的锐化核
    Image<Pixel<int, 1>> createSharpenKernel()
    {
        Image<Pixel<int, 1>> kernel(3, 3, ColorSpace::UNKNOWN);
        kernel.at(0, 0) = {0};
        kernel.at(0, 1) = {-1};
        kernel.at(0, 2) = {0};
        kernel.at(1, 0) = {-1};
        kernel.at(1, 1) = {5};
        kernel.at(1, 2) = {-1};
        kernel.at(2, 0) = {0};
        kernel.at(2, 1) = {-1};
        kernel.at(2, 2) = {0};
        return kernel;
    }

    // 创建简单的浮雕效果核
    Image<Pixel<int, 1>> createEmbossKernel()
    {
        Image<Pixel<int, 1>> kernel(3, 3, ColorSpace::UNKNOWN);
        kernel.at(0, 0) = {-2};
        kernel.at(0, 1) = {-1};
        kernel.at(0, 2) = {0};
        kernel.at(1, 0) = {-1};
        kernel.at(1, 1) = {1};
        kernel.at(1, 2) = {1};
        kernel.at(2, 0) = {0};
        kernel.at(2, 1) = {1};
        kernel.at(2, 2) = {2};
        return kernel;
    }
    // 创建 Sobel X 方向算子核
    Image<Pixel<int, 1>> createSobelKernelX(int ksize)
    {
        if (ksize != 3)
        { 
            throw std::invalid_argument("createSobelKernelX: ksize must be 3 for this simplified version.");
        }
        Image<Pixel<int, 1>> kernel(3, 3, ColorSpace::UNKNOWN, true);
        kernel.at(0, 0)[0] = -1;
        kernel.at(0, 1)[0] = 0;
        kernel.at(0, 2)[0] = 1;
        kernel.at(1, 0)[0] = -2;
        kernel.at(1, 1)[0] = 0;
        kernel.at(1, 2)[0] = 2;
        kernel.at(2, 0)[0] = -1;
        kernel.at(2, 1)[0] = 0;
        kernel.at(2, 2)[0] = 1;
        return kernel;
    }

    // 创建 Sobel Y 方向算子核
    Image<Pixel<int, 1>> createSobelKernelY(int ksize)
    {
        if (ksize != 3)
        {
            throw std::invalid_argument("createSobelKernelY: ksize must be 3 for this simplified version.");
        }
        Image<Pixel<int, 1>> kernel(3, 3, ColorSpace::UNKNOWN, true);
        kernel.at(0, 0)[0] = -1;
        kernel.at(0, 1)[0] = -2;
        kernel.at(0, 2)[0] = -1;
        kernel.at(1, 0)[0] = 0;
        kernel.at(1, 1)[0] = 0;
        kernel.at(1, 2)[0] = 0;
        kernel.at(2, 0)[0] = 1;
        kernel.at(2, 1)[0] = 2;
        kernel.at(2, 2)[0] = 1;
        return kernel;
    }

    // Sobel 梯度计算
    void sobelGradients(const ImageU8C1 &src, Image<Pixel<short, 1>> &dst_dx, Image<Pixel<short, 1>> &dst_dy, int ksize)
    {
        if (src.empty())
        {
            dst_dx.release();
            dst_dy.release();
            return;
        }
        Image<Pixel<int, 1>> kernelX = createSobelKernelX(ksize);
        Image<Pixel<int, 1>> kernelY = createSobelKernelY(ksize);

        filter2D(src, dst_dx, kernelX); 
        filter2D(src, dst_dy, kernelY); 
    }

    // Sobel 幅度与阈值处理
    void sobelMagnitude(const ImageU8C1 &src, ImageU8C1 &dst_edge, int ksize, double threshold, bool use_l1_norm)
    {
        if (src.empty())
        {
            dst_edge.release();
            return;
        }
    
        Image<Pixel<short, 1>> grad_x, grad_y;
        sobelGradients(src, grad_x, grad_y, ksize);
        dst_edge.create(src.width(), src.height(), ColorSpace::GRAYSCALE);

#pragma omp parallel for
        for (size_t y = 0; y < src.height(); ++y)
        {
            const Pixel<short, 1> *gx_row = grad_x.ptr(y);
            const Pixel<short, 1> *gy_row = grad_y.ptr(y);
            GrayscalePixelU8 *edge_row = dst_edge.ptr(y); // 输出是 uchar

            for (size_t x = 0; x < src.width(); ++x)
            {
                double mag = 0.0;
                short gx = gx_row[x][0]; 
                short gy = gy_row[x][0];

                if (use_l1_norm)
                { // 使用 L1 范数 (绝对值和)
                    mag = std::abs(static_cast<double>(gx)) + std::abs(static_cast<double>(gy));
                }
                else
                { // 使用 L2 范数 (平方和根)
                    mag = std::sqrt(static_cast<double>(gx) * gx + static_cast<double>(gy) * gy);
                }
                // 应用阈值
                edge_row[x][0] = (mag > threshold) ? 0 : 255; // 边缘为白色，背景为黑色。（虽然一般是相反的）
            }
        }
    }

    // 拉普拉斯算子
    void laplacian(const ImageU8C1 &src, Image<Pixel<short, 1>> &dst, int ksize)
    {
        if (src.empty())
        {
            dst.release();
            return;
        }
        Image<Pixel<int, 1>> kernel = createLaplacianKernel(ksize);
        filter2D(src, dst, kernel);
    }
    // 灰色调转换
    ImageU8C1 convertToGrayscale(const ImageU8C3 &srcImage)
    {
        if (srcImage.empty())
        {
            throw std::runtime_error("convertToGrayscale: Source image is empty.");
        }
        // 标准亮度公式： Y = 0.299*R + 0.587*G + 0.114*B
        const double R_WEIGHT = 0.299;
        const double G_WEIGHT = 0.587;
        const double B_WEIGHT = 0.114;

        ImageU8C1 dstImage(srcImage.width(), srcImage.height(), ColorSpace::GRAYSCALE);
        if (dstImage.empty())
        { 
            throw std::runtime_error("convertToGrayscale: Failed to create destination grayscale image.");
        }

        const size_t width = srcImage.width();
        const size_t height = srcImage.height();

#pragma omp parallel for schedule(static)
        for (size_t y = 0; y < height; ++y)
        {
            const BGRPixelU8 *src_row = srcImage.ptr(y); 
            GrayscalePixelU8 *dst_row = dstImage.ptr(y); 

            for (size_t x = 0; x < width; ++x)
            {
                uchar b = src_row[x][0]; // Blue
                uchar g = src_row[x][1]; // Green
                uchar r = src_row[x][2]; // Red

                double gray_double = B_WEIGHT * static_cast<double>(b) +
                                     G_WEIGHT * static_cast<double>(g) +
                                     R_WEIGHT * static_cast<double>(r);

                long long gray_ll = static_cast<long long>(std::round(gray_double));
                uchar gray_uchar = static_cast<uchar>(std::clamp(gray_ll, 0LL, 255LL));

                dst_row[x][0] = gray_uchar; 
            }
        }

        return dstImage;
    }
    ImageU8C3 adjustBrightness_simd(const ImageU8C3 &src, int value_offset)
    {
        if (src.empty())
        {
            throw std::runtime_error("adjustBrightness_simd: Source image is empty.");
        }
        if (src.getColorSpace() != ColorSpace::BGR && src.getColorSpace() != ColorSpace::RGB && src.getColorSpace() != ColorSpace::UNKNOWN)
        {
            std::cerr << "Warning: adjustBrightness_simd applied to non-BGR/RGB image. Color space tag is " << static_cast<int>(src.getColorSpace()) << std::endl;
        }

        ImageU8C3 dst = src.clone(); // 创建副本进行修改

        const size_t width = dst.width();
        const size_t height = dst.height();
        const int channels = 3; 

        // --- SIMD 处理 (如果支持 AVX2) ---
#ifdef __AVX2__
        // AVX2 一次处理 32 字节 (256位)
        const size_t simd_step = 32;
        // 计算每行能处理的完整 SIMD 块数 (注意是字节数)
        const size_t num_simd_blocks_per_row = (width * channels) / simd_step;
        // 每行末尾剩余的字节数
        const size_t remaining_bytes_per_row = (width * channels) % simd_step;

        const short offset_s = static_cast<short>(std::clamp(value_offset, -32767, 32767));

        const __m256i offset_vec_s16 = _mm256_set1_epi16(offset_s);

#pragma omp parallel for schedule(static)
        for (size_t y = 0; y < height; ++y)
        {
            uchar *dst_row_ptr = reinterpret_cast<uchar *>(dst.ptr(y)); 
            for (size_t block = 0; block < num_simd_blocks_per_row; ++block)
            {
                size_t offset_bytes = block * simd_step;
                uchar *current_ptr = dst_row_ptr + offset_bytes;

                // 1. 加载 32 字节 (256位) 的 BGR 数据到 AVX 寄存器
                __m256i bgr_vec = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(current_ptr));

                // 2. 将 8 位无符号整数 (uchar) 扩展为 16 位有符号整数 (short)
                //    我们需要分两次进行，因为一个 __m256i 只能存 16 个 short
                //    获取低 16 字节 (128位)
                __m128i bgr_low128 = _mm256_extracti128_si256(bgr_vec, 0);
                //    获取高 16 字节 (128位)
                __m128i bgr_high128 = _mm256_extracti128_si256(bgr_vec, 1);

                //    将低 128 位 uchar 扩展为两个 128 位 short (__m128i)
                __m128i zero128 = _mm_setzero_si128();                      // 用于 unpack
                __m128i bgr_s16_0 = _mm_unpacklo_epi8(bgr_low128, zero128); // 扩展低 8 字节
                __m128i bgr_s16_1 = _mm_unpackhi_epi8(bgr_low128, zero128); // 扩展高 8 字节
                __m128i bgr_s16_2 = _mm_unpacklo_epi8(bgr_high128, zero128);
                __m128i bgr_s16_3 = _mm_unpackhi_epi8(bgr_high128, zero128);

                // 将两个 128 位 short 合并回 256 位 short (__m256i)
                __m256i bgr_s16_low = _mm256_set_m128i(bgr_s16_1, bgr_s16_0); // 注意顺序 low/high
                __m256i bgr_s16_high = _mm256_set_m128i(bgr_s16_3, bgr_s16_2);

                // 3. 执行 16 位有符号整数加法 (可以处理正负偏移)
                __m256i result_s16_low = _mm256_add_epi16(bgr_s16_low, offset_vec_s16);
                __m256i result_s16_high = _mm256_add_epi16(bgr_s16_high, offset_vec_s16);

                // 4. 将 16 位结果饱和转换回 8 位无符号整数 (uchar)
                //    _mm256_packus_epi16 会将两个 256 位 short 压缩并饱和转换为一个 256 位 uchar
                __m256i result_uchar = _mm256_packus_epi16(result_s16_low, result_s16_high);
                // 注意 packus 可能需要调整顺序或者使用 _mm256_permutevar8x32_epi32 等指令重排

                // 5. 将结果写回内存
                _mm256_storeu_si256(reinterpret_cast<__m256i *>(current_ptr), result_uchar);
            }

            // 处理行末剩余的像素 (使用标量代码)
            for (size_t offset_bytes = num_simd_blocks_per_row * simd_step;
                 offset_bytes < width * channels;
                 offset_bytes++)
            {
                uchar current_val = dst_row_ptr[offset_bytes];
                long long new_val_ll = static_cast<long long>(current_val) + value_offset;
                dst_row_ptr[offset_bytes] = static_cast<uchar>(std::clamp(new_val_ll, 0LL, 255LL));
            }
        }

#else // 如果不支持 AVX2，则回退到普通的 C++ 实现 (或者调用模板版本)
#warning "AVX2 not supported or enabled. Fail to run simd version"
std::cout << "Warning: AVX2 not supported or enabled. Fail to run simd version" << std::endl;
#endif // __AVX2__
        return dst;
    }
} // namespace ILib