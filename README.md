<div align="center">

# SUSTech CS219 (Advance Programming) Projects

📖 Language: **中文** | [English](./README_en.md)

</div>

## 📚 简介 

**Semester:** 2025 Spring  
**Lecturer:** Prof. Shiqi Yu

本仓库包含了 南方科技大学 2025 春季学期 **CS219 Advanced Programming** 课程我的 Project 1~5 所有源码以及报告，包含 *.c/cpp, *.tex, *.pdf，但不包含Lab资料。计算机科学与工程系在2025 Spring将 **CS205 C/C++ Program Design** 替换为 **CS219 Advanced Programming**，课程大纲几乎一致。

### Projects

|  #  |                  名称              |     简介      | 分数 | 大约用时 |
|:---:|-------------------------------------|--------------|:---:|---------:|
|  1  | A Simple Calculator                | 高精度计算器     | 97 | 40 hrs   |
|  2  | Dot Product of Two Vectors          | 利用点积熟悉C性能特点  | 99 | 20 hrs   |
|  3  | BMP Image Processing          | 处理BMP图像   | 98 | 15 hrs   |
|  4  | A Simple Image Library          | 图像库   | 95 | 15 hrs   |
|  5  | Cross-Language Performance Analysis          | 比较不同语言速度   | 95 | 15 hrs   |

[![Visitors](https://api.visitorbadge.io/api/visitors?path=https%3A%2F%2Fgithub.com%2FBrightonXX%2FSUSTech-CPP-Project&label=Visitors&countColor=%23263759)](https://visitorbadge.io/status?path=https%3A%2F%2Fgithub.com%2FBrightonXX%2FSUSTech-CPP-Project)

## 💻 对于这门课程的建议

### Report：

1. **请使用 LaTeX 写报告。**  
   根据我对存于 GitHub 中课程其他人报告的阅读和得分经验，LaTeX 报告得分大约会比 Markdown 报告平均高 3-5 分，因为LaTex的确从精致性上超越 Markdown 很多。就算按最少的提升 3 分来算，使用 LaTeX 至少能够提供 `3 * 5 / 500 * 65 = 1.95` 分的总评加成。而且我非常感谢这门课让我具备了 LaTeX 几乎全套的使用能力，并能够马上运用在其他许多课程。

   假如你是个 LaTeX 新手，我的建议是：
   - 先叫 GPT 帮你生成一段简单的 LaTeX；
   - 然后在在线平台 [Overleaf](https://www.overleaf.com) 实时编译（配置本地环境相对复杂）。如果你处于南方科技大学校园网内，推荐使用[Overleaf镜像](https://sharelatex.cra.ac.cn/project)；
   - 逐步修改你的第一个tex，也可以参考我在 Project3-5 中的 `.tex` 文件修改，也可以一步步问GPT实现某一个效果该怎么做；
   - Overleaf上有很多很漂亮的模版，也可以直接拿来使用，推荐 IEEE Trans。
   - LaTeX 很简单，在LLM的帮助下学习很快，只需几个小时就可以投产。

2. **请务必推测于老师本人出 Project 的意图。**  
   Project的一般展开逻辑是：熟悉语法 → 熟悉 C 性能 → 提升计算密集任务的性能 / 设计库

   推测出意图后，要利用好这个信息。例如：
   - 在第一个 Project 中，既然目的是熟悉语法，那么你的 Report 中就需要多写你如何防止内存泄露、进行错误处理等内容；
   - 于老师非常喜欢**矩阵（图像）与性能优化**方面的内容，如果你在 Project 中展示了对性能的剖析或性能提升的深入探究，那肯定是高分。

3. **Uniqueness**  
   你需要思考自己的报告会和主流报告有什么不同的特点，需要在原任务上加上一些自己的特色与工作。
   - 这不是必须的，但能帮助你从 90 分段再往上再拿到额外分；
   - 推荐参考 [LHB 的 Report](https://github.com/HaibinLai/CS205-CPP-Programing-Project)，看看他是如何做到 Unique 的（虽然他有点太 Unique 了不太好学）。


### 期末考试建议：

0. **往年均分均在63上下**

1. **考试形式非常像 JavaA (CS109)**  
   拿到试卷的第一体感就是勾起了那段悲伤的回忆。

   - 20 个选择题（40 分），主要考查细节知识点，例如 `const` 关键词放在不同位置的作用；并且陷阱很多，比如一道看似考 struct 的 4 字节对齐，实际 `sizeof` 操作的是一个指针，不认真读题会寄的比较惨；
   - 10 个判断题（20 分），主要考察文科能力，需要仔细阅读课件；
   - 10 个填空题（20 分）；
   - 2 个手写 `template class`（20 分），建议考前手动练习不同的构造函数、运算符重载，避免上考场直接脑袋空空不知道从何写起。

2. **善用前人经验**  

   - 请一定要利用好 Maystern 总结的 Quiz Analysis。这可以帮助从获得平时高 Quiz 分到最后的期末复习：链接：[Maystern 的 Quiz 分析](https://github.com/Maystern/SUSTech_CS205_Cpp_Projects/tree/main/Quiz)
   - 期末复习开始可以看看由 LHB 同学制作的精美速通视频：[Bilibili 视频](https://www.bilibili.com/video/BV1PFf6YGEyW/)。这个视频很好地总结了课件的全部内容，平时上课效率低的可以看看。

3. **关于 Rust 和 Python 的考试内容：**

   - 对于Rust，考试考了大约10分的内容，我对这门课加入Rust抱着否定态度，因为实在是太浅了，考着也不知道意义在哪里，连iterator都没讲到。而且考试内容比较怪异，除了考考控制权和mut，还考了一点边枝抹角的东西，意义不明。
   - Python 仅在填空题中出现了一句话。

## 🔗 References
Shiqi Yu的官方C++课程资料仓库：[CPP](https://github.com/ShiqiYu/CPP)

Haibin Lai的 2024 Spring Projects仓库： [CS205-CPP-Programing-Project](https://github.com/HaibinLai/CS205-CPP-Programing-Project)

Maystern 的超级全面课程仓库： [SUSTech_CS205_Cpp_Projects](https://github.com/Maystern/SUSTech_CS205_Cpp_Projects)

Monad 的 2022 Fall Projects仓库：[SUSTech_CS205_Projects](https://github.com/YanWQ-monad/SUSTech_CS205_Projects)

##

<div align="center">
  <p>如果这个仓库对你有帮助，请给一个 ⭐️ <strong>Star</strong>！这对我很重要！</p>
  <p>If this project helps you, please give it a ⭐️ Star!</p>
</div>
