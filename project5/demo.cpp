#include "image.h" 
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <numeric>
#include <filesystem>
#include <iomanip>
#include <map>

const int BRIGHTNESS_OFFSET_VAL = 50;
const int GAUSSIAN_KSIZE_VAL = 5;
const double GAUSSIAN_SIGMA_VAL = 1.5;
const int SOBEL_KSIZE_VAL = 3;
const double SOBEL_THRESHOLD_VAL = 100.0;
const bool SOBEL_USE_L1_NORM_VAL = true;

const int NUM_RUNS_PERF = 5;
const int WARMUP_RUNS_PERF = 2;

struct ImageTimings {
    double brightness_normal_ms = -1.0;
    double brightness_simd_ms = -1.0;
    double gaussian_ms = -1.0;
    double sobel_ms = -1.0;
};

int main() {
    std::vector<std::pair<std::string, std::string>> image_files_to_test = {
        {"Small (S)", "/mnt/c/Users/Brigh/Desktop/works/CPP/project5/latex/pics/Image_S.bmp"},
        {"Medium (M)", "/mnt/c/Users/Brigh/Desktop/works/CPP/project5/latex/pics/Image_M.bmp"},
        {"Large (L)", "/mnt/c/Users/Brigh/Desktop/works/CPP/project5/latex/pics/Image_L.bmp"}
    };

    std::string output_dir_cpp = "output_cpp";
    try {
        if (!std::filesystem::exists(output_dir_cpp)) {
            std::filesystem::create_directories(output_dir_cpp);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error creating directory " << output_dir_cpp << ": " << e.what() << std::endl;
        return 1;
    }

    std::cout << std::fixed << std::setprecision(2);
    std::map<std::string, ImageTimings> all_results;

    for (const auto& image_entry : image_files_to_test) {
        const std::string& image_name_full = image_entry.first;
        const std::string& image_path = image_entry.second;
        std::string image_name_short = "unknown";
        if (image_name_full.find("Small (S)") != std::string::npos) image_name_short = "s";
        else if (image_name_full.find("Medium (M)") != std::string::npos) image_name_short = "m";
        else if (image_name_full.find("Large (L)") != std::string::npos) image_name_short = "l";

        std::cout << "--- Processing Image: " << image_name_full << " (" << image_path << ") ---" << std::endl;
        ImageTimings current_image_timings;

        ILib::ImageU8C3 original_bgr_img;
        try {
            original_bgr_img = ILib::loadImageFromFile(image_path);
        } catch (const std::exception& e) {
            std::cerr << "Error loading image " << image_path << ": " << e.what() << std::endl;
            all_results[image_name_full] = current_image_timings; // Store default/error values
            continue;
        }
        if (original_bgr_img.empty()) {
            std::cerr << "Error: Loaded image is empty " << image_path << std::endl;
            all_results[image_name_full] = current_image_timings;
            continue;
        }

        std::vector<long long> durations;
        ILib::ImageU8C3 result_img_bgr; 
        ILib::ImageU8C1 result_img_gray; 

        //  Brightness Adjustment (Normal) 
        durations.clear();
        for (int i = 0; i < WARMUP_RUNS_PERF; ++i) {
           [[maybe_unused]] volatile ILib::ImageU8C3 temp_res = ILib::adjustBrightness(original_bgr_img, BRIGHTNESS_OFFSET_VAL);
        }
        for (int i = 0; i < NUM_RUNS_PERF; ++i) {
            ILib::ImageU8C3 temp_src = original_bgr_img.clone();
            auto start = std::chrono::high_resolution_clock::now();
            result_img_bgr = ILib::adjustBrightness(temp_src, BRIGHTNESS_OFFSET_VAL);
            auto end = std::chrono::high_resolution_clock::now();
            durations.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }
        if (!durations.empty()) {
            long long sum_d = std::accumulate(durations.begin(), durations.end(), 0LL);
            current_image_timings.brightness_normal_ms = static_cast<double>(sum_d) / durations.size() / 1000.0;
            if (!result_img_bgr.empty()) ILib::saveImageToFile(result_img_bgr, output_dir_cpp + "/" + image_name_short + "_brightness_normal.bmp");
        }
        std::cout << "  Brightness Adjustment (Normal): Avg Time = " << current_image_timings.brightness_normal_ms << " ms" << std::endl;

        //  Brightness Adjustment (SIMD) 
#ifdef __AVX2__
        durations.clear();
        for (int i = 0; i < WARMUP_RUNS_PERF; ++i) {
            [[maybe_unused]] volatile ILib::ImageU8C3 temp_res = ILib::adjustBrightness_simd(original_bgr_img, BRIGHTNESS_OFFSET_VAL);
        }
        for (int i = 0; i < NUM_RUNS_PERF; ++i) {
            ILib::ImageU8C3 temp_src = original_bgr_img.clone();
            auto start = std::chrono::high_resolution_clock::now();
            result_img_bgr = ILib::adjustBrightness_simd(temp_src, BRIGHTNESS_OFFSET_VAL);
            auto end = std::chrono::high_resolution_clock::now();
            durations.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }
        if (!durations.empty()) {
            long long sum_d = std::accumulate(durations.begin(), durations.end(), 0LL);
            current_image_timings.brightness_simd_ms = static_cast<double>(sum_d) / durations.size() / 1000.0;
            if (!result_img_bgr.empty()) ILib::saveImageToFile(result_img_bgr, output_dir_cpp + "/" + image_name_short + "_brightness_simd.bmp");
        }
        std::cout << "  Brightness Adjustment (SIMD)  : Avg Time = " << current_image_timings.brightness_simd_ms << " ms" << std::endl;
#else
        std::cout << "  Brightness Adjustment (SIMD)  : Skipped (AVX2 not enabled/compiled)" << std::endl;
        current_image_timings.brightness_simd_ms = -1.0; // Indicate skipped
#endif

        // Gaussian Blur
        durations.clear();
        for (int i = 0; i < WARMUP_RUNS_PERF; ++i) {
            [[maybe_unused]] volatile ILib::ImageU8C3 temp_res = ILib::gaussianBlur(original_bgr_img, GAUSSIAN_KSIZE_VAL, GAUSSIAN_SIGMA_VAL);
        }
        for (int i = 0; i < NUM_RUNS_PERF; ++i) {
            ILib::ImageU8C3 temp_src = original_bgr_img.clone();
            auto start = std::chrono::high_resolution_clock::now();
            result_img_bgr = ILib::gaussianBlur(temp_src, GAUSSIAN_KSIZE_VAL, GAUSSIAN_SIGMA_VAL);
            auto end = std::chrono::high_resolution_clock::now();
            durations.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }
        if (!durations.empty()) {
            long long sum_d = std::accumulate(durations.begin(), durations.end(), 0LL);
            current_image_timings.gaussian_ms = static_cast<double>(sum_d) / durations.size() / 1000.0;
            if (!result_img_bgr.empty()) ILib::saveImageToFile(result_img_bgr, output_dir_cpp + "/" + image_name_short + "_gaussian.bmp");
        }
        std::cout << "  Gaussian Blur                 : Avg Time = " << current_image_timings.gaussian_ms << " ms" << std::endl;

        // Sobel Edge Detection 
        durations.clear();
        ILib::ImageU8C1 temp_gray_for_sobel_warmup;
        for (int i = 0; i < WARMUP_RUNS_PERF; ++i) {
            temp_gray_for_sobel_warmup = ILib::convertToGrayscale(original_bgr_img);
            ILib::sobelMagnitude(temp_gray_for_sobel_warmup, result_img_gray, SOBEL_KSIZE_VAL, SOBEL_THRESHOLD_VAL, SOBEL_USE_L1_NORM_VAL);
        }
        for (int i = 0; i < NUM_RUNS_PERF; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            ILib::ImageU8C1 temp_gray = ILib::convertToGrayscale(original_bgr_img); // Convert fresh each time
            ILib::sobelMagnitude(temp_gray, result_img_gray, SOBEL_KSIZE_VAL, SOBEL_THRESHOLD_VAL, SOBEL_USE_L1_NORM_VAL);
            auto end = std::chrono::high_resolution_clock::now();
            durations.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }
        if (!durations.empty()) {
            long long sum_d = std::accumulate(durations.begin(), durations.end(), 0LL);
            current_image_timings.sobel_ms = static_cast<double>(sum_d) / durations.size() / 1000.0;
            if (!result_img_gray.empty()) ILib::saveImageToFile(result_img_gray, output_dir_cpp + "/" + image_name_short + "_sobel_edge.bmp");
        }
        std::cout << "  Sobel Edge Detection        : Avg Time = " << current_image_timings.sobel_ms << " ms" << std::endl;

        all_results[image_name_full] = current_image_timings;
        std::cout << "-----------------------------------------" << std::endl;
    }

    std::cout << "\n--- Summary of C/C++ Timings (ms) ---" << std::endl;
    printf("| %-12s | %-12s | %-12s | %-13s | %-11s |\n", "Image Size", "Bright Norm", "Bright SIMD", "Gaussian Blur", "Sobel Edges");
    printf("|%s|\n", std::string(74, '-').c_str()); // Adjust length based on columns

    for (const auto& image_entry : image_files_to_test) {
        const std::string& image_name_full = image_entry.first;
        if (all_results.count(image_name_full)) { // Check if results exist (e.g. image loaded成功)
            const auto& timings = all_results.at(image_name_full);
            printf("| %-12s | %-12.2f | %-12.2f | %-13.2f | %-11.2f |\n",
                   image_name_full.c_str(),
                   timings.brightness_normal_ms,
                   timings.brightness_simd_ms,
                   timings.gaussian_ms,
                   timings.sobel_ms);
        }
    }
    printf("%s\n", std::string(74, '-').c_str());
    return 0;
}