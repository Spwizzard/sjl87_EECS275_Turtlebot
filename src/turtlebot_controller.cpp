#include "minimal_turtlebot/turtlebot_controller.h"

#define PI 3.14159265

uint8_t currentState = 0;

//0 = moving forward
//1 = backing up from left bumper press
//2 = backing up from right or center bumper press
//3 = turning right from left bumper press
//4 = turning left from right or center bumper press
//5 = wheel drop state, stop moving until wheel drops are ok
//6 = stop state, due to obstacle closer than half a meter
//7 = spin in place state, looking for direction that isn't blocked
//8 = slow down and start turning
//9 = stop due to tilt

uint8_t pathFindingState = 3;

//0 = not reached destination
//1 = reached destination, spinning
//2 = returning home
//3 = waiting for next goal

uint64_t backupTime = 1000000000;
uint64_t currentBackupStartTime = 0;

uint64_t turnTime = 2000000000; 
uint64_t currentTurnStartTime = 0;

uint64_t stopTime = 15000000000; 
uint64_t currentStopStartTime = 0;

uint64_t destinationTurnTime = 34000000000; 
uint64_t currentDestTurnStartTime = 0;

float forwardSpeed = 0.15;
float slowSpeed = 0.1;
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

float goalXPos;
float goalYPos;

float initialXPos;
float initialYPos;

void turtlebot_controller(turtlebotInputs turtlebot_inputs, uint8_t *soundValue, float *vel, float *ang_vel)
 {
	//Place your code here! you can access the left / right wheel 
	//dropped variables declared above, as well as information about
	//bumper status. 
	
	//outputs have been set to some default values. Feel free 
	//to change these constants to see how they impact the robot. 

	int totalIndex = 0;
	int numPoints = 0;

	if(currentState == 0){
		for(int indx=0; indx < 640; indx++) {
		  	float range = turtlebot_inputs.ranges[indx];
		  	if(range < 0.5 && range >= 0.0001){

		  		currentState = 6;
		  		//std::cout << range << std::endl;
		  		currentStopStartTime = turtlebot_inputs.nanoSecs;
		  		break;
		  	}
		  	else if(range < 1.5 && range >= 0.5){
		  		totalIndex += indx;
		  		numPoints++;
		  		currentState = 8;
		  	}
		  
		}
	}

	if(currentState == 8 && numPoints == 0){
		currentState = 0;
	}



	float currentXPos = turtlebot_inputs.x;
	float currentYPos = turtlebot_inputs.y;
	float currentAngle = turtlebot_inputs.z_angle;

	if(std::isnan(currentXPos)){
		return;
	}

	if(pathFindingState == 3){
		std::cout << "goalXPos: \n";
		std::cin >> goalXPos;
		std::cout << "goalYPos: \n";
		std::cin >> goalYPos;
		goalXPos = float(goalXPos);
		goalYPos = float(goalYPos);
		initialXPos = currentXPos;
		initialYPos = currentYPos;
		pathFindingState = 0;
	}

	currentAngle = -2* atan2(turtlebot_inputs.orientation_omega, currentAngle) * 180 / PI;
	if(currentAngle < 0){
		currentAngle += 360;
	}
	if(currentAngle > 180){
		currentAngle -= 180;
	}
	else if(currentAngle <= 180){
		currentAngle += 180;
	}
	float goalAngle =  atan2(goalYPos - currentYPos , goalXPos - currentXPos) * 180 / PI;
	if(goalAngle < 0){
		goalAngle = 360 + goalAngle;
	}
	
	if(turtlebot_inputs.linearAccelZ < 9.0 && turtlebot_inputs.linearAccelZ >= 0.0001){
		currentState = 9;
		*soundValue = ERROR;
	}

	if(turtlebot_inputs.leftBumperPressed == 1 || turtlebot_inputs.sensor0State == 1){
		currentState = 1;
		currentBackupStartTime = turtlebot_inputs.nanoSecs; 
	}
	else if(turtlebot_inputs.centerBumperPressed == 1 || turtlebot_inputs.rightBumperPressed == 1|| turtlebot_inputs.sensor1State == 1 || turtlebot_inputs.sensor2State == 2){
		currentState = 2;
		currentBackupStartTime = turtlebot_inputs.nanoSecs;
	}
	
	if(turtlebot_inputs.leftWheelDropped == 1 || turtlebot_inputs.rightWheelDropped == 1){
		currentState = 5;
	}


	switch(currentState){
	case 0:
		if(pathFindingState == 0 || pathFindingState == 2){
			if(goalAngle - currentAngle > 10){
				*vel = 0.0;
				*ang_vel = leftSpeed / 3;
			}
			else if(goalAngle - currentAngle < -10){
				*vel = 0.0;
				*ang_vel = rightSpeed / 3;
			}
			else{
				*vel = forwardSpeed;
				*ang_vel = 0.0;
			}
		}
		else if(pathFindingState == 1){
			*vel = 0.0;
			*ang_vel = rightSpeed;
		}
		break;
	case 1:
		*vel = backupSpeed;
		*ang_vel = leftSpeed;
		if(turtlebot_inputs.nanoSecs >= currentBackupStartTime + backupTime){
			currentState = 3;
			*vel = 0.0;

			currentTurnStartTime = turtlebot_inputs.nanoSecs;
		}
		break;
	case 2:
		*vel = backupSpeed;
		*ang_vel = rightSpeed;
		if(turtlebot_inputs.nanoSecs >= currentBackupStartTime + backupTime){
			currentState = 4;
			*vel = 0.0;
			currentTurnStartTime = turtlebot_inputs.nanoSecs;
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
	case 5:
		*vel = 0.0;
		*ang_vel = 0.0;
		if(turtlebot_inputs.leftWheelDropped == 0 && turtlebot_inputs.rightWheelDropped == 0){
			currentState = 0;
		}
		break;
	case 6:
		*vel = 0.0;
		*ang_vel = 0.0;
		*soundValue = RECHARGE;
		//std::cout << "test" << std::endl;
		if(turtlebot_inputs.nanoSecs >= currentStopStartTime + stopTime){
			//std::cout << turtlebot_inputs.nanoSecs << std::endl;
			//std::cout << currentStopStartTime << std::endl;

			currentState = 7;
		}
		break;
	case 7: {
		*vel = 0.0;
		*ang_vel = rightSpeed;
		bool obstacleStillThere = false;		
		for(int indx=0; indx < 640; indx++) {
	  		float range = turtlebot_inputs.ranges[indx];
		  	if(range < 0.5){
		  		obstacleStillThere = true;
		  		break;
		  	}
		}
		if(obstacleStillThere == false){
			currentState = 0;
		}
		break;
	}	
	case 8: {
		*vel = slowSpeed;
		float averageObstacleIndex = ((float) totalIndex / numPoints) - 320;
		float scaledVal = averageObstacleIndex * 0.002;
		if(scaledVal > 0.0){
			*ang_vel = scaledVal - 0.7;
		}
		else{
			*ang_vel = scaledVal + 0.7;
		}
		break;	
	}
	case 9:
		*vel = 0.0;
		*ang_vel = 0.0;
		if(turtlebot_inputs.linearAccelZ > 9.0){
			currentState = 0;
			*soundValue = 7;
		}
		break;
	}

	switch(pathFindingState){
	case 0:
		if(std::isnan(currentXPos) || std::isnan(currentYPos)){
			std::cout << "ALERT " << "\n";
			break;
		}
		if((fabs(currentXPos - goalXPos) < 0.1) && (fabs(currentYPos - goalYPos) < 0.1)){
			pathFindingState = 1;
			currentDestTurnStartTime = turtlebot_inputs.nanoSecs;
		}
		break;
	case 1:
		if(turtlebot_inputs.nanoSecs >= currentDestTurnStartTime + destinationTurnTime){
			pathFindingState = 2;
			goalXPos = initialXPos;
			goalYPos = initialYPos;
			std::cout << "new goal: " << goalXPos << "," << goalYPos << std::endl;
		}
		break;
	case 2:
		if(std::isnan(currentXPos) || std::isnan(currentYPos)){
			std::cout << "ALERT " << "\n";
			break;
		}
		if((fabs(currentXPos - goalXPos) < 0.1) && (fabs(currentYPos - goalYPos) < 0.1)){
			pathFindingState = 3;
		}
		break;
	case 3:
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

	if(currentState == 6){
		*soundValue = RECHARGE;
	}
	else if(currentState == 5){
		*soundValue = ERROR;
	}
	else {
		*soundValue = 7;
	}


}

