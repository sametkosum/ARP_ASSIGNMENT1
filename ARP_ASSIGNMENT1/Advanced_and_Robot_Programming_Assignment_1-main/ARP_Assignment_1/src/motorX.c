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
float X_velocity=0;
float X_velocity_Level=0;
float X_position=0;

// Positions and Velocity limits
const float X_pos_min=0;
const float X_pos_max=40;
const float X_level_max=2;

// File descriptor for the log file
int fd_log;

void X_position_sender(const char * myfifo, float X_position){
    int fd;
    // Open the named pipe for writing
    if ((fd = open(myfifo, O_WRONLY))==-1){
       // If there was an error opening the named pipe, close the log file and print an error message
       close(fd_log);
       perror("Error opening fifo");
       exit(1);
     }

    // Write the position data to the named pipe
    if(write(fd, &X_position, sizeof(X_position))==-1){
    // If there was an error writing to the named pipe, close the named pipe and the log file, and print an error message
      close(fd);
      close(fd_log);
      perror("Error in writing function");
      exit(1);
    }

    // Close the named pipe
    close(fd);
}

// Raw time and struct tm to store current time information
time_t rawtime;
struct tm *info;

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
float Velocity_x_calculator(const char * myfifo, int command, float X_velocity){
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
    if(command==0){Log_writer("Vx-- button pressed."); return -1.0;} 
    else if(command==1){Log_writer("Vx++ button pressed."); return 1.0;} 
    else if(command==2){Log_writer("Vx stop button pressed."); return 2.0;} 
    else {return 0.0;}
    }
    // Closing the named pipe if there is an error opening it
    close(fd);
    perror("Error opening fifo");
    exit(1);
}

void stop_handler(int sig){
    if(sig==SIGUSR1){
        // Stop the motor
        X_velocity=0;

        // Listen for stop signal
        if(signal(SIGUSR1, stop_handler)==SIG_ERR){
            exit(1);
        }
    }
}

// Reset signal handler
void reset_handler(int sig){
    if(sig == SIGUSR2){
        // Setting velocity to -5
        X_velocity=-5;
        
        // Listen for stop signal
        if(signal(SIGUSR2, reset_handler)==SIG_ERR){
            exit(1);
        }
    }
}

int main(int argc, char const *argv[]){
    // definition of the second named pipe between the command console and motor2
    int fd;
    char * Vel_x_fifo = "/tmp/Vel_x_fifo";
    
    if ( mkfifo(Vel_x_fifo, 0666)==-1){
       // Check if the named pipe already exists
       if(errno != EEXIST){
       printf("Could not create fifo file (Motor X - Inspection)\n");
       // Return error if the named pipe could not be created
       return 1;
       }
}
    char * Pos_x_fifo = "/tmp/Pos_x_fifo";
    
    if ( mkfifo(Pos_x_fifo, 0666)==-1){
       // Check if the named pipe already exists
       if(errno != EEXIST){
       printf("Could not create fifo file (Motor X- World)\n");
       // Return error if the named pipe could not be created
       return 1;
       }
}

    int command = -1;

    // Open the log file
    if ((fd_log = open("log/motorX.log",O_WRONLY|O_APPEND|O_CREAT, 0666))==-1){
        perror("Error opening motorX log file.");
        return 1;
    }

    // Listen for signals
    if(signal(SIGUSR1, stop_handler)==SIG_ERR || signal(SIGUSR2, reset_handler)==SIG_ERR){
        // Close file descriptors
        close(Vel_x_fifo);
        close(Pos_x_fifo);
        close(fd_log);
        exit(1);
    }

    while (1) {
        // Get the current time
        time(&rawtime);
        info = localtime(&rawtime);

        // Calculate the new X velocity
        if (X_velocity != -5) {X_velocity_Level = Velocity_x_calculator(Vel_x_fifo, command, X_velocity);

            // If Velocity_x_calculator returns 2, reset X velocity and X_velocity_Level
            if (X_velocity_Level == 2) {X_velocity = 0; X_velocity_Level = 0;} 
            else {X_velocity = X_velocity + X_velocity_Level;

                // Limit X velocity to X_level_max
                if (X_velocity < -X_level_max) {X_velocity = -X_level_max;} 
               else if (X_velocity > X_level_max) {X_velocity = X_level_max;}
            }
        }

        // Calculate the new X position
        X_position = X_position + X_velocity;

        // Limit X position to X_pos_min and X_pos_max
        if (X_position < X_pos_min) {X_position = X_pos_min; X_velocity = 0;} 
        else if (X_position > X_pos_max) {X_position = X_pos_max; X_velocity = 0;}

        // Send the new X position
        X_position_sender(Pos_x_fifo, X_position);

        // Wait for 1 second
        sleep(1);
}

    close(fd_log);
    return 0;
}
