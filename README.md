### Assignment 1 : File Explorer

##### COMPILATION
```
g++ --std=c++14 main.cpp -o main
```
##### STARTING THE EXPLORER
```
./main
```
##### NORMAL MODE DETAILS
-	Up arrow and down can be used for scrolling.
-	k and l can be used to change window.
-	If filename or other fields are too long to fit to screen the fielname is truncated and ".." is added at the end.
-	Enter on directory goes to the directory.
-	Enter on file will open the file in vi.
-	-> and <- can be used to navigate between users actions.
-	'h' will land on the location from where the application started.
-	':' press will initialize command mode.
-	'q' will exit the application.

##### COMMAND MODE DETAILS
-	~ Referes to location from where application started.
-	. Refers to the current location you are in
-	/ Refers to system root.
-	If simply filenames are mentioned it is assumed to be in current path the application is in.
-	Esc key will exit to normal mode

Examples of path resolutions
```
rename abc.txt def.txt // Renames abc.txt in current path to def.txt
```
```
copy dir1 dir2 // Copies dir1 in current path to dir2 in current path
```
```
copy ~/dir1 dir2 // Copies dir1 in app root to dir2 in current path
```
```
copy /dir1 dir2 // Copies dir1 in / to dir2 in current path
```
```
copy dir1 ~ // Copies dir1 in curret path to app root
```
```
copy dir1 ./dir2 // Copies dir1 in current path to dir2 in current path
```
