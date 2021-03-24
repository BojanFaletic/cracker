# uart_id

Guess ID for microprocessor. 

# How to run

1) Install Python+3.5, pip

``` shell
 $ pip install -r requrements.txt
```

2) Connect logic analizer: TX = channel 0, RX = channel 1
3) Set baud rate to 115200
4) For now serial port is 'COM3'

``` shell
$ python main.py
```

# Output

* Log of program is stored in program.log. 
* Folder tree contains explored graph in .pkl file, this is used
for restoring state in case of early termination. 
* Folder checkpoint contains .npy file of key this is for debugging purposes. 
* Folder plots contains images of average time, per digit, used for debugging.
