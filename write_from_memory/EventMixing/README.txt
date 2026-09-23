一、归一化
EventMixing主要给出本底分布的形状，不能直接给出真实本底的绝对计数，因此通常需要归一化。
归一化只能改变MixedEvent谱的整体高度，不能修正错误的本底形状。

二、事件池的选取
EventMixingExToy_1.C的事件池为“真实信号S+真实本底B”。
因此EventMixing后会出现SS、SB、BS、BB四类非关联组合。
其中只有BB的单粒子分布与当前代码中定义的“真实本底”一致。
SS、SB、BS虽然也属于非关联组合，但会受到signal单粒子分布的污染，因此其重建出的\(E_x\)形状与真实本底不同。
所以EventMixingExToy_1.C中MixedEvent不能很好复现TrueBackground。

EventMixingExToy_2.C的事件池只包含真实本底B。
因此EventMixing只产生BB组合。
由于当前toyMC中本底的\(^{6}\mathrm{He}\)和\(\alpha\)本来就是彼此独立的，BB Mixing可以很好复现真实本底。

注意：上述“只有BB才是真实本底”的说法仅适用于当前toyMC对本底的定义。
实际实验中并不能直接知道哪些事件是signal、哪些是background，因此事件池的合理选取是EventMixing中非常关键的一步。

要人为筛选出进入Event Mixing"事件池"的事件.
事件池的选择通常依据能够表征实验条件和事件类别的可观测量。
常见变量包括反应顶点位置 z_vertex、事件 multiplicity、束流能量 E_beam、粒子或母核出射角 theta 以及 run period。
这些变量会影响粒子的单粒子动量分布、探测器接受度、能损和探测效率。
Event Mixing 通常只在这些条件相同或相近的事件之间进行。
事件池过宽时，不同实验条件会被混在一起，可能导致错误的本底形状。
事件池过窄时，可用于 Mixing 的事件数太少，统计误差会增大。
因此实际分析中需要在“事件条件相似”和“足够统计量”之间取得平衡。
事件池选择的目标是让 Mixed Event 的单粒子相空间尽可能接近真实组合本底。