#include <iostream>

int main()
{
    int a = 10;
    int *p = &a;//定义一个指针p，指向a的地址

    std::cout << a << std::endl;
    std::cout << &a << std::endl;
    //&a：获取变量的内存地址,例如0x16bd4ab8c
    
    std::cout << p << std::endl;
    std::cout << *p << std::endl;
    //p是指针p保存的地址
    //*p是地址中存储的值


    return 0;
}