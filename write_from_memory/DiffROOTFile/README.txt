用于练习合并分支不同的ROOT文件

情况1：事件数目相同===========================================
FileA.root
└── Events
    ├── EventID
    ├── x
    └── y = 2*x

FileB.root
└── Events
    ├── EventID
    ├── z = 2*y = 4*x


两个文件都有10个事件，可以按照EventID进行对齐

情况2：事件数目不同===========================================
A EventID:
0 1 2 3 4 5 6 7 8 9

B EventID:
1 3 5 7 9 

情况3:分支各不相同，且事件数目也会丢失============================
A EventID:
0 1 2 3 4 5 6 7 8 9
FileA.root
└── Events
    ├── EventID
    ├── x = i
    └── y = 2*i
    |__ t = 3*i

B EventID:
1 3 5 7 9 
FileB.root
└── Events
    ├── EventID
    ├── z = 4*i
    |__ u = 5*i


这三个case属于横向拼接，合并前后Entry数目不变==========================================
TreeA:
EventID   x   y
0         1   2
1         3   4
2         5   6
TreeB:
EventID   z
0         10
1         20

如果按TreeA的EventID 把同一个事件合并：
EventID   x   y   z
0         1   2   10
1         3   4   20
2         5   6   -999

纵向拼接======================================================
TreeA:
EventID   x   y
0         1   2
1         3   4
2         5   6
TreeB:
EventID   z
0         10
1         20

把两个 Tree 的事件依次写入同一个输出 Tree
EventID   x      y      z
0         1      2     -999
1         3      4     -999
2         5      6     -999
0        -999   -999    10
1        -999   -999    20


情况4:纵向拼接，合并后Entry数目等于输入的Entry的和=================================================
Case3_1.root和Case3_2.root拼接起来

横向拼接不增加Entry数，所以是一个for循环；横向拼接要增加Entry数，所以是两个for循环