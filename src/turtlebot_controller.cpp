#include "minimal_turtlebot/turtlebot_controller.h"

uint8_t currentState = 0;

//0 = moving forward
//1 = backing up from left bumper press
//2 = backing up from right or center bumper press
//3 = turning right from left bumper press
//4 = turning left from right or center bumper press

uint64_t backupTime = 1000000000;
uint64_t currentBackupStartTime = 0;

uint64_t turnTime = 2000000000; 
uint64_t currentTurnStartTime = 0;

float forwardSpeed = 0.2;
float backupSpeed = -0.2;
float rightSpeed = -0.785;
float leftSpeed = 0.785;

uint8_t ON=0;
uint8_t OFF=1;
uint8_t RECHARGE=2;
uint8_t BUTTON=3;
uint8_t ERROR=4;
uint8_t CLEANINGSTART=5;
uint8_t CLEANINGEND=6;


void turtlebot_controller(turtlebotInputs turtlebot_inputs, uint8_t *soundValue, float *vel, float *ang_vel)
 {
	//Place your code here! you can access the left / right wheel 
	//dropped variables declared above, as well as information about
	//bumper status. 
	
	//outputs have been set to some default values. Feel free 
	//to change these constants to see how they impact the robot. 
	
	if(turtlebot_inputs.leftBumperPressed == 1){
		currentState = 1;
		currentBackupStartTime = turtlebot_inputs.nanoSecs; 
		*soundValue = RECHARGE;
	}
	else if(turtlebot_inputs.centerBumperPressed == 1 || turtlebot_inputs.rightBumperPressed == 1){
		currentState = 2;
		currentBackupStartTime = turtlebot_inputs.nanoSecs;
		*soundValue = RECHARGE; 
	}

	
	switch(currentState){
	case 0:
		*vel = forwardSpeed;
		*ang_vel = 0.0;
		break;
	case 1:
		*vel = backupSpeed;
		if(turtlebot_inputs.nanoSecs >= currentBackupStartTime + backupTime){
			currentState = 3;
			*vel = 0.0;
			currentTurnStartTime = turtlebot_inputs.nanoSecs;
			*soundValue = CLEANINGSTART; 
		}
		break;
	case 2:
		*vel = backupSpeed;
		if(turtlebot_inputs.nanoSecs >= currentBackupStartTime + backupTime){
			currentState = 4;
			*vel = 0.0;
			currentTurnStartTime = turtlebot_inputs.nanoSecs;
			*soundValue = CLEANINGEND; 
		}
		break;
	case 3:
		*vel = 0.0;
		*ang_vel = rightSpeed;
		if(turtlebot_inputs.nanoSecs >= currentTurnStartTime + turnTime){
			currentState = 0;
			*ang_vel = 0.0;
		}
		break;
	case 4:
		*vel = 0.0;
		*ang_vel = leftSpeed;
		if(turtlebot_inputs.nanoSecs >= currentTurnStartTime + turnTime){
			currentState = 0;
			*ang_vel = 0.0;
		}
		break;
	}

	// Robot forward velocity in m/s
	//0.7 is max and is a lot
	// Robot angular velocity in rad/s
	//0.7 is max and is a lot 
 
  
	//here are the various sound value enumeration options
	//soundValue.OFF
	//soundValue.RECHARGE
	//soundValue.BUTTON
	//soundValue.ERROR
	//soundValue.CLEANINGSTART
	//soundValue.CLEANINGEND 

}

