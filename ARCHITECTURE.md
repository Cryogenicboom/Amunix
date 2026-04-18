This is documentation for AMUNIX covers architecture and working. For installation refer to the (README.md)[README.md] here.

```
        .o.       ooo        ooooo ooooo     ooo  oooo        oooo  ooooo ooooooo  ooooo
       .888.      `88.       .888' `888'     `8'   `88         88   `888`  `8888    d8'
      .8\"888.     888b     d'888   888       8     888b       88    888     Y888..8P
     .8' `888.     8 Y88. .P  888   888       8     8 Y88.     88    888      `8888'
    .88ooo8888.    8  `888'   888   888       8     8  `888'   88    888     .8PY888.
   .8'     `888.   8    Y     888   `88.    .8'     8    Y888 .88    888     d8'  `888b
  o88o     o8888o o8o        o888o    `YbodP'      o8o      88888o  o888o oo888o  o88888o 
```
<br>

#### Yes this ASCII title was way harder than making the Shell.
<br> 

## What is Shell?
<p>
Shell is a CLI (Command Line Interface) that acts as an interface for an Operating System. Shell enables user to interact with kernal. it provides user with commands, execute programmes for them and manages Input and Output functionality. 
</p>

## Why AMUNIX? 
<p>
I know there exist professional shells already. I built AMUNIX as a curiosity driven project. I was studying (OS:TEP)["https://pages.cs.wisc.edu/~remzi/OSTEP/"], this book ignited the spark in me to understand computers at fundamentals. I thought "why not learn by doing?". I hope that this shell will also help other curious minds to understand the shell development. 
</p>

# ARCHITECTURE 
Shell is divided into 3 parts: 
- [Tokenizer](#tokenizer) : splits the user input commands into array of strings. 
- [PARSING](#PARSING) : gives meaning to Tokenized command. 
- [Executor](#EXECUTOR) : executes the parsed command and returns output.

## Tokenizer: 

