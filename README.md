# PL0-Compiler

USTC 编译原理 2022FA.zql 大作业

## 构建

```
mkdir cmake-build-debug
cd cmake-build-debug
cmake ..
cmake --build .
```

若运行时工作目录位于 `cmake-build-debug` 中，则直接输入 `example.txt` 即可运行测试样例。

## 项目结构

```
.
├── src                 # 源代码放置于该目录下
│   ├── pl0.c
│   ├── pl0.h
│   ├── set.c
│   └── set.c
├── test                # 测试样例放置于该目录下
│   ├── example.txt
│   ├── example2.txt
│   └── example3.txt
├── .gitignore
├── CMakeLists.txt
├── README.md
└── 编译原理和技术实践2017.pdf
```