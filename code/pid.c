
#include "pid.h"


//extern float Kp,Ki,Kd;   //角度环参数
//float err;               //此次误差
//float last_err;          //上次误差
//float err_sum=0;         //误差累加
//float err_difference;    //误差的差值

//extern float Kp1,Ki1,Kd1;   //角度环参数
//float err1;               //此次误差
//float last_err1;          //上次误差
//float err_sum1=0;         //误差累加
//float err_difference1;    //误差的差值

typedef struct
{
	float target;	
	float now;
	float error[3];		
	float p,i,d;
	float pout, dout, iout;
	float out;   
}pid_t;


extern float VKp,VKi;    //速度环参数
float err2;               //此次误差
float filt_velocity;     //滤波后的速度
float last_filt_velocity;//上一次的滤波后的速度
//float velocity_err_sum=0;    //速度的累加
float velocity_sum;

extern float VKp1,VKi1;    //速度环参数
float err21;               //此次误差
float filt_velocity1;     //滤波后的速度
float last_filt_velocity1;//上一次的滤波后的速度
//float velocity_err_sum=0;    //速度的累加
float velocity_sum1;

extern float VKp2,VKi2;    //速度环参数
float err22;               //此次误差
float filt_velocity2;     //滤波后的速度
float last_filt_velocity2;//上一次的滤波后的速度
//float velocity_err_sum=0;    //速度的累加
float velocity_sum2;


extern float VKp3,VKi3;    //速度环参数
float err23;               //此次误差
float filt_velocity3;     //滤波后的速度
float last_filt_velocity3;//上一次的滤波后的速度
//float velocity_err_sum=0;    //速度的累加
float velocity_sum3;


//转向环
extern float tkp,tkd;
float line_err,line_err_differnence,last_line_err;

void I_xianfu(float *velocity_sum,int max)
{
	if(*velocity_sum>max)  *velocity_sum=max;
	if(*velocity_sum<-max) *velocity_sum=-max;
}



int velocity_PID_value(int velocity_measure,int velocity_calcu)
{
	float a=0.3; 	//滤波系数（反映滤波程度）
	err2=velocity_measure-velocity_calcu;
	filt_velocity = a*err2 + (1-a)*last_filt_velocity; //一阶速度滤波
	
	velocity_sum+=  filt_velocity;                        //速度的累加
	I_xianfu(&velocity_sum,1500);                         //累加限幅,调参时可以去掉
	last_filt_velocity = filt_velocity;                   //此次速度记录为“上次速度”

	return VKp*filt_velocity + VKi*velocity_sum;
}

int velocity_PID_value1(int velocity_measure,int velocity_calcu)
{
	float a=0.3; 	//滤波系数（反映滤波程度）
	err21=velocity_measure-velocity_calcu;
	filt_velocity1 = a*err21 + (1-a)*last_filt_velocity1; //一阶速度滤波
	
	velocity_sum1+=  filt_velocity1;                        //速度的累加
	I_xianfu(&velocity_sum1,1500);                         //累加限幅,调参时可以去掉
	last_filt_velocity1 = filt_velocity1;                   //此次速度记录为“上次速度”

	return VKp1*filt_velocity1 + VKi1*velocity_sum1;
}

int velocity_PID_value2(int velocity_measure,int velocity_calcu)
{
	float a=0.3; 	//滤波系数（反映滤波程度）
	err22=velocity_measure-velocity_calcu;
	filt_velocity2 = a*err22 + (1-a)*last_filt_velocity2; //一阶速度滤波
	
	velocity_sum2+=  filt_velocity2;                        //速度的累加
	I_xianfu(&velocity_sum,1500);                         //累加限幅,调参时可以去掉
	last_filt_velocity2 = filt_velocity2;                   //此次速度记录为“上次速度”

	return VKp2*filt_velocity2 + VKi2*velocity_sum2;
}

int velocity_PID_value3(int velocity_measure,int velocity_calcu)
{
	float a=0.3; 	//滤波系数（反映滤波程度）
	err23=velocity_measure-velocity_calcu;
	filt_velocity3 = a*err23 + (1-a)*last_filt_velocity3; //一阶速度滤波
	
	velocity_sum3+=  filt_velocity3;                        //速度的累加
	I_xianfu(&velocity_sum,1500);                         //累加限幅,调参时可以去掉
	last_filt_velocity3 = filt_velocity3;                   //此次速度记录为“上次速度”

	return VKp3*filt_velocity3 + VKi3*velocity_sum3;
}



//转向环
int line_pid_value(int error){ 

	line_err=error;

	line_err_differnence=line_err-last_line_err;
	last_line_err=line_err;
	return tkp*line_err+tkd*line_err_differnence;
}


//角度环：
int vertical_PID_value1(int measure,int calcu)
{
  if(calcu<0) 
	{
		  if(measure<0) measure=measure;
		  else measure=measure-360;
	}
	else if(calcu>0)
	{
		 if(measure>0) measure=measure;
		else measure=measure+360;
	}
	err = measure - calcu;             //误差
	err_sum+=err;                      //误差的累加
	err_difference = err - last_err;   //误差的差值
	last_err = err;                    //此次误差记录为“上次误差”
	
	return Kp*err + Ki*err_sum + Kd*err_difference;
}

//角度环：
//int vertical_PID_value1(float measure,float calcu)
//{

//	err1 = measure - calcu;             //误差
//	err_sum1+=err1;                      //误差的累加
//	err_difference1 = err1 - last_err1;   //误差的差值
//	last_err1 = err1;                    //此次误差记录为“上次误差”
//	
//	return Kp1*err1 + Ki1*err_sum1 + Kd1*err_difference1;
//}



void PWM_Xianfu(int max,int *PWM)
{
	if(*PWM>max)  *PWM = max;
	if(*PWM<-max) *PWM =-max;
}

//#define Const_2pi       (6.283185)
//#define Const_TS        (0.0001) //100us
// 
////二阶低通滤波器,程序没用到,如果使用,参数会发生变化
//float LPF2(float xin) {
//   float f= 20;
//   float wc = Const_2pi * f;
//   float dampingRatio = 0.707;
// 
//   float lpf2_b0 = wc*wc*Const_TS*Const_TS;
//   float lpf2_a0 = 4 + 4*dampingRatio*wc*Const_TS + lpf2_b0;
//   float lpf2_a1 = -8 + 2*lpf2_b0;
//   //float lpf2_a2 = 4 - 4*dampingRatio*wc*Const_TS  + lpf2_a0; //原始这里应该有误
//   float lpf2_a2 = lpf2_b0 + 4 - 4*dampingRatio*wc*Const_TS;
// 
//   static float lpf2_yout[3] = {0};
//   static float lpf2_xin[3] = {0};
// 
//   lpf2_xin[2] = xin;
//   lpf2_yout[2] = (lpf2_b0 * lpf2_xin[2] + 2*lpf2_b0 *lpf2_xin[1] + lpf2_b0 *lpf2_xin[0] -lpf2_a1 *lpf2_yout[1] - lpf2_a2*lpf2_yout[0]) / lpf2_a0;
//   lpf2_xin[0] = lpf2_xin[1];
//   lpf2_xin[1] = lpf2_xin[2];
//   lpf2_yout[0] = lpf2_yout[1];
//   lpf2_yout[1] = lpf2_yout[2];
// 
//   return lpf2_yout[2];
//}
