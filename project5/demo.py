import cv2
import numpy as np
import time
import os

BRIGHTNESS_OFFSET = 50
GAUSSIAN_KSIZE = 5
GAUSSIAN_SIGMA = 1.5
SOBEL_KSIZE = 3
SOBEL_THRESHOLD = 100.0
SOBEL_MAGNITUDE_APPROX_L1_LIKE = True


def measure_time(func, *args, num_runs=5, warmup_runs=1):
    # 预热
    for _ in range(warmup_runs):
        result = func(*args)

    times = []
    final_result = None
    for _ in range(num_runs):
        start_time = time.perf_counter()
        result = func(*args)
        end_time = time.perf_counter()
        times.append(end_time - start_time)
        final_result = result

    if not times:
        return 0.0, None
    return sum(times) / len(times), final_result

def process_brightness(image_path, offset, output_path=None):
    img = cv2.imread(image_path)
    if img is None:
        print(f"Error: Could not read image {image_path}")
        return None
    bright_img = cv2.convertScaleAbs(img, alpha=1.0, beta=float(offset))

    if output_path:
        cv2.imwrite(output_path, bright_img)
    return bright_img

def process_gaussian_blur(image_path, ksize, sigma, output_path=None):

    img = cv2.imread(image_path)
    if img is None:
        print(f"Error: Could not read image {image_path}")
        return None

    blurred_img = cv2.GaussianBlur(img, (ksize, ksize), sigmaX=sigma, sigmaY=sigma)

    if output_path:
        cv2.imwrite(output_path, blurred_img)
    return blurred_img

def process_sobel_edges(image_path, ksize, threshold_val, output_path=None):

    img = cv2.imread(image_path)
    if img is None:
        print(f"Error: Could not read image {image_path}")
        return None

    gray_img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    grad_x = cv2.Sobel(gray_img, cv2.CV_16S, 1, 0, ksize=ksize)
    grad_y = cv2.Sobel(gray_img, cv2.CV_16S, 0, 1, ksize=ksize)

    if SOBEL_MAGNITUDE_APPROX_L1_LIKE:
        abs_grad_x = cv2.convertScaleAbs(grad_x)
        abs_grad_y = cv2.convertScaleAbs(grad_y)
        grad_magnitude = cv2.addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0)
    else:
        grad_x_f = np.float32(grad_x)
        grad_y_f = np.float32(grad_y)
        grad_magnitude_f = cv2.magnitude(grad_x_f, grad_y_f)
        grad_magnitude = cv2.convertScaleAbs(grad_magnitude_f)


    _, edge_img = cv2.threshold(grad_magnitude, threshold_val, 255, cv2.THRESH_BINARY)
    if output_path:
        cv2.imwrite(output_path, edge_img)
    return edge_img

if __name__ == "__main__":
    output_dir_python = "output_python"
    os.makedirs(output_dir_python, exist_ok=True)

    image_s_path = r"C:\Users\Brigh\Desktop\works\CPP\project5\latex\pics\Image_S.bmp"
    image_m_path = r"C:\Users\Brigh\Desktop\works\CPP\project5\latex\pics\Image_M.bmp"
    image_l_path = r"C:\Users\Brigh\Desktop\works\CPP\project5\latex\pics\Image_L.bmp"

    if not os.path.exists(image_s_path):
        print(f"Error: Small test image not found at {image_s_path}. Please create it.")

    test_images = {
        "Small (S)": image_s_path,
        "Medium (M)": image_m_path,
        "Large (L)": image_l_path,
    }

    num_runs_perf = 5
    warmup_runs_perf = 2

    print(f"--- Python (OpenCV) Performance Tests ---")
    print(f"Number of runs for timing: {num_runs_perf} (after {warmup_runs_perf} warmup runs)\n")

    results_summary = {}

    for img_name, img_path in test_images.items():
        if not os.path.exists(img_path):
            print(f"Skipping {img_name} ({img_path}): File not found.")
            continue

        print(f"--- Processing Image: {img_name} ({img_path}) ---")
        results_summary[img_name] = {}

        # 1. 亮度调整
        output_brightness_path = os.path.join(output_dir_python, f"{img_name.split(' ')[0].lower()}_brightness.bmp")
        avg_time_brightness, _ = measure_time(
            process_brightness,
            img_path, BRIGHTNESS_OFFSET, output_brightness_path,
            num_runs=num_runs_perf, warmup_runs=warmup_runs_perf
        )
        print(f"  Brightness Adjustment: Avg Time = {avg_time_brightness*1000:.2f} ms")
        results_summary[img_name]["Brightness"] = avg_time_brightness * 1000

        # 2. 高斯模糊
        output_gaussian_path = os.path.join(output_dir_python, f"{img_name.split(' ')[0].lower()}_gaussian.bmp")
        avg_time_gaussian, _ = measure_time(
            process_gaussian_blur,
            img_path, GAUSSIAN_KSIZE, GAUSSIAN_SIGMA, output_gaussian_path,
            num_runs=num_runs_perf, warmup_runs=warmup_runs_perf
        )
        print(f"  Gaussian Blur: Avg Time         = {avg_time_gaussian*1000:.2f} ms")
        results_summary[img_name]["Gaussian Blur"] = avg_time_gaussian * 1000

        # 3. Sobel 边缘检测
        output_sobel_path = os.path.join(output_dir_python, f"{img_name.split(' ')[0].lower()}_sobel.bmp")
        avg_time_sobel, _ = measure_time(
            process_sobel_edges,
            img_path, SOBEL_KSIZE, SOBEL_THRESHOLD, output_sobel_path,
            num_runs=num_runs_perf, warmup_runs=warmup_runs_perf
        )
        print(f"  Sobel Edge Detection: Avg Time  = {avg_time_sobel*1000:.2f} ms")
        results_summary[img_name]["Sobel Edges"] = avg_time_sobel * 1000
        print("-" * 30)

    print("\n--- Summary of Python (OpenCV) Timings (ms) ---")
    header = "| Image Size | Brightness | Gaussian Blur | Sobel Edges |"
    print(header)
    print("|" + "-" * (len(header)-2) + "|")
    for img_name, timings in results_summary.items():
        row = f"| {img_name:<10} | "
        row += f"{timings.get('Brightness', -1):<10.2f} | "
        row += f"{timings.get('Gaussian Blur', -1):<13.2f} | "
        row += f"{timings.get('Sobel Edges', -1):<11.2f} |"
        print(row)