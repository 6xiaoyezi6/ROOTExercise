alpha-proton弹性散射
使用ClusterSSD与RecoilSSD的击中条带位置坐标进行反应顶点的重建


要先用PreProcess.C代码进行预处理，生成一个只包含有效击中事件的简单ROOT文件,Processd.root
这里需要把一个事件的ClusterSSD的四层击中位置与RecoilSSD的三层击中位置进行匹配，
ClusterSSD的Left应该配对RecoilSSD的RU或RD
ClusterSSD的Right应该配对RecoilSSD的LU或LD

这里ClusterSSD取世界坐标系下的；RecoilSSD取局域坐标系下的；重建的顶点是世界坐标系下的

tree
--ClusterSSDX1
--ClusterSSDZX1
--ClusterSSDY1
--ClusterSSDZY1
--CLusterSSDX2
--CLusterSSDZX2
--ClusterSSDY2
--ClusterSSDZY2

--RecoilSSDX1Locus
--RecoilSSDX2Locus
--RecoilSSDY1Locus

--VertexX
--VertexY
--VertexY


建议进行机器学习之前，先用.C文件进行数据清洗！