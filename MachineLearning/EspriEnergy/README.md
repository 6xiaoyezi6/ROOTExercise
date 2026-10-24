# ESPRI 三输入 MLP 质子能量重建

## 输入与模型

输入严格为 **`[ENaI, RDCX, RDCY]`**，单位为 MeV、mm、mm；输出为质子发射顶点动能（MeV）。
左右臂分别训练：Left 对应实验 `espriSide=1`，Right 对应 `espriSide=0`。
实验输入为 `[naiEnergy, rdcX[espriSide], rdcY[espriSide]]`，RDC 使用原始读出坐标及训练集归一化。
原 `protonEnergy` 是实验已有重建结果，不是真值；MLP 谱是模型对同批实验事件的重建谱。
当前网络为 **`3→128→128→128→1`、Tanh**，此前为三个 64 节点隐藏层。
增加 NaIID（`NaIBarID`）作为第四输入后，峰位和宽度变化很小，几乎没有改善，因此保留三输入。

## Mylar 主要修正激发能谱的位置

模拟在两臂加入 **0.85 mm 被动 Mylar 等效层**，重新模拟、预处理和训练后应用于实验。
实验统一采用 55°≤θp≤70°、Backtracking Z 5σ、Phi12 cut，合并同事件样本为 39,860 条。
激发能均采用相同实验 protonTheta 和 150 MeV/u 束流；以下数值单位均为 MeV。

| MLP 激发能峰位 μ | 无 Mylar：64 网络 | 有 Mylar：64 网络 | 有 Mylar：128 网络 |
|---|---:|---:|---:|
| 基态 | 0.6310 ± 0.0201 | −0.0499 ± 0.0231 | −0.0663 ± 0.0235 |
| 第一激发态 | 3.9311 ± 0.0093 | 3.2900 ± 0.0093 | 3.2846 ± 0.0093 |

**加入 Mylar 可以修正此前激发能谱约 0.6 MeV 的位置偏移，使 MLP 谱与原实验谱的峰位基本对齐。**
无 Mylar 的历史结果使用双高斯加指数本底；有 Mylar 的结果为各峰附近独立单高斯。
旧三输入逐事件缓存缺失，尚不能按相同方法重拟合，因此不直接量化 Mylar 前后 σ 的变化。

## 实验、64 和 128 网络的激发能 σ 对比

以下 64、128 结果均已加入 Mylar，并使用相同的峰附近窗口：基态 −1～1、第一激发态 2.3～4.3 MeV。
每个峰单独做窗口归一化的非分箱单高斯扩展似然拟合，不加本底、不共享参数；误差仅含拟合统计项。

| 激发能峰宽 σ（MeV） | 原实验重建 | 64 网络 MLP | 128 网络 MLP |
|---|---:|---:|---:|
| 基态 | 0.6788 ± 0.0255 | 0.7378 ± 0.0322 | 0.7393 ± 0.0325 |
| 第一激发态 | 0.6698 ± 0.0111 | 0.6890 ± 0.0121 | 0.6893 ± 0.0121 |

**网络从 64 加宽到 128 对激发能 σ 的影响很小，没有明显收窄**：两态 σ 分别变化约 +0.20%、+0.05%。
两种网络的 σ 均未小于原实验重建；当前结果不支持仅靠加宽网络显著改善激发能分辨率。
128 训练时模拟样本也接近翻倍，不能将差异全部归因于网络结构；局部 σ 不是本征分辨率，也不能据此认定已到物理极限。
这里比较的是激发能峰的高斯 σ，不是模拟质子动能残差的中央 68% 半宽。

## 下一步与运行

建议生成**关闭 NaI 读出展宽的模拟对照并重新训练**，保持 Mylar、其他几何及选样一致，检查读出展宽对中央峰宽的贡献。
依次运行 `PreProcess.C`、`EspriEnergy.ipynb`、`EspriEnergyEvaluation.ipynb`、`ForExperiment.ipynb`。
训练产物位于 `EspriEnergy_output/`，实验结果位于 `ForExperiment_output/`；输入与预处理文件位于 `InputRootFiles/`。
ForExperiment 末尾用 iminuit MIGRAD/HESSE 输出一行两列拟合图：左为原实验，右为 MLP；显示 −20～28 MeV，数据 alpha=0.5。
已有 `ExcitationEnergyComparison.npz` 时，可先运行导入/配置，再运行末尾拟合单元格，无需重新训练或推理。

## 结果来源

- [无 Mylar 的 64 网络结果](ForExperiment_output/output_20261024_172043/FitSummary.json)
- [有 Mylar 的 64 网络局部拟合](ForExperiment_output/output_20261024_214349/MylarLatestFitSummary.json)
- [有 Mylar 的 128 网络局部拟合](ForExperiment_output/ExcitationPeakGaussianFits.json)
- [增加 NaIID 的历史比较](ForExperiment_output/output_20261024_190940/ThreeVsFourInputs.md)
