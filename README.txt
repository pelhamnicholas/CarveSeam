Author:
	Nicholas Pelham (npelh001@ucr.edu)

Required Libraries:
	OpenCV <opencv.org>
		Core
		HighGUI

Compiling the program:
	This program submision includes a Makefile to make compiling and linking the 
	proper files easy. Simply run make from the program directoy.

Running the program:
	The program is intended to be run from the terminal window. The name of the 
	file that is to be traced is passed as an argument from the ternimal window. 
	Sample: "npelh001_program02 input0large.txt".
	Acceptable file extensions are: ".txt" ".png" and ".jpg"

Expected Output:
	<input_filename>.txt
		<input_filename>_trace.txt
			A text file that lists the minimum seam cost, and each vertex involved in
			that cost.
			The input_filename is printed to the terminal

	<input_filename>.png -or- <input_filename>.jpg
		<input_filename>_trace.png -or- <input_filename>_trace.jpg
			A jpeg image with the minimum seam highlighted in red.
			The minimum seam is printed to the terminal
