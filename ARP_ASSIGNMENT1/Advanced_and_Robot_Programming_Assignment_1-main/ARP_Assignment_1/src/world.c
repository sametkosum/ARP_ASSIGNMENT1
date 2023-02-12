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
float X_pos_old=0;
float Z_pos_old=0;
float X_position=0;
float Z_position=0;
float X_pos_real=0;
float Z_pos_real=0;
float Positions_Array[2];
const float X_pos_min=0;
const float X_pos_max=40;
const float Z_pos_min=0;
const float Z_pos_max=10;


// File descriptor for the log file
int fd_log;

float GenerateError(float tempPos) {
    // Check if tempPos is equal to zero
    if (tempPos == 0.0) {
        // If tempPos is zero, return zero
        return 0.0;
    }

    // Cast tempPos to an integer
    int error = (int)tempPos;
    // Generate a random number between -error and error
    int randomNumber = (rand() % (error + error + 1)) - error;
    // Write a log message indicating that an error has been generated
    Log_writer("Error generated.");
    // Divide the random number by 100 and return it as a float
    return (float)randomNumber / 100;
}

void SendPosition(char *myfifo, float Real_position[2]) {
    int fd;  // File descriptor for the named pipe

    // Open the named pipe
    if ((fd = open(myfifo, O_WRONLY)) == -1) {
        close(fd_log);  // Close the log file
        perror("Error opening fifo");  // Print an error message
        exit(1);  // Exit the program with an error code
    }

    // Write to the named pipe
    if (write(fd, Real_position, sizeof(Real_position)) == -1) {
        close(fd_log);  // Close the log file
        perror("Error in writing function");  // Print an error message
        exit(1);  // Exit the program with an error code
    }

    // Log_writer("Position sent.");

    close(fd);  // Close the named pipe
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

int main(int argc, char const *argv[]){
    int fd;
    char * Pos_x_fifo = "/tmp/Pos_x_fifo";
    if ( mkfifo(Pos_x_fifo, 0666)==-1){
       // Check if the named pipe already exists
       if(errno != EEXIST){
       printf("Could not create fifo file (Motor X - World)\n");
       // Return error if the named pipe could not be created
       return 1;
       }
}

    char * Pos_z_fifo = "/tmp/Pos_z_fifo";
    if ( mkfifo(Pos_z_fifo, 0666)==-1){
       // Check if the named pipe already exists
       if(errno != EEXIST){
       printf("Could not create fifo file (Motor Z - World)\n");
       // Return error if the named pipe could not be created
       return 1;
       }
}
    char * Real_pos_fifo = "/tmp/Real_pos_fifo";
    if ( mkfifo(Real_pos_fifo, 0666)==-1){
       // Check if the named pipe already exists
       if(errno != EEXIST){
       printf("Could not create fifo file (World - Inspection)\n");
       // Return error if the named pipe could not be created
       return 1;
       }
}
    
    // Open the log file
    if ((fd_log = open("log/world.log",O_WRONLY|O_APPEND|O_CREAT, 0666))==-1){
        perror("Error opening world log file");
        return 1;
    }

   while (1) {
        // Get the current time
        time(&rawtime);
        info = localtime(&rawtime);

        // Read the X position from the named pipe
        if (fd = open(Pos_x_fifo, O_RDONLY)) {
            if (read(fd, &X_position, sizeof(X_position)) == -1) {
                close(fd_log);  // Close the log file
                close(fd);  // Close the named pipe
                perror("Error reading fifo");  // Print an error message
                exit(1);  // Exit the program with an error code
            }
            close(fd);  // Close the named pipe
        } else {
            close(fd_log);  // Close the log file
            close(fd);  // Close the named pipe
            perror("Error opening fifo");  // Print an error message
            exit(1);  // Exit the program with an error code
        }

        // Read the Z position from the named pipe
        if (fd = open(Pos_z_fifo, O_RDONLY)) {
            if (read(fd, &Z_position, sizeof(Z_position)) == -1) {
                close(fd_log);  // Close the log file
                close(fd);  // Close the named pipe
                perror("Error reading fifo");  // Print an error message
                exit(1);  // Exit the program with an error code
            }
            close(fd);  // Close the named pipe
        } else {
            close(fd_log);  // Close the log file
            close(fd);  // Close the named pipe
            perror("Error opening fifo");  // Print an error message
            exit(1);  // Exit the program with an error code
        }

        // Calculate the real X position
        if (X_position != X_pos_old) {
            X_pos_real = X_position + GenerateError(X_position);
            if (X_pos_real > 40) {
                X_pos_real = 40;
            } else if (X_pos_real < 0) {
                X_pos_real = 0;
            }
            X_pos_old = X_position;
        }

        // Calculate the real Z position
        if (Z_position != Z_pos_old) {
            Z_pos_real = Z_position + GenerateError(Z_position);
            if (Z_pos_real > 10) {
                Z_pos_real = 10;
            } else if (Z_pos_real < 0) {
                Z_pos_real = 0;
            }
            Z_pos_old = Z_position;
        }

        // Store the real X and Z positions in an array
        Positions_Array[0] = X_pos_real;
        Positions_Array[1] = Z_pos_real;

        // Send the real position to another named pipe
        SendPosition(Real_pos_fifo, Positions_Array);
}

    close(fd_log);

    return 0;
}
