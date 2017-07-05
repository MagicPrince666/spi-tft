/*****************************************************************************************

 * 本程序只供学习使用，未经作者许可，不得用于其它任何用途 *
 * 文件名  pwmconfig.h
 * 描述    ：
 * 平台    ：linux
 * 版本    ：V1.0.0
 * 作者    ：小王子与木头人  QQ：846863428
 *修改时间  ：2017-07-4

*****************************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>


/*** constants ***/
#define SYSFS_PWM_DIR "/sys/class/pwm"
#define MAX_BUF 64

/*** PWM functions ***/
/* PWM export */
int pwm_export(unsigned int pwm);
/* PWM unexport */
int pwm_unexport(unsigned int pwm);
/* PWM configuration */
int pwm_config(unsigned int pwm, unsigned int period, unsigned int duty_cycle);
/* PWM enable */
int pwm_enable(unsigned int pwm);
/* PWM disable */
int pwm_disable(unsigned int pwm);

int pwm_init();