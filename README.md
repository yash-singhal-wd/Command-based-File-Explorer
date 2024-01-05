# Linux based File Explorer
A file explorer for linux systems that runs on terminal in 2 modes- Normal and Command mode.
## Prerequisites

**1. Platform:** Linux 

**2. Software Requirement:**
* G++ compiler
   * **To install G++ :** ```sudo apt-get install g++```

## Steps to run the project
- Open your terminal with present working directory as the project folder. Then run the **g++ main.cpp** command to compile and get the executable.
    ```make```
- Then finally run the project as shown below, 
    ```./a.out```
    
## Assumptions
* Default mode is Normal mode and press **:** to switch to Command mode.
* Press **ESC** key to go back to normal mode from command mode.
* Press **q** key from normal mode to exit from program.
* In Command mode, to edit the command use backspace only.
* Preferable to open terminal in full size. if one record spans over more than one line then navigation won't work correctly.
* if **Enter** is pressed on some file and if there is no default application found then there will be no movement.

## Normal Mode
* Displays a list of directories and files in the current folder 
* The file explorer shows entries “.” and “..” for current and parent directory respectively.
* The file explorer handles scrolling using the up and down arrow keys.
* User can navigate up and down in the file list using the corresponding up and down arrow keys.
* Open directories and files When enter key is pressed.
* Traversal
- Go back - Left arrow key should take the user to the previously visited directory
- Go forward - Right arrow key should take the user to the next directory
- Up one level - Backspace key should take the user up one level
- Home – h key should take the user to the home folder 

## Command Mode
* Copy – ‘$ copy <source_file(s)> <destination_directory>’
* Move – ‘$ move <source_file(s)> <destination_directory>’
* Rename – ‘$ rename <old_filename> <new_filename>’ 
* Create File – ‘$ create_file <file_name> <destination_path>’
* Create Directory – ‘$ create_dir <dir_name> <destination_path>’ 
* Delete File – ‘$ delete_file <file_path>’
* Delete Directory – ‘$ delete_dir <dir_path>’ 
* Goto – ‘$ goto <location>’
* Search – ‘$ search <file_name>’ or ‘$ search <directory_name>’
