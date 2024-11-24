Dear Professor,

Sri Sruti here! I encountered an issue where Webots does not allow me to run the same world file I created and worked on using my device on other devices. To resolve this, please follow these steps:

Steps to Resolve the Issue

1. Locate the Controller Files:
Navigate to the controllers directory and find the folder named epuckk_moves. Inside, you will see the following files:
        epuckk_moves.c
        epuckk_moves.exe
        build folder
        makefile

2. Delete Specific Files:
Delete the following files and folder:
        epuckk_moves.exe
        build folder
        makefile

3. Verify Controller Settings in Webots:
Open Webots and ensure the e-puck robot's controller is correctly set to epuckk_moves.c.

4. Build the Controller:
Use Webots to build the controller. This step will regenerate the required files and allow the simulation to run properly.

Thank you!