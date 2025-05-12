#include "image.h" 
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <chrono>  // For timing

// --- 辅助函数：打印图像信息 ---
template <typename PixelType>
void printImageInfo(const ILib::Image<PixelType> &img, const std::string &name = "Image")
{
    if (img.empty())
    {
        std::cout << name << " is empty." << std::endl;
        return;
    }
    std::cout << name << " Info:" << std::endl;
    std::cout << "  Dimensions: " << img.width() << "x" << img.height() << std::endl;
    std::cout << "  Channels: " << img.getPixelTypeChannels() << std::endl;
    std::cout << "  Color Space: ";
    switch (img.getColorSpace())
    {
    case ILib::ColorSpace::BGR:
        std::cout << "BGR";
        break;
    case ILib::ColorSpace::RGB:
        std::cout << "RGB";
        break;
    case ILib::ColorSpace::GRAYSCALE:
        std::cout << "Grayscale";
        break;
    case ILib::ColorSpace::HSV:
        std::cout << "HSV";
        break;
    default:
        std::cout << "Unknown";
        break;
    }
    std::cout << " (Continuous: " << (img.isContinuous() ? "Yes" : "No") << ")" << std::endl;
}

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input_bmp> [<input_bmp2>]" << std::endl;
        std::cerr << "Example: " << argv[0] << " input.bmp second_input.bmp" << std::endl;
        std::cerr << "Running with default: input.bmp" << std::endl;
    }
    std::string input_filename = (argc > 1) ? argv[1] : "input.bmp";   // 默认 input.bmp
    std::string input_filename2 = (argc > 2) ? argv[2] : "input1.bmp"; // 第二个文件可选
    std::string input_filename3 = (argc > 3) ? argv[3] : "input2.bmp"; // 第三个文件可选
    try
    {
        // --- 1. 加载与基本信息 ---
        std::cout << "\n=== Loading Image ===\n"
                  << std::endl;
        ILib::ImageU8C3 image_orig = ILib::loadImageFromFile(input_filename);
        printImageInfo(image_orig, "Original Image");
        if (image_orig.empty())
            return 1;

        ILib::ImageU8C3 image_2 = ILib::loadImageFromFile(input_filename2);
        printImageInfo(image_2, "Second Image");
        if (image_2.empty())
            return 1;
        
        ILib::ImageU8C3 image_3 = ILib::loadImageFromFile(input_filename3);
        printImageInfo(image_3, "Third Image");
        if (image_3.empty())
            return 1;
        // --- 2. 操作符重载演示 ---
        std::cout << "\n=== Operator Overloading Demo ===\n"
                  << std::endl;

        // a) 亮度增加 (使用 operator+)
        std::cout << "Increasing brightness (+ 50)..." << std::endl;
        ILib::ImageU8C3 image_bright = image_orig + 50; // 使用重载的 +
        ILib::saveImageToFile(image_bright, "output_demo_bright.bmp");
        printImageInfo(image_bright, "Brightened Image");

        // b) 原地亮度减少 (使用 operator+=)
        std::cout << "Decreasing brightness in-place (-= 30)..." << std::endl;
        image_bright += -30; // 原地修改 image_bright
        ILib::saveImageToFile(image_bright, "output_demo_bright_decreased.bmp");
        printImageInfo(image_bright, "Brightness Decreased Image");

        // c) 图像混合 (使用 blend 函数，因为 + 可能有歧义)
        if (!input_filename2.empty())
        {
            std::cout << "Loading second image for blending..." << std::endl;
            ILib::ImageU8C3 image2 = ILib::loadImageFromFile(input_filename2);
            if (!image2.empty())
            {
                printImageInfo(image2, "Second Image");
                std::cout << "Blending original image and second image (alpha=0.7)..." << std::endl;
                // 使用 blend 函数，可以指定混合比例 alpha
                ILib::ImageU8C3 image_blend = ILib::blend(image_orig, image2, 0.7); // 70% 来自 image_orig
                ILib::saveImageToFile(image_blend, "output_demo_blend.bmp");
                printImageInfo(image_blend, "Blended Image");
            }
            else
            {
                std::cerr << "Warning: Failed to load second image " << input_filename2 << std::endl;
            }
        }
        else
        {
            std::cout << "Skipping blend demo (no second input image provided)." << std::endl;
        }

        // --- 3. 内置核效果展示 ---
        std::cout << "\n=== Built-in Kernel Effects ===\n"
                  << std::endl;

        // 转灰度图 
        std::cout << "Converting to grayscale..." << std::endl;
        ILib::ImageU8C1 image_gray = ILib::convertToGrayscale(image_orig);
        ILib::saveImageToFile(image_gray, "output_demo_gray.bmp"); 
        printImageInfo(image_gray, "Grayscale Image");

        if (!image_gray.empty())
        {
            // Sobel 边缘检测
            std::cout << "Applying Sobel edge detection..." << std::endl;
            ILib::ImageU8C1 image_sobel_edge;
            ILib::sobelMagnitude(image_gray, image_sobel_edge, 3, 120.0, true);
            ILib::saveImageToFile(image_sobel_edge, "output_demo_sobel.bmp");
            printImageInfo(image_sobel_edge, "Sobel Edges");
            
        }
        else
        {
            std::cerr << "Warning: Grayscale conversion failed, skipping edge detection." << std::endl;
        }

        // 高斯模糊 
        std::cout << "Applying Gaussian blur (5x5, sigma=1.5)..." << std::endl;
        ILib::ImageU8C3 image_gaussian = ILib::gaussianBlur(image_orig, 5, 1.5);
        ILib::saveImageToFile(image_gaussian, "output_demo_gaussian.bmp");
        printImageInfo(image_gaussian, "Gaussian Blurred");

        // 锐化 
        std::cout << "Applying Sharpen filter..." << std::endl;
        ILib::ImageU8C3 image_sharpen = ILib::sharpen(image_orig);
        ILib::saveImageToFile(image_sharpen, "output_demo_sharpen.bmp");
        printImageInfo(image_sharpen, "Sharpened");

        // 浮雕
        std::cout << "Applying Emboss filter..." << std::endl;
        ILib::ImageU8C3 image_emboss = ILib::emboss(image_orig);
        ILib::saveImageToFile(image_emboss, "output_demo_emboss.bmp");
        printImageInfo(image_emboss, "Embossed");
        

        // --- 4. 自定义核演示 ---
        // a) 创建一个简单的边缘增强核 (类似拉普拉斯的变种)
        ILib::Image<ILib::Pixel<float, 1>> edge_enhance_kernel(3, 3, ILib::ColorSpace::UNKNOWN);
        edge_enhance_kernel.at(0, 0) = {1.0f};
        edge_enhance_kernel.at(0, 1) = {-1.0f};
        edge_enhance_kernel.at(0, 2) = {-0.4f};
        edge_enhance_kernel.at(1, 0) = {-0.5f};
        edge_enhance_kernel.at(1, 1) = {1.4f};
        edge_enhance_kernel.at(1, 2) = {-1.9f}; 
        edge_enhance_kernel.at(2, 0) = {1.9f};
        edge_enhance_kernel.at(2, 1) = {0.8f};
        edge_enhance_kernel.at(2, 2) = {-1.0f};

        std::cout << "Applying custom edge enhancement kernel..." << std::endl;
        ILib::ImageU8C3 image_custom_edge;
        ILib::filter2D(image_orig, image_custom_edge, edge_enhance_kernel);
        ILib::saveImageToFile(image_custom_edge, "output_demo_custom_edge.bmp");
        printImageInfo(image_custom_edge, "Custom Edge Enhanced");

        // b) 创建一个奇怪的核
        ILib::Image<ILib::Pixel<float, 1>> weird_kernel(3, 3, ILib::ColorSpace::UNKNOWN, true); 
        weird_kernel.at(0, 0) = {1.0f};
        weird_kernel.at(0, 1) = {2.0f};
        weird_kernel.at(0, 2) = {1.2f};
        weird_kernel.at(1, 0) = {0.4f};
        weird_kernel.at(1, 1) = {-0.5f};
        weird_kernel.at(1, 2) = {0.5f};
        weird_kernel.at(2, 0) = {-2.0f};
        weird_kernel.at(2, 1) = {0.1f};
        weird_kernel.at(2, 2) = {0.0f};

        std::cout << "Applying weird kernel (top-left impulse)..." << std::endl;
        ILib::ImageU8C3 image_weird;
        // 应用在模糊后的图像上看看效果
        ILib::filter2D(image_gaussian, image_weird, weird_kernel);
        ILib::saveImageToFile(image_weird, "output_demo_weird.bmp");
        printImageInfo(image_weird, "Weird Kernel Result");
        
        // --- 5. ROI 操作展示 ---
        std::cout << "\n=== ROI Demo ===\n"
                  << std::endl;
        if (image_orig.width() >= 100 && image_orig.height() >= 100)
        {
            ILib::ImageU8C3 image_roi_test = image_orig.clone(); // 在副本上操作，不影响原图
            Rect roi_rect = {image_roi_test.width() / 4, image_roi_test.height() / 4,
                             image_roi_test.width() / 2, image_roi_test.height() / 2}; // 中间一半区域

            std::cout << "Applying blur only to the center ROI..." << std::endl;
            // 1. 获取 ROI 视图 (浅拷贝，指向 image_roi_test 的数据)
            ILib::ImageU8C3 roi_view = image_roi_test.roi(roi_rect);
            // ILib::ImageU8C3 roi_view = image_roi_test(roi_rect); // 也可以用 () 操作符

            // 2. 对 ROI 区域应用模糊 (创建模糊核)
            ILib::Image<ILib::Pixel<float, 1>> blur_kernel_roi = ILib::createBoxKernel(7); // 7x7 均值模糊

            // 3. 需要一个临时的、与 ROI 等大的图像来接收 filter2D 的结果
            ILib::ImageU8C3 roi_blurred_temp;

            // 4. 对 ROI 视图应用 filter2D，结果放入临时图像
            ILib::filter2D(roi_view, roi_blurred_temp, blur_kernel_roi);

            // 5. 将模糊后的结果复制回 ROI 区域
            std::cout << "Decreasing brightness (-80) only in the center ROI..." << std::endl;
            roi_view += -80; 

            // 保存修改了 ROI 的图像
            ILib::saveImageToFile(image_roi_test, "output_demo_roi_modified.bmp");
            printImageInfo(image_roi_test, "Image with Modified ROI");
        }
        else
        {
            std::cout << "Skipping ROI demo (image too small)." << std::endl;
        }

        const int num_runs = 5; 
        // --- 测试模板版 adjustBrightness ---
        std::cout << "\n=== Testing Template adjustBrightness Speed ===\n";
        std::vector<long long> template_durations;
        ILib::ImageU8C3 result_template;

        try
        {
            // 预热
            result_template = ILib::adjustBrightness(image_3, 1);
            for (int i = 0; i < num_runs; ++i)
            {
                auto start_time = std::chrono::high_resolution_clock::now();
                result_template = ILib::adjustBrightness(image_3, 50); // 使用实际偏移量
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                template_durations.push_back(duration.count());
            }
            if (!template_durations.empty())
            {
                long long sum = 0;
                for (long long d : template_durations)
                    sum += d;
                double avg_duration = static_cast<double>(sum) / template_durations.size();
                std::cout << "Template adjustBrightness average time (" << num_runs << " runs): "
                          << avg_duration << " ms" << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error during template adjustBrightness test: " << e.what() << std::endl;
        }

        // --- 测试 SIMD 版 adjustBrightness_simd ---
        std::cout << "\n=== Testing SIMD adjustBrightness_simd Speed ===\n";
        std::vector<long long> simd_durations;
        ILib::ImageU8C3 result_simd; 

#ifdef __AVX2__ // 只在支持并启用 AVX2 时测试
        try
        {
            // 预热
            result_simd = ILib::adjustBrightness_simd(image_3, 1);

            for (int i = 0; i < num_runs; ++i)
            {
                auto start_time = std::chrono::high_resolution_clock::now();
                result_simd = ILib::adjustBrightness_simd(image_3, 50); // 使用相同偏移量
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                simd_durations.push_back(duration.count());
            }

            if (!simd_durations.empty())
            {
                long long sum = 0;
                for (long long d : simd_durations)
                    sum += d;
                double avg_duration = static_cast<double>(sum) / simd_durations.size();
                std::cout << "SIMD adjustBrightness_simd average time (" << num_runs << " runs): "
                          << avg_duration << " ms" << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error during SIMD adjustBrightness_simd test: " << e.what() << std::endl;
        }
#else
        std::cout << "SIMD (AVX2) version not compiled/enabled. Skipping test." << std::endl;
#endif // __AVX2__
    }
    catch (const std::exception &e)
    {
        std::cerr << "\n*** An error occurred: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "\nDemo finished successfully!" << std::endl;
    return 0;
}