#include "./../include/command_utilities.h"
#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <math.h>
#include <errno.h>

// File descriptor for the log file
int fd_log;

// Raw time and struct tm to store current time information
time_t rawtime;
struct tm *info;

// Function to send velocity data through a named pipe to another process
void Velocity_command_sender(char * myfifo, int Vel){
    // File descriptor for the named pipe
    int fd;

    // Open the named pipe in write-only mode
    if((fd = open(myfifo, O_WRONLY))==-1){
        // If the open fails, close the log file and print an error message
        close(fd_log);
        perror("Error while opening fifo");
        exit(1);
    }

    // Write the velocity data to the named pipe
    if(write(fd, &Vel, sizeof(Vel))==-1){
        // If the write fails, close the log file and the named pipe, and print an error message
        close(fd_log);
        close(fd);
        perror("Error while writing to fifo");
        exit(1);
    }

    // Close the named pipe file descriptor
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

int main(int argc, char const *argv[]){

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize User Interface 
    init_console_ui();

    // Define the first named pipe between the command window and motor 1
    int fd;

    // Name of the named pipe for velocity in the X direction
    char * Vel_x_fifo = "/tmp/Vel_x_fifo";

    // Create the named pipe for velocity in the X direction
    if ( mkfifo(Vel_x_fifo, 0666)==-1){
       // Check if the named pipe already exists
       if(errno != EEXIST){
       printf("Could not create fifo file (Command Console - Motor X)\n");
       // Return error if the named pipe could not be created
       return 1;
       }
}

    // Name of the named pipe for velocity in the Z direction
    char * Vel_z_fifo = "/tmp/Vel_z_fifo";

    // Create the named pipe for velocity in the Z direction
    if ( mkfifo(Vel_z_fifo, 0666)==-1){
       // Check if the named pipe already exists
       if(errno != EEXIST){
       printf("Could not create fifo file (Command Console - Motor Z)\n");
       // Return error if the named pipe could not be created
       return 1;
       }
}

    // Open the log file for writing
    if ((fd_log = open("log/command.log",O_WRONLY|O_APPEND|O_CREAT, 0666)) == -1){
    // If the log file could not be opened, print an error message and exit
    perror("Error opening command file");
    exit(1);
}

    // Infinite loop
    while(TRUE){

        // Get current time
        time(&rawtime);
        info = localtime(&rawtime);
        
        fflush(stdout);

        // Get mouse/resize commands in non-blocking mode...
        int cmd = getch();

        // If user resizes screen, re-draw UI
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            } else {
                reset_console_ui();
            }
        }
        // Else if mouse has been pressed
        else if(cmd == KEY_MOUSE) {

            // Check which button has been pressed...
            if(getmouse(&event) == OK) {

                //Vx-- button pressed
                if(check_button_pressed(vx_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Decreased");

                    Velocity_command_sender(Vel_x_fifo,0);
                    Log_writer("Vx-- button pressed.");

                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vx++ button pressed
                else if(check_button_pressed(vx_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Increased");

                    Velocity_command_sender(Vel_x_fifo,1);
                    Log_writer("Vx++ button pressed.");

                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vx stop button pressed
                else if(check_button_pressed(vx_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");

                    Velocity_command_sender(Vel_x_fifo,2);
                    Log_writer("Vx stop button pressed.");

                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vz++ button pressed
                else if(check_button_pressed(vz_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Decreased");

                    Velocity_command_sender(Vel_z_fifo,0);
                    Log_writer("Vz++ button pressed.");

                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vz-- button pressed
                else if(check_button_pressed(vz_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Increased");
                    
                    Velocity_command_sender(Vel_z_fifo,1);
                    Log_writer("Vz-- button pressed.");

                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vz stop button pressed
                else if(check_button_pressed(vz_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Motor Stopped");

                    Velocity_command_sender(Vel_z_fifo,2);
                    Log_writer("Vz stop button pressed.");

                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }  
            }
        }
        refresh();
	}

    close(fd_log);

    // Terminate
    endwin();
    return 0;
}
