#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>

// Init
float Z_velocity=0;
float Z_velocity_Level=0;
float Z_position=0;

// Positions and Velocity limits
const float Z_pos_min=0;
const float Z_pos_max=10;
const float Z_level_max=2;

// File descriptor for the log file
int fd_log;

// Raw time and struct tm to store current time information
time_t rawtime;
struct tm *info;

void Z_position_sender(const char * myfifo, float Z_position){
    int fd;
    // Open the named pipe for writing
    if ((fd = open(myfifo, O_WRONLY))==-1){
       // If there was an error opening the named pipe, close the log file and print an error message
       close(fd_log);
       perror("Error opening fifo");
       exit(1);
    }
    
    // Write the position data to the named pipe
    if(write(fd, &Z_position, sizeof(Z_position))==-1){
       // If there was an error writing to the named pipe, close the named pipe and the log file, and print an error message
       close(fd);
       close(fd_log);
       perror("Error in writing function");
       exit(1);
    }
    
    // Close the named pipe
    close(fd);
}

// Function to write log messages to a file
void Log_writer(char * msg){
    // Create a buffer to store the log entry
    char log_entry[200];

    // Concatenate the log message and the current time into the log_entry buffer
    sprintf(log_entry, "%s%s", msg, asctime(info));

    // Write the log entry to the log file
    if(write(fd_log, log_entry, strlen(log_entry))==-1){
        // If the write fails, close the log file and print an error message
        close(fd_log);
        perror("Error writing the log file");
        exit(1);
    }
}

// Function to calculate the velocity based on the given command and current velocity
float Velocity_z_calculator(const char * myfifo, int command, float Z_velocity){
    // Opening the named pipe for reading the command value
    int fd;
   
    if(fd = open(myfifo, O_RDONLY|O_NDELAY)){
      // Reading the value from the pipe
      if(read(fd, &command, sizeof(command))==-1){
        // Closing the log file and named pipe if there is an error while reading
        close(fd_log);
        close(fd);
        perror("Error reading from fifo");
        exit(1);
    }
    // Checking the value of command and returning the corresponding velocity
    if(command==0){Log_writer("Vz-- button pressed."); return -1.0;} 
    else if(command==1){Log_writer("Vz++ button pressed."); return 1.0;} 
    else if(command==2){Log_writer("Vz stop button pressed."); return 2.0;} 
    else {return 0.0;}
    }
    // Closing the named pipe if there is an error opening it
    close(fd);
    perror("Error opening fifo");
    exit(1);
}

// Raw time and struct tm to store current time information
time_t rawtime;
struct tm *info;


void stop_handler(int sig){
    if(sig==SIGUSR1){
        // Stop the motor
        Z_velocity=0;

        // Listen for stop signal
        if(signal(SIGUSR1, stop_handler)==SIG_ERR){
            exit(1);
        }
    }
}

// Reset signal handler
void reset_handler(int sig){
    if(sig == SIGUSR2){
        // Setting velocity to -3
        Z_velocity=-3;
        
        // Listen for stop signal
        if(signal(SIGUSR2, reset_handler)==SIG_ERR){
            exit(1);
        }
    }
}

int main(int argc, char const *argv[]){
    // definition of the second named pipe between the command console and motor2
    int fd;
    char * Vel_z_fifo = "/tmp/Vel_z_fifo";
        
    if ( mkfifo(Vel_z_fifo, 0666)==-1){
       // Check if the named pipe already exists
       if(errno != EEXIST){
       printf("Could not create fifo file (Motor Z - Inspection)\n");
       // Return error if the named pipe could not be created
       return 1;
       }
}
    char * Pos_z_fifo = "/tmp/Pos_z_fifo";
    
    if ( mkfifo(Pos_z_fifo, 0666)==-1){
       // Check if the named pipe already exists
       if(errno != EEXIST){
       printf("Could not create fifo file (Motor Z- World)\n");
       // Return error if the named pipe could not be created
       return 1;
       }
}

    int command = -1;

    // Open the log file
    if ((fd_log = open("log/motorZ.log",O_WRONLY|O_APPEND|O_CREAT, 0666))==-1){
        perror("Error opening motorZ log file.");
        return 1;
    }

    // Listen for signals
    if(signal(SIGUSR1, stop_handler)==SIG_ERR || signal(SIGUSR2, reset_handler)==SIG_ERR){
        // Close file descriptors
        close(Vel_z_fifo);
        close(Pos_z_fifo);
        close(fd_log);
        exit(1);
    }

    while(1){
        // Get the current time
        time(&rawtime);
        info = localtime(&rawtime);

        // Calculate the new Z velocity
        if (Z_velocity != -3) {Z_velocity_Level = Velocity_z_calculator(Vel_z_fifo, command, Z_velocity);

            // If Velocity_z_calculator returns 2, reset Z velocity and Z_velocity_Level
            if (Z_velocity_Level == 2) {Z_velocity = 0; Z_velocity_Level = 0;} 
            else {Z_velocity = Z_velocity + Z_velocity_Level;

                // Limit Z velocity to Z_level_max
                if (Z_velocity < -Z_level_max) {Z_velocity = -Z_level_max;} 
               else if (Z_velocity > Z_level_max) {Z_velocity = Z_level_max;}
            }
        }

        // Calculate the new Z position
        Z_position = Z_position + Z_velocity;

        // Limit Z position to Z_pos_min and Z_pos_max
        if (Z_position < Z_pos_min) {Z_position = Z_pos_min; Z_velocity = 0;} 
        else if (Z_position > Z_pos_max) {Z_position = Z_pos_max; Z_velocity = 0;}

        // Send the new Z position
        Z_position_sender(Pos_z_fifo, Z_position);

        // Wait for 1 second
        sleep(1);
}

    close(fd_log);
    return 0;
}
