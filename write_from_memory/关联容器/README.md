方法	定义方式	推荐程度
map + struct	map<ID, EventData>	最推荐
map + tuple	map<ID, tuple<E, θ, Ex>>	适合简单数据


#include <iostream>
#include <map>
#include "RtypesCore.h"

struct EventData {
    Double_t Energy;
    Double_t Theta;
    Double_t Ex;
};

void TestMap() {
    std::map<Int_t, EventData> data;

    // 插入数据
    data[1] = {100.5, 55.2, 3.368};
    data[2] = {120.3, 60.5, 6.179};
    data[3] = {135.8, 65.1, 7.540};

    // 根据 ID 读取数据
    std::cout << data.at(2).Energy << std::endl;
    std::cout << data.at(2).Theta << std::endl;
    std::cout << data.at(2).Ex << std::endl;

    // 遍历全部数据
    for (const auto& [id, event] : data) {
        std::cout << "ID = " << id
                  << ", Energy = " << event.Energy
                  << ", Theta = " << event.Theta
                  << ", Ex = " << event.Ex << std::endl;
    }
}

### 完全可以直接用结构体代替，没有必要用std::map了！！！！！！