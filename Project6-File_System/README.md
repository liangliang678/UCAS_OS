# Project6: File System

## 实验说明
### 开发环境
本实验用到的RISC-V开发板是XILINX PYNQ Z2开发板，开发板上有ARM核和FPGA，RISC-V核烧写到FPGA中。在开发板上电时由ARM核启动相关程序(BOOT.bin)，将RISC-V核烧入FPGA。之后，RISC-V核自动加载相关程序。RISC-V核为采用SiFive开源的Rocket核心。其基本信息为：
1. RV64IMAFDC(GC)指令集
2. 双核五级流水处理器
3. 数据/指令缓存大小各为16KB，4路组相联
4. 时钟频率为100MHz

另外我们需要一张SD卡。将SD卡格式化为两个分区：第一个分区34MB，采用fat32文件系统；剩余空间全部划入第二个分区，保持第二个分区为空分区。制作完以后，将预先提供的BOOT.bin和RV_BOOT.bin拷入到第一个分区。

### 运行方法
拿到裸开发板以后，需要将开发板按如下方式配置：
1. 将开发板设置为SD卡启动。PYNQ Z2支持从SD、QSPI和JTAG启动。我们将跳线插到最左侧，选择用SD卡启动。
2. 设置电源选项。PYNQ Z2支持电源供电和Micro-USB供电。我们把跳线插到USB这个选项上，用Micro-USB直接供电。

配置完开发板后：
1. 将SD卡插入电脑中，将Makefile中的DISK的路径改为SD卡所在的路径。
2. 执行make all命令和make floppy命令，将内核镜像拷贝到SD卡中。
3. 将SD卡插入开发板中，使用Micro-USB线连接电脑和开发板。该线同时兼顾供电和传输数据的功能。
4. 打开开发板电源开关，开发板上会有红色和绿色的指示灯亮起，表明开发板已开始工作。
5. 使用minicom连接开发板，然后按下开发板的RESET按键。
6. 在minicom中出现bbl字样后，输入loadbootm以双核方式启动操作系统。

---
## 内核部分说明
### 相关目录结构
| 文件夹 | 说明 |
| :--- | :--- |
| arch/riscv | 体系结构相关代码 |
| drivers | 驱动代码 |
| include | 头文件 |
| init | 初始化代码 |
| kernel | 内核主要文件 |
| libs | 内核使用的库 |

### 文件系统
本操作系统的文件系统实现包括超级块、i-node bitmap、block bitmap、i-node、数据区，共有4096个i-node和1GB的数据区，数据区中一个block为4KB。在shell中文件系统相关的命令包括mkfs、statfs、mkdir、rmdir、ls、cd、touch、cat、ln。文件系统的主要文件在kernel/fs/中：disk.c中包括内存和磁盘的数据交换操作；fs.c中包括文件系统的基本操作；file.c中包括文件的相关操作。

磁盘的布局请参考disk.h中的注释和宏定义。alloc_inode可以申请一个空闲的i-node并返回空闲i-node的id，如果返回0xffff则申请失败；free_inode可以将id为inode_id的i-node释放。alloc_block可以申请一个空闲的block并返回空闲block的id，如果返回0xffffffff则申请失败；free_block可以将id为block_id的block释放。disk.c中的read函数负责将相应的磁盘块读取到内存相应区域中，write函数负责将内存相应区域中的内容写到磁盘中。读写i-node和block时要提供相应的id，内存中缓存的i-node的id记录在cached_inode_id中，缓存的block的id记录在cached_block_id中。另外，因为一个扇区包括8个i-node，read_inode会返回i-node在内存中的地址。

磁盘块对应的缓存信息、超级块、i-node、目录项的数据结构请参考fs.h。初始化文件系统的流程包括检查超级块的magic，清空block map和i-node map，设置根目录，设置超级块。mkdir，rmdir需要解析路径到倒数第二级目录，然后进行操作；ls，cd需要解析路径到最后一级目录。硬链接文件直接使用目标文件的i-node；软链接文件有单独的i-node，但是使用目标文件的block，此处与linux的做法略有不同，这一点造成删除软链接文件不会影响原文件，但删除原文件后使用软链接文件可能造成错误。

文件描述的数据结构请参考file.h，文件描述使用内核中的全局数组，由所有的进程共用。do_touch函数先解析目录，然后申请i-node并修改倒数第二级目录。do_cat函数先解析目录，然后将文件第一个扇区的内容拷贝到buffer中。do_fopen函数先解析路径，然后申请并初始化文件描述。do_fread函数将文件的内容拷贝到buffer中，要注意间址块的情况。do_fwrite函数先申请需要的block，然后将buffer的内容写到文件中。do_fclose函数释放相应的文件描述。

---
## 用户部分说明
### shell支持的命令
| 命令 | 参数 | 说明 |
| :---: | :---: | :--- |
| ps |  无 | 显示所有进程的pid，状态，MASK和运行在哪个核上 |
| reset | 无 | 清空下半屏（命令行） |
| clear | 无 | 清空上半屏（用户输出） |
| exec | name | 将名称为name的任务启动 |
| kill | pid | 杀死进程号为pid的进程 |
| taskset | -p mask pid | 设置进程号为pid的进程的MASK |
| mkfs | (-f) | （强制）初始化文件系统 | 
| statfs | 无 | 打印文件系统相关信息 |
| mkdir | path | 新建目录 | 
| rmdir | path | 删除目录 | 
| ls | -k或(-al) (path) | -k表示打印内核镜像中的可执行文件；否则打印目录中的所有内容，-al表示打印详细信息，不输入path表示当前目录 |  
| cd | path | 更改目录 | 
| touch | path | 新建文件 | 
| cat | path | 打印所选文件内容 | 
| ln |  (-s) source file | 将source(软)硬链接到file |

注：MASK=1表示进程被允许运行在core 0；MASK=2表示进程被允许运行在core 1；MASK=3表示进程被允许运行在core 0和core 1。
