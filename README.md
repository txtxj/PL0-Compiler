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

## TODO

- [ ] 数组
- [ ] for 语句
  - [ ] 含 step
  - [ ] 不含 step
- [x] else 子句
- [x] 赋值语句扩展为赋值表达式
- [ ] 内置函数
  - [x] print
  - [ ] setjmp
  - [ ] longjmp

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
│   ├── example3.txt
│   ├── example4.txt    # 测试 if-else
│   ├── example5.txt    # 测试 赋值表达式
│   └── example6.txt    # 测试 print
├── .gitignore
├── CMakeLists.txt
├── README.md
└── 编译原理和技术实践2017.pdf
```