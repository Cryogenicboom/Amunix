Welcome to the AMUNIX Tutorial. This Tutorial will trace all the functions, files, concepts to help you build your own shell. Reminder, this is a basic shell prototype covering Lexical analysis, Parsing, C libs, Raw Mode, Jobs, Pipes, File redirection, Memory cleanup, Makefile. Enjoy the process 
<br>

__I would not like to spoonfeed you, hence you should use this material as a guide. This will help you avoid wasting time googling the stuff. But syntax, definations, use-case, that's what you have to do. You know They say "RTFM".__

<br>

**The shell is built for UNIX like systems, Windows users should use "WSL". You can use it with your Code IDE** 

<br>

# Part 0

### 0.0 Prerequisits 
0. You must know C, just the basics that 4 hour tutorials teach you. Other specific libraries and functions will be mentioned, but you have to read the man pages.

1. Core structure of shell is to run a loop; 
> - print the prompt 
> - take user input 
> - perform lexical analysis on the user input 
> - Parsing 
> - execute 

2. Stduy __Standard Output__ and __Standard Input__ and __File Descriptor Table__,this is an important keyword for understanding pipes and Raw mode.

3. RAW MODE: When you write your command in command prompt, you can observe that it only works on your command after you hit the enter key. But Try to press `up arrow key` and `down arrow key`, you might see history of previous commands you used. But how does this work? You never press enter key with those directional keys. Raw mode is your answe then, refer to this [article](https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html). you should learn these concepts before moving ahead: 
- Termios 
- ECHO and ICANON flags 
- get/set attr in TERMIOS 
- Escape Sequences 

4. Follow the code lines as you read the guide. i have mentioned which file and functions to look upon after each title [ file.c/function() ], if it consist of file name only [ file.c/ ] that means you have to look upon the whole file, it might be declared or defined externally.

5. we will follow some commands and trace their route throughout the code and understand different concepts as we comes up with new stuffs.
---

<br>

### 0.1 Prompt [ main.c/int main() ]
- Initialize a loop keeping it's condition always TRUE, this loop will be called __core loop__. We will handle exit with function's call. 
- `getcwd()` function in C retrieves the absolute path of current working directory. It is defined under `unistd` header file. Print it at start of every loop iteration before taking user input. 
- Open up your Terminal and look the first lines it prints, observe it consist of a `$` sign, this gives visual reference to user as end of prompt. 

<br>

### 0.2 Saving Original State and loading RAW MODE. [ main.c/int main() {declared externally }]
- i hope you must have studied above given article on RAW MODE. 
- We would need to initialize two Termios structs, 1. one to store the original state of your terminal which is Cooked mode. 2. And another to handle the raw state. 
- We will use `tcgetattr` to store the current terminal state in both of Termios Struct. 
- A termios struct have multiple flags that define a terminal state, these are called "Bitmask Flags".
- Use one of the struct ( one to handle the raw_state) to Unset the `ICANON` and `ECHO` flags. use `c_lflag` to do so. 
- `ICANON` when unset --> terminal runs in non canonical mode, where terminal operates in "byte-by-byte" rather than "line-by-line". Input is available as soon as a key is pressed. Before that, input used to be stored in buffer and is not passed to terminal unless enter key is pressed.
- `ECHO` when unset --> your input, typed characters are not printed into screen. 
- Use `tcsetattr` to set the attributes of terminal to RAW STATE. 
<br>

> But why do this ? 
> Shell takes your input when you press enter i.e. `\n`, but pressing `up arrow key` and `down arrow key` prints previous commands, hence acting like input too. In raw state every key you press, every character is considered as an input, it's us who will decide how to handle them. 

<br>

### 0.3 Taking input [ main.c/int main(), include/command.h ]
- we need to store changes as we play with the commmand. 
- You would like to store; __user input, tokens, parsed commands.__
- Our main goal is to take user input and change it into a proper command structure that our shell will understand. To do this you must make a structure using `struct` as following; 
> a command argument that usually takes the first word of command. 
> and flag, variable, arguments that are used with command argument. 
- these struct will give your commands an end goal. This structure will be used by shell's executor to execute the commands.

<br>

- since we are in RAW MODE where each keystroke is an input, you need to give each keystroke a meaning. 

> 1. Arrow keys to navigate the history of commands.
> 2. left-right arrow keys to navigate the command line.
> 3. a-z A-z 0-9 `&, |, >, <` as string characters. these are only passed to input buffer once `enter` key is pressed. 
> 4. Backspace key to erase.

### 0.4 Saving Input [ main.c/int main() ]
- Inside the [core loop](#01-prompt--maincint-main-) register user input only when they press `enter` key. Else let them write on shell. 
- Read single character from standard input (stdin) using `getchard()`.
- Write a single character to from standard outoput (stdout) to user_input array using `putchar`. 
- Each user input must end with a `NULL Terminater`, this will be helpful during lexical analysis. So at end of command add a `\0`.

### 0.5 Backspace [ main.c/int main() ]
- Your Input logic must also detect backspace, what ascii code or Caret Notation it generates.
- use backspace escape sequence `\b`.
- remember `\b` only moves your cursor backward. So you want to manually erase content 
> Think hard! i won't spoonfeed. 

### 0.6 History navigation [ main.c/int main(), main.c/int arrow_keys() ]
- In your input logic, you should also handle `^[[A` and `^[[B` for handling UP and DOWN arrow keys respectively. 
- Make a ([Singly Linked list](https://www.geeksforgeeks.org/c/c-program-to-implement-singly-linked-list/)) top store the user input commands throughout the session. 
- use UP arrow keys and DOWN arrow keys to traverse this linked list you made. 
- Flow should be like this 
> When pressed UP arrow key: 
>   1. Erase the current command line. 
>   2. Traverse the linked list using a different pointer. 
>   3. Pass those commnads to user_input array you made. 
>   4. print them on command line using `putchar()`.

<br>

#### That's it you got the intuition, see you in the next part with lexical analysis.
__NEXT PARTS__:
<br>

[PART1](Part1.md)
<br>

[PART2](Part2.md)


