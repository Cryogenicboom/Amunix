Previous Parts : [PART 0](Part0.md)      [PART 1](Part1.md)

# PART 2 : PARSING 

Hello, welcome to Parsing tutorial, this guide is little big with many boring stuffs, so please bear with me. 

### 2.0 : Introduction and Limitations
<br>

- In the previous part, we converted our user input into individual tokens. But there is still one problem. Our shell has the tokens, but it still doesn't understand what they actually mean.
```text
Is "echo" the command?
Is "hello" an argument?
Does | connect two commands?
Is "out.txt" a normal argument or the output file?
```

- Right now, everything is just sitting inside one big array. This is where parsing comes in.
- The parser's job is to understand the meaning of every token and organize them into a proper structure.
- Once the parser finishes its job, executing the command becomes easier because everything is already organized.
<br>

* This parser is **not the proper way** a real shell parser works.
* Most production shells don't directly start interpreting `"|"`, `">"` and words like we are doing here.
* Instead, after tokenization they first generate another layer called **token types** or **token kinds**.

For example,

```text
echo hello | grep txt > out.txt
               ↓
WORD   WORD   PIPE   WORD   WORD   REDIRECT_OUT   WORD
```

or internally something similar to

```text
WORD = 1
PIPE = 2
REDIRECT_OUT = 3
BACKGROUND = 4
```

* After generating these token types, the parser reads those token kinds instead of repeatedly comparing strings like

```c
strcmp(token, "|")
strcmp(token, ">")
strcmp(token, "<")
```

* That approach is cleaner, faster and much easier to extend later.
* So why aren't we doing that?
* Because this project is meant to teach you how a shell works, not how to build Bash from scratch. Parsing directly from strings is much easier to understand for beginners.
* So don't worry if you see people online talking about ASTs, parser generators or token enums. We are trying to keep things simple here.

<br>

### 2.1 : Prerequisites [ parser/parser.c / void parse_struct() ]

- Before reading ` parse_struct() ` , I would highly recommend opening ` command.h ` first. Almost everything this function does revolves around one structure called Command.
- Instead of using lots of arrays and variables, we store everything related to one shell command inside this single structure.
- Think of it as a container. Every field inside this structure stores one specific piece of information. For example, suppose the user enters

```bash
cat input.txt | grep hello > output.txt &
```

This single command actually contains lots of information.
<br>

1. It has two commands connected using a pipe.
2. It has arguments for both commands.
3. It has an output file.
4. It runs in the background.
5. It also has a total command count.

- Instead of remembering all of these separately, we simply fill different members of our structure. Something like

```text
Command
│
|-- simpleCommands
|-- inputfile
|-- outputfile
|-- bg_status
|-- count
```

- Later, when the executor runs, it doesn't have to figure anything out again. It simply reads this structure and knows exactly what needs to be executed.

- __The parser is not executing commands, it is only organizing information.__

### 2.2 Initializing the Structure [ parser/parser.c / void parse_struct() ]

- Before we begin reading the tokens, the very first thing we do is prepare our Command structure.
think of it like cleaning your desk before starting a new assignment. If old information is still lying around, there is a good chance it will mix with the new one. The same thing can happen with our shell.

```c
int c_idx = 0;
int argc = 0;

cmd->count = 1;
cmd->inputfile = NULL;
cmd->outputfile = NULL;
cmd->bg_status = 0;
```

Let's understand why each of these variables exists.

1. Command Index

```c
int c_idx = 0 
```
stands for Current Command Index. Remember that one shell command can actually contain multiple commands because of pipes. For example,

```bash
ls | grep txt | wc
```

- This is not one command. It is Combination of 3 different commands (ls, grep, wc) joined by pipes.

- So while parsing, we need to know which command we are currently filling. That's exactly what c_idx keeps track of.
- Since we always begin with the first command, its value starts from 0.

2. Argument Index 

```c
int argc = 0;
```
- This one is easy to understand. Every command contains arguments. Arguments are like providing a command / a fucntion some resources. For example,
```bash
grep hello file.txt
```

where, argv[0] = grep, argv[1] = hello, argv[2] = file.txt

- As we keep discovering new arguments, we need to know where to place them.` argc ` simply points to the next free position inside ` argv `.
- Every time we add another argument, this value increases by one.

3. Number of Commands

```c
cmd->count = 1;
```
- You may wonder, why are we starting from 1 instead of 0 ? Because before seeing any pipes, we already have one command. For example, ` pwd ` contains 1 command.
- Similarly, ` echo hello ` also contains 1 command.
<br>
 
- A new command is only created when we encounter a pipe. For example, ` ls | grep txt `
The moment we see ` | `, we increase this count. Until then, ` cmd->count = 1; ` is good.

4. Input and Output Files
```c
cmd->inputfile = NULL;
cmd->outputfile = NULL;
```

- These variables will later store filenames used for redirection. For example,
```bash
cat < input.txt
```

- becomes inputfile => input.txt. Similarly,

```bash
echo hello > output.txt
```
- becomes outputfile => output.txt

- But at starting, we haven't parsed anything. We don't know whether the user even used ` < ` or ` > `. So both pointers should start as ` NULL `. NULL simply means "No file has been assigned yet."

5. Background Status

```c
cmd->bg_status = 0;
```

- Later studied, if the user writes ` sleep 10 & ` this value will become 1 . But before reading the command, we assume every process runs on foreground. So we initialize it with 0.

6. Resetting Every Command
```c
for(int j = 0; j < 10; j++)
{
    cmd->simpleCommands[j].argc = 0;
}
```

- Remember that our shell keeps running forever. It parses one command then executes it. Then parses another command, then another.

- If we never reset these values, some information from the previous command may still remain inside the structure.

- This is why we reset everything before starting a new parse. It is always better to start with an empty structure than trying to guess which values should be reused.

- At this point, our parser has not read a single token. All we have done is prepare an empty structure that is ready to receive information.
- So lets begin reading the tokens one by one and deciding what each one means.

### 2.3 Traversing Token by Token [ parser/parser.c / void parse_struct() ]

- Until now, our parser was only preparing an empty Command structure. But now comes the actual implementation.
- Instead of reading the user input character by character like our tokenizer did, we now read one token at a time.
- Remember, the tokenizer has already done the hard work of splitting the command for us.

- So instead of looking at individual characters, our parser simply walks through this token array.
```c
for(int i = 0; tokens[i] != NULL; i++)
```

- This loop starts from the first token and keeps moving forward until it reaches NULL.
- Just like '\0' tells that a string has ended, NULL tells us that there are no more tokens left to read.
- During every iteration, the parser asks one simple question.
```text
"What kind of token am I currently looking at?"
```
- Depending on the answer, it performs different actions. If the token is a normal word, store it as part of the current command. 
```text
1. If it is a pipe ( | ), finish the current command and begin a new one. 
2. If it is ` > `, the next token must be the output filename. 
3. If it is ` < `, the next token must be the input filename. 
4. If it is ` & `, mark the command as a background process.
```

- So the entire parser is just repeatedly asking this one question for every token. "What should i do with the current token?"

### 2.4 : Handling Normal Words [ parser/parser.c / void parse_struct() ]

- Let's begin with the easiest case. Suppose the current token is neither a pipe, nor a redirection operator, nor &, just a word. For exampl ` echo hello world `

- Our tokenizer has already converted it into ` [ "echo" ] [ "hello" ] [ "world" ] `. None of these are special operators. They are simply words. So where should they go? They belong inside the current command.

This is exactly what the final else block do.

```c
else
{
    cmd->simpleCommands[c_idx].argv[argc] = tokens[i];
    argc++;
    cmd->simpleCommands[c_idx].argc = argc;
}
```

- Let's understand it one line at a time. 
#### 1. 
```c
cmd->simpleCommands[c_idx].argv[argc] = tokens[i];
```

This simply copies the current token into the current command's argument list. Suppose we are parsing <br>
```bash 
grep hello file.txt
```
- Initially, ` argc = 0 ` so the parser stores ` argv[0] -> grep `. 
- Then, ` argc = 1 `, therefore it stores ` argv[1] -> hello ` 
- Next ` argc = 2 `, thus ` argv[2] -> file.txt `

#### 2. 
- By the end, every normal word simply occupies the next available position. After storing the word, we write
```c
argc++;
```

- Think what would happen if we never increased argc. Suppose we parsed
```bash
echo hello world
```

- Without incrementing argc, we would keep writing into ` argv[0] ` again and again. Everything would be overwritten. That's why after storing one argument, we immediately move to the next empty position.

#### 3.

```c
cmd->simpleCommands[c_idx].argc = argc;
```

- This line keeps track of how many arguments the current command has. For example, ` grep hello file.txt ` contains 3 arguments. So,  ` argc = 3 ` is saved inside the structure.

- Later, if we ever need to know how many arguments belong to this command, we don't have to count them again. The parser has already done that work for us.

- Don't be confused there isn't much logic involved, we are just copying words one after another.
The parser only becomes interesting when it encounters a special token like ` | `,  ` < ` or ` > `. These tokens don't become arguments, but they change how the command should be interpreted.
- That's exactly what we'll look at in the next section when we handle pipes.


__NOTE : You will learn Pipes and FD table in PART-3, just know what is pipe.__

### 2.5 : Handling Pipes [ parser/parser.c / void parse_struct() ]

- Until now, every token we encountered was stored inside the ` current command ` structure. But the moment we encounter a pipe (`|`), things become a little different. A pipe does not belong to either command as an argument. Instead, it acts as a separator and tells the parser, __"The current command has finished, now start building the next one."__

- For example,
```text
ls -l | grep txt
```

After tokenization, our parser receives

```text
["ls"] ["-l"] ["|"] ["grep"] ["txt"] [NULL]
```

When the parser reaches `|`, it already knows that `"ls"` and `"-l"` belong together. Everything after the pipe should belong to another command. This is why we cannot simply continue storing arguments in the same `argv` array.

- Before creating the next command, you should perform a small syntax check. It is always better to reject an invalid command early than let it fail later during execution. So let user know they wrote wrong command.

- Once we know the current command is valid, we terminate its argument list.

```c
cmd->simpleCommands[c_idx].argv[argc] = NULL;
```

Remember from the previous section that every command stores its arguments inside an `argv` array. Just like normal C strings end with `'\0'`, an argument vector ends with a `NULL` pointer.

Suppose we have parsed

```text
ls -l
```

The array now becomes

```text
argv :

[0] -> "ls"
[1] -> "-l"
[2] -> NULL
```

This `NULL` is very important because later, when we call `execvp()`, it uses this marker to know where the argument list ends.

- After finishing the current command, we simply move to the next one.

```c
c_idx++;
cmd->count++;
argc = 0;
cmd->simpleCommands[c_idx].argc = 0;
```

- Each of these lines has a specific purpose.

* `c_idx++` tells the parser that we are now working on the next command.
* `cmd->count++` increases the total number of commands present in this pipeline.
* `argc = 0` resets the argument index because the next command starts with its own `argv[0]`.
* Finally, we also reset that command's argument count to zero.

- Notice that the __pipe itself is never stored inside the structure__. Its only purpose is to tell the parser where one command ends and the next one begins. Once that job is done, the parser simply moves on to reading the remaining tokens.

<br>

### 2.6 : Handling Special Tokens [ parser/parser.c / void parse_struct() ]

- Until now, every token we encountered was simply becoming part of the current command. But shell commands also contain special tokens like `|`, `<`, `>`, and `&`. These are 
__not arguments__. Instead, they tell the shell __how__ the command should be executed. This is why the parser treats them differently.

- The pipe (`|`) marks the end of one command and the beginning of another. Before moving to next command, we terminate the current `argv` with `NULL`, increase the command count, and reset the argument index. This ensures every command in a pipeline gets its own independent argument list.

```text
ls -l | grep txt

       ↓

Command 0 : [ "ls", "-l", NULL ]
Command 1 : [ "grep", "txt", NULL ]
```

- Redirection operators work differently. When the parser encounters `<` or `>`, it knows that the **next token is no longer an argument**, but a filename. Instead of storing it inside `argv`, it is copied into either `inputfile` or `outputfile`, and the parser skips over it since it has already been processed.

```text
echo hello > out.txt

         ↓

outputfile → "out.txt"
```

- Finally, the background operator (`&`) is the simplest one. It does not create a new command or require another token. The parser simply remembers that this command should run in the background by setting `bg_status`. The executor will later use this flag to decide whether it should wait for the process or immediately return control to the user.

<br>

### 2.7 : Handling Quoted Strings [ parser/parser.c / void parser_for_quotes() ]

- If you remember Part 1, I mentioned that our tokenizer intentionally does not handle quoted strings. It simply splits the input whenever it encounters whitespace. This means a command like

```text
echo "Hello World"
```

initially becomes

```text
["echo"] ["\"Hello"] ["World\""]
```

which is not what we want. `"Hello World"` should be treated as a single argument.

- Instead of making the tokenizer more complicated, we solve this problem in a separate function called `parser_for_quotes()`. Its job is simple: whenever it finds a token beginning with `"`, it keeps joining the following tokens until it reaches the closing quote. The quotes are then removed, and the entire sentence is stored as one token. 

```text
["echo"] ["Hello World"]
```

- Go write code, i am not gonna spoonfeed you. 
<br>

### 2.8 Conclusiion 

That's it for parsing.
<br>
This was a little harder than tokenization. Instead of splitting text, we had to understand what every token means and organize them into something our shell can work with. 
If some parts didn't click immediately, don't worry. 
<br>

open `parser.c`, and try following the execution with a debugger or a few `printf()` statements. You know i add printf lines called debug lines to see what is happening behind the scene. You may find them in older version of AMUNIX or somewhere commented out.
<br>

We have successfully converted a raw command into a structured representation. From this point onward, the executor simply follows the instructions we have already prepared.
<br>

In the next part, you will finally stop organizing data and start doing the fun part, creating processes, setting up pipes, redirecting files, and executing commands using `fork()` and `execvp()`. This is where our shell actually starts behaving like a shell.
<br>

Whoops... that was a long one. If you somehow made it till here without questioning your life choices, congratulations. You're officially one step closer to writing your own shell. 

As always, if you find any mistakes (or if I wrote something dumb at 2 AM), feel free to open an issue or a pull request. I'm always happy to improve the project. 
<br>

Gotta earn money, i need food. See ya!

<br>

Next Parts:

<br>

[PART3](Part3.md)


