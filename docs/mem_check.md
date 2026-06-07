FILE : Tokenizer.c
Allocation : tok_cmds[token_idx] = strdup(temp);
owner : tok_cmds[]
Freed : freed 
status : 0 bytes returns 

---

FILE : main.c
Allocation : tail->h_cmds = strdup(user_input);
owner : tail.h_cmds
Freed : freed 
status : 0 bytes returns 

---

FILE : parser.c
Allocation : cmd->outputfile = strdup(tokens[i+1]);
owner : cmd.outputfile
Freed : freed 
status : 0 bytes returns 

---

FILE : parser.c
Allocation : parsed_cmds[j] = strdup(temp);
owner : parsed_cmds[]
Freed : freed 
status : 0 bytes returns 

---

