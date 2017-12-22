#include "pwmconfig.h"

/* PWM export */
int pwm_export(unsigned int pwm)
{
	int fd;

	switch(pwm){
	case 1:{
		fd = open(SYSFS_PWM_DIR "/pwmchip0/export", O_WRONLY);
		if (fd < 0) {
			printf ("\nFailed export PWM_A\n");
			return -1;
		}

		write(fd, "0", 2);
		close(fd);
		return 0;
		break;
	}
	case 2:{
		fd = open(SYSFS_PWM_DIR "/pwmchip0/export", O_WRONLY);
		if (fd < 0) {
			printf ("\nFailed export PWM_B\n");
			return -1;
		}

		write(fd, "1", 2);
		close(fd);
		return 0;
		break;
	}
	case 3:{
		fd = open(SYSFS_PWM_DIR "/pwmchip0/export", O_WRONLY);
		if (fd < 0) {
			printf ("\nFailed export PWM_C\n");
			return -1;
		}

		write(fd, "2", 2);
		close(fd);
		return 0;
		break;
	}
	case 4:{
		fd = open(SYSFS_PWM_DIR "/pwmchip0/export", O_WRONLY);
		if (fd < 0) {
			printf ("\nFailed export PWM_D\n");
			return -1;
		}

		write(fd, "3", 2);
		close(fd);
		return 0;
		break;
	}
	default:{
		printf("wrong PWM channel input\n");
		return -1;
	}
	}

}

/* PWM unexport */
int pwm_unexport(unsigned int pwm)
{
	int fd;

	switch(pwm){
	case 1:{
		fd = open(SYSFS_PWM_DIR "/pwmchip0/unexport", O_WRONLY);
		if (fd < 0) {
			printf ("\nFailed unexport PWM_A\n");
			return -1;
		}

		write(fd, "0", 2);
		close(fd);
		return 0;
        break;
	}
	case 2:{
		fd = open(SYSFS_PWM_DIR "/pwmchip0/unexport", O_WRONLY);
		if (fd < 0) {
			printf ("\nFailed unexport PWM_B\n");
			return -1;
		}

		write(fd, "1", 2);
		close(fd);
		return 0;
		break;
	}
	case 3:{
		fd = open(SYSFS_PWM_DIR "/pwmchip0/unexport", O_WRONLY);
		if (fd < 0) {
			printf ("\nFailed unexport PWM_C\n");
			return -1;
		}

		write(fd, "2", 2);
		close(fd);
		return 0;
		break;
	}
	case 4:{
		fd = open(SYSFS_PWM_DIR "/pwmchip0/export", O_WRONLY);
		if (fd < 0) {
			printf ("\nFailed unexport PWM_D\n");
			return -1;
		}

		write(fd, "3", 2);
		close(fd);
		return 0;
		break;
	}
	default:{
		printf("wrong PWM channel input\n");
		return -1;
	}
	}
}

/* PWM configuration */
int pwm_config(unsigned int pwm, unsigned int period, unsigned int duty_cycle)
{
	int fd,len_p,len_d;
	char buf_p[MAX_BUF];
	char buf_d[MAX_BUF];

	len_p = snprintf(buf_p, sizeof(buf_p), "%d", period);
	len_d = snprintf(buf_d, sizeof(buf_d), "%d", duty_cycle);

	switch(pwm){
	case 1:{
			/* set pwm period */
		    fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm0/period", O_WRONLY);
			if (fd < 0) {
				printf ("\nFailed set PWM_A period\n");
				return -1;
			}

			write(fd, buf_p, len_p);
			/* set pwm duty cycle */
			fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm0/duty_cycle", O_WRONLY);
			if (fd < 0) {
				printf ("\nFailed set PWM_A duty cycle\n");
				return -1;
			}

			write(fd, buf_d, len_d);

			close(fd);
			return 0;
			break;
		}
		case 2:{
			/* set pwm period */
			fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm1/period", O_WRONLY);
			if (fd < 0) {
				printf ("\nFailed set PWM_B period\n");
				return -1;
			}

			write(fd, buf_p, len_p);
			/* set pwm duty cycle */
			fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm1/duty_cycle", O_WRONLY);
			if (fd < 0) {
				printf ("\nFailed set PWM_B duty cycle\n");
				return -1;
			}

			write(fd, buf_d, len_d);

			close(fd);
			return 0;
			break;
		}
		case 3:{
			/* set pwm period */
			fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm2/period", O_WRONLY);
			if (fd < 0) {
				printf ("\nFailed set PWM_C period\n");
				return -1;
			}

			write(fd, buf_p, len_p);
			/* set pwm duty cycle */
			fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm2/duty_cycle", O_WRONLY);
			if (fd < 0) {
				printf ("\nFailed set PWM_C duty cycle\n");
				return -1;
			}

			write(fd, buf_d, len_d);

			close(fd);
			return 0;
			break;
		}
		case 4:{
			/* set pwm period */
			fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm3/period", O_WRONLY);
			if (fd < 0) {
				printf ("\nFailed set PWM_D period\n");
				return -1;
			}

			write(fd, buf_p, len_p);
			/* set pwm duty cycle */
			fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm3/duty_cycle", O_WRONLY);
			if (fd < 0) {
				printf ("\nFailed set PWM_D duty cycle\n");
				return -1;
			}

			write(fd, buf_d, len_d);

			close(fd);
			return 0;
			break;
		}
		default:{
			printf("wrong PWM channel input\n");
			return -1;
		}
		}
}

/* PWM enable */
int pwm_enable(unsigned int pwm)
{
	int fd;

	switch(pwm){
	case 1:{
		    fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm0/enable", O_WRONLY);
			if (fd < 0) {
				printf ("\nFailed enable PWM_A\n");
				return -1;
			}
			write(fd, "1", 2);

			close(fd);
			return 0;
			break;
		}
		case 2:{
			fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm1/enable", O_WRONLY);
			if (fd < 0) {
				printf ("\nFailed enable PWM_B\n");
				return -1;
			}
			write(fd, "1", 2);

			close(fd);
			return 0;
			break;
			}
		case 3:{
			fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm2/enable", O_WRONLY);
			if (fd < 0) {
				printf ("\nFailed enable PWM_C\n");
				return -1;
			}
			write(fd, "1", 2);

			close(fd);
			return 0;
			break;
			}
		case 4:{
			fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm3/enable", O_WRONLY);
			if (fd < 0) {
				printf ("\nFailed enable PWM_D\n");
				return -1;
			}
			write(fd, "1", 2);

			close(fd);
			return 0;
			break;
		}
		default:{
			printf("wrong PWM channel input\n");
			return -1;
		}
		}
}

/* PWM disable */
int pwm_disable(unsigned int pwm)
{
	int fd;

		switch(pwm){
		case 1:{
			    fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm0/enable", O_WRONLY);
				if (fd < 0) {
					printf ("\nFailed disable PWM_A\n");
					return -1;
				}
				write(fd, "0", 2);

				close(fd);
				return 0;
				break;
			}
			case 2:{
				fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm1/enable", O_WRONLY);
				if (fd < 0) {
					printf ("\nFailed disable PWM_B\n");
					return -1;
				}
				write(fd, "0", 2);

				close(fd);
				return 0;
				break;
				}
			case 3:{
				fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm2/enable", O_WRONLY);
				if (fd < 0) {
					printf ("\nFailed disable PWM_C\n");
					return -1;
				}
				write(fd, "0", 2);

				close(fd);
				return 0;
				break;
				}
			case 4:{
				fd = open(SYSFS_PWM_DIR "/pwmchip0/pwm3/enable", O_WRONLY);
				if (fd < 0) {
					printf ("\nFailed disable PWM_D\n");
					return -1;
				}
				write(fd, "0", 2);

				close(fd);
				return 0;
				break;
			}
			default:{
				printf("wrong PWM channel input\n");
				return -1;
			}
			}
}