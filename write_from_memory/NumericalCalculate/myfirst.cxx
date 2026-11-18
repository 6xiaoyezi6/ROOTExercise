#include <iostream>

int main()
{
    using namespace std;
    cout << "Come up" <<endl;

    char character = 'Z';
    cout << static_cast<int>(character)<<endl;
    //强制类型转换，把字符串转换为整型

    return 0;
}