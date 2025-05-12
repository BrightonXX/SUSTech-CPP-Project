#ifndef SIMPLE_IMAGE_LIB_IMAGE_H // 防止头文件被重复包含的宏
#define SIMPLE_IMAGE_LIB_IMAGE_H

#include <cstddef>
#include <memory>
#include <string>
#include <stdexcept>
#include <vector>
#include <cstring> // For std::memset, std::memcpy
#include <algorithm>
#include <fstream> // For std::ifstream, std::ofstream
#include <cmath>
#include <iostream>         // For std::cout, std::cerr
#include <type_traits>      // For std::is_arithmetic_v, etc.
#include <initializer_list> // 确保 Pixel 的构造函数能用
#ifdef _OPENMP
#include <omp.h> // OpenMP 头文件
#else
#warning "OpenMP not enabled"
#endif

#ifdef __AVX2__ // 仅在支持 AVX2 时编译此部分
#include <immintrin.h> // AVX/AVX2 指令集
#else
#warning "AVX2 not enabled"
#endif

struct Rect
{                  // 用于表示区域，ROI会用到
    size_t x;      // 矩形左上角的 x 坐标
    size_t y;      // 矩形左上角的 y 坐标
    size_t width;  // 矩形的宽度
    size_t height; // 矩形的高度
};

namespace ILib // 命名空间，这里放像素struct
{
    template <typename T, size_t Channels> // 像素向量结构体模板,T代表数据类型（eg,int）
    struct Pixel
    {
    public:
        static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>, "Pixel channel type 'T' must be an arithmetic type or an enum type.");
        static_assert(Channels > 0, "Pixel must have at least one channel");
        T data[Channels]; // 像素数据数组
        Pixel()
        { // 默认构造函数
            for (size_t i = 0; i < Channels; ++i)
                data[i] = T{};
        }
        Pixel(const std::initializer_list<T> &list) // 允许初始化列表构造
        {
            size_t i = 0;
            for (const T &val : list)
            {
                if (i < Channels)
                    data[i++] = val; // 将值赋给数组
                else
                    break; // 超出范围则停止
            }
            for (; i < Channels; ++i)
                data[i] = T{}; // 用默认值填充剩余部分
        }
        T &operator[](size_t i)
        { // 下标操作符重载
            if (i >= Channels)
                throw std::out_of_range("Pixel Channel index out of range");
            return data[i];
        }
        const T &operator[](size_t i) const
        {
            if (i >= Channels)
                throw std::out_of_range("Pixel Channel index out of range");
            return data[i];
        } // （常量版本）
        static constexpr size_t num_channels = Channels;
        using value_type = T;
    };
};

namespace ILib
{ // 这里放描述图像的类
    enum class ColorSpace
    {
        UNKNOWN,
        BGR,
        RGB,
        GRAYSCALE,
        HSV,
        BGRA,
        RGBA,
        BGRF,
        NOTPICTURE
    };
    // 为常用的像素类型定义易懂的别名
    using uchar = unsigned char;
    using GrayscalePixelU8 = Pixel<uchar, 1>;
    using BGRPixelU8 = Pixel<uchar, 3>;
    using RGBPixelU8 = Pixel<uchar, 3>;
    using HSVPixelU8 = Pixel<uchar, 3>;
    using BGRAPixelU8 = Pixel<uchar, 4>;
    using RGBAPixelU8 = Pixel<uchar, 4>;
    using BGRPixelF32 = Pixel<float, 3>; //??
    // 图像容器类

    template <typename PixelType>
    class Image
    {
    private:
        size_t width_ = 0;
        size_t height_ = 0;
        size_t steps_in_bytes_ = 0;

        std::shared_ptr<PixelType[]> data_holder_;     // 智能指针管理 PixelType 数组
        PixelType *p_data_ = nullptr;                  // 指向实际 PixelType 数据块的起始位置
        ColorSpace color_space_ = ColorSpace::UNKNOWN; // 图像的颜色空间标签
        void allocateAndInit(size_t width, size_t height, ColorSpace cs, bool initialize_data_to_default = false)
        {
            release();
            if (width == 0 || height == 0)
            {
                return;
            }
            width_ = width;
            height_ = height;
            color_space_ = cs;
            steps_in_bytes_ = width_ * sizeof(PixelType);
            size_t num_pixels = width_ * height_;
            if (num_pixels == 0)
            {
                width_ = height_ = steps_in_bytes_ = 0;
                color_space_ = ColorSpace::UNKNOWN;
                return;
            }

            try
            {
                data_holder_ = std::shared_ptr<PixelType[]>(new PixelType[num_pixels]);
                p_data_ = data_holder_.get(); 

                // 初始化像素数据，一般不需要，默认为false。
                if (initialize_data_to_default)
                {
                    if constexpr (std::is_arithmetic_v<PixelType> || std::is_enum_v<PixelType>)
                    {
                        for (size_t i = 0; i < num_pixels; ++i)
                        {
                            p_data_[i] = PixelType{};
                        }
                    }
                }
            }
            catch (const std::bad_alloc &)
            {
                release();
                throw std::runtime_error("Image memory allocation failed, try smaller Image or larger memory.");
            }
        } // 初始化像素结束
    public:
        using CurrentPixelType = PixelType;
        Image() = default;
        Image(size_t width, size_t height, ColorSpace cs = ColorSpace::UNKNOWN, bool initialize_data_to_default = false)
        { // 标准构造方法
            allocateAndInit(width, height, cs, initialize_data_to_default);
        }
        Image(Image<PixelType> &parent_image, const Rect &roi)
        { // ROI读取
            if (parent_image.empty())
            {
                throw std::runtime_error("Cannot create ROI from an empty parent image.");
            }
            // 边界检查 ROI 是否在父图像内部
            if (roi.x + roi.width > parent_image.width() ||
                roi.y + roi.height > parent_image.height() ||
                roi.width == 0 || roi.height == 0)
            {
                throw std::out_of_range("ROI dimensions are out of parent image bounds or invalid.");
            }

            width_ = roi.width;
            height_ = roi.height;
            steps_in_bytes_ = parent_image.steps_in_bytes_; // ROI 使用父图像的行字节距
            color_space_ = parent_image.color_space_;       // ROI 通常继承父图像的颜色空间
            data_holder_ = parent_image.data_holder_;       // 共享内存所有权 (浅拷贝)

            size_t pixels_per_parent_row = parent_image.steps_in_bytes_ / sizeof(PixelType);
            p_data_ = parent_image.p_data_ + (roi.y * pixels_per_parent_row) + roi.x;
        }
        // 拷贝构造函数 (浅拷贝)
        Image(const Image<PixelType> &other) = default;
        Image(Image<PixelType> &&other) noexcept = default;
        ~Image() = default;
        
        // -访问函数-
        size_t width() const { return width_; }
        size_t height() const { return height_; }
        ColorSpace getColorSpace() const { return color_space_; }
        bool empty() const { return !p_data_ || width_ == 0 || height_ == 0; }

        size_t steps() const { return steps_in_bytes_; } 
        size_t stepElements() const
        { 
            if (empty() || sizeof(PixelType) == 0)
                return 0;
            return steps_in_bytes_ / sizeof(PixelType);
        }
        bool isContinuous() const
        {
            return !empty() && (steps_in_bytes_ == width_ * sizeof(PixelType));
        }
        size_t totalPixels() const { return empty() ? 0 : width_ * height_; }

        // 获取 PixelType 定义的通道数
        static constexpr int getPixelTypeChannels()
        {
            if constexpr (std::is_arithmetic_v<CurrentPixelType> || std::is_enum_v<CurrentPixelType>)
            {
                return 1;
            }
            else
            {
                return CurrentPixelType::num_channels;
            }
            throw std::runtime_error("Channel is not defined yet.");
        }

        PixelType *data() { return p_data_; } 
        const PixelType *data() const { return p_data_; }

        // --- 像素访问 ---
        PixelType &at(size_t y, size_t x)
        {
            if (y >= height_ || x >= width_)
            {
                throw std::out_of_range("Image::at() - y or x index out of image bounds.");
            }
            return p_data_[y * stepElements() + x];
        }

        const PixelType &at(size_t y, size_t x) const
        {
            if (y >= height_ || x >= width_)
            {
                throw std::out_of_range("Image::at() - y or x index out of image bounds (const).");
            }
            return p_data_[y * stepElements() + x];
        }

        // 获取第 y 行的起始像素指针
        PixelType *ptr(size_t y_row_index)
        {
            if (empty())
                return nullptr;
            if (y_row_index >= height_)
            {
                throw std::out_of_range("Image::ptr() - y_row_index out of bounds.");
            }
            return p_data_ + y_row_index * stepElements();
        }
        const PixelType *ptr(size_t y_row_index) const
        {
            if (empty())
                return nullptr;
            if (y_row_index >= height_)
            {
                throw std::out_of_range("Image::ptr() - y_row_index out of bounds (const).");
            }
            return p_data_ + y_row_index * stepElements();
        }

        void release()
        {
            data_holder_.reset(); // shared_ptr 释放其管理的内存 (如果引用计数为0)
            p_data_ = nullptr;
            width_ = 0;
            height_ = 0;
            steps_in_bytes_ = 0;
            color_space_ = ColorSpace::UNKNOWN;
        }

        void create(size_t width, size_t height, ColorSpace cs = ColorSpace::UNKNOWN, bool initialize_data_to_default = false)
        {
            allocateAndInit(width, height, cs, initialize_data_to_default);
        }

        Image<PixelType> clone() const
        {
            if (empty())
                return Image<PixelType>();
            Image<PixelType> cloned_img(width_, height_, color_space_); // 创建同样尺寸和类型的新图像
            this->copyTo(cloned_img);
            return cloned_img;
        }

        void copyTo(Image<PixelType> &dst) const
        {
            if (empty())
            {
                dst.release();
                return;
            }
            dst.create(width_, height_, color_space_);
            if (isContinuous() && dst.isContinuous())
            {
                std::memcpy(dst.p_data_, p_data_, totalPixels() * sizeof(PixelType));
            }
            else
            {
                size_t effective_bytes_per_row_to_copy = width_ * sizeof(PixelType);
                for (size_t y = 0; y < height_; ++y)
                {
                    std::memcpy(dst.ptr(y), this->ptr(y), effective_bytes_per_row_to_copy);
                }
            }
        }

        void setColorSpace(ColorSpace cs) { color_space_ = cs; }

        // --- ROI ---
        Image<PixelType> roi(const Rect &roi_rect)
        {
            return Image<PixelType>(*this, roi_rect);
        }
        Image<PixelType> operator()(const Rect &roi_rect)
        {
            return Image<PixelType>(*this, roi_rect);
        }

        // --- 操作符重构 ---
        Image<PixelType> &operator=(const Image<PixelType> &other) = default;     // 浅拷贝赋值
        Image<PixelType> &operator=(Image<PixelType> &&other) noexcept = default; // 移动赋值

        Image<PixelType> &operator+=(int value_offset)
        {
            if (empty())
            {
                throw std::runtime_error("operator+=: Cannot adjust brightness of an empty image.");
            }
            using ChannelValueType = typename PixelType::value_type;
            constexpr bool is_single_channel_arithmetic = std::is_arithmetic_v<PixelType> && getPixelTypeChannels() == 1;
            constexpr int Channels = getPixelTypeChannels();

#pragma omp parallel for
            for (size_t y = 0; y < height_; ++y)
            {
                PixelType *row = this->ptr(y);
                for (size_t x = 0; x < width_; ++x)
                {
                    if constexpr (is_single_channel_arithmetic)
                    {
                        ChannelValueType current_val = row[x];
                        long long new_val_ll = static_cast<long long>(current_val) + value_offset;
                        ChannelValueType min_val = std::numeric_limits<ChannelValueType>::lowest();
                        ChannelValueType max_val = std::numeric_limits<ChannelValueType>::max();
                        row[x] = static_cast<ChannelValueType>(
                            std::clamp(new_val_ll, static_cast<long long>(min_val), static_cast<long long>(max_val)));
                    }
                    else
                    {
                        for (int c = 0; c < Channels; ++c)
                        {
                            ChannelValueType current_val = row[x][c];
                            long long new_val_ll = static_cast<long long>(current_val) + value_offset;
                            ChannelValueType min_val = std::numeric_limits<ChannelValueType>::lowest();
                            ChannelValueType max_val = std::numeric_limits<ChannelValueType>::max();
                            row[x][c] = static_cast<ChannelValueType>(
                                std::clamp(new_val_ll, static_cast<long long>(min_val), static_cast<long long>(max_val)));
                        }
                    }
                }
            }
            return *this;
        }
    };
    template <typename PixelType>
    Image<PixelType> operator+(const Image<PixelType> &img1, const Image<PixelType> &img2)
    {
        return blend(img1, img2, 0.5);
    }
    // 这里定义一些常用的图像类型
    using ImageU8C1 = Image<GrayscalePixelU8>;
    using ImageU8C3 = Image<BGRPixelU8>;
    using ImageU8C4 = Image<BGRAPixelU8>;
    using ImageF32C3 = Image<BGRPixelF32>; // 主要用于高精度 BGR, RGB, 或直接存储浮点 HSV
    ImageU8C3 loadImageFromFile(const std::string &filename);
    int saveImageToFile(const ImageU8C3 &img, const std::string &filename);
    int saveImageToFile(const ImageU8C1 &gray_img, const std::string &filename);

}

namespace ILib
{                                 // 这里放图像处理的函数
    template <typename PixelType> // 图像亮度增加，返回一个增加后的图像，而非对原图修改。
    Image<PixelType> adjustBrightness(const Image<PixelType> &src, int value_offset)
    {
        if (src.empty())
        {
            throw std::runtime_error("adjustBrightness: Source image is empty.");
        }

        // 检查 PixelType 是否适合进行亮度调整
        using ChannelValueType = typename PixelType::value_type; // 假设 PixelType 有 value_type
        constexpr bool is_single_channel_arithmetic = std::is_arithmetic_v<PixelType> && Image<PixelType>::getPixelTypeChannels() == 1;

        Image<PixelType> dst = src.clone();
        constexpr int Channels = Image<PixelType>::getPixelTypeChannels();

#pragma omp parallel for
        for (size_t y = 0; y < dst.height(); ++y)
        {
            PixelType *dst_row = dst.ptr(y);
            for (size_t x = 0; x < dst.width(); ++x)
            {
                if constexpr (is_single_channel_arithmetic)
                {
                    ChannelValueType current_val = dst_row[x];
                    long long new_val_ll = static_cast<long long>(current_val) + value_offset;
                    ChannelValueType min_val = std::numeric_limits<ChannelValueType>::lowest();
                    ChannelValueType max_val = std::numeric_limits<ChannelValueType>::max();
                    dst_row[x] = static_cast<ChannelValueType>(
                        std::clamp(new_val_ll, static_cast<long long>(min_val), static_cast<long long>(max_val)));
                }
                else
                {
                    for (int c = 0; c < Channels; ++c)
                    {
                        ChannelValueType current_val = dst_row[x][c]; // 假设 PixelType 有 operator[]
                        long long new_val_ll = static_cast<long long>(current_val) + value_offset;
                        ChannelValueType min_val = std::numeric_limits<ChannelValueType>::lowest();
                        ChannelValueType max_val = std::numeric_limits<ChannelValueType>::max();
                        dst_row[x][c] = static_cast<ChannelValueType>(
                            std::clamp(new_val_ll, static_cast<long long>(min_val), static_cast<long long>(max_val)));
                    }
                }
            }
        }
        dst.setColorSpace(src.getColorSpace()); // 确保颜色空间标签一致
        return dst;
    }

    template <typename PixelType> 
    Image<PixelType> operator+(const Image<PixelType> &img, int offset)
    {
        return adjustBrightness(img, offset);
    }

    template <typename PixelType>
    Image<PixelType> operator+(int offset, const Image<PixelType> &img)
    {
        return adjustBrightness(img, offset);
    }
    ImageU8C3 adjustBrightness_simd(const ImageU8C3& src, int value_offset); // SIMD 版本的亮度调整函数,放在proc实现。

    template <typename PixelType> // 融合两个图像
    Image<PixelType> blend(const Image<PixelType> &img1, const Image<PixelType> &img2, double alpha = 0.5)
    {
        if (img1.empty() || img2.empty())
        {
            throw std::runtime_error("blend: One or both input images are empty.");
        }
        if (img1.width() != img2.width() || img1.height() != img2.height())
        {
            throw std::runtime_error("blend: Input images must have the same dimensions.");
        }
        if (Image<PixelType>::getPixelTypeChannels() != Image<PixelType>::getPixelTypeChannels())
        {
            throw std::runtime_error("blend: Input images must have the same number of channels.");
        }
        if (img1.getColorSpace() != img2.getColorSpace())
        {
            std::cerr << "Warning: Blending images with different color spaces ("
                      << static_cast<int>(img1.getColorSpace()) << " and "
                      << static_cast<int>(img2.getColorSpace()) << ")." << std::endl;
        }

        using ChannelValueType = typename PixelType::value_type;
        constexpr bool is_single_channel_arithmetic = std::is_arithmetic_v<PixelType> && Image<PixelType>::getPixelTypeChannels() == 1;

        Image<PixelType> dst(img1.width(), img1.height(), img1.getColorSpace()); 
        double beta = 1.0 - alpha;
        constexpr int Channels = Image<PixelType>::getPixelTypeChannels();

#pragma omp parallel for
        for (size_t y = 0; y < dst.height(); ++y)
        {
            const PixelType *row1 = img1.ptr(y);
            const PixelType *row2 = img2.ptr(y);
            PixelType *dst_row = dst.ptr(y);
            for (size_t x = 0; x < dst.width(); ++x)
            {
                if constexpr (is_single_channel_arithmetic)
                {
                    ChannelValueType val1 = row1[x];
                    ChannelValueType val2 = row2[x];
                    double blended_val = alpha * static_cast<double>(val1) + beta * static_cast<double>(val2);
                    long long blended_ll = static_cast<long long>(std::round(blended_val));
                    ChannelValueType min_val = std::numeric_limits<ChannelValueType>::lowest();
                    ChannelValueType max_val = std::numeric_limits<ChannelValueType>::max();
                    dst_row[x] = static_cast<ChannelValueType>(
                        std::clamp(blended_ll, static_cast<long long>(min_val), static_cast<long long>(max_val)));
                }
                else
                {
                    for (int c = 0; c < Channels; ++c)
                    {
                        ChannelValueType val1 = row1[x][c]; 
                        ChannelValueType val2 = row2[x][c];
                        double blended_val = alpha * static_cast<double>(val1) + beta * static_cast<double>(val2);
                        long long blended_ll = static_cast<long long>(std::round(blended_val));
                        ChannelValueType min_val = std::numeric_limits<ChannelValueType>::lowest();
                        ChannelValueType max_val = std::numeric_limits<ChannelValueType>::max();
                        dst_row[x][c] = static_cast<ChannelValueType>(
                            std::clamp(blended_ll, static_cast<long long>(min_val), static_cast<long long>(max_val)));
                    }
                }
            }
        }
        return dst;
    }

    // --- 通用二维卷积函数 ---
    // InputPixelType: 输入图像像素类型
    // KernelPixelType: 卷积核像素类型
    // OutputPixelType: 输出图像像素类型

    template <typename InputPixelType, typename KernelPixelType, typename OutputPixelType>
    void filter2D(
        const Image<InputPixelType> &src,     // 输入图像
        Image<OutputPixelType> &dst,          // 输出图像 (引用，会被修改)
        const Image<KernelPixelType> &kernel, // 卷积核
        double delta = 0.0                    // 可选的添加到最终卷积结果的值
    )
    {
        //  输入有效性检查
        if (src.empty())
        {
            throw std::runtime_error("filter2D: Source image is empty.");
        }
        if (kernel.empty())
        {
            throw std::runtime_error("filter2D: Kernel image is empty.");
        }
        if (kernel.width() == 0 || kernel.height() == 0)
        {
            throw std::runtime_error("filter2D: Kernel dimensions cannot be zero.");
        }

        // 准备输出图像 
        dst.create(src.width(), src.height(), src.getColorSpace());
        if (dst.empty())
        { 
            throw std::runtime_error("filter2D: Failed to create destination image.");
        }

        // 获取维度和通道信息
        const size_t src_width = src.width();
        const size_t src_height = src.height();
        const size_t kernel_width = kernel.width();
        const size_t kernel_height = kernel.height();

        // 使用静态方法获取通道数
        constexpr int src_channels = Image<InputPixelType>::getPixelTypeChannels();
        constexpr int kernel_channels = Image<KernelPixelType>::getPixelTypeChannels();
        constexpr int dst_channels = Image<OutputPixelType>::getPixelTypeChannels();

        // 检查通道数配置 
        static_assert(kernel_channels > 0, "filter2D: Kernel must have at least one channel.");
        static_assert(src_channels > 0, "filter2D: Source image must have at least one channel.");
        static_assert(dst_channels > 0, "filter2D: Destination image must have at least one channel.");

        static_assert(kernel_channels == 1 || kernel_channels == src_channels,
                      "filter2D: Kernel must have 1 channel, or the same number of channels as source.");
        static_assert(kernel_channels == 1 ? (src_channels == dst_channels) : true,
                      "filter2D: If kernel is single-channel, source and destination must have same number of channels.");
        static_assert(kernel_channels > 1 ? (src_channels == dst_channels && kernel_channels == src_channels) : true,
                      "filter2D: If kernel is multi-channel, source, kernel, and destination must all have the same number of channels.");

        // 确定累加和输出通道类型
        using AccumulatorType = double;
        using OutputChannelType = typename OutputPixelType::value_type; 
        static_assert(std::is_arithmetic_v<OutputChannelType>,
                      "filter2D: Output pixel's channel type must be arithmetic.");
        using KernelChannelType = typename KernelPixelType::value_type;
        static_assert(std::is_arithmetic_v<KernelChannelType>,
                      "filter2D: Kernel pixel's channel type must be arithmetic.");
        using InputChannelType = typename InputPixelType::value_type;
        static_assert(std::is_arithmetic_v<InputChannelType>,
                      "filter2D: Input pixel's channel type must be arithmetic.");

        // 计算中心锚点偏移
        const long long anchor_offset_x = static_cast<long long>(kernel.width() / 2);
        const long long anchor_offset_y = static_cast<long long>(kernel.height() / 2);

        // 主卷积循环 (OpenMP 并行化) 
#pragma omp parallel for schedule(static) 
        for (size_t y = 0; y < src_height; ++y)
        {
            OutputPixelType *dst_row = dst.ptr(y); 

            for (size_t x = 0; x < src_width; ++x)
            {
                std::vector<AccumulatorType> accumulators(dst_channels, 0.0);

                for (size_t ky = 0; ky < kernel_height; ++ky)
                {
                    const KernelPixelType *kernel_row = kernel.ptr(ky); // 假设 ptr 内部有检查

                    for (size_t kx = 0; kx < kernel_width; ++kx)
                    {
                        
                        const long long ix_ll = static_cast<long long>(x) + static_cast<long long>(kx) - anchor_offset_x;
                        const long long iy_ll = static_cast<long long>(y) + static_cast<long long>(ky) - anchor_offset_y;

                        const size_t ix = static_cast<size_t>(std::clamp(ix_ll, 0LL, static_cast<long long>(src_width) - 1));
                        const size_t iy = static_cast<size_t>(std::clamp(iy_ll, 0LL, static_cast<long long>(src_height) - 1));

                        const InputPixelType &src_pixel = src.at(iy, ix);
                        const KernelPixelType &kernel_pixel = kernel_row[kx]; // 假设 ptr 返回有效指针

                        if constexpr (kernel_channels == 1)
                        {
                            constexpr bool kernel_is_single_arithmetic =
                                std::is_arithmetic_v<KernelPixelType>; // 对于单通道，类型本身就是通道类型

                            constexpr bool kernel_is_our_pixel_struct = !kernel_is_single_arithmetic;
                            AccumulatorType kernel_coeff = 0.0; // 初始化

                            if constexpr (kernel_is_single_arithmetic)
                            {
                                kernel_coeff = static_cast<AccumulatorType>(kernel_pixel);
                            }
                            else if constexpr (kernel_is_our_pixel_struct)
                            {
                                using KernelChannelType = typename KernelPixelType::value_type;
                                kernel_coeff = static_cast<AccumulatorType>(kernel_pixel[0]); // kernel_pixel 是 const Pixel<T,1>&
                            }
                            else
                            {
                                static_assert(kernel_is_single_arithmetic || kernel_is_our_pixel_struct,
                                              "Internal error: KernelPixelType is neither single arithmetic nor assumed Pixel<T,1>");
                            }

                            constexpr bool src_is_single_arithmetic =
                                std::is_arithmetic_v<InputPixelType> && (src_channels == 1);   
                            constexpr bool src_is_our_pixel_struct = !src_is_single_arithmetic; 

                            if constexpr (src_is_single_arithmetic)
                            {
                                accumulators[0] += static_cast<AccumulatorType>(src_pixel) * kernel_coeff;
                            }
                            else if constexpr (src_is_our_pixel_struct)
                            {
                                // 源是多通道 Pixel<T,N>
                                for (int c = 0; c < src_channels; ++c)
                                {
                                    accumulators[c] += static_cast<AccumulatorType>(src_pixel[c]) * kernel_coeff;
                                }
                            }
                            else
                            {
                                // 不支持的 InputPixelType
                                static_assert(src_is_single_arithmetic || src_is_our_pixel_struct,
                                              "Internal error: InputPixelType is invalid for single channel kernel.");
                            }
                        }
                        else
                        { 
                            // 多通道核，与输入像素逐通道相乘累加
                            for (int c = 0; c < src_channels; ++c)
                            {
                                accumulators[c] += static_cast<AccumulatorType>(src_pixel[c]) * static_cast<AccumulatorType>(kernel_pixel[c]);
                            }
                        }
                    } // end kx loop
                } // end ky loop

                // 写入输出像素
                OutputPixelType &dst_pixel = dst_row[x];
                for (int c = 0; c < dst_channels; ++c)
                {
                    AccumulatorType final_val = accumulators[c] + delta;

                    OutputChannelType min_val = std::numeric_limits<OutputChannelType>::lowest();
                    OutputChannelType max_val = std::numeric_limits<OutputChannelType>::max();

                    if constexpr (std::is_floating_point_v<OutputChannelType>)
                    {
                        dst_pixel[c] = std::clamp(static_cast<OutputChannelType>(final_val), min_val, max_val);
                    }
                    else
                    {
                        long long final_val_ll = static_cast<long long>(std::round(final_val));
                        dst_pixel[c] = static_cast<OutputChannelType>(
                            std::clamp(final_val_ll, static_cast<long long>(min_val), static_cast<long long>(max_val)));
                    }
                }
            } // end x loop
        } // end y loop (omp parallel for)
    } // end filter2D

    // --- 创建常用卷积核的函数声明 --- (实现在proc)
    Image<Pixel<int, 1>> createSobelKernelX(int ksize = 3);
    Image<Pixel<int, 1>> createSobelKernelY(int ksize = 3);
    Image<Pixel<float, 1>> createBoxKernel(int ksize);
    Image<Pixel<float, 1>> createGaussianKernel(int ksize, double sigma);
    Image<Pixel<int, 1>> createLaplacianKernel(int ksize = 1);
    Image<Pixel<int, 1>> createSharpenKernel();
    Image<Pixel<int, 1>> createEmbossKernel();

    // --- 便捷的图像处理函数 (调用 filter2D) ---

    // 均值模糊 (Box Blur)
    // PixelType: 输入/输出图像的像素类型
    // ksize: 模糊核的大小 (必须是正奇数)
    template <typename PixelType>
    Image<PixelType> boxBlur(const Image<PixelType> &src, int ksize)
    {
        if (src.empty())
            return Image<PixelType>(); 
        Image<Pixel<float, 1>> kernel = createBoxKernel(ksize);
        Image<PixelType> dst;
        filter2D(src, dst, kernel);
        return dst;
    }

    // 高斯模糊
    // PixelType: 输入/输出图像的像素类型
    // ksize: 高斯核大小 (必须是正奇数)
    // sigma: 高斯标准差
    template <typename PixelType>
    Image<PixelType> gaussianBlur(const Image<PixelType> &src, int ksize, double sigma)
    {
        if (src.empty())
            return Image<PixelType>();
        Image<Pixel<float, 1>> kernel = createGaussianKernel(ksize, sigma);
        Image<PixelType> dst;
        filter2D(src, dst, kernel);
        return dst;
    }

    // 锐化 (通常输入输出类型相同)
    template <typename PixelType>
    Image<PixelType> sharpen(const Image<PixelType> &src)
    {
        if (src.empty())
            return Image<PixelType>();
        Image<Pixel<int, 1>> kernel_i = createSharpenKernel();
        Image<Pixel<float, 1>> kernel_f(kernel_i.width(), kernel_i.height());
        for (size_t y = 0; y < kernel_i.height(); ++y)
        {
            for (size_t x = 0; x < kernel_i.width(); ++x)
            {
                kernel_f.at(y, x)[0] = static_cast<float>(kernel_i.at(y, x)[0]);
            }
        }
        Image<PixelType> dst;
        filter2D(src, dst, kernel_f); // 使用 float 核进行卷积
        return dst;
    }

    // 浮雕
    template <typename PixelType>
    Image<PixelType> emboss(const Image<PixelType> &src)
    {
        if (src.empty())
            return Image<PixelType>();
        Image<Pixel<int, 1>> kernel_i = createEmbossKernel();
        Image<Pixel<float, 1>> kernel_f(kernel_i.width(), kernel_i.height());
        for (size_t y = 0; y < kernel_i.height(); ++y)
        {
            for (size_t x = 0; x < kernel_i.width(); ++x)
            {
                kernel_f.at(y, x)[0] = static_cast<float>(kernel_i.at(y, x)[0]);
            }
        }
        Image<PixelType> dst;
        filter2D(src, dst, kernel_f, 128.0);
        return dst;
    }

    // Sobel 和 Laplacian (针对单通道灰度图操作)
    // 声明放在这里，实现放在 image_proc.cpp

    // 计算 X 和 Y 方向梯度，输出为 short 类型以存储负值和更大范围
    void sobelGradients(const ImageU8C1 &src, Image<Pixel<short, 1>> &dst_dx, Image<Pixel<short, 1>> &dst_dy, int ksize = 3);

    // 计算 Sobel 梯度幅度并应用阈值，生成二值边缘图 (uchar)
    // threshold: 幅度阈值
    // use_l1_norm: true 使用 |dx|+|dy| (更快)，false 使用 sqrt(dx^2+dy^2)
    void sobelMagnitude(const ImageU8C1 &src, ImageU8C1 &dst_edge, int ksize = 3, double threshold = 100.0, bool use_l1_norm = true);

    // 计算拉普拉斯算子，输出为 short 类型
    void laplacian(const ImageU8C1 &src, Image<Pixel<short, 1>> &dst, int ksize = 1);

    ImageU8C1 convertToGrayscale(const ImageU8C3 &srcImage);

} // namespace ILib

#endif // SIMPLE_IMAGE_LIB_IMAGE_H，回应第一行的if。