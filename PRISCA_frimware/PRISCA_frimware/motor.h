/*
 * motor.h
 *
 * Created: 4/26/2019 1:24:35 AM
 * Author  : NORHAN TAREK
 * Company : PRISCA
 */


#ifndef MOTOR_H_
#define MOTOR_H_


#include <stdbool.h>
#define M_PORT C
#define EN_DES_PORT B
#define X 0  //tHE STEP BIN FOR MOTOR X
#define Y 1  //tHE STEP BIN FOR MOTOR Y
#define Z 2  //tHE STEP BIN FOR MOTOR Z
#define E 3  //tHE STEP BIN FOR 1st EXTRUDER MOTOR
#define X_DIR_PIN 4
#define Y_DIR_PIN 5
#define Z_DIR_PIN 6
#define E_DIR_PIN 7
#define EN_DES_XPIN 4
#define EN_DES_YPIN 5
#define EN_DES_ZPIN 6
#define EN_DES_EPIN 7
void motor_init ();
void motor_movement(double step[4],long speed,long exspeed);
void motor_EN_DES(char motor,bool stu);

#endif /* MOTOR_H_ */