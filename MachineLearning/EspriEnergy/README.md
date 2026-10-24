# ESPRI 能量样本预处理与 RDC 对照

## 目录与使用方式

！！！模拟和实验的边缘难以对齐，估计要对模拟和实验，同时选择[55.70]的区间进行分析

本项目根据 NaI 沉积读出能量和 RDC 径迹位置，准备质子在靶内发射位置真实动能的回归样本。靶内均匀产生宽能量质子，不通过两体运动学关系生成训练标签。原始设想的能量范围约为 10～200 MeV；当前 `Espri.root` 保存的实际源配置为 10～180 MeV。

| 文件 | 用途 |
| --- | --- |
| `Espri.root` | 原始模拟输入，`SimulatedTree`，1000000 个事件 |
| `Process.C` | 直接读取 RDC 中面坐标，施加同臂符合、单命中和近似 NaI 未击穿筛选 |
| `Processed_Left.root` | 当前左臂处理结果，2766 条 |
| `Processed_right.root` | 当前右臂处理结果，2772 条 |
| `CompareRDC.py` | 比较当前输出与实验，实验严格要求 `protonTheta>52` |
| `NaIApproxStop_20261023_010429/` | 本轮筛选前后快照、对比 PDF、统计表和报告 |
| `RDCWithoutNaI/PlasticRDC_vs_PlasticRDCNaI.pdf` | 去掉 NaI 条件前后的分布图 |
| `RDCWithoutNaI/Distributions.root` | 三组控制样本的直方图及比较画布 |
| `README.md` | 当前运行方式、对照分析及注明日期的历史记录 |

在已配置 NPTool/ROOT 的环境中，从本目录运行：

```bash
root -l -b -q Process.C
```

运行会覆盖两份 `Processed` 文件。要求有效唯一初始质子、同臂塑闪和 NaI 正能量读出、所有符合臂中恰好一个有效 RDC 中面命中，再施加下文的近似 NaI 未击穿筛选。`ENaI` 取所选臂读出能量最大的 NaI 棒；RDCX/RDCY 直接读取 `GetRdcPosX/Y()`，不拟合、不重复转换坐标。

实验比较命令：

```bash
/opt/anaconda3/envs/nn/bin/python CompareRDC.py
```

比较脚本创建新的 `RDCAlignment_YYYYMMDD_HHMMSS` 目录，实验严格要求 `protonTheta>52`；不额外添加 phi12、激发能或 NaI 停止筛选。`Process.C` 本身不加角度门；模拟也限制位置散射角大于 52° 的结果单独标为辅助对照，不写回生产树。

每份文件只有一个 `tree`，含四个 `Double_t` 分支：

| 分支 | 含义 | 单位 |
| --- | --- | --- |
| `ENaI` | NaI 最大能量棒的模拟读出能量 | MeV |
| `ETrue` | `InitialConditions` 中初始质子在靶内发射位置的真实动能 | MeV |
| `RDCX`、`RDCY` | 模拟已保存的 RDC 中面位置，直接使用实验 `rdcX/rdcY` 坐标约定 | mm |

回归输入为 `ENaI`、`RDCX`、`RDCY`，标签为 `ETrue`。原始备注曾考虑将 NaI 编号作为候选输入；当前按确认后的要求仅输出上述四个分支，不含编号。

## 当前结论

**2026-10-23：近似 NaI 未击穿筛选已加入并实际运行，保留 5538 条事件。实验已严格施加 `protonTheta>52`。停止筛选使模拟能角谱的高能边界在视觉上更接近实验，但没有使 RDCX 分布整体对齐，不能据此追加坐标平移。**

## 2026-10-23：近似 NaI 未击穿筛选与重新对比

### 判据及适用范围

用户选择先用现有数据的近似判据：

```text
Edep_NaI >= K_after_first_step
```

这里两项均来自 `InteractionCoordinates`：`GetEnergy()` 是该轨迹在 NaI 阵列的累计沉积能，`GetKineticEnergy()` 是首次 NaI step 后的轨迹动能。累计过程及动能取值见 [InteractionScorers.cc](/Users/yemingxin/nptool/NPSimulation/Scorers/InteractionScorers.cc:66)。停止判据使用未展宽交互能量，输出 `ENaI` 仍保留最大单棒的展宽读出能量，不能用 `ENaI >= ETrue` 替代。

现有文件没有保存最终停止体或 NaI 出射动能，`TrackInfo` 只记录轨迹起点。因此本判据**不是严格停止真值**：首步已损失的能量可能使临界击穿事件通过；累计量跨 NaI 阵列，也不保证在输出 ENaI 对应的单根棒内阻停。

由于交互分支没有敏感体名称，当前代码用世界位置半径 `r>1100 mm` 识别 NaI 记录，按世界 X 正负确定臂，再要求恰好一个有效质子记录。对本轮 8020 条原符合事件，RDC 交互记录半径实测约 1010.43～1043.74 mm，NaI 约 1254.32～1454.41 mm，二者可分离。1100 mm 是当前几何下的记录识别阈值，换安装几何或输入版本后须重新核对。

### 实际运行与验证

本轮读取现有 `Espri.root` 的 1000000 个事件，执行 `root -l -b -q Process.C`，没有重新运行 Geant4。

| 处理阶段 | Left | Right | 合计 |
| --- | ---: | ---: | ---: |
| 同臂塑闪＋NaI 符合、唯一有效 RDC 命中 | 3992 | 4028 | 8020 |
| 加入近似 NaI 未击穿筛选后 | 2766 | 2772 | 5538 |
| 被新增判据拒绝 | 1226 | 1256 | 2482 |

缺失或歧义的 NaI 质子交互记录为 0。5538/8020 是本次新增 cut 的事件保留率，约 69.05%，不是探测效率或拟合产额比。

两份输出各只有一个 `tree` 和四个 `Double_t` 分支，全部数值有限。逐项验证了新输出的 `ENaI/ETrue/RDCX/RDCY` 与原样本中通过判据的对应事件完全一致。修改前后代码和输出保存在 [before](NaIApproxStop_20261023_010429/before/) 与 [after](NaIApproxStop_20261023_010429/after/)。

模拟坐标已经采用实验原点和轴方向。以模拟保存的世界位置和实验 [GenerateTrack.C](/Users/yemingxin/RIKEN_Inelastic_Be/ppBe10/BackTracking/BackAllTracking/GenerateTrack.C:249) 独立核对，X 最大残差小于 0.00008 mm、Y 小于 2e-13 mm，符合源代码旋转矩阵小数截断的精度。当前输出不再变换这些坐标。

### 实验选择与 RDC 分布

实验输入为 `/Users/yemingxin/RIKEN_Inelastic_Be/ppBe10/BackTracking/BackAllTracking/Ex5sigma.root/tree`。Left 读取 `espriSide=1` 的 `rdcX[1]/rdcY[1]`，Right 读取 `espriSide=0` 的 `rdcX[0]/rdcY[0]`；要求所选侧坐标有限且大于 -900，并严格施加 `protonTheta>52`。

实验最终为 Left **241096** 条、Right **216883** 条。没有对实验追加 NaI 停止判据，因此目前仍不是完全相同的模拟/实验事件选择。

下表为实际最小值至最大值，不按图窗裁剪，单位 mm：

| 坐标 | 模拟近似停止筛选后 | 实验 `protonTheta>52` |
| --- | ---: | ---: |
| Left X | 46.07～402.79 | 58.80～443.47 |
| Right X | 49.01～401.61 | 4.73～388.36 |
| Left Y | 28.01～421.51 | 5.83～442.39 |
| Right Y | 22.58～426.05 | 9.60～438.74 |

各组按自身事件数归一化，RDC bin 宽 2 mm。黑线为实验 `theta>52`，蓝线为新增停止筛选前的模拟符合样本，红虚线为筛选后的模拟。Y 的周期性峰结构大体对应，X 的权重及高角度端覆盖仍有明显差异。

| RDCX 中位数 / mm | 模拟筛选前 | 模拟筛选后 | 实验 `theta>52` |
| --- | ---: | ---: | ---: |
| Left | 280.73 | 269.04 | 309.25 |
| Right | 170.91 | 183.05 | 143.83 |

新增判据并没有使 X 的中位数更接近实验：左右臂的绝对差反而从约 28.52/27.08 mm 增至 40.21/39.23 mm。两者都限制对应位置散射角大于 52° 的辅助对照也保留这一趋势；不能将中位数差当成机械坐标偏移。

### RDCX 对应的能量和角度

模拟 `ETrue` 是初始质子动能；实验 `protonEnergy` 来自 NaI 能量刻度及靶、窗、塑闪、铜楔能损反推，见 [ProtonEvent.h](/Users/yemingxin/RIKEN_Inelastic_Be/SAMURAI_ppBe10/convert/ProtonEvent.h:196)。两者表示相近的目标物理量，但一个是真值、一个是重建量，并非独立精确等价。模拟角度由生成顶点指向 RDC 中面位置、相对 +Z 计算；实验 `protonTheta` 由靶点指向 RDC 位置、相对实测束流方向计算，见 [ProtonEvent.h](/Users/yemingxin/RIKEN_Inelastic_Be/SAMURAI_ppBe10/convert/ProtonEvent.h:531)。

为避免角度选择不一致，下表及能角联合图中模拟也要求上述位置散射角大于 52°，但这一额外门不写回生产树：

| 共同角度范围内的量 | Left 模拟前→后 | Right 模拟前→后 | Left 实验 | Right 实验 |
| --- | ---: | ---: | ---: | ---: |
| 事件数 | 3944→2721 | 3952→2705 | 241096 | 216883 |
| 能量中位数 / MeV | 119.87→98.81 | 120.91→100.12 | 65.22 | 69.58 |
| 位置散射角中位数 / deg | 63.73→63.05 | 63.27→62.70 | 65.15 | 64.75 |

新增判据去掉了高能击穿区域，使模拟能角谱的高能边界在视觉上接近实验的弯折包络；这一观察来自二维图，并未拟合边界或给出边界误差。但模拟源仍为 10～180 MeV 均匀能量、各向同性单质子源，实验有反应能角关联和触发/PID 预选。筛选后模拟的能量及角度权重仍与实验不同，停止筛选不能代替这些物理权重。

### 剩余高角度边缘差异的证据

下面引用同一当前输入、加入停止筛选之前的只读诊断快照；新增停止 cut 不会增加这些边缘事件。此前明显的 Left X≈40 mm、Right X≈405 mm 实验峰对应约 50°，已基本被 `protonTheta>52` 排除。仍定义 Left X>403.5 mm、Right X<49 mm 为高角度边缘诊断区；它们不是已测定的严格接受度边界。

| 高角度边缘诊断量 | Left | Right |
| --- | ---: | ---: |
| 模拟：只要求有效唯一 RDC 命中 | 1260 | 1294 |
| 模拟：再要求同臂塑闪 | 360 | 396 |
| 模拟：再要求同臂 NaI，停止筛选前/后 | 0 / 0 | 0 / 0 |
| 实验 `theta>52` | 16300 | 14537 |
| 实验位置散射角中位数 / deg | 71.12 | 70.78 |
| 实验所选侧 RDC 拟合径迹角中位数 / deg | 57.39 | 57.25 |
| 实验附加 `abs(theta_track-protonTheta)<2 deg` 控制 | 111 | 86 |
| 实验附加 `abs(phi12-180)<15 deg` 控制 | 385 | 331 |
| 实验同时附加方向及 phi12 控制 | 12 | 7 |

这些实验边缘事件全部有同臂塑闪和 NaI 正信号，但位置散射角与所选侧 `rdcA/rdcB` 给出的径迹方向之差，中位数约 -14°。按当前模拟几何作不含材料散射的直线检查，靶点→RDC 的外推在这些边缘区不与 NaI 相交；改用 RDC 拟合斜率后，约 74%～76% 可以与任一 NaI 棒相交。

这表明“同臂有塑闪/NaI 信号”和“来自靶点、沿该位置方向传播的同一条质子”并不等价。非靶起源散射、RDC/NaI 关联及重建尾部值得继续检查，但现有证据尚不能把它们确定为唯一原因，也不能把所有被方向或 phi12 条件拒绝的事件直接认作背景。上述控制门没有加入本轮实验主样本。

### 图表与可复核产物

- [本轮全部对比图 PDF](NaIApproxStop_20261023_010429/Comparison.pdf)
- [停止筛选前后 RDC 叠加图](NaIApproxStop_20261023_010429/RDC_BeforeAfter.pdf)
- [双方共同角度范围的 RDC 对照](NaIApproxStop_20261023_010429/RDC_CommonTheta52.pdf)
- [RDCX–能量及角度联合分布](NaIApproxStop_20261023_010429/RDCX_EnergyTheta.pdf)
- [能量–角度分布](NaIApproxStop_20261023_010429/EnergyTheta.pdf)
- [能量及角度一维分布](NaIApproxStop_20261023_010429/KinematicDistributions.pdf)
- [本轮完整统计 CSV](NaIApproxStop_20261023_010429/Summary.csv)、[分区计数](NaIApproxStop_20261023_010429/Regions.csv)、[筛选定义与报告 JSON](NaIApproxStop_20261023_010429/Report.json)
- [本轮直方图 ROOT](NaIApproxStop_20261023_010429/Distributions.root)，对象放在 `histograms` 目录。
- [筛选后模拟与实验的两曲线对比](RDCAlignment_20261023_010614/Comparison.pdf)、[坐标及统计核对](RDCAlignment_20261023_010614/Report.json)
- [停止筛选前的能角及方向诊断 PDF](RDCKinematics_20261023_004844/Diagnostics.pdf)、[方向控制计数](RDCKinematics_20261023_004844/DirectionControl.json)。

本轮已检查 PDF 的实际渲染，绘图区保持 4:3；未调整坐标、未拟合、未重新模拟，未改变实验输入文件。
