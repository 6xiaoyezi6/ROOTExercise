类：描述一种类型，是模板。
对象：根据类创建的具体实例。
继承：一个类从另一个类获得成员，并扩展或修改功能。

类与对象+类与继承

### 1.类与对象
（1）构造函数：在创建对象时，自动的进行初始化工作
语法：类名(){...},函数名必须与类名相同，没有返回值不写void
（2）析构函数：在销毁对象前，自动的完成清理工作
语法：~类名(){...}

使用Getter和Setter函数

# Getter 函数练习

这个文件夹使用 `run0291_BDC.root` 和 `run0291_ESPRI.root` 练习 C++ 类、Getter 函数以及 ROOT `TTree` 的事件读取。

## GetterBDC.h 和 GetterBDC.C

这组文件用于练习不带参数的 Getter 函数。

例如：

```cpp
Double_t GetBDC1ThetaX() { return BDC1_ThetaX; }
Double_t GetBDC1X() { return BDC1_X; }
```

每个 Getter 对应一个确定的成员变量，调用时不需要传入参数。`GetterBDC.C` 先调用 `GetEntry(entry)` 读取事件，再调用 `GetBDC1ThetaX()` 获得当前事件的 `BDC1_ThetaX`。

## GetterRDC.h 和 GetterRDC.C

这组文件用于练习带参数的 Getter 函数。

`rdcA`、`rdcB`、`rdcX` 和 `rdcY` 都是长度为 2 的数组，可以通过 `side` 参数选择数组元素：

```cpp
Double_t GetrdcA(Int_t side) { return rdcA[side]; }
```

例如：

```cpp
reader->GetrdcA(0);  // 左侧 rdcA
reader->GetrdcA(1);  // 右侧 rdcA
```

同时定义了不带参数的左右 Getter，用来和带参数的 Getter 进行对比：

```cpp
Double_t GetLeftrdcA() { return rdcA[0]; }
Double_t GetRightrdcA() { return rdcA[1]; }
```

## GetterBDCRDC.h 和 GetterBDCRDC.C

这组文件用于练习 C++ 类的继承，重点是多重继承。

`GetterBDCRDC.h` 是一个独立头文件，它没有包含 `GetterBDC.h` 或 `GetterRDC.h`，而是在文件内部定义了两个基类：

```cpp
GetterBDCBase
GetterRDCBase
```

派生类同时继承这两个基类：

```cpp
class GetterBDCRDCReader : public GetterBDCBase, public GetterRDCBase
```

因此，`GetterBDCRDCReader` 可以同时使用 BDC Getter 和 RDC Getter。

它的构造函数通过初始化列表调用两个基类的构造函数：

```cpp
GetterBDCRDCReader(const char *bdcFileName, const char *rdcFileName)
    : GetterBDCBase(bdcFileName), GetterRDCBase(rdcFileName) {}
```

派生类的 `GetEntry(entry)` 会使用相同的 entry 编号读取 BDC 和 RDC：

```cpp
void GetEntry(Long64_t entry) {
  GetBDCEntry(entry);
  GetRDCEntry(entry);
}
```

`GetterBDCRDC.C` 只包含 `GetterBDCRDC.h`。它使用一个 `3×2` 画布显示 BDC1 ThetaX 和四个 RDC A Getter 的结果。

## 三组文件之间的关系

- `GetterBDC.h/.C`：练习不带参数的 Getter。
- `GetterRDC.h/.C`：练习带参数的 Getter。
- `GetterBDCRDC.h/.C`：独立练习基类、派生类和多重继承。

三组练习可以分别阅读和使用。`GetterBDCRDC.h/.C` 不依赖前两组头文件或宏文件。
