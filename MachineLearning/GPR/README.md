# 高斯过程（Gaussian Process, GP）

## 1. 高斯分布与高斯过程

- **高斯分布（Gaussian Distribution）**：描述一个或多个随机变量的概率分布。
- **高斯过程（Gaussian Process, GP）**：描述一个函数在不同输入位置上可能取什么值，以及这些位置上的函数值之间如何关联。

| 比较 | 高斯分布 | 高斯过程 |
|---|---|---|
| 概率分布 | $\mathcal N(\mu,\sigma^2)$ | $\mathcal{GP}(m(x),k(x,x'))$ |
| 每次采样 | 一个随机数 | 一条随机函数 |
| 均值 | $\mu$ | 均值函数 $m(x)$ |
| 协方差 | 方差 $\sigma^2$（一维情况） | 协方差函数 $k(x,x')$ |
| 描述的不确定性 | 数值的不确定性 | 函数的不确定性 |

**原来是从高斯分布中抽取随机数，现在是从高斯过程中抽取随机函数**

### 1.1 高斯分布

对于一维高斯分布：

$$
x \sim \mathcal{N}(\mu,\sigma^2)
$$

其中：
- $\mu$：均值，描述随机变量的期望值。
- $\sigma^2$：方差，描述随机变量的不确定性。

### 1.2 高斯过程

对于高斯过程：

$$
f(x) \sim \mathcal{GP}(m(x),k(x,x'))
$$

其中：
- $f(x)$：随机函数。
- $m(x)$：均值函数，描述函数在每个输入位置的期望值。
- $k(x,x')$：协方差函数（核函数），描述不同输入位置 $x$ 和 $x'$ 对应的函数值之间的协方差。

均值函数定义为：

$$
m(x)=\mathbb{E}[f(x)]
$$

协方差函数定义为：

$$
k(x,x')=\operatorname{Cov}[f(x),f(x')]
$$

高斯过程的核心性质是：**任意有限个输入位置对应的函数值，都服从多元高斯分布。**

高斯过程回归（Gaussian Process Regression, GPR）中的“过程”，并不是指模型随着时间演化，而是指一组具有联合概率结构的随机变量。

## 2. 核函数（Kernel Function）

核函数表示为：

$$
k(x,x')
$$

其中，$x'$（读作 x prime）表示另一个输入位置。

- $x$：第一个输入位置。
- $x'$：第二个输入位置。
- $x$ 和 $x'$ 可以取相同的值，也可以取不同的值。

核函数的作用是描述两个输入位置上的函数值之间的协方差：

$$
k(x,x')=\operatorname{Cov}[f(x),f(x')]
$$

具体来说：

- 当 $x=x'$ 时，核函数给出该位置函数值的方差。
- 当 $x\neq x'$ 时，核函数给出两个位置上的函数值之间的协方差。
- 核函数决定了不同输入位置上的函数值如何关联，从而影响函数的平滑程度、变化尺度等性质。

### 2.1 常用核函数：RBF 核

平方指数核（Radial Basis Function Kernel, RBF）定义为：

$$
k(x,x')=\sigma_f^2\exp\left(-\frac{(x-x')^2}{2l^2}\right)
$$

其中：
- $\sigma_f^2$：信号方差，控制函数的整体变化幅度。
- $l$：长度尺度（Length Scale），控制函数值之间的相关性随输入距离增大而衰减的速度。
- $|x-x'|$：两个输入位置之间的距离。

对于 RBF 核：

- 当 $x=x'$ 时，$k(x,x')=\sigma_f^2$。
- 当 $x$ 和 $x'$ 接近时，协方差较大。
- 当 $x$ 和 $x'$ 相距较远时，协方差逐渐趋近于零。

因此，RBF 核假设距离较近的输入位置通常具有较强的函数值相关性。

## 3. 总结

**高斯分布描述随机变量的不确定性，而高斯过程描述整个函数的不确定性。**

高斯过程由均值函数和协方差函数共同定义：

$$
f(x)\sim\mathcal{GP}(m(x),k(x,x'))
$$

其中：

$$
m(x)=\mathbb{E}[f(x)]
$$

$$
k(x,x')=\operatorname{Cov}[f(x),f(x')]
$$

高斯过程回归的核心思想是：**利用已知数据以及不同输入位置之间的协方差结构，预测未知位置上的函数值，并给出预测的不确定度。**









# 高斯过程回归（Gaussian Process Regression, GPR）常用核函数

高斯过程回归（GPR）是一种基于**贝叶斯推断**的非参数回归方法。它使用均值函数和协方差函数（核函数）定义未知函数的先验分布，并结合观测数据得到后验预测分布，从而同时提供**预测均值与不确定度**。

核函数体现了模型对函数平滑性、相关长度、周期性和整体变化趋势的假设。不同的核函数可以相加或相乘，用来描述更复杂的数据结构。

## 一、常用核函数汇总

| 核函数 | scikit-learn 名称 | 函数特征 | 典型应用 |
|---|---|---|---|
| 常数核 | `ConstantKernel` | 常数协方差；乘法可控制信号幅度 | 与其他核组合 |
| 平方指数核 | `RBF` | 非常平滑 | 连续、平缓变化 |
| Matérn 1/2 | `Matern(nu=0.5)` | 较粗糙、样本路径通常不可微 | 不平滑的连续信号 |
| Matérn 3/2 | `Matern(nu=1.5)` | 中等平滑 | 实验响应、局部起伏 |
| Matérn 5/2 | `Matern(nu=2.5)` | 较平滑 | 探测器标定、物理响应 |
| 有理二次核 | `RationalQuadratic` | 混合多个相关长度 | 多尺度变化 |
| 周期核 | `ExpSineSquared` | 周期性 | 周期信号 |
| 线性核 | `DotProduct` | 线性趋势 | 近似线性关系 |
| 白噪声核 | `WhiteKernel` | 独立高斯噪声 | 测量噪声 |

RBF(1.0, (1e-2, 1e2))中第一个就是超参数l(相关长度)的初值以及范围

## 二、核函数的数学表达式

记两个输入向量为 $\mathbf{x}$ 和 $\mathbf{x}'$，距离为

$$
r=\|\mathbf{x}-\mathbf{x}'\|.
$$

对于下文含 $\sigma_f^2$ 的表达式，$\sigma_f^2$ 表示信号协方差幅度；在 scikit-learn 中，通常通过将基础核乘以 `ConstantKernel` 来引入该参数。

### 1. 常数核（Constant Kernel）

$$
\boxed{k_{\mathrm{Constant}}(\mathbf{x},\mathbf{x}')=c}
$$

其中 $c\geq0$ 是常数协方差，与输入距离无关。

- **乘法**：$k=c\,k_0$，缩放另一核的幅度；例如 `ConstantKernel(...) * RBF(...)` 中，$c$ 就是 $\sigma_f^2$。
- **加法**：$k=k_0+c$，为函数先验增加一个随机的常数偏移成分，其方差为 $c$。它不等同于直接指定确定性的均值函数。

`ConstantKernel` 不是用来替代 `RBF` 的长度尺度，而是控制协方差幅度的常用组件。

### 2. 平方指数核（RBF / Squared Exponential Kernel）

$$
\boxed{k_{\mathrm{RBF}}(\mathbf{x},\mathbf{x}')=\sigma_f^2\exp\left(-\frac{r^2}{2l^2}\right)}
$$

- $\sigma_f^2$：信号协方差幅度（对核函数对角线而言是先验方差）。
- $l$：相关长度（`length_scale`）。较大的 $l$ 意味着相关性随距离衰减得较慢，变化更平缓。
- $r$：输入向量之间的欧氏距离。

对于 $D$ 维输入，可以使用各向异性 RBF 核，为各维指定不同长度尺度：

$$
k_{\mathrm{RBF}}(\mathbf{x},\mathbf{x}')=\sigma_f^2\exp\left[-\frac12\sum_{d=1}^{D}\frac{(x_d-x_d')^2}{l_d^2}\right].
$$

RBF 对应的高斯过程具有任意阶均方导数，是一种很强的平滑性假设。

### 3. Matérn 1/2 核

$$
\boxed{k_{\mathrm{Mat\acute{e}rn}\,1/2}=\sigma_f^2\exp\left(-\frac{r}{l}\right)}
$$

也称为指数核（Exponential Kernel）。它允许更显著的局部不规则变化，对应的样本路径通常不可微。

### 4. Matérn 3/2 核

$$
\boxed{k_{\mathrm{Mat\acute{e}rn}\,3/2}=\sigma_f^2\left(1+\frac{\sqrt3r}{l}\right)\exp\left(-\frac{\sqrt3r}{l}\right)}
$$

比 RBF 更允许局部变化；对应的高斯过程具有一阶均方导数。

### 5. Matérn 5/2 核

$$
\boxed{k_{\mathrm{Mat\acute{e}rn}\,5/2}=\sigma_f^2\left(1+\frac{\sqrt5r}{l}+\frac{5r^2}{3l^2}\right)\exp\left(-\frac{\sqrt5r}{l}\right)}
$$

比 Matérn 3/2 更平滑，对应的高斯过程具有二阶均方导数。一般来说，Matérn 核的平滑度由 $\nu$ 控制，当 $\nu\to\infty$ 时趋近于 RBF 核。

### 6. 有理二次核（Rational Quadratic Kernel）

$$
\boxed{k_{\mathrm{RQ}}(\mathbf{x},\mathbf{x}')=\sigma_f^2\left(1+\frac{r^2}{2\alpha l^2}\right)^{-\alpha}}
$$

- $l$：相关长度。
- $\alpha>0$：尺度混合参数，注意它**不是** `GaussianProcessRegressor(alpha=...)` 中的噪声参数。

该核可以理解为对不同相关长度的 RBF 核进行尺度混合，适用于多尺度变化。当 $\alpha\to\infty$ 时，趋近于 RBF 核。

### 7. 周期核（Periodic / ExpSineSquared Kernel）

$$
\boxed{k_{\mathrm{Periodic}}(\mathbf{x},\mathbf{x}')=\sigma_f^2\exp\left[-\frac{2\sin^2(\pi r/p)}{l^2}\right]}
$$

- $p$：周期（`periodicity`）。
- $l$：控制周期形状的长度尺度。

适用于具有周期规律的信号，例如正弦型振荡。

### 8. 线性核（DotProduct Kernel）

$$
\boxed{k_{\mathrm{Linear}}(\mathbf{x},\mathbf{x}')=\sigma_0^2+\mathbf{x}^{T}\mathbf{x}'}
$$

$\sigma_0^2$ 对应贝叶斯线性模型截距项的先验方差。该核可表示线性趋势；若需要二次多项式型协方差，可以使用 `DotProduct(...) ** 2`，但它仍属于核回归，不等于直接进行多项式最小二乘拟合。

### 9. 白噪声核（White Kernel）

对于独立观测噪声，协方差可写为

$$
\boxed{k_{\mathrm{White}}(i,j)=\sigma_n^2\delta_{ij}},\qquad
\delta_{ij}=\begin{cases}1,&i=j,\\0,&i\neq j.\end{cases}
$$

因此，$N$ 个独立观测的噪声协方差矩阵为

$$
\mathbf K_{\mathrm{White}}=\sigma_n^2\mathbf I_N.
$$

其中 $\sigma_n^2$ 是**噪声方差**，不是标准差。这里的 $\delta_{ij}$ 是离散索引上的 Kronecker delta，不是 Dirac delta。

## 三、组合核函数（Composite Kernels）

scikit-learn 支持使用 `+` 和 `*` 直接组合核函数：

| 数学形式 | Python 形式 | 作用 |
|---|---|---|
| $k_{\mathrm{RBF}}+k_{\mathrm{White}}$ | `RBF() + WhiteKernel()` | 平滑信号与测量噪声 |
| $k_{\mathrm{Mat\acute{e}rn}}+k_{\mathrm{White}}$ | `Matern(nu=1.5) + WhiteKernel()` | 局部变化与噪声 |
| $k_{\mathrm{RBF}}+k_{\mathrm{Mat\acute{e}rn}}+k_{\mathrm{White}}$ | `RBF() + Matern(nu=1.5) + WhiteKernel()` | 不同平滑成分与噪声 |
| $k_{\mathrm{Linear}}+k_{\mathrm{RBF}}+k_{\mathrm{White}}$ | `DotProduct() + RBF() + WhiteKernel()` | 整体线性趋势与局部非线性变化 |
| $k_{\mathrm{Periodic}}\,k_{\mathrm{RBF}}$ | `ExpSineSquared() * RBF()` | 具有局部相关结构的周期成分 |
| $\sigma_f^2 k_{\mathrm{RBF}}$ | `ConstantKernel() * RBF()` | 优化 RBF 信号幅度 |

增加核函数数量也会增加超参数数量，因此应结合数据量、物理预期和模型验证决定是否需要复杂组合。

## 四、Python 中核函数的构造方法及参数含义

### 1. 重点示例：`ConstantKernel` 与 `RBF`

```python
from sklearn.gaussian_process.kernels import ConstantKernel, RBF

kernel = ConstantKernel(1.0, (1e-2, 1e3)) * RBF(1.0, (1e-2, 1e2))
```

这行代码完整展开为：

```python
kernel = ConstantKernel(constant_value=1.0, constant_value_bounds=(1e-2, 1e3)) * RBF(length_scale=1.0, length_scale_bounds=(1e-2, 1e2))
```

**注意：每个核类只有两个构造参数。** 看起来出现三个数字，是因为第二个参数是包含“下界、上界”的二元组。

| 代码 | 数值 | 参数名称 | 含义 |
|---|---:|---|---|
| `ConstantKernel(1.0, ...)` | `1.0` | `constant_value` | $\sigma_f^2$ 的初始值 |
| `ConstantKernel(..., (1e-2, 1e3))` | `1e-2` | `constant_value_bounds` 下界 | 允许的最小信号方差 $0.01$ |
| 同上 | `1e3` | `constant_value_bounds` 上界 | 允许的最大信号方差 $1000$ |
| `RBF(1.0, ...)` | `1.0` | `length_scale` | $l$ 的初始值 |
| `RBF(..., (1e-2, 1e2))` | `1e-2` | `length_scale_bounds` 下界 | 允许的最小相关长度 $0.01$ |
| 同上 | `1e2` | `length_scale_bounds` 上界 | 允许的最大相关长度 $100$ |

这对应以下核函数：

$$
\boxed{k(x,x')=\sigma_f^2\exp\left[-\frac{(x-x')^2}{2l^2}\right]},
\quad \sigma_f^2\in[0.01,1000],\quad l\in[0.01,100].
$$

一开始 $\sigma_f^2=1$、$l=1$；调用 `gpr.fit(...)` 后，若优化器启用且参数未固定，会在范围内寻找更优值。这里没有直接输入真实函数 $f(x)$ 的解析形式。

**上下界不是误差棒，也不是预测置信区间。** 它们仅约束核超参数优化时的搜索空间。

### 2. 常用核函数的 Python 写法

```python
import numpy as np
from sklearn.gaussian_process.kernels import ConstantKernel, RBF, Matern, RationalQuadratic, ExpSineSquared, DotProduct, WhiteKernel

k_const = ConstantKernel(constant_value=1.0, constant_value_bounds=(1e-2, 1e3))
k_rbf = RBF(length_scale=1.0, length_scale_bounds=(1e-2, 1e2))
k_m12 = Matern(length_scale=1.0, length_scale_bounds=(1e-2, 1e2), nu=0.5)
k_m32 = Matern(length_scale=1.0, length_scale_bounds=(1e-2, 1e2), nu=1.5)
k_m52 = Matern(length_scale=1.0, length_scale_bounds=(1e-2, 1e2), nu=2.5)
k_rq = RationalQuadratic(length_scale=1.0, alpha=1.0, length_scale_bounds=(1e-2, 1e2), alpha_bounds=(1e-2, 1e2))
k_periodic = ExpSineSquared(length_scale=1.0, periodicity=2*np.pi, length_scale_bounds=(1e-2, 1e2), periodicity_bounds=(1.0, 20.0))
k_linear = DotProduct(sigma_0=1.0, sigma_0_bounds=(1e-2, 1e2))
k_white = WhiteKernel(noise_level=0.16, noise_level_bounds=(1e-4, 1.0))
```

这些参数大多采用“初值 + 优化范围”的约定：

- `constant_value`：常数核的协方差幅度，常作为其他核的信号方差。
- `length_scale`：相关长度；多维 RBF / Matérn 可以使用向量，例如 `RBF([1.0, 2.0])`。
- `nu`：Matérn 平滑参数；标准优化器不会像优化 `length_scale` 那样自动优化它。
- `RationalQuadratic(alpha=...)`：尺度混合参数，与 GPR 构造器的 `alpha` 含义不同。
- `periodicity`：周期核的周期。
- `sigma_0`：线性核截距标准差参数，对应方差 $\sigma_0^2$。
- `noise_level`：白噪声**方差**，例如噪声标准差为 $0.4$ 时应使用 $0.4^2=0.16$。
- `*_bounds=(lower, upper)`：相应可优化参数的允许范围；`*_bounds="fixed"` 则固定该参数。

### 3. 固定某个参数，不进行优化

```python
kernel = ConstantKernel(1.0, constant_value_bounds="fixed") * RBF(1.0, length_scale_bounds="fixed")
```

此时 $\sigma_f^2=1$、$l=1$ 均保持不变。也可以只固定一个，允许另一个参数继续优化。

## 五、在 `GaussianProcessRegressor` 中使用核函数

以下示例与前面“二次函数 + 正弦函数”的模拟数据一致；假定上一单元格已定义 `x_train`、`y_train`、`x_plot` 和 `sigma`：

```python
from sklearn.gaussian_process import GaussianProcessRegressor
from sklearn.gaussian_process.kernels import ConstantKernel, RBF

kernel = ConstantKernel(1.0, (1e-2, 1e3)) * RBF(1.0, (1e-2, 1e2))
gpr = GaussianProcessRegressor(kernel=kernel, alpha=sigma**2, normalize_y=True, n_restarts_optimizer=10, random_state=42)
gpr.fit(x_train.reshape(-1, 1), y_train)
y_pred, y_std = gpr.predict(x_plot.reshape(-1, 1), return_std=True)

print('初始核函数：', gpr.kernel)
print('优化后核函数：', gpr.kernel_)
print('优化后信号方差：', gpr.kernel_.k1.constant_value)
print('优化后相关长度：', gpr.kernel_.k2.length_scale)
```

- `gpr.kernel`：传入模型的初始核函数。
- `gpr.kernel_`：拟合得到的核函数，包含优化后的超参数。
- `n_restarts_optimizer=10`：除了初始值对应的一次优化，还会用 10 组随机起点重新优化（在允许范围内）。
- `alpha=sigma**2`：将已知观测噪声方差加入训练协方差矩阵对角线；这里的 `alpha` **不是**有理二次核中的尺度混合参数。
- `return_std=True`：返回后验预测标准差 `y_std`。对于未加入 `WhiteKernel` 的模型，这通常被用于绘制潜在函数的预测不确定度。

如果希望直接让模型**估计**未知噪声方差，可改用：

```python
kernel = ConstantKernel(1.0, (1e-2, 1e3)) * RBF(1.0, (1e-2, 1e2)) + WhiteKernel(noise_level=0.16, noise_level_bounds=(1e-4, 1.0))
gpr = GaussianProcessRegressor(kernel=kernel, alpha=1e-10, normalize_y=True, n_restarts_optimizer=10, random_state=42)
```

上面使用很小的 `alpha` 保证数值稳定，并通过 `WhiteKernel` 学习噪声方差。已知噪声时通常直接用 `alpha=sigma**2`；未知噪声时通常使用 `WhiteKernel`。除非明确要表示两种独立的噪声来源，否则不要为同一份噪声在二者中重复计入。

## 六、重要提醒

1. `ConstantKernel(1.0, (1e-2, 1e3))` 中，`(1e-2, 1e3)` 是**一个元组参数**，不是两个独立的位置参数；`RBF` 同理。
2. `ConstantKernel * RBF` 表示 $\sigma_f^2\exp[-r^2/(2l^2)]$；`ConstantKernel + RBF` 则是另一种协方差结构。
3. `RBF`、`Matern`、`RationalQuadratic`、`ExpSineSquared` 的基础核在 scikit-learn 中通常不单独包含一个可优化的 $\sigma_f^2$，需要乘 `ConstantKernel`。
4. `RBF`、`Matern` 支持各维不同的长度尺度；`RationalQuadratic`、`ExpSineSquared` 的长度尺度在这里采用标量形式。
5. 长度尺度 $l$ 的单位与输入 $x$ 一致。若更改 $x$ 的单位或归一化方式，应重新考虑其初值与边界。
6. 对于 $f(x)=0.2x^2+\sin x$，RBF 可以根据数据进行平滑拟合，但它并不知道真实解析式。若希望显式表达二次趋势与周期成分，可进一步尝试多项式核、周期核及它们与 RBF 的组合，并比较泛化能力。

## 参考资料

- [scikit-learn：Gaussian Processes 用户指南](https://scikit-learn.org/stable/modules/gaussian_process.html)
- [scikit-learn：ConstantKernel API](https://scikit-learn.org/stable/modules/generated/sklearn.gaussian_process.kernels.ConstantKernel.html)
- [scikit-learn：RBF API](https://scikit-learn.org/stable/modules/generated/sklearn.gaussian_process.kernels.RBF.html)
- [scikit-learn：WhiteKernel API](https://scikit-learn.org/stable/modules/generated/sklearn.gaussian_process.kernels.WhiteKernel.html)