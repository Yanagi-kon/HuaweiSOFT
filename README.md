货物 ratio： 价值 val + 直线距离 distance

泊位 ratio：装卸速度 velocity + time (轮船到虚拟点的时间) + 直线距离 distance



$goodsRatio=Val / distance $

$berthRatio = 1000 * Velocity / (time + distance) $



My goods 为空的时候，选择第二近的港口且有货物的港口，让它的机器人 A 从mygoods 队头（队尾）弹出一个货物分给mygoods为空的机器人，改变这个mygoods为空的机器人的港口目标。

如果原先的港口有n个货物了，这些货物还分配给这个机器人A，并让机器人A的目标港口改变回原先的港口。