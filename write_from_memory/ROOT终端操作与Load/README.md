# ROOT TTree::Scan() 使用说明

## 1. 基本语法
Length$ 是返回vector的长度

## 2. 实际示例

```cpp
tree->Scan("Entry$:FDC0_nhits:Length$(FDC0_TDC):FDC0_nlayerx:FDC0_nlayery", "", "", 10);
```

该命令表示：**从第 0 个事件开始，扫描前 10 个事件，显示 FDC0 的 hit 数量、TDC vector 长度以及 X、Y 方向的有效层数。**

输出结果：

```text
************************************************************************
*    Row   *    Entry$ * FDC0_nhit * Length$(F * FDC0_nlay * FDC0_nlay *
************************************************************************
*        0 *         0 *        34 *        34 *         4 *         4 *
*        1 *         1 *        27 *        27 *         0 *         0 *
*        2 *         2 *        11 *        11 *         4 *         4 *
*        3 *         3 *        27 *        27 *         4 *         4 *
*        4 *         4 *        16 *        16 *         3 *         4 *
*        5 *         5 *        32 *        32 *         4 *         4 *
*        6 *         6 *        31 *        31 *         0 *         0 *
*        7 *         7 *        27 *        27 *         4 *         0 *
*        8 *         8 *        17 *        17 *         4 *         4 *
*        9 *         9 *         0 *         0 *         0 *         0 *
************************************************************************
```

各变量含义：

- `Entry$`：ROOT 内置变量，表示当前事件编号，从 0 开始。
- `FDC0_nhits`：FDC0 记录的 hit 数量。
- `Length$(FDC0_TDC)`：当前事件中 `FDC0_TDC` vector 的元素数量。
- `FDC0_nlayerx`：X 方向有效层数。
- `FDC0_nlayery`：Y 方向有效层数。

例如：

- **事件 0**：34 个 hit，X、Y 方向均有 4 个有效层。
- **事件 1**：27 个 hit，但 X、Y 方向的有效层数均为 0。
- **事件 4**：16 个 hit，X 方向有 3 个有效层，Y 方向有 4 个有效层。
- **事件 9**：没有记录 hit。

需要注意，`FDC0_nlayerx/y` 的具体统计条件取决于数据处理代码。

## 3. 常见用法

**（1）查看前 20 个事件的 hit 数量**

```cpp
tree->Scan("Entry$:FDC0_nhits", "", "", 20);
```

**（2）查看第 100 个事件的数据**

```cpp
tree->Scan("Entry$:FDC0_nhits:FDC0_nlayerx:FDC0_nlayery", "", "", 1, 100);
```

其中 `1` 表示扫描一个事件，`100` 表示从事件 100 开始。

**（3）筛选 hit 数量大于 20 的事件**

```cpp
tree->Scan("Entry$:FDC0_nhits", "FDC0_nhits>20", "", 100);
```

从前 100 个事件中筛选 `FDC0_nhits>20` 的事件。

**（4）筛选 X、Y 方向均至少有 3 个有效层的事件**

```cpp
tree->Scan(
    "Entry$:FDC0_nhits:FDC0_nlayerx:FDC0_nlayery",
    "FDC0_nlayerx>=3 && FDC0_nlayery>=3",
    "",
    100
);
```

其中 `&&` 表示逻辑与，两个条件必须同时满足。

**（5）检查 vector 长度是否等于 hit 数量**

```cpp
tree->Scan(
    "Entry$:FDC0_nhits:Length$(FDC0_TDC)",
    "FDC0_nhits!=Length$(FDC0_TDC)",
    "",
    100
);
```

只显示前 100 个事件中，两者不相等的事件。

**（6）调整输出列宽**

```cpp
tree->Scan(
    "Entry$:FDC0_nhits:Length$(FDC0_TDC):FDC0_nlayerx:FDC0_nlayery",
    "",
    "colsize=20",
    10
);
```

设置列宽为 20 个字符，避免变量名称被截断。

## 4. Scan()、Draw() 和 GetEntries() 的区别

| 方法 | 用途 | 示例 |
|---|---|---|
| `Scan()` | 打印事件的具体数值 | `tree->Scan("FDC0_nhits");` |
| `Draw()` | 绘制变量的统计分布 | `tree->Draw("FDC0_nhits");` |
| `GetEntries()` | 统计满足条件的事件数量 | `tree->GetEntries("FDC0_nhits>20");` |

例如，统计 X、Y 方向均至少有 3 个有效层的事件数量：

```cpp
Long64_t n = tree->GetEntries(
    "FDC0_nlayerx>=3 && FDC0_nlayery>=3"
);

cout << "Selected events = " << n << endl;
```

**总结：** `Scan()` 适合逐事件检查数据，`Draw()` 适合观察整体分布，`GetEntries()` 适合统计满足条件的事件数量。