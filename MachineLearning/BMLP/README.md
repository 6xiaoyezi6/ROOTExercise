# 贝叶斯 MLP（BMLP）的基本原理

## 一、我们的目标究竟是什么？

贝叶斯公式为：

$$
\boxed{
p(\boldsymbol{\theta}\mid x)
=
\frac{p(x\mid\boldsymbol{\theta})p(\boldsymbol{\theta})}
{\displaystyle\int p(x\mid\boldsymbol{\theta}')p(\boldsymbol{\theta}')\,d\boldsymbol{\theta}'}
}
$$

其中：

- $x$：已经观测到的训练数据。
- $\boldsymbol{\theta}$：MLP 中所有需要学习的权重和偏置。
- $p(\boldsymbol{\theta}\mid x)$：给定训练数据后，权重的后验概率分布。

**贝叶斯 MLP 的最终目标不是寻找某一组最优权重 $\boldsymbol{\theta}^*$，而是求解整个权重的后验概率分布 $p(\boldsymbol{\theta}\mid x)$。**

注意：分母是对所有可能的权重 $\boldsymbol{\theta}'$ 积分，而不是对训练数据 $x$ 积分。

这个公式包含三个重要的概率分布。

### 1.1 先验分布 $p(\boldsymbol{\theta})$

这是我们在使用训练数据之前，对权重的概率假设。

例如，我们假设 MLP 中的每个权重都服从标准高斯分布：

$$
\boxed{p(\theta_j)=\mathcal N(0,1)}
$$

这意味着，在训练开始之前，我们假设权重接近 0 的可能性较高。

这里有两个数：

$$
\mu_{\mathrm{prior}}=0,\qquad
\sigma_{\mathrm{prior}}=1
$$

**这就是第一个高斯分布：权重的先验高斯分布。**

在最简单的 BMLP 中，这两个数由我们事先指定，训练过程中不改变。

### 1.2 似然函数 $p(x\mid\boldsymbol{\theta})$

似然回答的是：

**假设某一组权重 $\boldsymbol{\theta}$ 已经确定，它产生我们实际观测到的训练数据 $x$ 的概率密度有多大？**

注意，$p(x\mid\boldsymbol{\theta})$ 不是权重的概率分布。

在固定 $\boldsymbol{\theta}$ 时，它描述的是观测数据的概率模型；将数据固定后，它才成为关于权重的似然函数。

### 1.3 后验分布 $p(\boldsymbol{\theta}\mid x)$

这是我们真正想求解的量。

它表示：

**在已经知道训练数据 $x$ 的情况下，不同权重组合分别有多大可能性。**

例如对于一个权重，我们可能最终得到：

$$
p(\theta\mid x)=\mathcal N(0.8,0.1^2)
$$

表示权重的后验均值为 0.8，后验标准差为 0.1。

但是这里必须强调：

**真实后验分布不一定是高斯分布。**

以上只是一个可能的结果。在复杂 MLP 中，真实后验可能高度非对称，甚至具有多个峰。

---

## 三、为什么 MLP 的真实后验难以求解？

假设 MLP 网络为：

$$
9\rightarrow64\rightarrow64\rightarrow1
$$

为了符合监督学习的常见写法，我们将完整训练数据 $x$ 写成：

$$
x=\{(u_i,y_i)\}_{i=1}^N
$$

其中 $u_i$ 是输入特征，$y_i$ 是训练标签。仍然用 $\boldsymbol{\theta}$ 表示全部权重和偏置。

模型预测为：

$$
\hat y_i=f(u_i;\boldsymbol{\theta})
$$

假设观测误差为高斯噪声：

$$
p(y_i\mid u_i,\boldsymbol{\theta})
=
\mathcal N(y_i;f(u_i;\boldsymbol{\theta}),\sigma_y^2)
$$

于是数据似然为：

$$
p(x\mid\boldsymbol{\theta})
=
\prod_{i=1}^{N}
p(y_i\mid u_i,\boldsymbol{\theta})
$$

现在代入贝叶斯公式：

$$
p(\boldsymbol{\theta}\mid x)
=
\frac{
p(x\mid\boldsymbol{\theta})p(\boldsymbol{\theta})
}{
\displaystyle\int
p(x\mid\boldsymbol{\theta}')p(\boldsymbol{\theta}')
\,d\boldsymbol{\theta}'
}
$$

**困难主要出现在分母这个积分。**

因为 $\boldsymbol{\theta}$ 包含 MLP 中全部权重与偏置。

对于这个网络，一共有：

$$
(9\times64+64)+(64\times64+64)+(64\times1+1)
=4865
$$

个参数。

这意味着分母实际上是一个 **4865 维积分**：

$$
p(x)
=
\int_{\mathbb R^{4865}}
p(x\mid\boldsymbol{\theta})
p(\boldsymbol{\theta})
\,d\boldsymbol{\theta}
$$

而且神经网络的非线性结构使似然函数相当复杂。

一般无法像只有一个权重的简单模型那样，直接把积分计算出来。

因此：

**我们虽然能够写出后验分布的数学公式，却通常无法精确计算这个归一化后的后验分布。**

这就是为什么需要近似方法。

---

## 四、如何近似？这里才引入第二个权重高斯分布

既然真实后验：

$$
p(\boldsymbol{\theta}\mid x)
$$

难以计算，我们就主动构造一个简单、容易计算的概率分布：

$$
\boxed{
q_{\boldsymbol{\phi}}(\boldsymbol{\theta})
\approx
p(\boldsymbol{\theta}\mid x)
}
$$

这里 $q_{\boldsymbol{\phi}}$ 就叫**变分分布**，也叫近似后验分布。

### 4.1 这个分布为什么也是高斯？

这不是贝叶斯定理规定的，而是我们人为选择的一种近似方式。

最简单的方法是让每个权重都有一个独立的高斯分布：

$$
\boxed{
q_{\boldsymbol{\phi}}(\theta_j)
=
\mathcal N(\mu_j,\sigma_j^2)
}
$$

所有权重组成的联合分布是：

$$
q_{\boldsymbol{\phi}}(\boldsymbol{\theta})
=
\prod_{j=1}^{M}
\mathcal N(\theta_j;\mu_j,\sigma_j^2)
$$

这叫均值场高斯变分近似（Mean-field Gaussian Variational Approximation）。

这里出现了两种分布：

| 分布 | 数学表达式 | 均值、标准差如何确定 |
|---|---|---|
| 权重先验 | $p(\theta_j)=\mathcal N(0,1)$ | 人为预先指定 |
| 权重近似后验 | $q_{\phi}(\theta_j)=\mathcal N(\mu_j,\sigma_j^2)$ | 通过训练学习 |

**它们都是针对同一个权重 $\theta_j$ 的概率分布，不是两种不同的权重。**

先验描述我们训练前的认识；近似后验描述我们利用数据更新后的认识。

### 4.2 到底在训练什么？

普通 MLP 对每个权重直接训练：

$$
\theta_j
$$

而这个贝叶斯 MLP 对每个权重训练：

$$
\boxed{\mu_j,\qquad \sigma_j}
$$

更准确地说，通常使用一个无约束参数 $\rho_j$ 计算正的标准差：

$$
\sigma_j=\operatorname{softplus}(\rho_j)
$$

因此，训练参数是：

$$
\boldsymbol{\phi}
=
\{\mu_1,\rho_1,\mu_2,\rho_2,\ldots\}
$$

我们的新目标就变成：

**找到一组最合适的 $\boldsymbol{\phi}$，让高斯近似分布 $q_{\boldsymbol{\phi}}(\boldsymbol{\theta})$ 尽可能接近真实后验分布。**

---

## 五、既然真实后验算不出来，怎么知道近似得好不好？

这是变分推断的核心。

最直观的想法是，直接最小化：

$$
D_{\mathrm{KL}}
\left(
q_{\boldsymbol{\phi}}(\boldsymbol{\theta})
\Vert
p(\boldsymbol{\theta}\mid x)
\right)
$$

但是右边又包含难以计算的真实后验，因此不能直接计算这个目标。

通过贝叶斯公式，可以推导：

$$
\begin{aligned}
D_{\mathrm{KL}}(q_{\boldsymbol{\phi}}\Vert p(\boldsymbol{\theta}\mid x))
={}&
\log p(x)\\
&-\mathbb E_q[\log p(x\mid\boldsymbol{\theta})]\\
&+D_{\mathrm{KL}}(q_{\boldsymbol{\phi}}\Vert p(\boldsymbol{\theta}))
\end{aligned}
$$

其中 $\log p(x)$ 虽然难以计算，但它与需要优化的变分参数 $\boldsymbol{\phi}$ 无关。

所以我们只需要最小化后面两项：

$$
\boxed{
\mathcal L
=
-\mathbb E_q[\log p(x\mid\boldsymbol{\theta})]
+
D_{\mathrm{KL}}(q_{\boldsymbol{\phi}}(\boldsymbol{\theta})
\Vert p(\boldsymbol{\theta}))
}
$$

这就是负 ELBO（Negative Evidence Lower Bound）。

其对应的 ELBO 为：

$$
\mathrm{ELBO}
=
\mathbb E_q[\log p(x\mid\boldsymbol{\theta})]
-
D_{\mathrm{KL}}(q_{\boldsymbol{\phi}}\Vert p(\boldsymbol{\theta}))
$$

因此：

$$
\boxed{
\mathcal L_{\mathrm{BMLP}}
=
-\mathrm{ELBO}
=
\mathrm{NLL}+\mathrm{KL}
}
$$

它的含义：

1. **NLL（负对数似然）项**：要求从近似后验采样出来的权重能够解释训练数据。
2. **KL 散度项**：要求近似后验不要无代价地偏离预先指定的权重先验。

我们因此得到一个可以用梯度下降优化的损失函数。

需要特别区分两个 KL 散度。

**第一个 KL：**

$$
D_{\mathrm{KL}}
\left(
q_{\boldsymbol{\phi}}(\boldsymbol{\theta})
\Vert
p(\boldsymbol{\theta}\mid x)
\right)
$$

这是近似后验与真实后验之间的 KL 散度。我们希望它尽可能小，但由于真实后验难以计算，通常无法直接求值。

**第二个 KL：**

$$
D_{\mathrm{KL}}
\left(
q_{\boldsymbol{\phi}}(\boldsymbol{\theta})
\Vert
p(\boldsymbol{\theta})
\right)
$$

这是近似后验与先验之间的 KL 散度，可以在选定合适的分布形式后计算。

**变分推断的核心，就是通过优化可以计算的负 ELBO，间接使近似后验接近真实后验。**

---

## 七、把出现过的高斯分布彻底区分

| 名称 | 数学表达式 | 是权重分布吗？ | 是否训练 |
|---|---|---|---|
| 先验分布 | $p(\theta_j)=\mathcal N(0,1)$ | 是 | 通常否 |
| 真实后验 | $p(\boldsymbol{\theta}\mid x)$ | 是 | 希望求解，但通常无法精确求解 |
| 近似后验 | $q_\phi(\theta_j)=\mathcal N(\mu_j,\sigma_j^2)$ | 是 | 是，训练 $\mu_j,\sigma_j$ |
| 观测似然 | $p(y_i\mid u_i,\theta)=\mathcal N(f(u_i;\theta),\sigma_y^2)$ | 否，是数据分布 | $\sigma_y$ 可固定或学习 |
| 采样用标准高斯 | $\epsilon_j\sim\mathcal N(0,1)$ | 否，是辅助随机变量 | 否 |

需要注意，**真实后验 $p(\boldsymbol{\theta}\mid x)$ 不一定是高斯分布**。将近似后验 $q_\phi(\boldsymbol{\theta})$ 设为高斯，是一种便于计算的变分近似假设。

### 7.1 对应 PyTorch 中的参数

假设使用标准高斯先验：

$$
p(\theta_j)=\mathcal N(0,1)
$$

代码为：

```python
prior_mu = 0.0
prior_sigma = 1.0
```

人为构造的近似后验为：

$$
q_\phi(\theta_j)=\mathcal N(\mu_j,\sigma_j^2)
$$

代码为：

```python
mu = nn.Parameter(torch.tensor(0.0))
rho = nn.Parameter(torch.tensor(-2.0))
sigma = torch.nn.functional.softplus(rho)
```

从近似后验中随机采样权重：

$$
\boxed{
\theta_j=\mu_j+\sigma_j\epsilon_j
}
$$

其中：

$$
\epsilon_j\sim\mathcal N(0,1)
$$

对应代码：

```python
epsilon = torch.randn_like(mu)
theta = mu + sigma * epsilon
```

这里的 `theta` 是本次前向传播实际使用的权重。

通过这样的重参数化方法，可以保持计算过程对 $\mu_j$ 和 $\rho_j$ 可微，从而使用反向传播优化近似后验分布。

### 7.2 总结

整个贝叶斯 MLP 的学习过程可以总结为：

**第一步：明确求解目标。**

我们真正想求解的是：

$$
\boxed{p(\boldsymbol{\theta}\mid x)}
$$

即权重的真实后验，而不是单个最优权重。

**第二步：解释为什么需要近似。**

由于 MLP 权重数量巨大且模型非线性，贝叶斯公式中的高维积分通常无法精确计算。

因此选择一个便于计算的高斯分布：

$$
\boxed{
q_\phi(\boldsymbol{\theta})
\approx
p(\boldsymbol{\theta}\mid x)
}
$$

来近似真实后验。

**第三步：通过变分推断求解近似后验。**

通过最小化负 ELBO：

$$
\boxed{
\mathcal L_{\mathrm{BMLP}}
=
\mathrm{NLL}+\mathrm{KL}
}
$$

学习近似高斯分布的均值与标准差，最终利用该分布来采样网络权重。

整个优化过程可以表示为：

$$
\boxed{
\begin{aligned}
&\text{目标：}&&p(\boldsymbol{\theta}\mid x)\\[4pt]
&\text{近似：}&&q_\phi(\boldsymbol{\theta})\\[4pt]
&\text{优化：}&&\min_\phi(-\mathrm{ELBO})\\[4pt]
&\text{结果：}&&q_{\phi^*}(\boldsymbol{\theta})
\approx p(\boldsymbol{\theta}\mid x)
\end{aligned}
}
$$

# 贝叶斯神经网络（BMLP）与普通 MLP 的区别

## 问题1：贝叶斯 MLP 能否像普通 MLP 一样使用 nn.Sequential？

**可以。贝叶斯 MLP 与普通 MLP 可以使用完全相同的网络结构，区别主要在于权重的表示方式和训练方法。**

普通 MLP：

```python
class MLP(nn.Module):
    def __init__(self):
        super().__init__()
        self.network = nn.Sequential(
            nn.Linear(1, 4),
            nn.Tanh(),
            nn.Linear(4, 4),
            nn.Tanh(),
            nn.Linear(4, 1)
        )

    def forward(self, x):
        return self.network(x)
```

贝叶斯 MLP：

```python
class BMLP(nn.Module):
    def __init__(self):
        super().__init__()
        self.network = nn.Sequential(
            BayesianLinear(1, 4),
            nn.Tanh(),
            BayesianLinear(4, 4),
            nn.Tanh(),
            BayesianLinear(4, 1)
        )

    def forward(self, x):
        return self.network(x)

    def kl_loss(self):
        return sum(layer.kl_loss() for layer in self.network
                   if isinstance(layer, BayesianLinear))
```

两者均采用 **1-4-4-1** 网络结构，隐藏层使用 Tanh 激活函数，输出层不使用激活函数。

不同之处在于：

- **普通 MLP**：每个权重是确定的数值
- **贝叶斯 MLP**：每个权重具有概率分布，通过采样得到实际参与计算的权重

PyTorch 原生 `torch.nn` 没有提供 `BayesianLinear`，因此需要自己实现，或者使用 `torchbnn`、`bayesian-torch` 等第三方库。

其中 `nn.Sequential` 只负责按顺序执行各层，并不限制网络必须使用确定性权重。

---

## 问题2：为什么使用 rho，而不是直接使用 sigma？

贝叶斯 MLP 假设权重的近似后验分布为高斯分布：

$$
q(w_i)=\mathcal{N}(\mu_i,\sigma_i^2)
$$

其中：

- $\mu_i$：权重的均值
- $\sigma_i$：权重的标准差
- $\sigma_i^2$：权重的方差

**从数学上讲，高斯分布的参数确实是 $\mu$ 和 $\sigma$，但训练时不一定直接优化 $\sigma$。**

原因是标准差必须满足：

$$
\sigma_i>0
$$

而梯度下降、Adam 等优化器无法自动保证可训练参数始终为正数。

因此引入一个无约束的参数 $\rho$，通过 Softplus 函数得到标准差：

$$
\sigma_i=\operatorname{softplus}(\rho_i)=\ln(1+e^{\rho_i})
$$

对应 PyTorch 代码：

```python
self.weight_mu = nn.Parameter(torch.empty(out_features, in_features))
self.weight_rho = nn.Parameter(torch.full((out_features, in_features), -4.0))

@property
def weight_sigma(self):
    return F.softplus(self.weight_rho) + 1e-6
```

这里真正参与训练的参数是 $\mu_i$ 和 $\rho_i$，而 $\sigma_i$ 由 $\rho_i$ 计算得到。

也可以使用另一种常见参数化方式：

$$
\sigma_i=e^{s_i}
$$

其中 $s_i=\log\sigma_i$ 是可训练参数。

**需要区分权重的先验分布与后验分布。**

我们设置的权重先验为标准正态分布：

$$
p(w_i)=\mathcal{N}(0,1)
$$

但训练过程中学习的是近似后验：

$$
q(w_i)=\mathcal{N}(\mu_i,\sigma_i^2)
$$

后验的均值和标准差由数据决定，不要求仍然等于 0 和 1。

---

## 问题3：负 ELBO 是什么？与普通 MLP 的优化目标有什么区别？

### 3.1 普通 MLP 的优化目标

对于回归任务，普通 MLP 通常使用均方误差（MSE）作为损失函数：

$$
\mathcal{L}_{\mathrm{MLP}}
=
\frac{1}{N}\sum_{i=1}^{N}
\left[f(x_i;W)-y_i\right]^2
$$

其中：

- $N$：训练样本数量
- $x_i$：第 $i$ 个输入
- $y_i$：第 $i$ 个真实值
- $f(x_i;W)$：MLP 的预测值
- $W$：网络中全部待优化的权重与偏置

训练的目标是寻找一组确定的参数：

$$
W^*=\arg\min_W\mathcal{L}_{\mathrm{MLP}}
$$

对应 PyTorch：

```python
criterion = nn.MSELoss()

y_pred = model(X_train)
loss = criterion(y_pred, Y_train)

optimizer.zero_grad()
loss.backward()
optimizer.step()
```

**普通 MLP 直接寻找能够最小化预测误差的一组参数。**

### 3.2 贝叶斯 MLP 的优化目标

贝叶斯 MLP 不直接寻找唯一的最优权重，而是希望得到权重的后验分布：

$$
p(W\mid D)=\frac{p(D\mid W)p(W)}{p(D)}
$$

其中：

- $D$：训练数据
- $p(W)$：权重的先验分布
- $p(D\mid W)$：数据似然
- $p(W\mid D)$：权重的后验分布

由于真实后验通常难以直接求解，采用一个可计算的近似分布 $q(W)$。

变分推断的目标是最大化证据下界 ELBO（Evidence Lower Bound）：

$$
\mathrm{ELBO}
=
\mathbb{E}_{q(W)}[\log p(D\mid W)]
-
D_{\mathrm{KL}}(q(W)\Vert p(W))
$$

为了使用梯度下降算法，将其转换成最小化负 ELBO：

$$
\mathcal{L}_{\mathrm{BMLP}}
=
\underbrace{
-\mathbb{E}_{q(W)}[\log p(D\mid W)]
}_{\mathrm{NLL}}
+
\underbrace{
D_{\mathrm{KL}}(q(W)\Vert p(W))
}_{\mathrm{KL\ Divergence}}
$$

因此：

$$
\mathrm{Negative\ ELBO}=\mathrm{NLL}+\mathrm{KL}
$$

**负 ELBO 不等于 KL 散度，KL 只是负 ELBO 的一部分。**

两项具有不同的作用：

1. **NLL（负对数似然）**：要求网络预测与训练数据相符
2. **KL 散度**：约束学习得到的近似后验分布与先验分布之间的差异

### 3.3 BMLP 中的 NLL 与 MSE 有什么关系？

假设观测数据符合高斯似然：

$$
y_i\mid x_i,W\sim\mathcal{N}(f(x_i;W),\sigma_y^2)
$$

其中 $\sigma_y$ 是假设的观测噪声标准差。

则单个样本的负对数似然为：

$$
-\log p(y_i\mid x_i,W)
=
\frac{[y_i-f(x_i;W)]^2}{2\sigma_y^2}
+
\frac{1}{2}\log(2\pi\sigma_y^2)
$$

如果 $\sigma_y$ 固定，最后一项与模型参数无关。

因此，对于整个数据集的平均负 ELBO，可以省略常数项，得到：

$$
\mathcal L_{\mathrm{BMLP}}
=
\frac{1}{2N\sigma_y^2}
\sum_{i=1}^{N}
\mathbb E_{q(W)}[(f(x_i;W)-y_i)^2]
+
\frac{1}{N}D_{\mathrm{KL}}(q(W)\Vert p(W))
$$

可见，BMLP 的优化目标也包含平方预测误差，但额外考虑了权重分布及 KL 散度。

在我们的代码中：

```python
nll = torch.stack([
    0.5 * ((bmlp(X_train) - Y_train) / obs_sigma).square().mean()
    for _ in range(mc_train)
]).mean()

kl = bmlp.kl_loss()

loss = nll + kl / len(X_train)
```

这里采用 Monte Carlo 权重采样近似 NLL 中的期望。

需要注意，`obs_sigma` 是高斯似然中假定的观测噪声标准差，并不等同于权重分布的标准差。

### 3.4 MLP 与 BMLP 优化目标对比

| 比较项目 | 普通 MLP | 贝叶斯 MLP |
|---|---|---|
| 优化对象 | 确定的权重 $W$ | 权重近似后验 $q(W)$ 的参数 |
| 常用损失 | MSE | 负 ELBO |
| 数据拟合项 | MSE | NLL |
| KL 散度 | 通常不包含 | 包含 |
| 权重先验 | 通常不显式指定 | 显式指定 |
| 训练结果 | 一组确定参数 | 权重的近似后验分布 |
| 预测输出 | 一个预测值 | 可以得到预测分布 |
| 不确定性 | 不直接提供 | 可以估计参数引起的预测不确定性 |

需要注意：普通 MLP 也可以使用 NLL 等其他损失函数，而贝叶斯神经网络也有不同的推断方法。以上对比针对我们当前使用的 MSE-MLP 与变分推断 BMLP。

另外，BMLP 训练时最小化的是负 ELBO，但前面绘制的训练集与测试集损失曲线使用的是预测均值的 MSE，两者不是同一个量。

---

## 问题4：BMLP 的预测结果如何理解？哪些是均值，哪些是误差？

我们使用的目标函数为：

$$
y=\sin(x)e^{-0.5x}
$$

训练范围：

$$
x\in[0,2\pi]
$$

预测范围：

$$
x\in[-\pi,3\pi]
$$

### 4.1 为什么 BMLP 可以输出均值和标准差？

普通 MLP 对同一个输入 $x$，使用固定权重进行前向传播，一般得到固定的预测值：

$$
\hat y=f(x;W)
$$

而 BMLP 可以从权重近似后验中反复采样：

$$
W^{(k)}\sim q(W)
$$

第 $k$ 次预测结果为：

$$
\hat y^{(k)}=f(x;W^{(k)})
$$

例如采样 300 次：

$$
\hat y^{(1)},\hat y^{(2)},\ldots,\hat y^{(300)}
$$

预测均值：

$$
\bar y=\frac{1}{300}\sum_{k=1}^{300}\hat y^{(k)}
$$

预测标准差：

$$
\sigma_{\mathrm{pred}}
=
\sqrt{
\frac{1}{299}
\sum_{k=1}^{300}(\hat y^{(k)}-\bar y)^2
}
$$

注意：这里的标准差描述由**权重后验不确定性**产生的预测波动，不包含额外的观测噪声。

### 4.2 每一列是什么意思？

| 列名 | 数学含义 | 说明 |
|---|---|---|
| x | $x$ | 输入的横坐标 |
| True | $y_{\mathrm{true}}$ | 真实函数值 |
| Prediction | $\bar y$ | BMLP 的预测均值 |
| Std | $\sigma_{\mathrm{pred}}$ | 预测标准差 |
| Abs Error | $\lvert \bar y-y_{\mathrm{true}}\rvert$ | 预测均值的绝对误差 |
| 95% Interval | $[Q_{0.025},Q_{0.975}]$ | 预测分布的 95% 分位数区间 |
| IN / OUT | — | 是否处于训练区间内 |

其中：

- **Prediction 才是最终报告的预测均值**
- **Abs Error 是预测值与真实值的实际偏差**
- **Std 是模型自身估计的预测波动，并不等于实际误差**
- **95% Interval 是基于权重采样的区间估计**，并不保证真实值一定落入区间

### 4.3 结合实际输出分析

下面是实际得到的预测结果：

| x | True | Prediction | Std | Abs Error | 区域 |
|---:|---:|---:|---:|---:|:---:|
| -3.1416 | 0.000000 | -0.799764 | 0.014649 | 0.799764 | OUT |
| -1.5708 | -2.193280 | -0.664354 | 0.016187 | 1.528926 | OUT |
| 0.0000 | 0.000000 | 0.040887 | 0.024450 | 0.040887 | IN |
| 1.5708 | 0.455938 | 0.462718 | 0.013895 | 0.006780 | IN |
| 3.1416 | 0.000000 | 0.004767 | 0.010107 | 0.004767 | IN |
| 4.7124 | -0.094780 | -0.084075 | 0.009465 | 0.010705 | IN |
| 6.2832 | 0.000000 | 0.004583 | 0.009780 | 0.004583 | IN |
| 7.8540 | 0.019703 | 0.073111 | 0.010503 | 0.053408 | OUT |
| 9.4248 | 0.000000 | 0.111305 | 0.011046 | 0.111305 | OUT |

**例1：训练范围内，$x=\pi/2$**

```text
x              = 1.5708
True           = 0.455938
Prediction     = 0.462718
Std            = 0.013895
Abs Error      = 0.006780
95% Interval   = [0.43742, 0.49031]
```

说明：

- 真实值为 $0.455938$
- BMLP 预测均值为 $0.462718$
- 两者相差 $0.006780$
- 权重采样产生的预测标准差为 $0.013895$
- 真实值落在估计的 95% 区间内

因此，这一点不仅预测比较准确，给出的区间也覆盖了真实值。

**例2：外推范围内，$x=-\pi/2$**

```text
x              = -1.5708
True           = -2.193280
Prediction     = -0.664354
Std            = 0.016187
Abs Error      = 1.528926
95% Interval   = [-0.69495, -0.63115]
```

说明：

- 真实值为 $-2.193280$
- BMLP 预测均值为 $-0.664354$
- 实际绝对误差达到 $1.528926$
- 但预测标准差只有 $0.016187$
- 真实值远远落在预测区间之外

这是非常重要的现象：

**BMLP 在外推区域预测严重错误，却仍然给出了很小的预测不确定性。**

这表明当前模型的预测不确定性没有有效反映外推误差。

贝叶斯神经网络并不保证在训练分布以外一定能够识别自己的错误。

### 4.4 MSE、MAE 和 RMSE 分别是什么？

你得到的结果：

```text
Prediction Error:
MSE  = 0.33270618
MAE  = 0.28456956
RMSE = 0.57680689
```

**MSE（Mean Squared Error）：均方误差**

$$
\mathrm{MSE}
=
\frac{1}{N}\sum_{i=1}^{N}(\bar y_i-y_i)^2
$$

**MAE（Mean Absolute Error）：平均绝对误差**

$$
\mathrm{MAE}
=
\frac{1}{N}\sum_{i=1}^{N}|\bar y_i-y_i|
$$

**RMSE（Root Mean Squared Error）：均方根误差**

$$
\mathrm{RMSE}
=
\sqrt{
\frac{1}{N}\sum_{i=1}^{N}(\bar y_i-y_i)^2
}
$$

这里：

- MSE = 0.33270618
- MAE = 0.28456956
- RMSE = 0.57680689

这三个值是根据最后打印的 **9 个检查点**计算的，涵盖训练区间内外，因此不能直接用它们评价训练区间内的拟合精度。

### 4.5 插值误差与外推误差

你还得到了：

```text
Interpolation   MAE = 0.00632259
Extrapolation   MAE = 0.50328535
```

这两项是根据绘图时生成的密集采样点计算的，而不是仅用上面的 9 个检查点。

**插值（Interpolation）**

$$
x\in[0,2\pi]
$$

$$
\mathrm{MAE}_{\mathrm{interpolation}}=0.00632259
$$

**外推（Extrapolation）**

$$
x\in[-\pi,0)\cup(2\pi,3\pi]
$$

$$
\mathrm{MAE}_{\mathrm{extrapolation}}=0.50328535
$$

外推误差约为插值误差的 80 倍。

因此，可以得到两个结论：

1. 当前 BMLP 在训练范围内的插值效果较好
2. 在训练范围外，预测精度明显下降，而且标准差没有充分反映这种下降

这说明**引入贝叶斯权重并不意味着网络自动掌握函数的外推规律，也不保证外推不确定性一定增大**。

另外，当前网络采用独立高斯权重的均值场近似后验，外推不确定性的表现还会受到先验、网络结构、似然假设以及变分近似的影响。

---

## 问题5：nn.Linear 和 BayesianLinear 中的 Linear 是什么意思？

Linear 的字面意思是“线性的”。

在神经网络中，`Linear` 通常指**全连接层（Fully Connected Layer）**所执行的仿射变换：

$$
\boldsymbol y=W\boldsymbol x+\boldsymbol b
$$

其中：

- $\boldsymbol x$：输入向量
- $W$：权重矩阵
- $\boldsymbol b$：偏置向量
- $\boldsymbol y$：输出向量

严格来说，包含偏置项的变换是仿射变换（Affine Transformation）；只有偏置为零时才是严格的线性映射。

### 5.1 nn.Linear(1,4) 是什么意思？

```python
nn.Linear(1, 4)
```

表示：

- 输入特征数量为 1
- 输出特征数量为 4
- 该层包含 4 个输出神经元

其计算过程为：

$$
\begin{aligned}
y_1 &= w_{11}x+b_1\\
y_2 &= w_{21}x+b_2\\
y_3 &= w_{31}x+b_3\\
y_4 &= w_{41}x+b_4
\end{aligned}
$$

对应权重矩阵：

$$
W=
\begin{pmatrix}
w_{11}\\
w_{21}\\
w_{31}\\
w_{41}
\end{pmatrix}
$$

其中：

$$
W\in\mathbb R^{4\times1}
$$

这个 `nn.Linear(1,4)` 总共有：

$$
1\times4+4=8
$$

个可训练参数，包括 4 个权重和 4 个偏置。

### 5.2 为什么 Linear 后面还需要 Tanh？

例如：

```python
nn.Linear(1, 4),
nn.Tanh(),
nn.Linear(4, 4),
nn.Tanh(),
nn.Linear(4, 1)
```

`Linear` 负责：

$$
\boldsymbol z=W\boldsymbol x+\boldsymbol b
$$

`Tanh` 负责引入非线性：

$$
a=\tanh(z)=\frac{e^z-e^{-z}}{e^z+e^{-z}}
$$

如果连续使用多个 `Linear`，中间没有任何非线性激活函数，那么多个仿射变换仍然可以合并成一个仿射变换。

因此，**MLP 能够拟合复杂非线性函数，关键不仅在于 Linear 层，还在于 Tanh、ReLU、SiLU 等非线性激活函数。**

### 5.3 BayesianLinear 和 nn.Linear 的区别是什么？

普通 Linear：

$$
\boldsymbol y=W\boldsymbol x+\boldsymbol b
$$

其中 $W$ 和 $b$ 是确定值。

贝叶斯 Linear：

$$
\boldsymbol y=W^{(k)}\boldsymbol x+\boldsymbol b^{(k)}
$$

其中每次前向传播使用从近似后验采样的权重：

$$
W^{(k)}\sim q(W)
$$

$$
b^{(k)}\sim q(b)
$$

因此，**两者执行的矩阵运算相同，区别在于权重和偏置是固定参数还是从概率分布采样得到的。**

对于我们目前的独立高斯近似后验，每个标量权重都具有两个可训练的分布参数：

$$
\mu_i,\rho_i
$$

因此，相比普通 MLP，BMLP 通常需要学习更多参数。

以 1-4-4-1 网络为例：

普通 MLP 参数数量：

$$
(1\times4+4)+(4\times4+4)+(4\times1+1)=33
$$

贝叶斯 MLP 参数数量：

$$
2\times33=66
$$

这里的 66 是用于描述近似后验的可训练参数数量，而不是说网络有 66 条连接。

---

## 总结

| 问题 | 结论 |
|---|---|
| 1. BMLP 可以使用 nn.Sequential 吗？ | 可以，网络结构可以与普通 MLP 相同 |
| 2. 为什么使用 rho？ | 通过 Softplus 将无约束参数转换为正的标准差 |
| 3. 负 ELBO 是 KL 散度吗？ | 不是，负 ELBO = NLL + KL |
| 4. Prediction、Std、Abs Error 的区别？ | 分别是预测均值、预测标准差和实际绝对误差 |
| 5. Linear 是什么意思？ | 表示全连接层中的仿射变换，形式为 Wx+b |

**核心区别：普通 MLP 学习一组确定的网络权重，而当前的 BMLP 使用变分推断学习权重的近似后验分布，并通过权重采样估计预测均值和模型参数不确定性。**