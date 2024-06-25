#include "stdio.h"
#define GPKCON0     		(*((volatile unsigned long *)0x7F008800))
#define GPKDATA     			(*((volatile unsigned long *)0x7F008808))
#define GPNCON     			(*((volatile unsigned long *)0x7F008830))
#define GPNDAT     			(*((volatile unsigned long *)0x7F008834))

#define EINT0CON0  			(*((volatile unsigned long *)0x7F008900))
#define EINT0MASK  			(*((volatile unsigned long *)0x7F008920))
#define EINT0PEND  			(*((volatile unsigned long *)0x7F008924))
#define PRIORITY 	    	(*((volatile unsigned long *)0x7F008280))
#define SERVICE     		(*((volatile unsigned long *)0x7F008284))
#define SERVICEPEND 		(*((volatile unsigned long *)0x7F008288))
#define VIC0IRQSTATUS  		(*((volatile unsigned long *)0x71200000))
#define VIC0FIQSTATUS  		(*((volatile unsigned long *)0x71200004))
#define VIC0RAWINTR    		(*((volatile unsigned long *)0x71200008))
#define VIC0INTSELECT  		(*((volatile unsigned long *)0x7120000c))
#define VIC0INTENABLE  		(*((volatile unsigned long *)0x71200010))
#define VIC0INTENCLEAR 		(*((volatile unsigned long *)0x71200014))
#define VIC0PROTECTION 		(*((volatile unsigned long *)0x71200020))
#define VIC0SWPRIORITYMASK 	(*((volatile unsigned long *)0x71200024))
#define VIC0PRIORITYDAISY  	(*((volatile unsigned long *)0x71200028))
#define VIC0ADDRESS        	(*((volatile unsigned long *)0x71200f00))

#define		PWMTIMER_BASE			(0x7F006000)
#define		TCFG0    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x00)) )
#define		TCFG1    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x04)) )
#define		TCON      	( *((volatile unsigned long *)(PWMTIMER_BASE+0x08)) )
#define		TCNTB0    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x0C)) )
#define		TCMPB0    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x10)) )
#define		TCNTO0    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x14)) )
#define		TCNTB1    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x18)) )
#define		TCMPB1    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x1C)) )
#define		TCNTO1    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x20)) )
#define		TCNTB2    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x24)) )
#define		TCMPB2    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x28)) )
#define		TCNTO2    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x2C)) )
#define		TCNTB3    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x30)) )
#define		TCMPB3    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x34)) )
#define		TCNTO3    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x38)) )
#define		TCNTB4    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x3C)) )
#define		TCNTO4    	( *((volatile unsigned long *)(PWMTIMER_BASE+0x40)) )
#define		TINT_CSTAT 	( *((volatile unsigned long *)(PWMTIMER_BASE+0x44)) )

typedef void (isr) (void);
extern void asm_timer_irq();
extern void asm_k1_irq();
int t;
void irq_init(void)
{
	/* 在中断控制器里使能timer0中断 */
	VIC0INTENABLE |= (1<<23);

	VIC0INTSELECT =0;

	isr** isr_array = (isr**)(0x7120015C);

	isr_array[0] = (isr*)asm_timer_irq;


	/* 配置GPN0~5引脚为中断功能 */
	GPNCON &= ~(0xff);
	GPNCON |= 0xaa;

	/* 设置中断触发方式为: 下降沿触发 */
	EINT0CON0 &= ~(0xff);
	EINT0CON0 |= 0x33;

	/* 禁止屏蔽中断 */
	EINT0MASK &= ~(0x0f);
	
	// Select INT_EINT0 mode as irq  
	VIC0INTSELECT = 0;

	/* 在中断控制器里使能这些中断 */
	VIC0INTENABLE |= (0x3); /* bit0: eint0~3, bit1: eint4~11 */ 
	
	isr** isr_array_2 = (isr**)(0x71200100);

  	isr_array_2[0] = (isr*)asm_k1_irq;



	/*将GPK4-GPK7配置为输出口*/
	GPKCON0 = 0x11110000;
	
	/*熄灭四个LED灯*/
	GPKDATA = 0xff;
}

// timer0中断的中断处理函数
void do_irq()
{
	
	unsigned long uTmp;
	t++;
	switch(t)
	{
		case 1:
		GPKDATA = 0xef;
		break;
		case 2:
		GPKDATA = 0xdf;
		break;
		case 3:
		GPKDATA = 0xbf;
		break;
		case 4:
		GPKDATA = 0x7f;
		t = 0;
		break;
	}

	//清timer0的中断状态寄存器
	uTmp = TINT_CSTAT;
	TINT_CSTAT = uTmp;
	VIC0ADDRESS=0x0;	
}

void do_irq_key(void)
{
	int i = 0;

	//GPKDATA = 0x00;


	/* 分辨是哪个中断 */
	/*for (i = 0; i < 4; i ++)
	{
		if (EINT0PEND & (1<<i))
		{
			GPKDATA &= ~(1<<(i+4));
		}
		else
		{
			GPKDATA |= 1<<(i+4);
		}
	}
	*/
	for(i=0;i<4;i++){
		if(EINT0PEND & (1<<i)){
			if(i == 0)GPKDATA = 0xef;
			if(i == 1)GPKDATA = 0xdf;
			if(i == 2)GPKDATA = 0xbf;
			if(i == 3)GPKDATA = 0x7f;
		}
	}
	/*

	if (EINT0PEND & 0x8)
	{
		GPKDATA = 0xf0;
	}
	*/
	
	/* 清中断 */
	EINT0PEND   = 0x3f;
	VIC0ADDRESS = 0;
}

// 初始化timer
void timer_init(unsigned long utimer,unsigned long uprescaler,unsigned long udivider,unsigned long utcntb,unsigned long utcmpb)
{
	unsigned long temp0;

	// 定时器的输入时钟 = PCLK / ( {prescaler value + 1} ) / {divider value} = PCLK/(65+1)/16=62500hz

	//设置预分频系数为66
	temp0 = TCFG0;
	temp0 = (temp0 & (~(0xff00ff))) | (uprescaler<<0);
	TCFG0 = temp0;

	// 16分频
	temp0 = TCFG1;
	temp0 = (temp0 & (~(0xf<<4*utimer))& (~(1<<20))) |(udivider<<4*utimer);
	TCFG1 = temp0;

	// 1s = 62500hz
	TCNTB0 = utcntb;
}