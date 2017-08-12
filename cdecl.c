#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAXTOKENLEN 64
#define MAXTOKENS   100

enum type_tag { IDENTIFIER, QUALIFIER, TYPE };

struct token {
    char type;
    char string[MAXTOKENLEN];
};

int top = -1;
struct token stack[MAXTOKENS];
struct token this;

#define pop stack[top--] 
#define push(s) stack[++top] = s

#define STRCMP(a, b) (strcmp(a,b) == 0)

enum type_tag classify_string(void) {
    char *s = this.string;
    if(STRCMP(s, "volatile")) return QUALIFIER;
    if(STRCMP(s, "void")) return TYPE;
    if(STRCMP(s, "char")) return TYPE;
    if(STRCMP(s, "signed")) return TYPE;
    if(STRCMP(s, "unsigned")) return TYPE;
    if(STRCMP(s, "short")) return TYPE;
    if(STRCMP(s, "signed")) return TYPE;
    if(STRCMP(s, "int")) return TYPE;
    if(STRCMP(s, "long")) return TYPE;
    if(STRCMP(s, "double")) return TYPE;
    if(STRCMP(s, "float")) return TYPE;
    if(STRCMP(s, "struct")) return TYPE;
    if(STRCMP(s, "union")) return TYPE;
    if(STRCMP(s, "enum")) return TYPE;
    return IDENTIFIER;
}

void gettoken(void) {
    char *p = this.string;
    while((*p = getchar()) == ' ');
    if(isalnum(*p)) {
        while(isalnum(*++p = getchar()));
        ungetc(*p,stdin);
        *p = '\0';
        this.type = classify_string();
        return;
    }

    if(*p == '*') {
        strcpy(this.string,"pointer to");
        this.type = '*';
        return;
    }

    this.string[1] = '\0';
    this.type = *p;
    return;
}

read_to_first_identifier() {
    gettoken();
    while(this.type != IDENTIFIER) {
        push(this);
        gettoken();
    }
    printf("%s is ",this.string);
    gettoken();
}

deal_with_array() {
    while(this.type == '[') {
        printf("array ");
        gettoken();
        if(isdigit(this.string[0])) {
            printf("0..%d ",atoi(this.string)-1);
            gettoken();         // read ']'
        }
        gettoken();
        printf("of ");
    }
}

deal_with_function_args() {
    while(this.type != ')')  {
        gettoken();
    }
    gettoken();
    printf("function returning ");
}

deal_with_pointers() {
    while(stack[top].type == '*') {
        printf("%s ",pop.string);
    }
}

deal_with_declarator() {
    switch(this.type) {
        case '[':deal_with_array();break;
        case '(':deal_with_function_args();
    }

    deal_with_pointers();

    while(top >= 0){
        if(stack[top].type == '(') {
            pop;
            gettoken();
            deal_with_declarator();
        }else {
            printf("%s ",pop.string);
        }
    }
}

main() {
    read_to_first_identifier();
    deal_with_declarator();
    printf("\n");
    return 0;
}
