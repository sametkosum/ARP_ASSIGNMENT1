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



pid_t pid_cmd;
pid_t pid_inspection;
pid_t pid_motorX;
pid_t pid_motorZ;
pid_t pid_world;

int fd_log;


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

int spawn(const char * program, char * arg_list[]) {

  pid_t child_pid = fork();

  if(child_pid < 0) {
    perror("Error while forking...");
    return 1;
  }

  else if(child_pid != 0) {
    return child_pid;
  }

  else {
    if(execvp (program, arg_list) == 0);
    perror("Exec failed");
    return 1;
  }
}

// Function to get when a file was last modified
time_t get_last_modified(char *filename){
  struct stat attr;
  stat(filename, &attr);
  return attr.st_mtime;
}

int watchdog(){
  // Array of the log file paths
  char *log_files[5] = {"log/command.log", "log/motorX.log", "log/motorZ.log", "log/world.log", "log/inspection.log"};

  // Array of the PIDs
  pid_t pids[5] = {pid_cmd, pid_motorZ, pid_motorZ, pid_world, pid_inspection};

  // Flag to check if a file was modified
  int modified;

  // Variable to keep the number of seconds since the last modification
  int counter = 0;

  while (1){
    
    // Get current time
    time_t current_time = time(NULL);

    // Loop through the log files
    for (int i = 0; i < 5; i++){
      
      // Get the last modified time of the log file
      time_t last_modified = get_last_modified(log_files[i]);

      // Check if the file was modified in the last 3 seconds
      if (current_time - last_modified >= 2){
        modified = 0;
      } else {
        modified = 1;
        counter = 0;
      }
    }

    if (modified==0){
      counter += 2;
    }

    // If the counter is greater than 60, kill the child processes
    if (counter >= 60){
      Log_writer("Master process terminated after 60 seconds of inactivity.");
      close(fd_log);
      // Kill all the processes
      kill(pid_cmd, SIGKILL);
      kill(pid_motorX, SIGKILL);
      kill(pid_motorZ, SIGKILL);
      kill(pid_world, SIGKILL);
      kill(pid_inspection, SIGKILL);

      return 0;
    }

    // Sleep for 2 seconds
    sleep(2);
  }
}


int main() {
  if ((fd_log = open("log/master.log",O_WRONLY|O_APPEND|O_CREAT, 0666))==-1){
    perror("Error opening log file");
    return 1;
  }

  time( &rawtime );
  info = localtime( &rawtime );

  Log_writer("Master process started.");

  char * arg_list_command[] = { "/usr/bin/konsole", "-e", "./bin/command", NULL };
  char * arg_list_motorX[] = {"./bin/motorX", NULL };
  char * arg_list_motorY[] = {"./bin/motorZ", NULL };
  char * arg_list_world[] = {"./bin/world", NULL };

  pid_cmd = spawn("/usr/bin/konsole", arg_list_command);
  pid_motorX = spawn("./bin/motorX", arg_list_motorX);
  pid_motorZ = spawn("./bin/motorZ", arg_list_motorY);
  pid_world = spawn("./bin/world", arg_list_world);

  // Convert motor x pid to string
  char pid_motorX_str[10];
  sprintf(pid_motorX_str, "%d", pid_motorX);

  // Convert motor z pid to string
  char pid_motorZ_str[10];
  sprintf(pid_motorZ_str, "%d", pid_motorZ);  

  char * arg_list_inspection[] = { "/usr/bin/konsole", "-e", "./bin/inspection", pid_motorX_str, pid_motorZ_str, NULL };  
  pid_inspection = spawn("/usr/bin/konsole", arg_list_inspection);

  // Create the log files
  int fd_cmd = open("log/command.log", O_CREAT | O_RDWR, 0666);
  int fd_motorX = open("log/motorX.log", O_CREAT | O_RDWR, 0666);
  int fd_motorZ = open("log/motorZ.log", O_CREAT | O_RDWR, 0666);
  int fd_world = open("log/world.log", O_CREAT | O_RDWR, 0666);
  int fd_inspection = open("log/inspection.log", O_CREAT | O_RDWR, 0666);

  if(fd_cmd <0 || fd_inspection <0 || fd_motorX<0 || fd_motorX<0 || fd_world<0){
    printf("Error opening FILE");
  }

  // Close the log files
  close(fd_cmd);
  close(fd_motorX);
  close(fd_motorZ);
  close(fd_world);
  close(fd_inspection);

  // Whatchdog funcion call
  watchdog();

  // Get the time when the Master finishes its execution
  time( &rawtime );
  info = localtime(&rawtime);

  Log_writer("Master process terminated.");

  int status;
  waitpid(pid_cmd, &status, 0);
  waitpid(pid_inspection, &status, 0);
  waitpid(pid_motorX, &status, 0);
  waitpid(pid_motorZ, &status, 0);
  waitpid(pid_world, &status, 0);
  
  printf ("Main program exiting with status %d\n", status);
  close(fd_log);
  return 0;
}

