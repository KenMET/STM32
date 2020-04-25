#include "run.h" 


int main(void)
{
#ifdef SYSTEM_SUPPORT_OS
	unsigned char avoid_warrning = 1;
	OS_ERR err;
#endif

	hardware_init();

#ifdef SYSTEM_SUPPORT_OS
	create_init_task();
	while(avoid_warrning)
	{
		OSTimeDlyHMSM(0,0,0,1000,OS_OPT_TIME_HMSM_STRICT,&err);
	}
#else
	run();
#endif
	return 0;
}
