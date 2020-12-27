# Project5: Device Driver

## 实验说明
### 开发环境
本实验用到的RISC-V开发板是XILINX PYNQ Z2开发板，开发板上有ARM核和FPGA，RISC-V核烧写到FPGA中。
在开发板上电时由ARM核启动相关程序(BOOT.bin)，将RISC-V核烧入FPGA；
之后，RISC-V核自动加载相关程序。RISC-V核为采用SiFive开源的Rocket核心。其基本信息为：
1. RV64IMAFDC(GC)指令集
2. 双核五级流水处理器
3. 数据/指令缓存大小各为16KB，4路组相联
4. 时钟频率为50MHz

另外我们需要一张SD卡。将SD卡格式化为两个分区：第一个分区34MB，采用fat32文件系统；
剩余空间全部划入第二个分区，保持第二个分区为空分区。
制作完以后，将预先提供的BOOT.bin和RV_BOOT.bin拷入到第一个分区。

### 运行方法
拿到裸开发板以后，需要将开发板按如下方式配置：
1. 将开发板设置为SD卡启动。PYNQ Z2支持从SD、QSPI和JTAG启动。我们将跳线插到最左侧，选择用SD卡启动。
2. 设置电源选项。PYNQ Z2支持电源供电和Micro-USB供电。我们把跳线插到USB这个选项上，用Micro-USB直接供电。

配置完开发板后：
1. 将SD卡插入电脑中，将Makefile中的DISK的路径改为SD卡所在的路径。
2. 执行make all和make floppy，将内核镜像拷贝到SD卡中。
3. 将SD卡插入开发板中，使用Micro-USB线连接电脑和开发板。该线同时兼顾供电和传输数据的功能。
4. 使用网线连接电脑和开发板。
5. 打开开发板电源开关，开发板上会有红色和绿色的指示灯亮起，表明开发板已开始工作。
6. 在电脑中使用minicom连接开发板，然后按下开发板的RESET按键。
7. 在minicom中出现bbl字样后，输入loadbootm以双核方式启动操作系统。