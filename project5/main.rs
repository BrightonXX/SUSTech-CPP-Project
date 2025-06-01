use image::{
    DynamicImage,
    ImageBuffer,
    Luma,
    RgbImage, 
};
use image::io::Reader as ImageReader;
use imageproc::{
    filter,    
    gradients, 
};
use std::time::{Duration, Instant};
use std::path::Path;
use std::fs;

type GrayImage = ImageBuffer<Luma<u8>, Vec<u8>>;

const BRIGHTNESS_OFFSET: i16 = 50;
const GAUSSIAN_SIGMA: f32 = 1.5;
const SOBEL_THRESHOLD: f64 = 100.0;
const SOBEL_MAGNITUDE_L1_NORM: bool = true;

fn process_brightness(
    img_dyn: &DynamicImage,
    offset: i16,
) -> Result<RgbImage, image::ImageError> {
    let mut bright_img_buffer = img_dyn.to_rgb8();
   
    bright_img_buffer
        .pixels_mut()
        .for_each(|pixel_out| {
            // Rgb<u8> is [u8; 3]
            for i in 0..3 {
                let val = pixel_out[i] as i16 + offset;
                pixel_out[i] = val.clamp(0, 255) as u8;
            }
        });
    Ok(bright_img_buffer)
}

fn process_gaussian_blur(
    img_dyn: &DynamicImage,
    sigma: f32,
) -> Result<RgbImage, image::ImageError> {
    let img_rgb = img_dyn.to_rgb8(); // Convert to Rgb<u8>
    let blurred_img_u8 = filter::gaussian_blur_f32(&img_rgb, sigma);
    Ok(blurred_img_u8)
}

fn process_sobel_edges(
    img_dyn: &DynamicImage,
    threshold_val: f64,
) -> Result<GrayImage, image::ImageError> {
    let gray_img = img_dyn.to_luma8();
    let grad_x = gradients::horizontal_sobel(&gray_img); // Returns ImageBuffer<Luma<i16>, _>
    let grad_y = gradients::vertical_sobel(&gray_img); // Returns ImageBuffer<Luma<i16>, _>

    let (width, height) = gray_img.dimensions();
    let mut edge_img_buffer: GrayImage = ImageBuffer::new(width, height);

    edge_img_buffer
        .enumerate_pixels_mut()
        .for_each(|(x, y, pixel_out)| {
            let gx = grad_x.get_pixel(x, y)[0] as f64;
            let gy = grad_y.get_pixel(x, y)[0] as f64;

            let magnitude = if SOBEL_MAGNITUDE_L1_NORM {
                gx.abs() + gy.abs()
            } else {
                (gx.powi(2) + gy.powi(2)).sqrt()
            };

            if magnitude > threshold_val {
                *pixel_out = Luma([255u8]);
            } else {
                *pixel_out = Luma([0u8]);
            }
        });
    Ok(edge_img_buffer)
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let output_dir_rust = "output_rust";
    fs::create_dir_all(output_dir_rust)?;

    let image_s_path_str = r"C:\Users\Brigh\Desktop\works\CPP\project5\latex\pics\Image_S.bmp";
    let image_m_path_str = r"C:\Users\Brigh\Desktop\works\CPP\project5\latex\pics\Image_M.bmp";
    let image_l_path_str = r"C:\Users\Brigh\Desktop\works\CPP\project5\latex\pics\Image_L.bmp";

    let test_images = vec![
        ("Small (S)", Path::new(image_s_path_str)),
        ("Medium (M)", Path::new(image_m_path_str)),
        ("Large (L)", Path::new(image_l_path_str)),
    ];

    let num_runs_perf = 5;
    let warmup_runs_perf = 2;

    println!("--- Rust (image/imageproc/rayon) Performance Tests ---");
    println!("Brightness Offset: {}", BRIGHTNESS_OFFSET);
    println!("Gaussian sigma: {}", GAUSSIAN_SIGMA);
    println!("Sobel ksize: 3 (imageproc default), threshold: {}", SOBEL_THRESHOLD);
    println!("Number of runs for timing: {} (after {} warmup runs)\n", num_runs_perf, warmup_runs_perf);

    let mut results_summary: std::collections::HashMap<String, std::collections::HashMap<String, f64>> = std::collections::HashMap::new();

    for (img_name_str, img_path) in test_images {
        let img_name = img_name_str.to_string();
        if !img_path.exists() {
            println!("Skipping {} ({}): File not found.", img_name, img_path.display());
            continue;
        }

        println!("--- Processing Image: {} ({}) ---", img_name, img_path.display());
        results_summary.entry(img_name.clone()).or_insert_with(std::collections::HashMap::new);

        let original_image_dyn = ImageReader::open(img_path)?.decode()?;

        // 1. 亮度调整
        let mut total_duration_brightness = Duration::new(0, 0);
        let mut result_brightness_dyn: Option<DynamicImage> = None;
        for r_idx in 0..(warmup_runs_perf + num_runs_perf) {
            let img_copy = original_image_dyn.clone();
            let start_time = Instant::now();
            let processed_rgb_img = process_brightness(&img_copy, BRIGHTNESS_OFFSET)?;
            let duration = start_time.elapsed();
            if r_idx >= warmup_runs_perf {
                total_duration_brightness += duration;
            }
            if r_idx == (warmup_runs_perf + num_runs_perf - 1) {
                result_brightness_dyn = Some(DynamicImage::ImageRgb8(processed_rgb_img));
            }
        }
        let avg_time_brightness_ms = (total_duration_brightness.as_secs_f64() / num_runs_perf as f64) * 1000.0;
        if let Some(img_to_save) = result_brightness_dyn {
            let output_path_str = format!("{}/{}_brightness_core.bmp", output_dir_rust, img_name.split(' ').next().unwrap_or("unknown").to_lowercase());
            img_to_save.save(output_path_str)?;
        }
        println!("  Brightness Adjustment (core): Avg Time = {:.2} ms", avg_time_brightness_ms);
        results_summary.get_mut(&img_name).unwrap().insert("Brightness".to_string(), avg_time_brightness_ms);

        // 2. 高斯模糊
        let mut total_duration_gaussian = Duration::new(0,0);
        let mut result_gaussian_dyn: Option<DynamicImage> = None;
        for r_idx in 0..(warmup_runs_perf + num_runs_perf) {
            let img_copy = original_image_dyn.clone();
            let start_time = Instant::now();
            let processed_rgb_img = process_gaussian_blur(&img_copy, GAUSSIAN_SIGMA)?;
            let duration = start_time.elapsed();
            if r_idx >= warmup_runs_perf {
                total_duration_gaussian += duration;
            }
            if r_idx == (warmup_runs_perf + num_runs_perf - 1) {
                  result_gaussian_dyn = Some(DynamicImage::ImageRgb8(processed_rgb_img));
            }
        }
        let avg_time_gaussian_ms = (total_duration_gaussian.as_secs_f64() / num_runs_perf as f64) * 1000.0;
        if let Some(img_to_save) = result_gaussian_dyn {
            let output_path_str = format!("{}/{}_gaussian_core.bmp", output_dir_rust, img_name.split(' ').next().unwrap_or("unknown").to_lowercase());
            img_to_save.save(output_path_str)?;
        }
        println!("  Gaussian Blur (core): Avg Time        = {:.2} ms", avg_time_gaussian_ms);
        results_summary.get_mut(&img_name).unwrap().insert("Gaussian Blur".to_string(), avg_time_gaussian_ms);

        // 3. Sobel 边缘检测
        let mut total_duration_sobel = Duration::new(0,0);
        let mut result_sobel_dyn: Option<DynamicImage> = None;
        for r_idx in 0..(warmup_runs_perf + num_runs_perf) {
            let img_copy = original_image_dyn.clone();
            let start_time = Instant::now();
            // Pass SOBEL_THRESHOLD directly, ksize parameter removed from function
            let processed_luma_img = process_sobel_edges(&img_copy, SOBEL_THRESHOLD)?;
            let duration = start_time.elapsed();
            if r_idx >= warmup_runs_perf {
                total_duration_sobel += duration;
            }
            if r_idx == (warmup_runs_perf + num_runs_perf - 1) {
                  result_sobel_dyn = Some(DynamicImage::ImageLuma8(processed_luma_img));
            }
        }
        let avg_time_sobel_ms = (total_duration_sobel.as_secs_f64() / num_runs_perf as f64) * 1000.0;
        if let Some(img_to_save) = result_sobel_dyn {
            let output_path_str = format!("{}/{}_sobel_core.bmp", output_dir_rust, img_name.split(' ').next().unwrap_or("unknown").to_lowercase());
            img_to_save.save(output_path_str)?;
        }
        println!("  Sobel Edge Detection (core): Avg Time = {:.2} ms", avg_time_sobel_ms);
        results_summary.get_mut(&img_name).unwrap().insert("Sobel Edges".to_string(), avg_time_sobel_ms);

        println!("{}", "-".repeat(40)); 
    }

    println!("\n--- Summary of Rust Timings (ms) ---");
    let header = "| Image Size | Brightness | Gaussian Blur | Sobel Edges |";
    println!("{}", header);
    println!("|{}|", "-".repeat(header.len() - 2)); 
    let mut img_names_sorted: Vec<_> = results_summary.keys().cloned().collect();
    img_names_sorted.sort_by_key(|a| match a.as_str() { "Small (S)" => 0, "Medium (M)" => 1, "Large (L)" => 2, _ => 3 });
    for img_name_key in img_names_sorted {
        if let Some(timings) = results_summary.get(&img_name_key) {
            println!("| {:<10} | {:<10.2} | {:<13.2} | {:<11.2} |",
                     img_name_key.split(' ').next().unwrap_or("?"), 
                     timings.get("Brightness").unwrap_or(&-1.0),
                     timings.get("Gaussian Blur").unwrap_or(&-1.0),
                     timings.get("Sobel Edges").unwrap_or(&-1.0));
        }
    }
    Ok(())
}