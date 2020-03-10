#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    ROS_INFO_STREAM("Drive robot.");
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x  = lin_x;
    srv.request.angular_z = ang_z;

    if (!client.call(srv))
        ROS_ERROR("Failed to call service.");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    int white_pixel = 255;
    int left = 0, forward = 0, right = 0; // Weights to find to out, what direction has the biggest number of white pixels.
    for (int i = 0; i<img.height*img.step; i+=3){
        int pos = (i/3)%img.width;
        if ( (img.data[i] == 255) && (img.data[i+1] == 255) && (img.data[i+2] == 255) ){
            if (pos < (img.width/3))
                left++;
            else if ( (pos>=(img.width/3))&&(pos<(2*img.width/3)) )
                forward++;
            else
                right++;
        }
    }
    // Find the heaviest weight.
    if ((left == 0) && (right == 0) && (forward == 0))
       drive_robot(0,0);
    else if ( (forward>=left)&&(forward>=right) )
        drive_robot(0.5, 0);
    else if ( (right>=forward)&&(right>=left) )
        drive_robot(0.5, -0.5);
    else
        drive_robot(0.5, 0.5);
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;
    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
