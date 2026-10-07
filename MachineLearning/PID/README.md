# TOGAXSI $\Delta E$-$E$ 粒子鉴别的 MLP 分类任务

## 文件与输入约定

| Notebook | 输入数据 | 样本条件 | 模型输出 |
|---|---|---|---|
| `PID.ipynb` | `PreProcess.root/tree` | 两级 GAGG 有效能量，由 `PreProcess.C` 生成 | `PID_outputs/PID_model.pt` |
| `PID_SSD.ipynb` | `PreProcessSSD.root/tree` | 两级 GAGG 有效能量，且同一径迹 SSD X1/Y1/X2/Y2 四层有效 hit，由 `PreProcessSSD.C` 生成 | `PID_SSD_outputs/PID_model.pt` |

两份 Notebook 的网络输入顺序均为 **`[ClusterGAGGEnergy, DeltaEGAGGEnergy]`**，单位 MeV。`PID_SSD.ipynb` 使用 SSD 条件筛选后的样本，不把 `ClusterSSDEnergy` 输入网络；该分支可保留在 ROOT 文件中。

每行对应一个径迹样本。`EventID` 用于按事件划分，`TrackID`（若存在）与 ROOT Entry 用于对齐；这些编号不作为特征。

本任务使用 TOGAXSI 中 **DeltaE GAGG 与 Cluster GAGG 的能量信息**进行粒子鉴别。对于每一个径迹样本，MLP 的两个输入量按顺序为 Cluster GAGG 中的能量沉积和 DeltaE GAGG 中的能量沉积：

$$
\mathbf{x}
=
\begin{pmatrix}
E_{\mathrm{Cluster\,GAGG}} \\
E_{\mathrm{DeltaE\,GAGG}}
\end{pmatrix}.
$$

两者的单位均为 MeV。MLP 根据这两个输入量，将事件分类为五种粒子之一：

$$
p,\quad d,\quad t,\quad {}^{3}\mathrm{He},\quad {}^{4}\mathrm{He}.
$$

因此该任务可以表示为

$$
\left(
E_{\mathrm{Cluster\,GAGG}},
E_{\mathrm{DeltaE\,GAGG}}
\right)
\longrightarrow
\mathrm{PID}.
$$

ROOT 文件中的 `tree` 包含以下分支：

- `DeltaEGAGGEnergy`：DeltaE GAGG 中沉积的能量，单位 MeV；
- `ClusterGAGGEnergy`：Cluster GAGG 中沉积的能量，单位 MeV；
- `EventID`：事件编号；
- `TrackID`：径迹编号，当前预处理宏写出，Notebook 可选读取；
- `PID`：粒子真实类别，类型为 `Int_t`。

其中 `PID` 按照以下方式编码：

$$
\begin{aligned}
0 &\rightarrow p,\\
1 &\rightarrow d,\\
2 &\rightarrow t,\\
3 &\rightarrow {}^{3}\mathrm{He},\\
4 &\rightarrow {}^{4}\mathrm{He}.
\end{aligned}
$$

MLP 最后一层包含 5 个输出节点，对应上述 5 种粒子。网络的原始输出为 5 个 logits：

$$
\mathbf{z}
=
\left(
z_p,\,
z_d,\,
z_t,\,
z_{{}^{3}\mathrm{He}},\,
z_{{}^{4}\mathrm{He}}
\right),
$$

logits 本身不是概率。经过 Softmax 后得到各粒子类别的概率：

$$
P_i
=
\frac{e^{z_i}}
{\sum_{j=1}^{5}e^{z_j}},
$$

并根据最大概率对应的类别给出最终 PID：

$$
\mathrm{PID}_{\mathrm{pred}}
=
\operatorname*{arg\,max}_i P_i.
$$

训练时采用多分类交叉熵损失函数 `CrossEntropyLoss`，直接使用网络输出的 logits 与真实 `PID` 标签计算损失，不需要在模型最后显式加入 Softmax。

在分类性能评价中，**Precision（P，精确率/纯度）**表示所有被模型预测为某种粒子的事件中，有多少真正属于该粒子：

$$
P=\frac{TP}{TP+FP},
$$

而 **Recall（R，召回率/效率）**表示所有真实属于该粒子的事件中，有多少被模型正确识别：

$$
R=\frac{TP}{TP+FN}.
$$

例如对于 proton，假设测试集中共有 1000 个真实 proton，其中 900 个被模型正确识别，同时模型总共将 1200 个事件预测为 proton，则

$$
R_p=\frac{900}{1000}=90\%,
$$

表示模型成功识别了 90% 的真实 proton；而

$$
P_p=\frac{900}{1200}=75\%,
$$

表示模型选出的 proton 样本中有 75% 真的是 proton。因此对于粒子鉴别问题，可以近似理解为：

$$
\boxed{\mathrm{Recall}\approx\text{粒子鉴别效率}}
$$

$$
\boxed{\mathrm{Precision}\approx\text{所选粒子样本的纯度}}
$$


| 真实情况 | 模型判断 | 数量 |
|---|---|---:|
| proton | proton | 900，TP |
| proton | 其他粒子 | 100，FN |
| d/t/³He/⁴He | proton | 300，FP |


## ΔE-E 方法中使用SSD进行径迹约束与不使用SSD的区别

使用 **ΔE-E 方法**进行粒子鉴别时，需要对粒子的**径迹**进行一定约束。因为 ΔE 探测器中的能量损失不仅与粒子种类和能量有关，还取决于粒子在探测器中的实际穿行长度。若粒子从边缘进入、侧面穿出或以较大倾角穿过，其有效厚度会发生变化，导致同一种粒子的 ΔE 出现较大展宽。

如果只要求两级 GAGG 都有能量沉积，而不限制粒子的穿行径迹，就会混入较多非正常穿透事件。这些事件可能只经过部分 ΔE GAGG，或者在主 GAGG 中从侧面逃出，因此测得的 ΔE 和 E 不能很好地满足理想的 ΔE-E 关系。结果是在 ΔE-E 图中形成较宽的尾部和异常分布，使不同粒子的鉴别带发生重叠。

当前 `PreProcessSSD.C` 对同一 EventID、TrackID 要求 SSD X1/Y1/X2/Y2 四层均有有效 hit，StripEnergy 必须有限且大于 0。这会改变被保留的径迹和粒子样本分布，可作为间接的几何筛选；它并不严格保证粒子完整穿过 ΔE GAGG 或停在 E GAGG 中。是否使鉴别带更集中，应结合实际分布判断。

两份 Notebook 现在都只使用两项 GAGG 能量。即使 SSD 筛选样本上的准确率更高，也不能解释为网络从 SSD 能量特征中获得了增益；应考虑样本筛选、类别比例和难度的变化。两份 Notebook 各自按输入文件划分事件，不构成同一测试事件集合上的直接比较。

## 训练、评价与保存

- 依赖：numpy、awkward、uproot、torch、matplotlib。先由对应预处理宏生成 ROOT 输入。
- 固定 seed=42，按独立 EventID 进行 80%/20% 划分，同一事件的全部径迹留在同一集合。
- 两项能量均要求有限且大于 0；标签为 0–4 的整数。遇到无效数据报错，不静默删除。
- 仅训练集拟合均值和标准差，再用于训练、测试及绘图网格；标准差为零时替换为 1。
- CPU float64 网络为 `2→64→64→64→5`，三个隐藏层使用 ReLU；标签使用 int64。
- batch=256，Adam，lr=1e-3，固定训练 500 轮，保留最后一轮权重。
- 每轮更新完毕后，在同一模型状态下重算完整训练与测试监控集的交叉熵及准确率。
- 评价包含混淆矩阵、逐类 Precision/Recall/F1、Macro F1 和 Balanced accuracy；Softmax 概率未经校准。
- 测试结果已参与结构开发，应作为开发监控；独立最终评价需要另留未使用的数据。代码不早停，也不按最低测试损失选权重。
- 第 7 节保存完整模型、标准化参数、特征顺序、标签映射、事件划分及训练历史，覆盖对应目录的同名 `.pt`，不导出 CSV。完整模型只从可信检查点加载。

## 最后一个单元格：分类区域和边界

两份 Notebook 的最后一个绘图单元格代码一致，均使用当前内存中的两输入模型，显示一张图。第 2.1 节真实 PID 散点图与最后一张预测 PID 散点图含义不同。

| 图中元素 | 含义 |
|---|---|
| 半透明颜色背景（alpha=0.25） | 模型在该二维位置预测的类别 |
| 黑色实线 | 不同模型预测区域之间的边界 |
| 彩色测试散点 | 颜色表示预测 PID |
| 黑色 `×` | 预测 PID 与 ROOT 真实 PID 不一致的测试事件 |

边界由下面的步骤生成：

1. 在原始能量平面建立 `500×500` 网格，x 为 ClusterGAGGEnergy，y 为 DeltaEGAGGEnergy，范围从 0 到输入样本对应能量的最大值。
2. 按训练特征顺序填入网格，复用训练集均值和标准差进行标准化。
3. 在 `model.eval()`、`torch.no_grad()` 下分批直接调用模型，取五维 logits 的 `argmax` 决定每个网格点的类别。
4. 用 `pcolormesh` 绘制半透明分类区域；对每一类构建属于该类为 1、否则为 0 的二值区域，用 `contour(levels=[0.5])` 提取黑色轮廓。

`0.5` 是二值区域的轮廓阈值，**不是类别概率为 50%**。不对 PID 的 0–4 编号做连续插值，也不使用近邻概率插值或固定 SSD 切片。理论上，两类的有效决策交界处满足它们的 logits 相等且共同不低于其他类别。

背景是二维模型直接预测的分类结果；黑线是在有限网格上提取的数值轮廓，窄小区域可能受网格分辨率影响。即使没有训练事件，模型也会对网格给出类别，因此稀疏区或训练覆盖之外的区域属于外推，不应据此认定物理鉴别可靠。

按顺序执行配置、读取、标准化、模型定义、初始化、训练和评价后再绘图。最后一个单元格会重新计算当前模型的测试预测及误分类标记，不自动加载 `.pt` 或训练模型。修改输入维度或训练配置后，应重新执行相关单元格，避免混用旧模型与标准化参数。
