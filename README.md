# tmp_project
tmp project   in development
## for the raspberry pi part
need to close the uart of BLE.


```

1、在树莓派配置中打开串口开关

    sudo raspi-config

Expand filesystem  --------- advanced --------- serial(enable)

重启树莓派
2、树莓派3的蓝牙与串口是冲突的，只能2选1，系统默认是选择蓝牙，现在需要关闭蓝牙

 1）打开文件

    sudo nano /boot/config.txt

2）在文件的最后添加

    dtoverlay=pi3-disable-bt

该语句是关闭蓝牙 ，然后按下Ctrl+X, 再按 Y 保存文件 。（这句是给菜鸟用的，老鸟可无视~~~） 

3）重启树莓派

后发现任务栏上的蓝牙图标变灰色证明已经关闭蓝牙，（原来是蓝色的）
3、树莓派3的串口会用于用户登录，所以我们要关闭用户登录，保证树莓派串口只和我们的设备通讯，避免数据错乱。

1）关闭串口用户登录，打开文件

    sudo nano /boot/cmdline.txt

你会看到两种情况：
①第一种

    dwc_otg.lpm_enable=0 console=tty1 console=serial0(or ttyAMA0),115200 root=/dev/mmcblk0p2 rootfstype=ext4 elevator=deadline

    fsck.repair=yes rootwait

只删除以下字眼，其他不需要修改

    console=serial0(or ttyAMA0),115200

②第二种

    dwc_otg.lpm_enable=0 console=tty1 console=serial0,115200  console=ttyAMA0，115200 root=/dev/mmcblk0p2 rootfstype=ext4 elevator=deadline fsck.repair=yes rootwait

只删除以下字眼，其他不需要修改

    console=serial0,115200 console=ttyAMA0,115200

4、重启，完成！
 总而言之，是蓝牙冲突问题和用户串口登录问题导致树莓派串口失效。



```
