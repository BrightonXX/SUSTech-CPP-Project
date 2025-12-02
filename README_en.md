<div align="center">

# SUSTech CS219 (Advanced Programming) Projects

📖 Language: [中文](./README.md) | **English**

</div>

## 📚 Introduction

**Semester:** 2025 Spring  
**Lecturer:** Prof. Shiqi Yu

This repository contains all source codes and reports for my Projects 1~5 of the **CS219 Advanced Programming** course at SUSTech in the 2025 Spring semester, including `*.c/cpp`, `*.tex`, `*.pdf`, but excluding Lab materials.

The Department of Computer Science and Engineering replaced **CS205 C/C++ Program Design** with **CS219 Advanced Programming** in 2025 Spring. The course syllabus is almost identical, so it can be understood that there is no essential difference.

### Projects

|  #  |                  Name              |     Description      | Score | Approx. Time |
|:---:|-------------------------------------|--------------|:---:|---------:|
|  1  | A Simple Calculator                | High-precision calculator     | 97 | 40 hrs   |
|  2  | Dot Product of Two Vectors          | Utilize dot product to familiarize with C performance  | 99 | 20 hrs   |
|  3  | BMP Image Processing          | Process BMP images like Matrix  | 98 | 15 hrs   |
|  4  | A Simple Image Library          | Create a usable Image library   | 95 | 15 hrs   |
|  5  | Cross-Language Performance Analysis          | Compare characteristics of C/Python/Rust   | 95 | 15 hrs   |

[![Visitors](https://api.visitorbadge.io/api/visitors?path=https%3A%2F%2Fgithub.com%2FBrightonXX%2FSUSTech-CPP-Project&label=Visitors&countColor=%23263759)](https://visitorbadge.io/status?path=https%3A%2F%2Fgithub.com%2FBrightonXX%2FSUSTech-CPP-Project)

## ⚠️ WARNING

If you are cannot read Chinese, it's highly recommanded that **do not** study this course and repository, since everything is in CHINESE!  

## 💻 Suggestions for this Course

### Report:

1. **Please use LaTeX to write reports.**  
   Based on my experience reading other people's reports in this course on GitHub and their scores, LaTeX reports score about 3-5 points higher on average than Markdown reports, because LaTeX indeed surpasses Markdown significantly in terms of exquisiteness. Even calculating with the minimum improvement of 3 points, using LaTeX can provide at least a `3 * 5 / 500 * 65 = 1.95` point bonus to the overall grade. Moreover, I am very grateful that this course equipped me with almost a full set of LaTeX skills, which I can immediately apply to many other courses.

   If you are a LaTeX newbie, my suggestion is:
   - First, ask LLM to generate a simple piece of LaTeX for you;
   - Then compile it in real-time on the online platform [Overleaf](https://www.overleaf.com) (configuring a local environment is relatively complex). If you are within the SUSTech campus network, I recommend using the [Overleaf Mirror](https://sharelatex.cra.ac.cn/project);
   - Gradually modify your first tex. You can also refer to my `.tex` files in Project 3-5 for modifications, or ask GPT step-by-step how to achieve a certain effect;
   - There are many beautiful templates on Overleaf that you can use directly. I recommend IEEE Trans.
   - LaTeX is very simple. With the help of LLMs, learning is fast, and it takes only a few hours to put into production.

2. **Please be sure to speculate on Professor Yu's intention for the Project.**  
   The general logic of the Projects is: Familiarize with syntax → Familiarize with C performance → Improve performance of compute-intensive tasks / Design libraries.

   After speculating the intention, make good use of this information. For example:
   - In the first Project, since the purpose is to familiarize with syntax, your Report needs to write more about how you prevent memory leaks, handle errors, etc.;
   - Professor Yu likes content related to **Matrix (Image) and Performance Optimization** very much. If you demonstrate a dissection of performance or deep exploration of performance improvement in the Project, it will definitely get a high score.

3. **Uniqueness**  
   You need to think about what different characteristics your report will have compared to mainstream reports, and you need to add some of your own unique features and work to the original task.
   - This is not mandatory, but it can help you get extra points on top of the 90-point range;
   - I recommend referring to [LHB's Report](https://github.com/HaibinLai/CS205-CPP-Programing-Project) to see how he achieved Uniqueness (although he was a bit too Unique and it's hard to learn from).


### Final Exam Suggestions:

0. **Previous years' average scores are around 63.**

1. **The exam format is very similar to JavaA (CS109).**  
   The first feeling when getting the paper was that it evoked that sad memory.

   - 20 Multiple Choice Questions (40 points), mainly testing detailed knowledge points, such as the role of the `const` keyword in different positions; and there are many traps, for example, a question seemingly testing 4-byte alignment of a struct, but actually `sizeof` operates on a pointer. You will fail miserably if you don't read the question carefully;
   - 10 True/False Questions (20 points), mainly testing liberal arts abilities, requiring careful reading of courseware;
   - 10 Fill-in-the-blank Questions (20 points);
   - 2 Handwritten `template class` (20 points). It is suggested to manually practice different constructors and operator overloading before the exam to avoid having a blank mind in the exam hall and not knowing where to start.

2. **Make good use of predecessors' experience.**  

   - Please be sure to make good use of the Quiz Analysis summarized by Maystern. This can help from getting high Quiz scores in usual times to final review: Link: [Maystern's Quiz Analysis](https://github.com/Maystern/SUSTech_CS205_Cpp_Projects/tree/main/Quiz)
   - For final review, you can watch the exquisite speed-run video made by classmate LHB: [Bilibili Video](https://www.bilibili.com/video/BV1PFf6YGEyW/). This video summarizes all the contents of the courseware very well. Those who have low efficiency in class can watch it.

3. **About Rust and Python exam content:**

   - For Rust, the exam covered about 10 points of content. I hold a negative attitude towards the inclusion of Rust in this course because it is really too shallow. I don't know the significance of testing it, and it didn't even cover iterators. Moreover, the exam content is quite weird. Besides testing ownership and mut, it also tested some obscure things, the meaning of which is unclear.
   - Python only appeared as one sentence in the fill-in-the-blank questions.

## 🔗 References
Shiqi Yu's official C++ course material repository: [CPP](https://github.com/ShiqiYu/CPP)

Haibin Lai's 2024 Spring Projects repository: [CS205-CPP-Programing-Project](https://github.com/HaibinLai/CS205-CPP-Programing-Project)

Maystern's super comprehensive course repository: [SUSTech_CS205_Cpp_Projects](https://github.com/Maystern/SUSTech_CS205_Cpp_Projects)

Monad's 2022 Fall Projects repository: [SUSTech_CS205_Projects](https://github.com/YanWQ-monad/SUSTech_CS205_Projects)

## 
<div align="center">
  <p>如果这个仓库对你有帮助，请给一个 ⭐️ <strong>Star</strong>！这对我很重要！</p>
  <p>If this project helps you, please give it a ⭐️ Star!</p>
</div>
