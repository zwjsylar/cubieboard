cubieboard
==========
自己玩cubieboard写的代码

bluetooth中为蓝牙测试demo:检测周围的蓝牙设备，连接设备，循环发送数据

broadcom-bluetooth：固件源代码，以及加载固件的命令

com：cubieboard从串口读取数据，再通过tcp发送
  特点：epoll
com_bt：cubieboard从串口读取数据，通过tcp和蓝牙发送
  特点：epoll 线程 信号
