# EnergyLoss 常量列归一化修复对比

相同数据及 seed=42 的 80%/20% 划分；13→64→64→64→1 Tanh、float64、2000 轮最终权重。新训练在 CPU 单线程运行。

修复：训练集极差为零的列，其均值取实际常量、缩放量设为 1。ClusterSSDZX2 原缩放量约 1.123e-10 mm，现为 1。

| 数据集 | 事件数 | 原 RMSE (MeV) | 新 RMSE (MeV) | 原 MAE (MeV) | 新 MAE (MeV) |
|---|---:|---:|---:|---:|---:|
| training | 21768 | 1.485793 | 1.521723 | 1.136425 | 1.145565 |
| internal_test | 5442 | 1.949627 | 2.206122 | 1.352804 | 1.359548 |
| external_test | 13671 | 3.502560 | 2.235073 | 1.403182 | 1.355235 |

外部 RMSE 下降约 36.2%，但内部 RMSE 上升约 13.2%。外部 |误差|>100 MeV 从 5 个降至 1 个。仍存在异常长尾，不能将本次修复理解为全部误差问题已解决。

此前归一化饱和的四个外部事件，其新旧残差 (MeV)：
- entry 8127: -163.104091 → -1.443480
- entry 13323: -160.380075 → -0.147705
- entry 12439: -158.700755 → -0.504898
- entry 5305: -151.403976 → 2.236068

参考：nndl-practice/pytorch/chap4前馈神经网络；machine-learning/references/pytorch-examples/basics/regression/main.py。仅沿用标准化和回归训练模式，保留当前任务的参数。

comparison.json 保存完整指标；predictions_and_losses.npz 保存新旧残差及本次 2000 轮损失。旧 Notebook 和 EnergyLossMLP_before.pt 保留在本目录。

两个当前 Notebook 已执行并保存结果，当前 EnergyLoss_output/EnergyLossMLP.pt 为本次最终模型。原始 ROOT 和 SSD 子目录未修改。
