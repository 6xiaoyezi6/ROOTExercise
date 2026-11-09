# ROOT 树与分支的创建和读取

## 五个文件的职责

| 文件 | 职责 |
| --- | --- |
| `CreateNPToolTreeInitial.h` | 定义独立的 `BDC1`、`BDC2` 数据类、初始化函数和 ROOT 字典声明。 |
| `CreatNPToolTree.h` | 定义 `CreateNPToolTree` 创建器，创建 `tree` 及其 `BDC1`、`BDC2` 对象分支。 |
| `CreateTree.h` | 定义 `CreateTree` 创建器，创建 `ReconstructionTree` 及其 `Double_t Energy`、`Theta` 标量分支。 |
| `ReadTree.h` | 定义 `ReadTree` 读取器，获取已有的两棵树，通过 `SetBranchAddress` 绑定分支。 |
| `Create.C` | 提供 `Create()` 和 `ReadBefore()`，组织文件打开、事件循环、写入或读取及资源释放。 |

## 引用关系

```text
Create.C
├── CreateTree.h
├── CreatNPToolTree.h
│   └── CreateNPToolTreeInitial.h
└── ReadTree.h
    └── CreateNPToolTreeInitial.h
```

创建器和读取器共用 Initial 头文件中的数据类，保证对象分支的类型一致。
`BDC1`、`BDC2` 各自包含 `Double_t Energy` 和 `TVector3 Position`，可以分别扩展。
类名及分支名使用 `BDC1`、`BDC2`；实例指针使用 `bdc1`、`bdc2`，例如 `reader->bdc1->Energy`。

## 数据流程与使用

`Create()` 打开 `Before.root`，实例化两个创建器；每个事件先 `Initialize()`，再赋值、分别 `Fill()`，最后写入两棵树。
`Initialize()` 将标量和坐标三个分量设为 `-999`；示例随后为全部 10 条事件赋值。
`ReadBefore()` 打开 `Before.root`，实例化读取器，逐条调用 `GetEntry()`，打印两棵树的数据。
两棵树按相同条目号对应同一事件；先删除创建器或读取器，再关闭文件。

在 `Second` 目录启动 ROOT 后执行：

```cpp
.L Create.C+   // 编译加载并生成数据类字典
Create();      // 创建或覆盖 Before.root
ReadBefore();  // 读取并打印已有数据
```

增加对象分支：在 Initial 头文件中添加数据类及字典声明，再在创建器中补充指针、分配、绑定、初始化和释放，在读取器中补充对应绑定和管理。
增加普通标量分支：修改 `CreateTree.h` 中的成员、初始化及 `Branch`，同步修改 `ReadTree.h` 的成员、初始化及 `SetBranchAddress`。
