alpha-proton弹性散射
使用TOGAXSI的前向部分，ClusterSSD+ClusterGAGG+厚靶
看MLP能否学到ClusterGAGG的能量+DeltaEGAGG的能量+ClusterSSD的世界坐标系下的击中位置+反应顶点XYZ到Cluster再反应顶点处的动能KinematicsAtVertex，单位Mev

要先用PreProcess.C代码进行预处理，生成一个只包含有效击中事件的简单ROOT文件,Processd.root
tree
--ClusterGAGGEnergy ,Mev
--DeltaEGAGGEnergy,Mev
--ClusterSSDX1
--ClusterSSDY1
--CLusterSSDX2
--ClusterSSDY2
--VertexX
--VertexY
--VertexY

--KinematicsAtVertex,Mev

实际上就是学习alpha在靶子和SSD中的能损

注意这里是使用SSD的真实位置，而不是使用的是条带位置;SSD文件夹内是条带位置

建议进行机器学习之前，先用.C文件进行数据清洗！