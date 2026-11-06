
#include "Getter2.h"
#include <iostream>

void Getter2() {
    // 方法一：普通对象，使用 .
    //普通成员函数（非 static），必须先创建类的对象，再通过对象调用函数
    EventData event;
    std::cout << event.GetEnergy() << std::endl;

    // 方法二：对象指针，使用 ->
    EventData *reader = new EventData();
    std::cout << reader->GetEnergy() << std::endl;
    delete reader;

    // 方法三：静态成员函数，使用 ::
    ////静态成员函数（static），属于类本身，不需要创建对象，可以直接通过类名调用
    std::cout << TTwoBody::Energy(150.0) << std::endl;
}
