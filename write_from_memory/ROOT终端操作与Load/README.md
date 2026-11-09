
# ROOT 终端操作

## 1. TTree::Scan() 使用说明

### 1.1 基本语法

`Length$` 返回 vector 的长度。

### 1.2 实际示例

```cpp
tree->Scan("Entry$:FDC0_nhits:Length$(FDC0_TDC):FDC0_nlayerx:FDC0_nlayery", "", "", 10);
```

该命令表示：**从第 0 个事件开始，扫描前 10 个事件，显示 FDC0 的 hit 数量、TDC vector 长度以及 X、Y 方向的有效层数。**

输出结果：

```text
********************************************************************************************************
*    Row   *    Entry$ * FDC0_nhit * Length$(F * FDC0_nlay * FDC0_nlay *
********************************************************************************************************
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
********************************************************************************************************
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

### 1.3 常见用法

#### 1.3.1 查看前 20 个事件的 hit 数量

```cpp
tree->Scan("Entry$:FDC0_nhits", "", "", 20);
```

#### 1.3.2 查看第 100 个事件的数据

```cpp
tree->Scan("Entry$:FDC0_nhits:FDC0_nlayerx:FDC0_nlayery", "", "", 1, 100);
```

其中 `1` 表示扫描一个事件，`100` 表示从事件 100 开始。

#### 1.3.3 筛选 hit 数量大于 20 的事件

```cpp
tree->Scan("Entry$:FDC0_nhits", "FDC0_nhits>20", "", 100);
```

从前 100 个事件中筛选 `FDC0_nhits>20` 的事件。

#### 1.3.4 筛选 X、Y 方向均至少有 3 个有效层的事件

```cpp
tree->Scan(
    "Entry$:FDC0_nhits:FDC0_nlayerx:FDC0_nlayery",
    "FDC0_nlayerx>=3 && FDC0_nlayery>=3",
    "",
    100
);
```

其中 `&&` 表示逻辑与，两个条件必须同时满足。

#### 1.3.5 检查 vector 长度是否等于 hit 数量

```cpp
tree->Scan(
    "Entry$:FDC0_nhits:Length$(FDC0_TDC)",
    "FDC0_nhits!=Length$(FDC0_TDC)",
    "",
    100
);
```

只显示前 100 个事件中，两者不相等的事件。

#### 1.3.6 调整输出列宽

```cpp
tree->Scan(
    "Entry$:FDC0_nhits:Length$(FDC0_TDC):FDC0_nlayerx:FDC0_nlayery",
    "",
    "colsize=20",
    10
);
```

设置列宽为 20 个字符，避免变量名称被截断。

### 1.4 Scan()、Draw() 和 GetEntries() 的区别

| 方法 | 用途 | 示例 |
| --- | --- | --- |
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

## 2. ROOT 文件中存储的 TAsciiFile 用法

```text
InputRootFiles git:(main) ✗ root RIBL.root
add include path : /Users/yemingxin/anaroot/src/install/include
reading libXMLParser.so
reading libananadeko.dylib
reading libanacore.dylib
reading libanabrips.dylib
reading libanadali.dylib
reading libanasamurai.dylib
reading libanacatana.dylib
reading libanaespri.dylib
reading libanawinds.dylib
reading libanasilicons.dylib
reading libanatogaxsi.dylib
reading libanaloop.dylib
reading libanaloopexample.dylib
reading libanaloopencexample.dylib
root [0]
Attaching file RIBL.root as _file0...
(TFile *) 0x76773df800
root [1] .ls
TFile**     RIBL.root
 TFile*     RIBL.root
  KEY: TParameter<int>  RandomSeed;1
  KEY: TNamed   RandomSeedMode;1    automatic
  KEY: TAsciiFile   G4RandomFinalState;1    .geant4_random_state
  KEY: TAsciiFile   EventGenerator;1    RIBL_4He_p.reaction
  KEY: TAsciiFile   DetectorConfiguration;1 RIBLArray.detector
  KEY: TTree    SimulatedTree;1 Data created / analysed with the nptool package
root [2] TFile*f = _file0;
root [3] TAsciiFile* eventConfig = (TAsciiFile*)f->Get("EventGenerator");
root [4] eventConfig->Print()
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%% Reaction file for 1H(4He,p)4He elastic scattering %%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Beam
 Particle= 4He
 Energy= 126.973 MeV
 SigmaEnergy= 1.0 MeV
 SigmaThetaX= 0.1 deg
 SigmaPhiY= 0.1 deg
 SigmaX= 10 mm
 SigmaY= 10 mm
 MeanThetaX= 0 deg
 MeanPhiY= 0 deg
 MeanX= -4 mm
 MeanY= 3 mm

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
TwoBodyReaction
 Beam= 4He
 Target= 1H
 Light= 1H
 Heavy= 4He
 ExcitationEnergy3= 0.0 MeV
 ExcitationEnergy4= 0.0 MeV
 % EXFOR C1210-002: 31 MeV p+4He elastic distribution in direct-p CM angle.
 % NPTool applies theta_CM(recoil p) = 180 deg - theta internally.
 CrossSectionPath= /Users/yemingxin/nptool/Projects/RIBLArray/CrossSection/4He_p_elastic.txt CS4HeP
 ShootLight= 1
 ShootHeavy= 1


root [5] TParameter<int>* seed = (TParameter<int>*)f->Get("RandomSeed");
root [6] seed->GetVal()
(const int) 803735287
root [7]
```

### 2.1 查看 ReactionConditions 分支

```cpp
SimulatedTree->GetBranch("ReactionConditions")->Print();
```

或

```cpp
SimulatedTree->Print("ReactionConditions*");
```

用来看 `ReactionConditions` 分支中的内容。

### 2.2 只列出顶层分支

```text
root [2] SimulatedTree->Print("toponly")
********************************************************************************************************
*Tree    :SimulatedTree: Data created / analysed with the nptool package        *
*Entries :       20 : Total =          164793 bytes  File  Size =      57777 *
*        :          : Tree compression factor =   1.68                       *
********************************************************************************************************
branch: EventID                    104
branch: ReactionSkipped            111
branch: InteractionCoordinates      6820
branch: RIBL                     16141
branch: ReactionChannel            112
branch: InitialConditions         4662
branch: ReactionConditions        6841
branch: TrackInfo                10410
branch: Run                        134
root [11] .!clear
```

### 2.3 ROOT 终端执行 Shell 命令

`.!` 是 ROOT 提供的特殊前缀，意思是：把后面的内容交给系统 shell 执行。

例如：

```text
.! clear     // 清屏
.! pwd       // 显示当前目录
.! ls        // 列出文件
```

### 2.4 查看 ROOT 文件中有哪些 Getter 函数

```cpp
root [1] SimulatedTree->GetBranch("ReactionConditions")->GetClassName()
(const char *) "TReactionConditions"
root [2] TClass::GetClass("TReactionConditions")->GetListOfMethods()->ls()
```
