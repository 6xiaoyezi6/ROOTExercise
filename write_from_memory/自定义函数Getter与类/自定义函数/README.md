## C++ 类与成员函数的定义和调用

### 1. 定义类

`Getter.C` 代码如下：

```cpp
class EventData 
{
private:
    Double_t Energy = 150.0;

public:
    Double_t GetEnergy() {return Energy;}
};
```

### 2. ROOT 中的使用方法

```cpp
root [0] .L Getter.C
root [1] EventData event;
root [2] event.GetEnergy()
(double) 150.00000
```

### 3. 代码说明

- `EventData`：类（Class）。
- `EventData event;`：创建 `EventData` 类的对象（Object）`event`。
- `Energy`：私有成员变量（Member Variable），初始值为 `150.0`。
- `GetEnergy()`：公有成员函数（Member Function），用于返回 `Energy` 的值。
- `event.GetEnergy()`：通过 `.` 运算符调用对象 `event` 的成员函数。

NPLL::TTwoBody::Energy(150.0)可以分成三部分：
命名空间::类::静态成员函数