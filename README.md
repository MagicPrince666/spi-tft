# spi-tft
* driver for spi tft base on openwrt
#
<br> 基于widora NEO移植的2.8寸TFT例程，驱动接口为spi
<br> 实现ili9341液晶驱动，图片解码显示，电阻触摸屏，中文字库显示等功能
<br> 接口说明 widora --> tft
<br>         IO0(11) -- DC
<br>         CS1 ------ CS
<br>         MOSI ----- SDI
<br>         CK ------- SCK
<br>         MISO ----- SDO
<br>         IO14 ----- T_CS
<br>         IO15 ----- T_CLK
<br>         IO17 ----- T_DIN
<br>         IO16 ----- T_DO
<br>         IO41 ----- T_IRQ
<br>
<br> 另外还外接一个按键
<br> IO39 ---- key

<br> 为照顾初学者，创建simple分支，精简化的代码加入simple分支，去出中文，去除图片等功能
<br> 只留下基本的显示和触摸功能，如果不需要触摸功能，可以在代码里面去掉

<br>
<br>码农不易 尊重劳动
<br>作者：小王子与木头人
<br>功能：linux驱动 spi TFT触摸屏
<br>QQ：846863428 
<br>TEL: 15220187476 
<br>email: huangliquanprince@icloud.com 
<br>修改时间 ：2018-05-17

# Donation

If you find my work useful and you want to encourage the development of more free resources, you can do it by donating… 
觉得不错给鼓励一下

[![alipay](https://github.com/MagicPrince666/spi-tft/raw/master/picture/alipay.jpg)]