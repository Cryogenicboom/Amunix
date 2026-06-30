Hey you made it to PART 3 [ you didn't skip previous parts, right ? ]. This is where we play with POSIX systems calls. Before continuing, I highly recommend reading about fork(), exec(), dup2(), Child process, process groups and file descriptor tables. The executor becomes much easier after understanding these.
<br>

# PART 3.0 : EXECUTION
<br>

Welcome to the final stage of our shell. Until now, our shell has been doing only preparation work. In Part 1, we converted the user's command into individual tokens. Then in Part 2, we organized those tokens into a structure that the shell could understand.
<br>

our shell already knows

- which commands need to be executed
- what arguments belong to each command
- whether files should be redirected
- and whether the command should run in the background.

But there is one important thing we still haven't done. __Nothing has been actually executed yet.__
<br>

Our parser only prepares information. It never creates a process or runs a program. This is where the executor comes in. The executor takes the structure created by the parser and starts turning it into real running programs. It creates new processes, connects pipes, redirects files, and asks the operating system to execute the command.
<br>

Just like Part 2, this is __not exactly how a production shell works__.
<br>

Our goal is much simpler. We only want to understand the core idea behind execution. Once you understand this file, most Unix shells will stop looking like magic and start looking like ordinary C programs using powerful system calls.
<br>

Trying not to write anything dumb at 2:00 AM, here we go; 

### 3.1 : Prerequisites [ main.c / builtins.c / executor.c ]

- Before opening `executor.c`, I recommend opening `main.c` first. Many people assume every command immediately reaches the executor. But that isn't true.
- Every command first passes through one small checkpoint.

```c
if(built_ins(parsed_cmds) == 0)
{
    continue;
}

execute_command(&cmd, cmd.count);
```

- This decides __who should execute the command__

- Some commands are handled by the shell itself. These are called __built-in commands__. You must have seen them in README in table of commands.

```text
dbd , bahar ,jobs , fg , bg
```

- Everything else is treated as external program and is passed to the executor.

```text
ls, cat , grep , gcc , python
```
- __The executor only works with `external commands` . So before reading `execute_command()`, remember that every command has already been parsed, and built-in commands have already been filtered out.__

- Now lets see how our shell starts running programs.

### 3.1 : Built-in Commands [ main.c / builtins.c / int built_ins() ]

- Before our shell creates a new process, it first asks one simple question.

> "Can I execute this command myself?"

- This is done because not every command should create a child process. Some commands need to directly modify the shell itself. This decision happens inside `main.c`.

```c
if(built_ins(parsed_cmds) == 0)
{
    continue;
}

execute_command(&cmd, cmd.count);
```

- Here, `built_ins()` checks whether the entered command belongs to a list of commands implemented by our shell.

- If it returns `0`, the shell knows that the command has already been executed, so there is nothing left to do. It simply moves on and waits for the next user input.

- Else the command is treated as an external program and passed to the executor.

- You may wonder, why can't we simply execute every command using `fork()` and `execvp()`? consider the command

```bash
dbd Documents         ( dbd is our version of "cd" )
```

- Suppose we created a child process and asked it to change its current directory.

```text
Shell
   |
fork()
   |
   |--> Parent (still in old directory)
   |
   |--> Child
         |
         |--> chdir("Documents")
```

- The child successfully changes its working directory. But after finishing, the child exits. The parent shell is still standing inside the old directory. From the user's point of view, nothing changed, parent ( which is our shell ) is still in the old directory.

- That's exactly why commands like " dbd, fg, bg are called __built-in commands__. They directly modify the shell itself, therefore they need to be executed inside the shell itself, not inside a child process.

- Other commands like " ls, cat, grep " don't modify the shell. They simply perform some work and terminate. These commands are called __external commands__, and this is where our executor does work.

- In this project, our built-in function is simply a long list of `if-else` statements.

```c
if(strcmp(parsed_cmds[0], "dbd") == 0)
```

- This may not be the most efficient implementation, but it is very easy to understand. As your shell grows, you could replace this with a command table or a hash map for faster lookups.

- Once a command is identified as an external program, our shell has only one job left.

> "Create a new process and ask the operating system to execute it."


### 3.2 : Preparing for Execution [ executor.c / void execute_command() ]

- Now that our shell knows the command is not a built-in, it finally hands control to the executor.

```c
execute_command(&cmd, cmd.count);
```

- Remember, our parser has already done all the difficult work. The executor doesn't need to figure out where the pipes are or which file should be redirected.

- It simply reads the `Command` structure and starts preparing everything required to execute it.

- The very first thing we need to decide is "Where should the first command read its input from ? ". Normally, every program reads from the keyboard, also called standard input ( `stdin` ). I hope you have read about File Descriptor Table.

```text
Keyboard
    │
    ▼
 stdin (fd 0)
    │
    ▼
Program
```

- But shell commands can also redirect their input.

```bash
sort < names.txt
```

- But here ` sort ` do not reads from the keyboard. Instead, it should read from ` names.txt `. This is exactly why our executor first prepares an input file descriptor.

```c
int fdin;
```

- Think of `fdin` as __"Where should the current command read its input from?"__. If the parser found an input redirection ` < `, we open that file, and make the command read this file instead from keyboard.

```c
if(cmd->inputfile != NULL)
{
    fdin = open(cmd->inputfile, O_RDONLY);
}
```

- ` open() ` asks the operating system to open the file and returns a file descriptor. This descriptor is simply a small integer that represents an opened file.

- The ` O_RDONLY ` flag tells the operating system that we only want to read from this file. But what if the user didn't use ` < `? For example,

```bash
ls
```

- In this case, the command should behave normally and read from the terminal.

```c
else
{
    fdin = dup(0);
}
```

- Here we use `dup()`. It creates a copy of an existing file descriptor. Since file descriptor ` 0 ` represents ` `stdin`, ` dup(0) ` creates another descriptor pointing to the same terminal input.

- You may wonder, why don't we simply write

```c
fdin = 0;
```

- We could, but later during execution, ` fdin ` will be replaced again and again as commands are connected using pipes. By creating a duplicate, we keep the original standard input untouched while allowing the executor to freely close and replace ` fdin ` whenever required.

- At this point, we have prepared the input source for the first command. It may be the keyboard. It may be an input file. The executor doesn't really care. It simply knows that `fdin` always points to wherever the next command should read from.

- Once the input is ready, we can finally begin creating new processes and executing commands. 

### 3.3 : Creating New Processes [ executor.c / void execute_command() ]

- At this point, our executor knows where the first command should read its input from. Now comes the interesting part. Every external command must run inside its own process. For example,

```bash
ls | grep txt | wc
```

Remeber, This may look like a single command, but it actually consists of three independent programs.

```text
Command 0 : ls
Command 1 : grep txt
Command 2 : wc
```

- Since each program runs independently, our shell must create a separate process for each one. This is why the executor loops over every command stored inside our `Command` structure.

```c
for(int i = 0; i < cmd_count; i++)
```

- Every iteration of this loop is responsible for launching one command from the pipeline.

- Before creating the new process, there is one more thing we need to do.

```c
int fd[2];
```

- This array will later hold the two file descriptors returned by ` pipe() `. I hope you have read about pipes working. 

```text
fd[0] -> Read End
fd[1] -> Write End
```

- But notice that we dont always create a pipe.

```c
if(i < cmd_count - 1)
{
    pipe(fd);
}
```

- You may wonder, why dont we simply create a pipe for every command?

- Consider the command

```bash
ls | grep txt | wc
```

The connection looks something like this.

```text
ls -----> grep -----> wc
```

- The first command sends its output to the second command. The second command sends its output to the third command.But the last command has nobody to send its output to. Its output simply appears on the terminal. Creating another pipe for the last command would therefore serve no purpose. That's why we only create a pipe when another command still exists after the current one.

- Once everything is prepared, we finally ask the operating system to create a new process.

```c
pid_t pid = fork();
```

- `fork()` creates an almost identical copy of the current process.

```text
          Shell
            |
         fork()
        |---|----|
        │        │
     Parent     Child
```

- Both processes continue executing from the next line after ` fork() `. The only difference is the value returned by `fork()`.

```text
Child  -> returns 0

Parent -> returns Child's PID

Error  -> returns -1
```

- This allows both processes to know who they are without requiring any additional communication.

```c
if(pid == 0)
```

- This block executes the child process.

```c
else if(pid > 0)
```

- This block executes the parent shell.

```c
else
```

- This handles case where `fork()` fails and no child process could be created.

- From this point onward, the parent and child begin performing completely different jobs. The child prepares itself to execute the requested program. The parent stays alive so it can continue managing the shell.

### 3.4 : The Child Process [ executor.c / void execute_command() ]

- After `fork()`, we now have two independent processes. The parent continues managing the shell, while the child prepares itself to execute the requested program.

- The very first thing our child does is create its own process group.

```c
setpgid(pid, 0);
```

- You don't need to worry too much about process groups right now, think of them as a way to organize related processes together. They become useful later when we implement foreground and background jobs.

- Now to the important part. Remember the variable ` fdin ` from the previous section? It always points to where the current command should read its input from. Depending on the command, it could be

```text
Keyboard or Input file or Previous Pipe 
```

- But programs like ` ls `, ` grep `, ` cat ` or ` wc ` don't know anything about our variable ` fdin `.They simply read from standard input (`stdin`). So before executing the program, we need to make ` stdin ` point to whatever ` fdin ` currently represents.

```c
dup2(fdin, 0);
close(fdin);
```

- ` dup2() ` duplicates one file descriptor onto another.

```text
dup2 (oldfd, newfd)
```

In our case,

```text
dup2 (fdin, 0)
```
means
<br>

> "Replace standard input (`fd 0`) with `fdin`."

- After this call, whenever the program reads from ` stdin `, it is actually reading from whatever ` fdin ` was pointing to. Since ` stdin ` has already been updated, the old descriptor is no longer needed.

```c
close(fdin);
```

- Closing unused file descriptors is a good habit. It prevents descriptor leaks and ensures resources are released when they are no longer required. We don't want Deadlocks :(

- Next, we prepare the command's output.

```c
int fdout;
```

- Just like ` fdin ` represented the current input source, ` fdout ` represents

> "Where should this command write its output?"

- If this is the last command in the pipeline,

```c
if(i == cmd_count - 1)
```

the output has two possibilities.

- If the parser found an output redirection (`>`), we open that file.

```c
fdout = open(cmd->outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
```

- This opens the file for writing. If the file doesn't already exist, it is created. If it already exists, its previous contents are removed before writing the new output. 

- Otherwise, we simply duplicate the terminal's standard output.

```c
fdout = dup(1);
```

- This means the command prints directly to the terminal, just like any normal Unix program.

- But what if this is not the last command? For example,

```bash
ls | grep txt
```

- Here, `ls` should not print on the terminal. Instead, its output should travel through the pipe to the next command.

```c
fdout = fd[1];
close(fd[0]);
```

- Remember that every pipe has two ends.

```text
fd[0] -> Read End

fd[1] -> Write End
```

- Since the child only writes into the pipe, it immediately closes the read end. Keeping unused pipe ends open may cause unexpected behaviour later, so we close them as soon as they are no longer needed.

- Finally, just like we redirected the input, we redirect the output.

```c
dup2(fdout, 1);
close(fdout);
```

- This replaces standard output ( `stdout` ) with whatever `fdout` currently points to. Depending on the command, the program will now write to the terminal or an output File or a Pipe.

- At this point, both `stdin` and `stdout` have been completely prepared. The child process is ready to execute the requested program.

```c
execvp(cmd->simpleCommands[i].argv[0],
       cmd->simpleCommands[i].argv);
```

- `execvp()` asks the operating system to replace the current child process with the requested program. If everything succeeds, the child never returns from this function. The oOS completely replaces our shell code with the new program.

```text
Child Process
      |
      v
execvp()
      |
      v
   grep
```

- But if something goes wrong, `execvp()` returns instead of replacing the process.

```c
perror("execvp failed");
_exit(1);
```

- `perror()` tells us why execution failed. Then we immediately terminate the child using `_exit()`.

- Notice we use ` _exit() ` instead of the normal ` exit() ` because we only want the child process to terminate. It exits immediately without doing any additional cleanup, that will be done by the parent shell.

- Once the child begins executing the requested program, the parent process has a completely different job to perform. That's what we'll look at in the next section.

### 3.4 : The Parent Process [ executor.c / void execute_command() ]

- While the child prepares itself to execute the requested program, the parent shell has a completely different responsibility. The shell should not disappear after creating one process. It must stay alive so it can continue accepting commands from the user.

- This is exactly why the parent enters

```c
else if(pid > 0)
```

- The first thing the parent does is change the child into its own process group.

```c
setpgid(pid, pid);
```

- Just like we briefly discussed in the previous section, process groups help the operating system treat related processes as one unit. We will not go much deeper into process groups here. Just remember that they are useful when handling foreground and background jobs. Read on your own, I wont spoonfeed.

- Now comes an important decision.

```c
if(cmd->bg_status != 1)
```

- Remember from Part 2 that our parser stores whether the command ends with `&`. For example,

```bash
sleep 10 &
```

During parsing,

```text
bg_status = 1
```

This single variable tells the executor whether the command should run in the foreground or the background.
<br>

- If the command is a normal foreground command, we temporarily hand over the terminal to the child process.

```c
tcsetpgrp(STDIN_FILENO, pid);
```

- You can think of the terminal as having only one active owner at a time. Normally, the shell owns it. But when a foreground command starts, the shell steps aside and lets the child receive keyboard input instead.

- This is why programs like ` vim `, ` nano ` or ` cat ` can directly receive keyboard input while they are running.

- But background commands work differently.

```c
else if(cmd->bg_status == 1)
```

- The shell do not give the terminal to the child. Instead, it stores some information inside its job table.

```c
job_tble[i].pid
job_tble[i].gid
job_tble[i].job_id
```

- Think of the job table as a small notebook maintained by the shell.

```text
Job ID     PID      Status

1          4312     Running
2          4320     Running
```

- Later, built-in commands like ` jobs `, ` fg `, ` bg ` simply read this table to know which background jobs currently exist.

- After creating the child, the parent has one more important responsibility.

```c
close(fdin);
```

- Remember that ` fdin ` was only needed while launching the current command. Once the child has received it, the parent no longer needs this descriptor. If another command still exists in the pipeline,

```c
if(i < cmd_count - 1)
{
    close(fd[1]);
    fdin = fd[0];
}
```

the parent prepares the next iteration. The current pipe now becomes the input source for the next command.

```text
Command 1 ----> Pipe ----> Command 2
                 ^
                 |
                fdin
```

- This is why we keep updating ` fdin ` after every command. It allows the pipeline to flow from one process to the next.

<br>

- Once every command has been created, the executor exits the loop. If the command was running in the foreground, the shell waits until all child processes finish.

```c
for(int i = 0; i < cmd_count; i++)
{
    wait(NULL);
}
```

- `wait()` simply tells the shell,

> "Pause here until one of your child processes exits."

- This prevents the shell prompt from appearing while the command is still running.

- Finally at last, after every foreground process has finished, the shell takes back ownership of the terminal.

```c
tcsetpgrp(STDIN_FILENO, parent_gid);
```

- The shell is now ready to accept another command from the user.And that's the complete execution cycle.

1. The parser prepared the command.
2. The executor created new processes.
3. The operating system executes the requested programs.

### 3.5 : Conclusion

Congratulations! We have now built a complete shell from scratch. Of course, our shell is still far from something like Bash or Zsh. Real shells support command substitution, shell expansions, scripting, signal handling, aliases, wildcards, environment variables, subshells, and hundreds of other features that we haven't implemented.
<br>

But that's perfectly fine. The goal of this project was never to recreate Bash. It was to understand __how a shell actually works behind the scenes.__
<br>

If you understood these three parts, you'll realize that a shell is not some magical component of the operating system. It is simply another C program making clever use of a few operating system system calls.

<br>

I encourage you to experiment with the code. 
- Try adding your own built-in commands. 
- Support append redirection (`>>`). 
- Improve error handling.
- Implement wildcard expansion.
- Or maybe redesign the parser completely.
<br>
I am waiting for your additions and feedbacks.

<br>

And as usual, if you found any mistakes or my 2 AM dumb writing confuses you, feel free to open an issue or submit a pull request. I hope this project helped make shell development a little less mysterious.
<br>

Happy hacking, and see you in the next project. 
<br>

Previous Parts [Part0](Part0.md) || [Part1](Part1.md) || [Part2](Part2.md)