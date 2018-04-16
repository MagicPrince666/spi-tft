# spi-tft
# driver for spi tft base on openwrt
#
# 基于widora NEO移植的2.8寸TFT例程，驱动接口为spi
# 实现ili9341液晶驱动，图片解码显示，电阻触摸屏，中文字库显示等功能
# 接口说明 widora --> tft
#         IO0(11) -- DC
#         CS1 ------ CS
#         MOSI ----- SDI
#         CK ------- SCK
#         MISO ----- SDO
#         IO14 ----- T_CS
#         IO15 ----- T_CLK
#         IO17 ----- T_DIN
#         IO16 ----- T_DO
#         IO41 ----- T_IRQ
#
# 另外还外接一个按键
# IO39 ---- key