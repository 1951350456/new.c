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
double hundred = 0;
double ten = 0;
double one = 0;
int show_state = 0;

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
int flag = 0;
int flag2 = 0;
int cnt1 = 0;
int cnt2 = 0;
int cnt3 = -1;
// timer0中断的中断处理函数
void do_irq()
{
	//设LED1对应个位，LED2对应十位，LED3对应百位。K3按下时，三个LED灯显示当前通过按键设置的数字，
	//即从LED1到LED3，三个LED灯逐一按1秒（定时器中断实现）的间隔闪烁所对应数字的次数。



	//K3保持按下的状态下，按下K4，清除K1和K2按键状态，
	//所有LED灯按0.5秒（定时器中断实现）的间隔闪烁3次后熄灭，系统进入初始状态。
	unsigned long uTmp;
	if(show_state == 2){
		hundred = 0;
		ten = 0;
		one = 0;
		if(flag2 == 0){
			GPKDATA = 0X0f;
			flag2 = 1;
		}
		else if(flag2==1){
		    GPKDATA = 0xff;
			flag2 = 0;
		}
		if(cnt2==6){
			GPKDATA = 0xff;
			cnt2 = 0;
			cnt1 = 0;
			flag = 0;
			flag2 = 0;
			show_state = 0;
		}
		cnt2+=1;
	}
	//非偶数情况下，LED4按1秒（定时器中断实现）的间隔闪烁个位数字对应的次数。
	else if(show_state==3){
		//在最开始的时候设置好闪烁次数
		if(cnt3==-1){
			cnt3=one;
			cnt1=0;
			flag = 0;
		}
		//每过两次才转换一次状态（1秒间隔）
		if(cnt1%2==0&&cnt3){
			if(flag==0){
				GPKDATA = 0x7f;
				flag = 1;
			}
			else{
				GPKDATA = 0xff;
				flag = 0;
			}
			cnt3-=1;
		}
		cnt1+=1;
		if(cnt3==0){
			GPKDATA = 0xff;
			cnt3 = -1;
			show_state = 0;
			flag = 0;
			cnt1 = 0;
		}
	}
	//K4按下时，若所设置的数字是2的奇数倍，则四个LED灯按1秒（定时器中断实现）的间隔从LED1开始
	//按双向跑马灯循环显示（同一个时刻只有一个灯亮）；
	else if(show_state == 4){
	    if(cnt1%2==0){
			if(flag==0){
				if(cnt2 == 3)
					flag==1;
				GPKDATA = ~((1<<(cnt2+4))|0x00);
				cnt2+=1;
			}
			else{
				if(cnt2==0)
					flag==0;
				GPKDATA = ~((1<<(cnt2+4))|0x00);
				cnt2-=1;
			}
		}
		cnt1+=1;
	}
	//若是2的偶数倍，则四个LED灯按2秒（定时器中断实现）的间隔从LED4开始
	//按双向跑马灯循环显示（同一个时刻只有一个灯亮）。
	else if(show_state == 5){
		if(cnt1%4==0){
			if(flag==0){
				if(cnt2 == 3)
					flag==1;
				GPKDATA = ~((1<<(cnt2+4))|0x00);
				cnt2+=1;
			}
			else{
				if(cnt2==0)
					flag==0;
				GPKDATA = ~((1<<(cnt2+4))|0x00);
				cnt2-=1;
			}
		}
		cnt1+=1;
	}
	//K3按下时，三个LED灯显示当前通过按键设置的数字，
	//即从LED1到LED3，三个LED灯逐一按1秒（定时器中断实现）的间隔闪烁所对应数字的次数。 
	else if(show_state == 1){
		if(flag == 0){
			//0xef;0xdf;0xbf;0x7f;
			if(one != 0){
			
				GPKDATA = 0xef;
				one -= 0.5;
			}
			else if(ten != 0){
				GPKDATA = 0xdf;
				ten -= 0.5;
			}
			else if(hundred != 0){
				GPKDATA = 0xbf;
				hundred -= 0.5;
			}
			if(cnt1!=2)
				cnt1+=1;
			else
				flag=1;

		}else{
			GPKDATA = 0xff;
			if(cnt1!=0)
				cnt1-=1;
			else
				flag=0;
		}
		if(one==0&&ten==0&&hundred==0){
			show_state = 0;
			falg=0;
			cnt1=0;
		}
	}
	//清timer0的中断状态寄存器
	uTmp = TINT_CSTAT;
	TINT_CSTAT = uTmp;
	VIC0ADDRESS=0x0;	
}


void do_irq_key(void)
{
	long temp=0;
	int k1=0;
	//k2按下
	if(EINT0PEND & (1<<1)){
		while (EINT0PEND & (1<<1)&&(temp!=20))
		{
			temp+=1;
		}
		while (EINT0PEND & (1<<1)&&(temp==20))
		{
			if (EINT0PEND & (1<<0)){
				k1=1;
			}
		}//k2短按
		if(temp!=20){
			ten += 1;
		}
		//k2长按同时按k1
		if(k1)
		hundred+=1;// 百位
	}
	else if (EINT0PEND & (1<<2)){//k3按下
	//k3按住同时k4按下，则清除k1、k2按键状态
		long temp2=0;
		int k4=0;
		while(EINT0PEND & (1<<2)&&temp2!=20){
			temp2+=1;
		}
		while(EINT0PEND & (1<<2)&&(temp2==20)){
			//k4按下
			if(EINT0PEND & (1<<3)) k4=1;
		}
		//k3短按
		if(temp2!=20){
			show_state=1;
			flag = 0;
			cnt1 = 0;
		}
		//k3长按同时按k4
		if(k4){
			show_state = 2;
			flag2 = 0;
			cnt2 = 0;
		}
	}
	else if(EINT0PEND & (1<<3)){//k4按下
		//非偶数情况下(其他情况)
		if(hundred*100+ten*10+one%2!=0){
			show_state = 3;
		}
		//2的奇数倍
		else if(hundred*100+ten*10+one%4!=0){
			show_state = 4;
			flag = 0;
			cnt1 = 0;
			cnt2 = 0;
		}
		//2的偶数倍
		else{
			show_state = 5;
			flag = 1;
			cnt1 = 0;
			cnt2 = 3;
		}
	}
	else if(EINT0PEND & (1<<0))
		one+=1;
	
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
}