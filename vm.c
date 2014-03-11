/* Charles Drews (csd305@nyu.edu, N11539474)
 * Homework 3 (Lab 2)
 * Virtual Machines: Concepts and Applications , CSCI-GA.3033-015, Spring 2014
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define HASHSIZE 101
#define MAXLINE 100

// hash table & functions for labels in input code
typedef struct _label_list_t_ {
    // linked list of label:line_num pairs
    char *label;
    int line_num;
    struct _label_list_t_ *next;
} label_list_t;

typedef struct _label_hashtab_t_ {
    int size;
    label_list_t **table; // array of ptrs to label_list_t
                          // like "label_list_t *table[size]"
} label_hashtab_t;

label_hashtab_t *create_label_hashtab();
unsigned int hash_label(char *, label_hashtab_t *);
label_list_t *lookup_label(char *, label_hashtab_t *);
label_list_t *update_label(char *, int, label_hashtab_t *);
void free_label_hashtab(label_hashtab_t *);



// hash table & functions for memory addresses in input code
typedef struct _address_list_t_ {
    // linked list of address:value pairs
    int address;
    int value;
    struct _address_list_t_ *next;
} address_list_t;

typedef struct _address_hashtab_t_ {
    int size;
    address_list_t **table; // array of ptrs to adress_list_t
                            // like "address_list_t *table[size]"
} address_hashtab_t;

address_hashtab_t *create_address_hashtab();
unsigned int hash_address(int, address_hashtab_t *);
address_list_t *lookup_address(int, address_hashtab_t *);
address_list_t *update_address(int, int, address_hashtab_t *);
void free_address_hashtab(address_hashtab_t *);



// stack struct and related functions
typedef struct _stack_list_t_ {
    int data;
    struct _stack_list_t_ *next;
} stack_list_t;

typedef stack_list_t *stack_t; // declare stack_t as ptr to stack_list_t

void push_onto_stack(stack_t *, int);
int pop_from_stack(stack_t *);
void free_stack(stack_t *);



// struct for parsing a line of input code
struct code_line {
    char inst[5];
    char arg0[MAXLINE]; // could be a label; no rules on label length
    char arg1[50]; // could be a register name, int, or memory address
};



// struct for the source registers
typedef struct _registers_t_ {
	int eax;
	int ebx;
	int ecx;
	int edx;
	int esi;
	int edi;
	int esp;
	int ebp;
	int eip;
	int r08;
	int r09;
	int r10;
	int r11;
	int r12;
	int r13;
	int r14;
	int r15;
	int flags;
	int remreg;
} registers_t;



// helper functions
void usage(char *);
int is_number(char *);
int is_mem_addr(char *);
int resolve_mem_addr(char *);
int get_int(char *);
int *get_register_pointer(char *, registers_t *);
int get_arg_value(char *, address_hashtab_t *, registers_t *);
int *get_arg_location(char *, address_hashtab_t *, registers_t *);



// functions for each simplified assembly instruction
void mov(char *, char *, address_hashtab_t *, registers_t *);
void push(char *, address_hashtab_t *, stack_t *, registers_t *);
void pop(char *, address_hashtab_t *, stack_t *, registers_t *);
void pushf(stack_t *, registers_t *);
void popf(char *, address_hashtab_t *, stack_t *, registers_t *);
int call(char *, int, label_hashtab_t *, stack_t *);
int ret(stack_t *);
void arith_inc(char *, address_hashtab_t *, registers_t *);
void arith_dec(char *, address_hashtab_t *, registers_t *);
void arith_add(char *, char *, address_hashtab_t *, registers_t *);
void arith_sub(char *, char *, address_hashtab_t *, registers_t *);
void arith_mul(char *, char *, address_hashtab_t *, registers_t *);
void arith_div(char *, char *, address_hashtab_t *, registers_t *);
void arith_mod(char *, char *, address_hashtab_t *, registers_t *);
void arith_rem(char *, address_hashtab_t *, registers_t *);
void bin_not(char *, address_hashtab_t *, registers_t *);
void bin_xor(char *, char *, address_hashtab_t *, registers_t *);
void bin_or(char *, char *, address_hashtab_t *, registers_t *);
void bin_and(char *, char *, address_hashtab_t *, registers_t *);
void bin_shl(char *, char *, address_hashtab_t *, registers_t *);
void bin_shr(char *, char *, address_hashtab_t *, registers_t *);
void cmp(char *, char *, address_hashtab_t *, registers_t *);
int jmp(char *, int, address_hashtab_t *, label_hashtab_t *, registers_t *);
void prn(char *, address_hashtab_t *, registers_t *);



int main(int argc, char *argv[]) {
    if (argc != 2) {
        usage(argv[0]);
        exit(1);
    }

    // open specified input file; check if successful
    FILE *fp;
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("Error");
        exit(1);
    }
   
    // create hash table for labels used in input code 
    label_hashtab_t *label_hashtab = create_label_hashtab();
    
    // create hash table for memory addresses used in input code
    address_hashtab_t *address_hashtab = create_address_hashtab();
    
    // use the address hash table type to create a hash table
    // to capture how many times each dynamic basic block is
    // executed. Use line_num of first inst. of DBB as address
    // and keep track of execution count in value.
    address_hashtab_t *DBB_hashtab = create_address_hashtab();
     
    // create a pointer to a stack (ptr to head once items are pushed)
    stack_t *stack;
    if ((stack = malloc(sizeof(stack_t))) == NULL) {
        perror("malloc failure in main()");
        exit(1);
    }

    // create a pointer to a registers struct
    registers_t *registers;
    if ((registers = malloc(sizeof(registers_t))) == NULL) {
        perror("malloc failure in main()");
        exit(1);
    }

    // initialize the source registers to zero
	registers->eax = 0;
	registers->ebx = 0;
	registers->ecx = 0;
	registers->edx = 0;
	registers->esi = 0;
	registers->edi = 0;
	registers->esp = 0;
	registers->ebp = 0;
	registers->eip = 0;
	registers->r08 = 0;
	registers->r09 = 0;
	registers->r10 = 0;
	registers->r11 = 0;
	registers->r12 = 0;
	registers->r13 = 0;
	registers->r14 = 0;
	registers->r15 = 0;
	registers->flags = 0;
	registers->remreg = 0;

    // find labels in input file
    int linenum = 0; 
    char line[MAXLINE];
    char *token;
    while (fgets(line, MAXLINE, fp) != NULL) {
        linenum++;
        token = strtok(line, " ,\t\n");
        while (token != NULL) {
            if (token[strlen(token) - 1] == ':') {
                // if token is a label, add to label hash table
                update_label(token, linenum, label_hashtab);
                break;
            }
            token = strtok(NULL, " ,\t\n"); // move to next token
        }
    }

    // if start label present, start there; otherwise line 1
    int startline = 1;
    label_list_t *lp;
    if ((lp = lookup_label("start:", label_hashtab)) != NULL)
        startline = lp->line_num; // if found in table, update startline

    rewind(fp); // reset file pointer to beginning of input file

    // read input by iterating through lines of input file
    linenum = 0;
    struct code_line curr_inst;
    int jump_flag = 1; // capture first block
    int dbb_exec_count = 0;
    address_list_t *dbb = NULL;
    while (fgets(line, MAXLINE, fp) != NULL) {
        if (++linenum >= startline) {

            // if previous inst. was a jump or call, need to 
            // log the current inst. as the head of a DBB
            if (jump_flag) {
                //printf("jumped to line %d\n", linenum);
                
                // use functions intended for address hashtab
                if ((dbb = lookup_address(linenum, DBB_hashtab)) == NULL) {
                    // if that linenum hasn't already been the
                    // start of a DBB, add it w/ a zero value
                    dbb = update_address(linenum, 0, DBB_hashtab);
                }
                dbb->value = dbb->value + 1;
                dbb_exec_count++;
            }
            jump_flag = 0; // reset

            // reset curr_inst
            memset(curr_inst.inst, '\0', 5);
            memset(curr_inst.arg0, '\0', MAXLINE);
            memset(curr_inst.arg1, '\0', 50);
            
            // tokenize the new line
            token = strtok(line, " ,\t\n");

            // iterate through the tokens of the current line
            while (token != NULL) {	
                // refill curr_inst with new line
                if (token[0] == '#') {
                    break; // if comment, skip rest of line
                } else if (token[strlen(token) - 1] == ':') {
                    // if label do nothing, move to next token
                } else if (curr_inst.inst[0] == '\0') {
                    // populate instruction
                    snprintf(curr_inst.inst, 5, "%s", token);
                } else if (curr_inst.arg0[0] == '\0') {
                    // populate arg0
                    snprintf(curr_inst.arg0, MAXLINE, "%s", token);
                } else {
                    // populate arg1
                    snprintf(curr_inst.arg1, 50, "%s", token);
                }
                token = strtok(NULL, " ,\t\n");	// move to next token
            } // end while loop through tokens of current line
   
    
            // call subroutines for each instruction
            if (strcmp(curr_inst.inst, "mov") == 0)
                mov(curr_inst.arg0, curr_inst.arg1, address_hashtab,
                    registers);
            else if (strcmp(curr_inst.inst, "push") == 0)
                push(curr_inst.arg0, address_hashtab, stack, registers);
            else if (strcmp(curr_inst.inst, "pop") == 0)
                pop(curr_inst.arg0, address_hashtab, stack, registers);
            else if (strcmp(curr_inst.inst, "pushf") == 0)
                pushf(stack, registers);
            else if (strcmp(curr_inst.inst, "popf") == 0)
                popf(curr_inst.arg0, address_hashtab, stack, registers);
            else if (strcmp(curr_inst.inst, "call") == 0) {
                // send linenum + 1 which would be the next
                // instruction, which is what SPC would point to
                startline = call(curr_inst.arg0, linenum + 1,
                                 label_hashtab, stack);
                // reset file ptr so next loop won't
                // do anything until it hits the new startline
                rewind(fp);
                linenum = 0;
                jump_flag = 1;
            }
            else if (strcmp(curr_inst.inst, "ret") == 0) {
                startline = ret(stack);
                // reset file ptr so next loop won't
                // do anything until it hits the new startline
                rewind(fp); 
                linenum = 0;
                jump_flag = 1;
            }
            else if (strcmp(curr_inst.inst, "inc") == 0)
                arith_inc(curr_inst.arg0, address_hashtab, registers);
            else if (strcmp(curr_inst.inst, "dec") == 0)
                arith_dec(curr_inst.arg0, address_hashtab, registers);
            else if (strcmp(curr_inst.inst, "add") == 0)
                arith_add(curr_inst.arg0, curr_inst.arg1, address_hashtab,
                          registers);
            else if (strcmp(curr_inst.inst, "sub") == 0)
                arith_sub(curr_inst.arg0, curr_inst.arg1, address_hashtab,
                          registers);
            else if (strcmp(curr_inst.inst, "mul") == 0)
                arith_mul(curr_inst.arg0, curr_inst.arg1, address_hashtab,
                          registers);
            else if (strcmp(curr_inst.inst, "div") == 0)
                arith_div(curr_inst.arg0, curr_inst.arg1, address_hashtab,
                          registers);
            else if (strcmp(curr_inst.inst, "mod") == 0)
                arith_mod(curr_inst.arg0, curr_inst.arg1, address_hashtab,
                          registers);
            else if (strcmp(curr_inst.inst, "rem") == 0)
                arith_rem(curr_inst.arg0, address_hashtab, registers);
            else if (strcmp(curr_inst.inst, "not") == 0)
                bin_not(curr_inst.arg0, address_hashtab, registers);
            else if (strcmp(curr_inst.inst, "xor") == 0)
                bin_xor(curr_inst.arg0, curr_inst.arg1, address_hashtab,
                        registers);
            else if (strcmp(curr_inst.inst, "or") == 0)
                bin_or(curr_inst.arg0, curr_inst.arg1, address_hashtab,
                       registers);
            else if (strcmp(curr_inst.inst, "and") == 0)
                bin_and(curr_inst.arg0, curr_inst.arg1, address_hashtab,
                        registers);
            else if (strcmp(curr_inst.inst, "shl") == 0)
                bin_shl(curr_inst.arg0, curr_inst.arg1, address_hashtab,
                        registers);
            else if (strcmp(curr_inst.inst, "shr") == 0)
                bin_shr(curr_inst.arg0, curr_inst.arg1, address_hashtab,
                        registers);
            else if (strcmp(curr_inst.inst, "cmp") == 0)
                cmp(curr_inst.arg0, curr_inst.arg1, address_hashtab,
                    registers);
            else if (strcmp(curr_inst.inst, "jmp") == 0) {
                startline = jmp(curr_inst.arg0, linenum + 1, address_hashtab,
                                label_hashtab, registers);
                // reset file ptr so next loop won't
                // do anything until it hits the new startline
                rewind(fp);
                linenum = 0;
                jump_flag = 1;
            }
            else if (strcmp(curr_inst.inst, "je") == 0) {
                if (registers->flags == 0) {
                    // jump if cmp yielded 0
                    startline = jmp(curr_inst.arg0, linenum + 1,
                                    address_hashtab, label_hashtab,
                                    registers);
                    rewind(fp);
                    linenum = 0;
                }
                jump_flag = 1; // set jump flag even if jump didn't happen
            }
            else if (strcmp(curr_inst.inst, "jne") == 0) {
                if (registers->flags != 0) {
                    // jump if cmp did not yield 0
                    startline = jmp(curr_inst.arg0, linenum + 1,
                                    address_hashtab, label_hashtab,
                                    registers);
                    rewind(fp);
                    linenum = 0;
                }
                jump_flag = 1; // set jump flag even if jump didn't happen
            }
            else if (strcmp(curr_inst.inst, "jg") == 0) {
                if (registers->flags > 0) {
                    // jump if cmp yielded >0
                    startline = jmp(curr_inst.arg0, linenum + 1,
                                    address_hashtab, label_hashtab,
                                    registers);
                    rewind(fp);
                    linenum = 0;
                }
                jump_flag = 1; // set jump flag even if jump didn't happen
            }
            else if (strcmp(curr_inst.inst, "jge") == 0) {
                if (registers->flags >= 0) {
                    // jump if cmp yielded >=0
                    startline = jmp(curr_inst.arg0, linenum + 1,
                                    address_hashtab, label_hashtab,
                                    registers);
                    rewind(fp);
                    linenum = 0;
                }
                jump_flag = 1; // set jump flag even if jump didn't happen
            }
            else if (strcmp(curr_inst.inst, "jl") == 0) {
                if (registers->flags < 0) {
                    // jump if cmp yielded <0
                    startline = jmp(curr_inst.arg0, linenum + 1,
                                    address_hashtab, label_hashtab,
                                    registers);
                    rewind(fp);
                    linenum = 0;
                }
                jump_flag = 1; // set jump flag even if jump didn't happen
            }
            else if (strcmp(curr_inst.inst, "jle") == 0) {
                if (registers->flags <= 0) {
                    // jump if cmp yielded <=0
                    startline = jmp(curr_inst.arg0, linenum + 1,
                                    address_hashtab, label_hashtab,
                                    registers);
                    rewind(fp);
                    linenum = 0;
                }
                jump_flag = 1; // set jump flag even if jump didn't happen
            }
            else if (strcmp(curr_inst.inst, "prn") == 0)
                prn(curr_inst.arg0, address_hashtab, registers);

        } // end if linenum >= startline
    } // end while loop through lines w/ fgets


    // display profiling data from DBB_hashtab
    int i;
    address_list_t *list;
    int dbb_exec_count2 = 0;
    // iterate through the table array
    for (i = 0; i < DBB_hashtab->size; i++) {
        list = DBB_hashtab->table[i];
        // iterate through linked list in table[i] starting w/ head
        while (list != NULL) {
            printf("line_num %d's frequency: %d\n", list->address,
                   list->value);
            dbb_exec_count2 = dbb_exec_count2 + list->value;
            list = list->next;
        }
    }
    printf("dbb_exec_count:  %d\n", dbb_exec_count);
    printf("dbb_exec_count2: %d\n", dbb_exec_count2);


    // wrap it up
    fclose(fp);
    free_label_hashtab(label_hashtab);
    free_address_hashtab(address_hashtab);
    free_address_hashtab(DBB_hashtab);
    free_stack(stack);
    free(stack);
    free(registers);
    return 0;
} // end of main()




// label hash table functions
label_hashtab_t *create_label_hashtab() {
    label_hashtab_t *ht;

    // attempt to allocate memory for label_hashtab_t struct
    if ((ht = malloc(sizeof(label_hashtab_t))) == NULL) {
        perror("malloc failure in create_lable_hashtab()");
        return NULL;
    }
    
    // attempt to allocate memory for new_table->table struct
    // (an array of length HASHSIZE of pointers to label_list_t)
    if ((ht->table = malloc(HASHSIZE * sizeof(label_list_t *))) == NULL) {
        perror("malloc failure in create_hashtab()");
        return NULL;
    }
    
    // initialize the elements of the table
    int i;
    for (i = 0; i < HASHSIZE; i++)
        ht->table[i] = NULL;
    
    // set size and return
    ht->size = HASHSIZE;
    return ht;
}

unsigned int hash_label(char *str, label_hashtab_t *ht) {
    unsigned int hashval;
    
    for (hashval = 0; *str != '\0'; str++) {
        //hashval = *str + hashval * 31;
        hashval = *str + (hashval << 5) - hashval;
        // left shift (<<) by 5 is equiv to mult by 2^5 (=32)
        // then subtract 1 to get equiv of mult by 31
    }
    return hashval % ht->size;
}

label_list_t *lookup_label(char *str, label_hashtab_t *ht) {
    label_list_t *list;

    // iterate through the linked list in the correct array cell
    unsigned int hashval = hash_label(str, ht);
    for (list = ht->table[hashval]; list != NULL; list = list-> next)
        if (strcmp(str, list->label) == 0)
            return list; // str found as a ->label in ht
    return NULL;         // str not found
}

label_list_t *update_label(char *str, int ln, label_hashtab_t *ht) {
    label_list_t *list;
    unsigned int hashval;

    if((list = lookup_label(str, ht)) == NULL) {
        // if str not found in hashtab, add it

        // attempt to allocate memory for label_list_t struct
        if ((list = malloc(sizeof(label_list_t))) == NULL) {
            perror("malloc failure in update_label()");
            return NULL;
        }

        // attempt to allocate memory for list->label via strdup
        // (strdup uses malloc)
        if ((list->label = strdup(str)) == NULL) {
            perror("malloc failure in update_label()");
            return NULL;
        }

        // insert list at head of linked list in correct array cell
        hashval = hash_label(str, ht);
        list->next = ht->table[hashval];
        ht->table[hashval] = list;
    }

    // update entry with ln (doesn't matter if existing or new entry)
    list->line_num = ln;
    return list;
}

void free_label_hashtab(label_hashtab_t *ht) {
    int i;
    label_list_t *list;
    label_list_t *temp;

    if (ht == NULL)
        return;

    // iterate through ht's table array
    for (i = 0; i < ht->size; i++) {
        list = ht->table[i];
        // iterate through linked list in table[i] starting w/ head
        while (list != NULL) {
            temp = list;
            list = list->next;
            free(temp->label); // ->label was set w/ strdup which uses malloc
            free(temp); // free that node
        }
    }

    // free the table
    free(ht->table);
    free(ht);
}




// address hash table functions
address_hashtab_t *create_address_hashtab() {
    address_hashtab_t *ht;

    // attempt to allocate memory for address_hashtab_t struct
    if ((ht = malloc(sizeof(address_hashtab_t))) == NULL) {
        perror("malloc failure in create_lable_hashtab()");
        return NULL;
    }
    
    // attempt to allocate memory for new_table->table struct
    // (an array of length HASHSIZE of pointers to address_list_t)
    if ((ht->table = malloc(HASHSIZE * sizeof(address_list_t *))) == NULL) {
        perror("malloc failure in create_hashtab()");
        return NULL;
    }
    
    // initialize the elements of the table
    int i;
    for (i = 0; i < HASHSIZE; i++)
        ht->table[i] = NULL;
    
    // set size and return
    ht->size = HASHSIZE;
    return ht;
}

unsigned int hash_address(int addr, address_hashtab_t *ht) {
    // simplest hashing function: take the int input
    // and mod by HASHSIZE
    // (resolve address from hex/binary to decimal first)
    return addr % ht->size;
}

address_list_t *lookup_address(int addr, address_hashtab_t *ht) {
    address_list_t *list;

    // iterate through the linked list in the correct array cell
    unsigned int hashval = hash_address(addr, ht);
    for (list = ht->table[hashval]; list != NULL; list = list->next)
        if (addr == list->address)
            return list; // addr found as an ->address in ht
    return NULL;         // addr not found
}

address_list_t *update_address(int addr, int val, address_hashtab_t *ht) {
    address_list_t *list;
    unsigned int hashval;

    if((list = lookup_address(addr, ht)) == NULL) {
        // if addr not found in hashtab, add it
        
        // attempt to allocate memory for address_list_t struct
        if ((list = malloc(sizeof(address_list_t))) == NULL) {
            perror("malloc failure in update_address()");
            return NULL;
        }
        
        // insert list at head of linked list in correct array cell
        hashval = hash_address(addr, ht);
        list->next = ht->table[hashval];
        ht->table[hashval] = list;
    }

    // update entry with addr & val (doesn't matter if existing or new entry)
    list->address = addr;
    list->value = val;
    return list;
}

void free_address_hashtab(address_hashtab_t *ht) {
    int i;
    address_list_t *list;
    address_list_t *temp;

    if (ht == NULL)
        return;

    // iterate through ht's table array
    for (i = 0; i < ht->size; i++) {
        list = ht->table[i];
        // iterate through linked list in table[i] starting w/ head
        while (list != NULL) {
            temp = list;
            list = list->next;
            free(temp); // free that node
        }
    }

    // free the table
    free(ht->table);
    free(ht);
}



// stack functions
void push_onto_stack(stack_t *head, int d) {
    // allocate memory; check if successful
    stack_list_t *sl = malloc(sizeof(stack_list_t));
    if (sl == NULL) {
        perror("Error");
        exit(1);
    }
    sl->data = d;
    if (*head == NULL)
        sl->next = NULL;
    else
        sl->next = *head;
    *head = sl;
}

int pop_from_stack(stack_t *head) {
    if (*head == NULL) {
        printf("Error: cannot pop from empty stack\n");
        exit(1);
    } else {
        int d = (*head)->data;
        stack_list_t *tmp = *head;
        *head = (*head)->next;
        free(tmp);
        return d;
    }
}

void free_stack(stack_t *head) {
    int i;
    stack_list_t *sl = *head;
    while ( sl != NULL) {
        i = pop_from_stack(head); // remove existing head
        sl = *head;      // set n to new head
    }                   // repeat until no new head
}




// helper functions
void usage(char *exe) {
    printf("Usage: %s inputfile.vm\n", exe);
}

int is_number(char *str) {
    // returns 1 if yes, 0 if no
    
    if (str[0] == '0' && str[1] == 'x')
        return 1; // it's a hexidecimal
   
    if (str[strlen(str) - 2] == '|' && str[strlen(str) - 1] == 'h')
        return 1; // it's a hexidecimal

    if (str[strlen(str) - 2] == '|' && str[strlen(str) - 1] == 'b')
        return 1; // it's a binary number
    
    // if not hexidecimal or binary, maybe decimal
    // check if each character is a digit
    while (*str) {
        if (!isdigit(*str))
            return 0;
        else
            ++str;
    }
    return 1; // it's a decimal
}

int is_mem_addr(char *str) {
    // returns 1 if yes, 0 if no
    // test first char for [ and last for ]
    if (str[0] == '[' && str[strlen(str) - 1] == ']') {
        return 1;
    }
    else
        return 0;
}

int resolve_mem_addr(char *str) {
    // convert memory address to number; same address may appear
    // in multiple forms (hex, binary, decimal); need to hash
    // to same place regardless of form
    
    // strtok alters the parse string, so to preserve str,
    // first make a copy and pass that to strtok
    // and check if the malloc called by strdup is successful
    char *str_copy;
    if ((str_copy = strdup(str)) == NULL) {
        perror("malloc failure in update_label()");
        return -1;
    }

    char *trimmed_str;
    trimmed_str = strtok(str_copy, "[]"); // remove the [ and ]
   
    int addr = get_int(trimmed_str); 
    free(str_copy);
    return addr;
}

int get_int(char *str) {
    if (str[0] == '0' && str[1] == 'x') {
        // hexadecimal
        return (int)strtol(str, NULL, 16);
    } else if (str[strlen(str) - 2] == '|' && str[strlen(str) - 1] == 'h') {
        // hexadeximal
        return (int)strtol(str, NULL, 16);
    } else if (str[strlen(str) - 2] == '|' && str[strlen(str) - 1] == 'b') {
        // binary
        return (int)strtol(str, NULL, 2);
    } else {
        // decimal
        return atoi(str);
    }
}

int *get_register_pointer(char *reg, registers_t *r) {
    if (strcmp(reg, "eax") == 0) return &r->eax;
    else if (strcmp(reg, "ebx") == 0) return &r->ebx;
    else if (strcmp(reg, "ecx") == 0) return &r->ecx;
    else if (strcmp(reg, "edx") == 0) return &r->edx;
    else if (strcmp(reg, "esi") == 0) return &r->esi;
    else if (strcmp(reg, "edi") == 0) return &r->edi;
    else if (strcmp(reg, "esp") == 0) return &r->esp;
    else if (strcmp(reg, "ebp") == 0) return &r->ebp;
    else if (strcmp(reg, "eip") == 0) return &r->eip;
    else if (strcmp(reg, "r08") == 0) return &r->r08;
    else if (strcmp(reg, "r09") == 0) return &r->r09;
    else if (strcmp(reg, "r10") == 0) return &r->r10;
    else if (strcmp(reg, "r11") == 0) return &r->r11;
    else if (strcmp(reg, "r12") == 0) return &r->r12;
    else if (strcmp(reg, "r13") == 0) return &r->r13;
    else if (strcmp(reg, "r14") == 0) return &r->r14;
    else if (strcmp(reg, "r15") == 0) return &r->r15;
    else if (strcmp(reg, "flags") == 0) return &r->flags;
    else if (strcmp(reg, "remainder") == 0) return &r->remreg;
    else return 0;
}

int get_arg_value(char *arg, address_hashtab_t *ht,
                  registers_t *r) {
    // return the value specified by arg

    if (is_mem_addr(arg)) {
        // if arg is a memory address
        
        int addr = resolve_mem_addr(arg);
        address_list_t *ap = NULL;
        
        if ((ap = lookup_address(addr, ht)) == NULL) {
            // if that address hasn't already been used, put a zero value in it
            ap = update_address(addr, 0, ht);
        }
        return ap->value;
    }
    else if (is_number(arg)) {
        // if arg is a number
        return get_int(arg);
    }
    else {
        // if arg is a register name
        int *reg = get_register_pointer(arg, r);
        return *reg;
    }
}

int *get_arg_location(char *arg, address_hashtab_t *ht,
                      registers_t *r) {
    // return pointer to location specified by arg
    int *loc;
    
    if (is_mem_addr(arg)) {
        // if arg is a memory address
        
        int addr = resolve_mem_addr(arg);
        address_list_t *ap = NULL;
        
        if ((ap = lookup_address(addr, ht)) == NULL) {
            // if that address hasn't already been used, put a zero value in it
            ap = update_address(addr, 0, ht);
        }
        loc = &(ap->value); // pointer to ap->value
    }
    else {
        // if arg is a register name
        loc = get_register_pointer(arg, r);
    }
    return loc;
}



// functions for each simplified assembly instruction
void mov(char *arg0, char *arg1, address_hashtab_t *ht,
         registers_t *r) {
    // move value specified by arg1 to 
    // location specified by arg0
    int val = get_arg_value(arg1, ht, r);
    int *location = get_arg_location(arg0, ht, r);
    *location = val;
}

void push(char *arg, address_hashtab_t *ht, stack_t *s,
          registers_t *r) {
    // push value specified by arg onto the stack
    push_onto_stack(s, get_arg_value(arg, ht, r));
}

void pop(char *arg, address_hashtab_t *ht, stack_t *s,
         registers_t *r) {
    // pop a value from the stack and put it in the
    // location specified by arg
    int *location = get_arg_location(arg, ht, r);
    *location = pop_from_stack(s);
}

void pushf(stack_t *s, registers_t *r) {
    push_onto_stack(s, r->flags);
}

void popf(char *arg, address_hashtab_t *ht, stack_t *s,
          registers_t *r) {
    // this is the same as pop
    
    // pop a value from the stack and put it in the
    // location specified by arg
    int *location = get_arg_location(arg, ht, r);
    *location = pop_from_stack(s);
}

int call(char *arg, int SPC, label_hashtab_t *ht, stack_t *s) {
    // push current address (SPC / linenum) to stack
    push_onto_stack(s, SPC);
    // lookup label specified in arg 
    // labels include : in hash table, so concatenate a : before lookup	
    char lookupstr[MAXLINE];
    snprintf(lookupstr, MAXLINE, "%s:", arg);	
    label_list_t *lp;
    if ((lp = lookup_label(lookupstr, ht)) != NULL)
        return lp->line_num; // if label found
    else
        return SPC; // if label not found, continue at next inst.
}

int ret(stack_t *s) {
    // pop previous address (SPC / linenum) from stck
    // resume interpretation there
    return pop_from_stack(s);
}

void arith_inc(char *arg, address_hashtab_t *ht,
               registers_t *r) {
    // increment the value stored in the location specified by arg
    int *location = get_arg_location(arg, ht, r);
    *location = *location + 1;
}

void arith_dec(char *arg, address_hashtab_t *ht,
               registers_t *r) {
    // decrement the value stored in the location specified by arg
    int *location = get_arg_location(arg, ht, r);
    *location = *location - 1;
}

void arith_add(char *arg0, char *arg1, address_hashtab_t *ht,
               registers_t *r) {
    // add the values specified by arg1 and arg0, and
    // store the result in the location specified by arg0
    int *location = get_arg_location(arg0, ht, r);
    *location = *location + get_arg_value(arg1, ht, r);
}

void arith_sub(char *arg0, char *arg1, address_hashtab_t *ht,
               registers_t *r) {
    // subtract arg1 from arg0, and store the result
    // in the location specified by arg0
    int *location = get_arg_location(arg0, ht, r);
    *location = *location - get_arg_value(arg1, ht, r);
}

void arith_mul(char *arg0, char *arg1, address_hashtab_t *ht,
               registers_t *r) {
    // multiply arg1 and arg0; store result in location
    // specified by arg0
    int *location = get_arg_location(arg0, ht, r);
    *location = *location * get_arg_value(arg1, ht, r);
}

void arith_div(char *arg0, char *arg1, address_hashtab_t *ht,
               registers_t *r) {
    // divide arg0 by arg1; store result in location
    // specified by arg0
    int val0 = get_arg_value(arg0, ht, r);
    int val1 = get_arg_value(arg1, ht, r);
    
    int *location = get_arg_location(arg0, ht, r);
    *location = val0 / val1; // quotient only (int math)
    r->remreg = val0 % val1;    // remainder into remainder register
}

void arith_mod(char *arg0, char *arg1, address_hashtab_t *ht,
               registers_t *r) {
    // calculate arg0 modulus arg1;
    // store result in remainder register
    int val0 = get_arg_value(arg0, ht, r);
    int val1 = get_arg_value(arg1, ht, r);
    
    r->remreg = val0 % val1; // remainder into remainder register
}

void arith_rem(char *arg, address_hashtab_t *ht,
               registers_t *r) {
    // retrieve value from remainder register;
    // store it in location specified by arg
    int *location = get_arg_location(arg, ht, r);
    *location = r->remreg;
}

void bin_not(char *arg, address_hashtab_t *ht,
             registers_t *r) {
    // calc binary NOT of value specified by arg;
    // store result in location specified by arg
    int *location = get_arg_location(arg, ht, r);
    *location = ~ *location;
}

void bin_xor(char *arg0, char *arg1, address_hashtab_t *ht,
             registers_t *r) {
    // calc binary XOR of values specified by arg0 & arg1;
    // store result in location specified by arg0
    int *location = get_arg_location(arg0, ht, r);
    *location = *location ^ get_arg_value(arg1, ht, r);
}

void bin_or(char *arg0, char *arg1, address_hashtab_t *ht,
            registers_t *r) {
    // calc binary OR of values specified by arg0 & arg1;
    // store result in location specified by arg0
    int *location = get_arg_location(arg0, ht, r);
    *location = *location | get_arg_value(arg1, ht, r);
}

void bin_and(char *arg0, char *arg1, address_hashtab_t *ht,
             registers_t *r) {
    // calc binary AND of values specified by arg0 & arg1;
    // store result in location specified by arg0
    int *location = get_arg_location(arg0, ht, r);
    *location = *location & get_arg_value(arg1, ht, r);
}

void bin_shl(char *arg0, char *arg1, address_hashtab_t *ht,
             registers_t *r) {
    // shift the value specified by arg0 left by the
    // number of places specified by arg1;
    // store result in location specified by arg0
    int *location = get_arg_location(arg0, ht, r);
    *location = *location << get_arg_value(arg1, ht, r);
}

void bin_shr(char *arg0, char *arg1, address_hashtab_t *ht,
             registers_t *r) {
    // shift the value specified by arg0 right by the
    // number of places specified by arg1;
    // store result in location specified by arg0
    int *location = get_arg_location(arg0, ht, r);
    *location = *location >> get_arg_value(arg1, ht, r);
}

void cmp(char *arg0, char *arg1, address_hashtab_t *ht,
         registers_t *r) {
    // compare the values specified by arg0 and arg1
    // (by subtracting arg1 from arg0);
    // store the result in the flags register
    r->flags = get_arg_value(arg0, ht, r) - get_arg_value(arg1, ht, r);
}

int jmp(char *arg, int SPC, address_hashtab_t *aht,
        label_hashtab_t *lht, registers_t *r) {
    if (is_mem_addr(arg) || is_number(arg)) {
        // if address or number, return the specified value
        return get_arg_value(arg, aht, r);
    }
    else { // otherwise arg must be a label
        // lookup label; concatenate a : first
        // (labels are identified and stored with a : at the end)
        char lookupstr[MAXLINE];
        snprintf(lookupstr, MAXLINE, "%s:", arg);	
        label_list_t *lp;
        if ((lp = lookup_label(lookupstr, lht)) != NULL)
            return lp->line_num; // if label found in table
        else
            return SPC; // if label not found, continue at next inst.
    }
}

void prn(char *arg, address_hashtab_t *ht,
         registers_t *r) {
    // print the value specified by arg
    int val = get_arg_value(arg, ht, r);
    printf("%d\n", val);    
}
