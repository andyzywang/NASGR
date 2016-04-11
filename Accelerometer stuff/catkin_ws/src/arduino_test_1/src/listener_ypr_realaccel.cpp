#include "ros/ros.h"
#include "std_msgs/Float32MultiArray.h"
#include <iostream>
#include <iomanip> 

#define AVERAGE_NUMBER_ELEMENTS 2		//used for averageing numbers purposes. Must be positive integer

//deals with data that is called from ROS network
void chatterCallback(const std_msgs::Float32MultiArray::Ptr & msg)
{
	//variable to keep track of number of times program runs
	static long long int cycles(0);
		
	//get (*msg).data[] into more intuitive variables
	float xAccel = (*msg).data[0];
	float yAccel = (*msg).data[1];
	float zAccel = (*msg).data[2];
	float yaw = (*msg).data[3];
	float pitch = (*msg).data[4];
	float roll = (*msg).data[5];
	
	//variables for printing
	float xAccelOut(0), yAccelOut(0), zAccelOut(0), yawOut(0), pitchOut(0), rollOut(0);
	
	//containers for averaging data
	static float preAverageData[6][AVERAGE_NUMBER_ELEMENTS];

	//gets pitch and roll for compensation later
	float pitchTemp 	= (*msg).data[4];		
	float zPitchTemp 	= (*msg).data[4];
	float rollTemp 	= (*msg).data[5];
	float zRollTemp 	= (*msg).data[5];
	
//make some stuff positive only
	if (pitchTemp 	< 0) pitchTemp = -pitchTemp;	//only positive
	if (rollTemp 	< 0) rollTemp 	= -rollTemp;	//only positive

	//compensation calculations for accelerations. Makes the chip read close to zero acceleration when there is actually no acceleration
	float xComp = (pitchTemp/90)*.06*4096.0;	//xAccel compensation
	float yComp = (rollTemp/90)*.02*4096.0;	//yAccel compensation
	float zComp = 1900-(((zRollTemp+zPitchTemp))*21.1);	//zAccel compensation
	
	//correct accelerations
	xAccel -= xComp;
	yAccel -= yComp;
	zAccel -= zComp;

	//gathers data to average
	//variable to know which column to deal with
	int column = cycles%AVERAGE_NUMBER_ELEMENTS;
	preAverageData[0][column] = xAccel;
	preAverageData[1][column] = yAccel;
	preAverageData[2][column] = zAccel;
	preAverageData[3][column] = yaw;
	preAverageData[4][column] = pitch;
	preAverageData[5][column] = roll;

	// averages and prints data in a pretty way every 4 cycles to get average.
	if (cycles%AVERAGE_NUMBER_ELEMENTS == 0) 
	{
		for (int i = 0; i < AVERAGE_NUMBER_ELEMENTS; i++)
		{
			xAccelOut += preAverageData[0][i]/AVERAGE_NUMBER_ELEMENTS;
			yAccelOut += preAverageData[1][i]/AVERAGE_NUMBER_ELEMENTS;
			zAccelOut += preAverageData[2][i]/AVERAGE_NUMBER_ELEMENTS;
			yawOut	 += preAverageData[3][i]/AVERAGE_NUMBER_ELEMENTS;
			pitchOut  += preAverageData[4][i]/AVERAGE_NUMBER_ELEMENTS;
			rollOut 	 += preAverageData[5][i]/AVERAGE_NUMBER_ELEMENTS;
		}

		//printf("xyz: %8.3f %8.3f %8.3f ypr: %8.3f %8.3f %8.3f \n",xAccelOut, yAccelOut, zAccelOut, yawOut, pitchOut, rollOut);
		printf(" %8.3f %8.3f %8.3f \n",yawOut, pitchOut, rollOut);
		fflush(stdout);
	}
	//increment cycles
	cycles++;
}	

int main(int argc, char **argv)
{
	//Initializer?
	ros::init(argc, argv, "listener");
	
	//variable n of type NodeHandle
	ros::NodeHandle n;

	//variable sub of type Subscriper. Also allows chatterCallback function to do its thing
	ros::Subscriber sub = n.subscribe("chatter", 1000, chatterCallback);

	//no idea. 
	ros::spin();

return 0;
}
