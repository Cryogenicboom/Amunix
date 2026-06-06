Welcome to the AMUNIX guide. This guide will trace all the functions, files, concepts to help you build your own shell. Reminder, this is a basic shell prototype covering Lexical analysis, Parsing, C libs, Raw Mode, Jobs, Pipes, File redirection, Memory cleanup, Makefile. Enjoy the process 
<br>
**The shell is built for UNIX like systems, Windows users should use "WSL". You can use it with your Code IDE** 
<br>

### 0.0 Prerequisits 
0. You must know C, just the basic that 4 hour tutorials teach you. Other specific libraries and functions will be mentioned, but you have to read the man pages, because "RTFM".

1. Core structure of shell is to run a loop; 
> - print the prompt 
> - take user input 
> - perform lexical analysis on the user input 
> - Parsing 
> - execute 

2. we will follow some commands and trace their route throughout the code and understand different concepts as we comes up with new stuffs. 

3. RAW MODE: When you write your command in command prompt, you can observe that it only works on your command after you hit the enter key. But Try to press `up arrow key` and `down arrow key`, you might see history of previous commands you used. But how does this work? You never press enter key with those directional keys. Raw mode is your answe then, refer to this [article](https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html). you should learn these concepts before moving ahead: 
- Termios 
- ECHO and ICANON flags 
- get/set attr in TERMIOS 
- Escape Sequences 
- File Descriptor Table (FD)

4. Follow the code lines as you read the guide. i have mentioned which file and functions to look upon after each title [ file.c/function() ], if it consist of file name only [ file.c/ ] that means you have to look upon the whole file, it might be declared and defined externally.

### 0.1 Prompt [ main.c/int main() ]
- Initialize a loop keeping it always TRUE, we will handle exit with functions. 
- `getcwd()` function in C retrieves the absolute path of current working directory. It is defined under `unistd` header file. Print it at start of every loop iteration before taking user input. 
- Open up your Terminal and look the first lines it prints, observe it consist of a `$` sign, this gives visual reference to user as end of prompt. 

### 0.2 Saving Original State and loading RAW MODE. [ main.c/int main() {declared externally }]
- i hope you must have studied above given article on RAW MODE. 
- We would need to initialize two Termios structs, 1. one to store the original state of your terminal which is Cooked mode. 2. And another to handle the raw state. 
- We will use `tcgetattr` to store the current terminal state in both of Termios Struct. 
- A termios struct have multiple flags that define a terminal state, these are called "Bitmask Flags".
- Use one of the struct ( one to handle the raw_state) to Unset the `ICANON` and `ECHO` flags. use `c_lflag` to do so. 
- `ICANON` when unset --> terminal runs in non canonical mode, where terminal operates in "byte-by-byte" rather than "line-by-line". Input is available as soon as a key is pressed. Before that, input used to be stored in buffer and is not passed to terminal unless enter key is pressed.
- `ECHO` when unset --> your input, typed characters are not printed into screen. 
- Use `tcsetattr` to set the attributes of terminal to RAW STATE. 
> But why did this ? 
> Shell takes your input when you press enter i.e. `\n`, but pressing `up arrow key` and `down arrow key` prints previous commands, hence acting like input too. In raw state every key you press, every character is considered as an input, it's us who will decide how to handle them. 

### 0.3 Taking input [ main.c/int main() ]




