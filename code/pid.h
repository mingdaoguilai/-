#ifndef __pid_h
#define __pid_h



int velocity_PID_value(int velocity_measure,int velocity_calcu);
int velocity_PID_value1(int velocity_measure,int velocity_calcu);
int line_pid_value(int error);
int vertical_PID_value1(int measure,int calcu);
void I_xianfu(float *velocity_sum,int max);

int velocity_PID_value2(int velocity_measure,int velocity_calcu);
int velocity_PID_value3(int velocity_measure,int velocity_calcu);
//int vertical_PID_value1(float measure,float calcu);




void PWM_Xianfu(int max,int *PWM);

//int yaw_pid_value(float yaw_measure,float yaw_calcu);
//float LPF2(float xin);

#endif
