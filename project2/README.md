# Project 2: Dot Product of Two Vectors

> Part of [SUSTech CS219 Projects](../README.md)

## 📖 Introduction

本项目深入对比了 **C (GCC -O3)** 与 **Java (HotSpot VM)** 在向量点积运算中的性能差异，源码详见 [Source Code](./dotproduct.c)。

报告通过 **Intel VTune Profiler** 详细分析了 **SIMD (AVX2) 指令集**、**内存带宽瓶颈** 以及 **JVM JIT (Just-In-Time)** 动态编译优化，并解释了为何在大数据规模下 Java 能够反超未充分优化的 C。

## 🖋 Feedback

Grading: 99/100

Comment from Instructor: 100w是不规范用法，可以用1M。 (额，好奇怪的评价角度)
