# ESPRI 质子能量分析

模拟与实验的分布边缘难以对齐，分析时对两者统一选取 **55°≤θp≤70°** 区间。

## 输入

左右臂分别处理，输入为：

- NaI 沉积能量
- 局域 RDCX
- 局域 RDCY
- NaI 棒编号 `NaIBarID`（整数 1–14，作为第四个标量输入）

输入顺序为 `[ENaI, RDCX, RDCY, NaIBarID]`，网络为 `4→64→64→64→1`。

模拟棒编号取与 `ENaI` 相同的最大能量读出：Left/arm 0 使用 `NaIBarID = 7-GetNaIEId(hit)`，
Right/arm 1 使用 `NaIBarID = 8+GetNaIEId(hit)`。
模拟 Left 的原始棒号须反序，Right 保持原序；统一后 Left 为 1–7、Right 为 8–14，均按 RDCY 从高到低排列。
实验 `espriNaiBarId` 为 0–6，统一映射为 `7*(1-espriSide)+espriNaiBarId+1`，不翻转棒顺序。
Left 对应实验 side 1；Right 对应实验 side 0。

先运行 `PreProcess.C` 生成训练与独立评价的四个 ROOT 文件，再依次运行
`EspriEnergy.ipynb`、`EspriEnergyEvaluation.ipynb`、`ForExperiment.ipynb`。
必须重新训练四输入模型；三输入旧模型不能用于新的预测 Notebook。

## 输出

质子的能量。

## 事件筛选

- 质子同时击中塑闪和 NaI。
- 近似排除击穿 NaI 的事件。

## 结果记录：MLP 质子动能偏低及 NaI 棒号的影响（2026-10-24）

### 质子动能与激发能谱

在当前实验样本上，MLP 预测的质子顶点动能相对原 `protonEnergy` 整体偏低。
四输入模型在 39,860 条 `ComparisonOK` 事件上的平均动能差
`ProtonEnergyNN − protonEnergy` 为 **−1.469 MeV**（左右臂合并）。
两种重建使用相同的 `protonTheta` 和束流参数；在当前运动学区间，质子动能偏低会使缺失质量重建的激发能偏高。
因此，MLP 激发能谱的基态和第一激发态均向高能方向移动，峰位没有更接近已知能级 0 和 3.368 MeV。
这里的原 `protonEnergy` 是实验已有重建结果，并非已知真值；上述差异是两种重建之间的比较。

### 增加 NaIID 输入后的比较

文中的 NaIID 对应代码和 ROOT 分支中的 `NaIBarID`。
三输入模型使用 `[ENaI, RDCX, RDCY]`，四输入模型增加 `NaIBarID`，其他训练参数和物理筛选保持一致。
棒号已按实验位置对齐：模拟 Left 使用 `7-localID`，Right 使用 `8+localID`；
实验使用 `7*(1-espriSide)+espriNaiBarId+1`。

实验比较沿用 **55°≤θp≤70°、Backtracking Z 5σ、Phi12 cut**，有效同事件样本为 **39,860 条**。
在 **−2.0–5.8 MeV** 区间内，以两个独立高斯峰加指数背景进行非分箱扩展似然拟合；峰位与宽度自由变化。
以下均为左右臂合并结果，单位为 MeV，误差仅含拟合统计误差。

| 态及参数 | 原实验重建 | 三输入 MLP | 四输入 MLP（加入 NaIID） |
|---|---:|---:|---:|
| 基态 μ | −0.0646 ± 0.0199 | 0.6310 ± 0.0201 | 0.6520 ± 0.0205 |
| 基态 σ | 0.5879 ± 0.0259 | 0.6554 ± 0.0248 | 0.6756 ± 0.0258 |
| 第一激发态 μ | 3.2892 ± 0.0080 | 3.9311 ± 0.0093 | 3.9438 ± 0.0094 |
| 第一激发态 σ | 0.6273 ± 0.0089 | 0.6439 ± 0.0103 | 0.6521 ± 0.0104 |

加入 NaIID 后，相对三输入模型，基态峰位增加约 **0.0210 MeV**、σ 增加约 **3.1%**；
第一激发态峰位增加约 **0.0128 MeV**、σ 增加约 **1.3%**。
这表明在本次训练和选样下，**增加 NaIID 对实验激发能谱的影响较小，没有改善质子动能偏低导致的峰位偏高，也没有使峰宽减小**。
新旧结果来自相关事件，且拟合范围与背景模型会影响宽度；上述小变化不作统计显著性判断，也不代表完整系统误差评估。

### 结果来源

- [三输入拟合结果](ForExperiment_output/output_20261024_172043/FitSummary.json)
- [四输入拟合结果](ForExperiment_output/output_20261024_190940/FitSummary.json)
- [三输入与四输入数值对比（含分臂）](ForExperiment_output/output_20261024_190940/ThreeVsFourInputs.csv)
- [四输入拟合图](ForExperiment_output/output_20261024_190940/ExcitationGaussianFits.pdf)
