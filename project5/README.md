# Project 5: Cross-Language Performance Analysis

> Part of [SUSTech CS219 Projects](../README.md)

## 📖 Introduction

本项目是一个跨语言性能对比实验，设计了一个对比框架，并在图像处理任务（亮度调整、高斯模糊、Sobel 边缘检测）中横向测评了以下三种语言：

*   **Python**: 基于 **OpenCV** (底层高度优化的 C++)。
*   **C++**: 基于 Project 4 自研的图像库 (OpenMP + AVX2)。
*   **Rust**: 基于 `image` 和 `imageproc` crate (Rayon 并行)。

## 🖋 Feedback

Grading: 95/100

Comment from Instructor：“当时埋下的种子正中了这个学期的我的眉心”，恭喜恭喜！

由于这个Project无限靠近期末周，已经力竭了。
