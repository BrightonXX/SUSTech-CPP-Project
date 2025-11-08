<div align="center">

# SUSTech CS219 (Advance Programming, former CS205) Projects

</div>

Semester: 2025 Spring  
Lecturer: Prof. Shiqi Yu

### Projects

|  #  |                  名称              |     简介      | 分数 | 大约用时 |
|:---:|-------------------------------------|--------------|:---:|---------:|
|  1  | A Simple Calculator                 | 高精度计算器     | 97 | 50 hrs   |
|  2  | Dot Product of Two Vectors          | 使用C和Java计算点积   | 99 | 20 hrs   |
|  3  | BMP Image Processing          | 处理BMP图像   | 98 | 15 hrs   |
|  4  | A Simple Image Library          | 图像库   | 95 | 15 hrs   |
|  5  | Cross-Language Performance Analysis          | 比较不同语言速度   | 95 | 15 hrs   |

[![Visitors](https://api.visitorbadge.io/api/visitors?path=https%3A%2F%2Fgithub.com%2FBrightonXX%2FSUSTech-CPP-Project&label=Visitors&countColor=%23263759)](https://visitorbadge.io/status?path=https%3A%2F%2Fgithub.com%2FBrightonXX%2FSUSTech-CPP-Project)

## 对于这门课程的建议

### Report：

1. **请使用 LaTeX 写报告。**  
   根据我对存于 GitHub 中课程其他人报告的阅读和得分经验，LaTeX 报告得分大约会比 Markdown 报告平均高 3-5 分，因为latex的确从精致性上完全碾压 Markdown。就算按最少的提升 3 分来算，使用 LaTeX 至少能够提供 `3 * 5 / 500 * 65 = 1.95` 分的总评加成。而且我非常感谢这门课让我具备了 LaTeX 几乎全套的使用能力，并能够马上运用在其他课程包括以后的毕业设计。

   假如之前没有使用过 LaTeX，我的建议是：
   - 先叫 GPT 帮你生成一段简单的 LaTeX；
   - 然后在在线平台 [Overleaf](https://www.overleaf.com) 实时编译（因为配置本地环境相对复杂）；
   - 再逐步修改，也可以参考我在 Project3-5 中的 `.tex` 文件修改，也可以一步步问GPT实现某一个效果该怎么做；
   - LaTeX 并不是什么困难的东西，本质上是一个线性调用函数的松散程序；
   - 摸索起来很快，只要小几个小时就可以正式投产。

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

1. **考试形式非常像 JavaA。**  
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
