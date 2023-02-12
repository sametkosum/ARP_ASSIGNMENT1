 ADVANCED & ROBOT PROGRAMMING(ARP) ASSIGNMENT 1
================================

## This Assignment Made By:

1. Ahmet Samet KOŞUM    S5635830
2. Mustafa Melih TOSLAK S5431021

----------------------------------------

## Goal Of The Assignment 

1. To design an interactive simulator of a hoist with two degrees of freedom (2 d.o.f)
2. To develop two different consoles for user interaction with the hoist
3. To simulate the motion of the hoist along the X and Z axes with user commands
4. To simulate the real-time position of the hoist and report it on the inspection console
5. To simulate environmental effects on the motion of the hoist
6. To provide emergency stop (S) and reset (R) functionality
7. To simulate the motion of the hoist along the X and Z axes with two separate processes (motor X and motor Z)
8. To develop a watchdog process to monitor the overall system and reset it in case of inactivity for a certain period of time (60 seconds)

<p align="center" width="100%">
    <img width="100%" height="350" src="https://user-images.githubusercontent.com/58879182/218331460-47d5b789-a187-4209-abcb-534f2c5f1d69.png">
    
</p>

----------------------------------

## Introduction

The project aims to control a hoist with two degrees of freedom, allowing the user to input commands to control its movement. The hoist has two motors: one for movement on the x-axis and the other for movement on the z-axis.

---------------------------------

## Command Console

The first window is the command console, where the user can input commands by clicking on buttons. There are six buttons in total: three buttons each for controlling the velocity on the x-axis and z-axis. The velocity steps can range from -2 to 2 with two velocity steps. The command console also includes two pipes to send inputs to the motor x and motor z processes.

<p align="center" width="100%">
    <img width="100%" height="350" src="https://user-images.githubusercontent.com/58879182/218332141-a8f08313-6d4b-4689-910e-2f2c654badcc.png">
    
</p>

---------------------------------

## Motor x and Motor z Processes

The second and third processes are the motor x and motor z windows, respectively. The pipes from the command console are read and the inputs received. The velocity and position of the hoist are then calculated every second. Pipes are also created to send the position values to the world console.

-----------------------------------

## World

The fourth process, the world , receives data about the position from the motor x and motor z processes. It simulates the real-life behavior of the hoist, taking into account environmental factors such as friction and wind by adding random values to the received data. The updated "real position" is then sent to the inspection console via a pipe.


-----------------------------------

## Inspection Console

The inspection console is used to show the movement of the hoist to the user. It includes two important buttons: "R" (Reset), which moves the hoist to its initial position on the x and z axes, and "S" (Stop), which stops all processes immediately.

<p align="center" width="100%">
    <img width="100%" height="350" src="https://user-images.githubusercontent.com/58879182/218332314-cb219b8e-7e5f-4dbc-a5e0-a185667f72f6.png">
    
</p>


------------------------------------------

## Watchdog Process

The sixth and final process is the watchdog, which kills all processes and itself if no buttons are pressed for 60 seconds.

## How to Install and Run

1- Install the ncursers Library:
```command
sudo apt-get install libncurses-dev
```
                                     
2- Change the permissions of .c files inside the src/ folder to be executable by running: 
```command
chmod +x *.c
```
3- Change the permissions of .sh files inside the ARP_Assignment_1/ folder to be executable by running:
```command
chmod +x compile.sh
chmod +x run.sh
```
4- Compile the code by running:
```command
./compile.sh
```
6- Run the code by typing:
```command
./run.sh
```
------------------------------------------

## Troubleshooting

In the event that any unusual issues arise after starting the application, such as buttons not appearing in the GUI or graphical elements appearing misaligned, simply try resizing the terminal window as a solution.

--------------------------------------

## Conclusion

The project has successfully achieved its goal of allowing the user to control the movement of a hoist with two degrees of freedom. By using the command console, motor x and motor z processes, world console, inspection console, and watchdog process, the movement of the hoist can be accurately simulated and monitored.

