PREVIOUS PART : [PART0](Part0.md)

# PART 1 : Lexical Analysis
<br>

Before we begin, it is suggested to read about lexical analysis [here](https://en.wikipedia.org/wiki/Lexical_analysis).
<br>

### 1.0 : Prerequisites [ main.c / int main() ]
- if you look up `tokenize()` function in tokenizer.c, it takes 2 arguments, a user input and an empty array to store the tokens.

```c
void tokenize(char *user_input, char * tok_cmds[])
```
<br>

- Before we move ahead, we need to format our user inputted commands. There is a rule that a command must adhere to get entry into tokenizing. __Each command must end with a null terminator `\0`. This helps us know if a command has end or not.__

- [Delimiter](https://en.wikipedia.org/wiki/Delimiter) is one or more characters that separate text strings. These are used to separate tokens. Common delimiters are commas ( , ), semicolon ( ; ), quotes ( ", ' ), pipes ( | ), whitespace ( ) or tab space ( \t ).

<img src="docs_med/tokens.png" alt="Window6" width="600"/>
<br>

did you see in above image, we know that quoted content must be together, not to be separated, that's why we need to handle it manually. That's why you can find the following function in `parser.c` 
<br>

```c
void parser_for_quotes(char * cmds[ ], char * parsed_cmds[ ] )
```

But that's in Part 3 so we will not worry for it, for now.

### 1.1 : Traversing character by character [ parser/tokenizer.c / void tokenize() ]
- Now comes the actual implementation. Instead of asking C to split our string, we manually trace every character of the user input ourselves. This gives us complete control over what becomes a token and what doesn't.
- We create a loop that starts from index 0 and keeps moving forward until we reach the null terminator '\0'.
<br>

> while(user_input[i] != '\0')

- At every iteration, we ask ourselves a simple question "What kind of character am I currently standing on?"

- Depending on the answer, we take different actions. Sometimes we keep building the current word, sometimes we save it as a token, and sometimes we create a brand new token for operators like | or >.
- think of it like a person reading a sentence one letter at a time. Until they see a separator, they keep reading the same word. The moment they encounter a separator, they know the previous word has finished.

### 1.2 : Buffer [ parser/tokenizer.c / void tokenize() ]
- Before we can store a token, we first need somewhere to collect its characters. This is why we create a temporary buffer.
- While tracing the input, we need some place to temporarily store the characters of the current word. `char buffer[100]; `. 
<br>

<img src="docs_med/tokens2.png" alt="Window7" width="600"/>

- Whenever the user enters a command, our tokenizer does not immediately create tokens. Instead, it starts reading the command one character at a time from left to right.
- While reading, every normal character is copied into a temporary array called __buffer__. You can think of this buffer as a small waiting room. Characters stay here until we are sure that the current word has finished.

- Let's use the command

> ls > file.txt
> 
- The tokenizer first sees the character 'l'. Since it is not a delimiter, it gets copied into the buffer.

> Buffer : l

- Next it reads 's'. Again, this is just another character, so it also goes into the same buffer.

> Buffer: [ ls ]

- Now the tokenizer encounters a whitespace.

> ls > file.txt
>   ^

- This whitespace tells us something important: "The current word has ended."

- Since we now know "ls" is complete, we flush the buffer. Flushing simply means taking whatever is inside the temporary buffer and moving it into our __final token array__.

> Token Array : [ "ls" , ]

- After moving the word, we don't delete the buffer. Instead, we simply reset its index back to 0, allowing us to reuse the same memory for the next word. The tokenizer then continues reading from where it left off.

- Next it encounters the character '>'.
- Unlike normal letters, '>' is itself a token. It doesn't belong to any word, so instead of copying it into the buffer, we immediately store it inside our token array.

> Token Array : [ "ls", ">", ]

- Finally, the tokenizer starts reading "file.txt" one character at a time. Every character is copied into the buffer until the end of the command is reached.

> Buffer: [ file.txt ]

- Since we reached the null terminator ('\0'), there are no more characters left to read. We perform one final buffer flush and move "file.txt" into our token array.

> Token Array : [ "ls", ">", "file.txt" ]

- By the end of tokenization, our command has been converted into independent tokens that can be easily understood by the parser.

> __["ls"] [">"] ["file.txt"] [NULL]__

### 1.3 : Handling Delimiters [ parser/tokenizer.c / void tokenize() ]

- Until now, we have only discussed normal characters like l, s, f, i, l, e, etc. But a shell command also contains special characters that have a meaning of their own.

- For example, `ls | grep txt` or `echo hello > out.txt`

- Here, the characters ` | `, ` < `, ` > `, ` >> ` and ` & ` are not part of a word. They are shell operators, which means they should become separate tokens.

- This is exactly why our tokenizer checks for delimiters before copying characters into the buffer.
```c
else if(user_input[i] == '|' ||
        user_input[i] == '>' ||
        user_input[i] == '<')
```

- The moment one of these characters is encountered, we stop building the current word. For example,

> echo>file.txt 
> <br>
>     ^

- Notice that there isn't even a space between echo and >. But a beginner malike us may think that the tokenizer will produce
` [ "echo>file.txt" ] ` 

- But that's incorrect, our tokenizer understands that ` > `  itself is an operator. So before handling ` > `, it first flushes the current buffer.

### 1.4 : Single Character Operators [ parser/tokenizer.c / void tokenize() ]

- After the buffer has been flushed, we are finally ready to store the operator itself. Suppose the current character is ` | `.

- Unlike normal words, operators contain only one character. But, [strdup()](https://man7.org/linux/man-pages/man3/strdup.3.html) takes a string, not a single character. That's why we first convert the character into a string.

```c
char temp[2];

temp[0] = '|';
temp[1] = '\0';
```

- Now ` temp ` contains ` |\0 `, which can safely be copied.

```c
tok_cmds[token_idx] = strdup(temp);
```

- The resulting token array becomes [ "ls", "\0" ] [ "|", "\0" ] instead of [ "ls|" ]. The same logic is used for <, >, &. since all of them are single-character operators.

### 1.5 : Looking Ahead (Check the next character)  [ parser/tokenizer.c / void tokenize() ]

- Some shell operators are made of two characters instead of one. For example, >>. If our tokenizer immediately stored the first ` > ` : ` [ "echo" ] [ ">" ] [ ">" ] `. that would be incorrect.
The shell expects ` [ "echo" ] [ ">>" ] ` instead.

- So before storing >, we quickly peek at the next character.

```c
user_input[i + 1]
``` 
<br>

- This is called look-ahead. We are simply asking, "Before I decide what this character is, let me quickly see the next one" . If the next character is another >

- both characters are combined into one token. which becomes ">>"
<br>

```c
temp[0] = '>';
temp[1] = '>';
temp[2] = '\0';
```

- After storing it, we move the input index by two positions. ` i += 2; `, because both characters have already been processed.

### 1.6 : Unsupported Operators

- While implementing a shell, you don't have to support every operator from Day 1. Some operators like <<, <&, >& are slightly more advanced and require extra logic later. ( Yes, i am lazy to go that beyond ).

- Instead of pretending to support them, our tokenizer immediately informs the user that they are not implemented yet.
<br>

```c
printf("\nsorry user but this command is still in development\n");
```

- This is actually a good programming practice. It is better to accept limitation and tell the user that a feature is unavailable than to silently produce incorrect results.

### 1.8 : Final Buffer Flush  [ parser/tokenizer.c / void tokenize() ]

- After the loop finishes, there is still one small problem. Imagine the command 
> pwd
<br>

There are no spaces, no pipe, no redirections. The loop reaches the null terminator directly.

> pwd\0
> <br>
>    ^
<br>

- At this point, Buffer : [ pwd ] still contains the last word. Since no delimiter appeared after "pwd", buffer never gets flushed in the loop.
- That's why we perform one last check after the loop finishes.

```c
if(word_idx > 0)
```

- If the buffer still contains a word, we save it into the token array exactly the same way as before. Without this final flush, we would get [ ] instead of [ "pwd" ]

- This final check ensures that the last token is never lost, regardless of how the command ends.

<br>

That's it see you at the Parsing. Sorry for the grammatical mistakes, Feel free to raise issues regarding the project. 
<br>

Next Parts : 
<br>

[PART2](Part2.md)