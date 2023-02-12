#include "./../include/inspection_utilities.h"
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

// X and Y positions array
float Positions_Array[2];

// File descriptor for the log file
int fd_log;

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

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // End-effector coordinates
    float ee_x, ee_y; // these values and the parameters which are located at the below need to make sence together be in attention to them 
    ee_x = 0.0;
    ee_y = 0.0; 

    // Initialize User Interface 
    init_console_ui();

    int fd;
    char * Real_pos_fifo = "/tmp/Real_pos_fifo";
    mkfifo(Real_pos_fifo, 0666);

    pid_t pid_motorX = atoi(argv[1]);
    pid_t pid_motorZ = atoi(argv[2]); 

    // Open the log file
    if ((fd_log = open("log/inspection.log",O_WRONLY|O_APPEND|O_CREAT, 0666))==-1){
        // If the file could not be opened, print an error message and exit
        perror("Error opening command file");
        // exit(1);
    }

    // Infinite loop
    while(TRUE){
        //perror("inspection_console: FD5r: open()");

        // Get current time
        time(&rawtime);
        info = localtime(&rawtime);
        
        // Get mouse/resize commands in non-blocking mode...
        int cmd = getch();

        // If user resizes screen, re-draw UI
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }
        // Else if mouse has been pressed
        else if(cmd == KEY_MOUSE) {

            // Check which button has been pressed...
            if(getmouse(&event) == OK) {

                // STOP button pressed
                if(check_button_pressed(stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "STP button pressed");

                    Log_writer("STOP button pressed.");

                    kill(pid_motorX, SIGUSR1);
                    kill(pid_motorZ, SIGUSR1);

                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // RESET button pressed
                else if(check_button_pressed(rst_button, &event)) {
                    mvprintw(LINES - 1, 1, "RST button pressed");

                    Log_writer("RESET button pressed.");

                    kill(pid_motorX, SIGUSR2);
                    kill(pid_motorZ, SIGUSR2);

                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }
        

        // Open the FIFOs
        if ((fd = open(Real_pos_fifo, O_RDONLY))==-1){
            close(fd);
            perror("Error opening fifo");
        }

        // Read the FIFOs from World
        if(read(fd, Positions_Array, sizeof(Positions_Array)) == -1){
            close(fd);
            perror("Error reading fifo");
        }

        close(fd);

        ee_x=Positions_Array[0];
        ee_y=Positions_Array[1];
       
        // Update UI
        update_console_ui(&ee_x, &ee_y);
        sleep(1);
	}

    close(fd_log);

    // Terminate
    endwin();
    return 0;
}
