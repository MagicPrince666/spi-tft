#include "pwmconfig.h"


int pwm_init()
{
    int MyPeriod = 20000000; //period 设置 1s
    int MyDuty =    1500000;

	/*export corresponding PWM Channel*/
	if(pwm_export(2) < 0){
		printf("PWM 2 export error!\n");
		return(-1);
	}
	if(pwm_disable(2) < 0){
		printf("PWM 2 disable error!\n");
		return(-1);
	}
	/* set period and duty cycle time in ns */
	if(pwm_config(2, MyPeriod, MyDuty) < 0){
		printf("PWM 2 configure error!\n");
		return(-1);
	}
	/* enable corresponding PWM Channel */
	if(pwm_enable(2) < 0){
		printf("PWM 2 enable error!\n");
		return(-1);
	}
	//printf("PWM_B successfully enabled with period - %dus, duty cycle - %dms\n", MyPeriod/1000, MyDuty/1000);

	if(pwm_export(3) < 0){
		printf("PWM 3 export error!\n");
		return(-1);
	}
	if(pwm_disable(3) < 0){
		printf("PWM 3 disable error!\n");
		return(-1);
	}
	/* set period and duty cycle time in ns */
	if(pwm_config(3, MyPeriod, MyDuty) < 0){
		printf("PWM 3 configure error!\n");
		return(-1);
	}
	/* enable corresponding PWM Channel */
	if(pwm_enable(3) < 0){
		printf("PWM 3 enable error!\n");
		return(-1);
	}
	//printf("PWM_C successfully enabled with period - %dus, duty cycle - %dus\n", MyPeriod/1000, MyDuty/1000);

	
	return 0;
}