# AI-Checkers
AI Checkers for class project.  
AI跳棋 课程项目

# Version
* 0.1.0.21w18a (2021-5-9)  
基础框架，基于Baseline，完善AI，现在AI知道自己应该走哪个子了。Bug：AI只有在白子时会吃到黑子，黑子时会吃队友。
* 0.1.1.21w19a (2021-5-10)  
修复AI吃自己子的bug，可以进行正常对局（应该）。  
* 1.0.0.21w19b (2021-5-11)  
AI现在会随机落子了；输出信息全部加上了debug，正式提交第一版（虽然战绩惨不忍睹）。  
* 1.0.1.21w19c (2021-5-12)  
修复了AI在随机落子时，没有进行最多吃子的bug，现在无论如何都会对每一棋子进行一次判断，但有多个最长时，以随机到的第一个为准。  
* 1.0.1.21w19d (2021-5-14)  
将己方的所有步骤存储起来，待日后判断。  
* 1.1.0.21w19e (2021-5-16)  
暂时停用随机落子功能，AI现在会进行第一层的搜索。支持输入棋盘功能。修复了到达底线不升王的bug。修复了对方王吃我方棋子时程序崩溃的bug。  
* 1.2.0.21w19f (2021-5-16)  
完善王的行动逻辑，现在王会正常吃子。  
* 1.3.0.21w20a (2021-5-21)  
添加评估功能，将第一层搜索的结果用局面评估代替吃子数。修复了王吃两个以上子会消失的问题。  

# License
This project is open source with Apache-2.0 License, for more detail, please view LICENSE file.  
本项目开源协议为Apache-2.0，更多细节请查阅LICENSE文件.  
**Copyright (c) 2021 LPH**  
**版权所有 2021 LPH**  

![lphlch](https://github.com/lphlch/AI-Checkers/blob/main/lphlch.png)
