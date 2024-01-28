/*
Maksim Petrushin
COP3402 HW4
Procedure version of Compiler for PL/0
11/21/2023
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_LINES 2048
#define MAX_TOKENS 2048
#define ARRAY_SIZE 2048

typedef struct
{
int kind; // const = 1, var = 2, proc = 3.
char name[13]; // name up to 11 chars
char val[13]; // number (ASCII value)
int level; // L level
int adr; // M address
int mark; // 0 = in use for code generation; 1 = unavailable
} namerecord_t;

typedef struct
{
char OP[15];
char L[15];
char M[15];
} LINE;

FILE *fInput;
FILE *fOutput;
LINE code[ARRAY_SIZE], code_num[ARRAY_SIZE];

namerecord_t symbol_table[500];
char message[135];
char lexeme[MAX_TOKENS][35];
int type[MAX_TOKENS];
int 
    TOKEN_INDEX = 0,
    CODE_INDEX = 0,
    tokenIndex = 0,
    levelAddress = 3,
    symbolTableSize = -1;
char mode[15], mode2[15];

//token types
typedef enum{
    skipsym = 1, identsym, numbersym, plussym, minussym,
    multsym, slashsym, ifelsym, eqsym, neqsym, lessym, leqsym,
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
    periodsym, becomessym, beginsym, endsym, ifsym, thensym,
    whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
    readsym , elsesym, modsym, oddsym}token_type;

//whether errors are present or not
typedef enum{
no = 0, yes = 1 } errors;

//types of errors
typedef enum{
numberTooLong = 100, 
nameTooLong, 
invalidSymbol} error_type;


// #HELPERS
int close_error(void);

// #LEX
int isValidIdent(char* currLexeme);
int isValidReserve(char* currLexeme);
int isValidNumber(char* currLexeme);
int getType(char currChar[2], char prevChar[2]);

// #SYNTAX
void emit(char* comand, char* lex, char* m);
int symbol_table_lookup(char* name, int level);
void symbol_table_insert(int kind, char* name, char* val, int lex_lvl, int adr, int mark);
void symbol_table_delete(int index);
void load_token(void);
int program(void);
int block(int lev);
void return_from_block(int lev);
int const_declaration(int lev);
int var_declaration(int lev, int *numVars);
int procedure_declaration(int lev, int* numVars);
int statement(int lev);
int expression(int lev);
int condition(int lev);
int term(int lev);
int factor(int lev);


int main(int argc, char** argv){
    
    //helper variable definition
    int count,
        i, 
        j, 
        numLines, 
        currType,
        prevType=-1,
        savedPrevType=-1, 
        error = no,
        flag = 0,
        commentError = no, 
        currSpace = no,
        commentMode = no,
        lineLen,
        outputPrint = yes;
    char ch;
    
    char lexemeList[MAX_TOKENS][35];
    int typeList[MAX_TOKENS];
    char lines[MAX_LINES][100];
    char currLexeme[35] = "";
    char savedLexeme[35] = "";
    char currChar[2] = "";
    char prevChar[2] = "";
    char savedPrevChar[2] = "";

    //initializing tokens to type 0 (none from 1-33)
    for(i=0;i<MAX_TOKENS;i++){
        type[i]=0;
    }
    i=0;

    //input and output file pointer declaration
    fInput = fopen(/*"in23.txt"*/argv[1] /**/, "r");
    if (fInput == NULL){
        printf("Sorry can not find the file, FILE Error!");
        exit(1);
    }

    fOutput = fopen("elf.txt", "w");
    if (fOutput == NULL){
        outputPrint = no;
    }

    // Block for fetching input program lines
    for (i = 0; i < MAX_LINES; i++){
        count = 0;
        ch = getc(fInput);
        while ((ch != '\n') && (ch != EOF)){
            lines[i][count] = ch;
            count++;
            ch = getc(fInput);
        }
        if (ch == EOF){ // stops from iterating
            lines[i][count] = '\0';
            break;
        }
        lines[i][count] = '\0';
    }
    numLines = i + 1;
    fclose(fInput);

    // Block for fetching lexeme table
    for (i = 0; i < numLines; i++){
        lineLen = strlen(lines[i]);

        if(lineLen==0){
            lineLen=1;
        }
        for(j = 0; j < lineLen; j++){
            currChar[0] = lines[i][j];
            currChar[1] = '\0';
            currType = getType(currChar, prevChar);
            if((flag>1 && currType == slashsym && prevType == multsym) || (commentMode==yes && i ==(numLines-1) && j == (lineLen-1))){
                commentMode = no;
                flag=0;
                if(currType == slashsym && prevType == multsym){
                    commentError = no;
                }

                
                prevType = savedPrevType;
                prevChar[0] = savedPrevChar[0];
                strcpy(currLexeme, savedLexeme);
                
                continue;
            }

            if(currType == multsym && prevType == slashsym){
                commentMode = yes;
                
                commentError = invalidSymbol;
                savedPrevChar[0] = lexeme[tokenIndex-1][0];
                savedPrevType = type[tokenIndex-1];
                strcpy(savedLexeme, lexeme[tokenIndex-1]);
                lexeme[tokenIndex-1][0] ='\0';
                tokenIndex--;    
            }
            if(commentMode == yes){
                flag++;
            }
            //finding the type of the current character. reserved sym and ident sym are combined here
            
            if(commentMode == no){
                if(isspace(currChar[0]) || lines[i][0]=='\0'){
                    currSpace = yes;
                    prevChar[0] = ' ';
                    continue;
                }

                //we have a "current token". that is the one that is being handled right now
                //we have 2 decisions: 1) to keep adding chars to the current token
                //or 2) to move on, and "submit" the current token to our final list of tokens
                // if statement below is true (all the exceptions and when whe SHOULD keep adding)
                // then we will concat "current char" to "current token"
                if( !(strlen(currLexeme)>35) &&
                    ((j != 0) || i == 0) &&
                    ((currSpace == no) || (prevType == -1)) &&
                    (
                        !(currType == plussym && prevType == plussym) &&
                        !(currType == minussym && prevType == minussym) &&
                        !(currType == multsym && prevType == multsym) &&
                        !(currType == slashsym && prevType == slashsym) &&
                        !(currType == eqsym && prevType == eqsym) &&
                        !(currType == lparentsym && prevType == lparentsym) &&
                        !(currType == rparentsym && prevType == rparentsym) &&
                        !(currType == commasym && prevType == commasym) &&
                        !(currType == semicolonsym && prevType == semicolonsym) &&
                        !(currType == periodsym && prevType == periodsym) &&
                        !(currType == lessym && prevType == lessym) &&
                        !(currType == gtrsym && prevType == gtrsym) 
                    )  &&
                    ((currType == prevType && prevType!=invalidSymbol) || //continuing same time
                    (currType == identsym && prevType == numbersym && getType(currLexeme,"") == identsym) || //alpha-num 
                    (currType == numbersym && prevType == identsym && getType(currLexeme,"") == identsym) || //alpha-num 
                    (currType == becomessym) ||
                    (currType == neqsym) ||
                    (currType == geqsym) ||
                    (currType == leqsym) ||
                    (prevType == -1))){
                    strcat(currLexeme, currChar);
                }
                //else we "submit" the current token to the table of tokens
                //in this branch we also check the validity of the current token
                // if it is NOT valid, it will still be added to table, but it
                // will then just be submitted with an "error" flag
                else{
                    //checking if we can call our current token a "valid number"
                    if(isValidNumber(currLexeme)==1){
                        strcpy(lexeme[tokenIndex],currLexeme);
                        type[tokenIndex] = numbersym;
                        tokenIndex++;
                    }
                    //else, checking if we can call our current token an "invalid (long) number"
                    else if(isValidNumber(currLexeme)==numberTooLong){
                        strcpy(lexeme[tokenIndex],currLexeme);
                        type[tokenIndex] = numberTooLong;
                        error = yes;
                        tokenIndex++;
                    }
                    //else, checking if we can call our current token a "valid reserve word"
                    else if(isValidReserve(currLexeme)){
                        strcpy(lexeme[tokenIndex],currLexeme);
                        type[tokenIndex] = isValidReserve(currLexeme);
                        tokenIndex++;
                    }
                    //else, checking if we can call our current token an "valid identifier (var or conts)"
                    else if(isValidIdent(currLexeme)==nameTooLong){
                        strcpy(lexeme[tokenIndex],currLexeme);
                        type[tokenIndex] = nameTooLong;
                        error = yes;
                        tokenIndex++;
                    }
                    //else, checking if we can call our current token an "invalid (too long) identifier (var or conts)"
                    else if(isValidIdent(currLexeme)){
                        strcpy(lexeme[tokenIndex],currLexeme);
                        type[tokenIndex] = identsym;
                        tokenIndex++;
                    }
                    //else, checking if we can call our current token a "terminal symbol", (the prevType decides which one)
                    else if(prevType>1 && prevType<36){    
                        strcpy(lexeme[tokenIndex],currLexeme);
                        type[tokenIndex] = prevType;
                        tokenIndex++;
                    }
                    //esle it's an invalid symbol.
                    else{
                        strcpy(lexeme[tokenIndex],currLexeme);
                        type[tokenIndex] = prevType;
                        error = yes;
                        tokenIndex++;
                    }
                    currLexeme[0] = '\0';
                    strcat(currLexeme, currChar);
                }
            }
            
            //updating prev char
            //the whole idea of keeping track of character is that
            // each character also is being checked for it's type.
            currSpace = no;
            prevType = currType;
            prevChar[0] = currChar[0];
            prevChar[1] = '\0';
        }
        
    }
    
    //a cycle to finish the last token
    //all the same that inside the loop, but just the last token needs to be manually added like this
    if(commentMode == no){//this only happens if the comments are valid (properly closed)

        if(isValidNumber(currLexeme)==1){
            strcpy(lexeme[tokenIndex],currLexeme);
            type[tokenIndex] = numbersym;
            tokenIndex++;
        }
        else if(isValidNumber(currLexeme)==numberTooLong){
            strcpy(lexeme[tokenIndex],currLexeme);
            type[tokenIndex] = numberTooLong;
            error = yes;
            tokenIndex++;
        }
        else if(isValidReserve(currLexeme)){
            strcpy(lexeme[tokenIndex],currLexeme);
            type[tokenIndex] = isValidReserve(currLexeme);
            tokenIndex++;
        }
        else if(isValidIdent(currLexeme)==nameTooLong){
            strcpy(lexeme[tokenIndex],currLexeme);
            type[tokenIndex] = nameTooLong;
            error = yes;
            tokenIndex++;
        }
        else if(isValidIdent(currLexeme)){
            strcpy(lexeme[tokenIndex],currLexeme);
            type[tokenIndex] = identsym;
            tokenIndex++;
        }
        else if(prevType>1 && prevType<36){    
            strcpy(lexeme[tokenIndex],currLexeme);
            type[tokenIndex] = prevType;
            tokenIndex++;
        }
        else{
            strcpy(lexeme[tokenIndex],currLexeme);
            type[tokenIndex] = prevType;
            error = yes;
            tokenIndex++;
        }
        currLexeme[0] = '\0';
        strcat(currLexeme, currChar);
    }
    // Block for printing the source input program
    printf("\n");
    for (int i = 0; i < numLines; i++)
        printf("%s\n",lines[i]);

    // #SCANNER
    // Comment Closure Check Block
    if(commentError == invalidSymbol){
        printf("   ***** Error number 024, (LEXICAL) Comment is most likely not closed\n");
        return close_error();
    }


    // #SCANNER
    // Lexical Check Block
    for(i=0; i<tokenIndex; i++){   
        if(type[i]== numberTooLong){
            printf("   ***** Error number 025, (LEXICAL) This number is too large: %s\n", lexeme[i]);
            return close_error();
        }
        if(type[i]== nameTooLong){
            printf("   ***** Error number 026, (LEXICAL) Identifier too long: %s\n", lexeme[i]); 
            return close_error();
        }   
        if(type[i]== invalidSymbol){
            printf("   ***** Error number 027, (LEXICAL) Invalid symbol: %s\n", lexeme[i]);
            return close_error();
        }
    }
    

    
    // Block for running the program
    if(program() == -1){
        printf("   %s\n", message);
        return close_error();
    }

    
    printf("\nNo errors, program is syntactically correct.\n\n");
    
    // Block for translating instructions mnemonics to numbers
    for(int i=0; i<CODE_INDEX/3; i++){
        int index = i;

        strcpy(code_num[index].L,code[index].L);
        strcpy(code_num[index].M,code[index].M);

        if(strcmp(code[index].OP, "LIT") == 0){
            strcpy(code_num[index].OP,"1");
        }
        else if(strcmp(code[index].OP, "OPR") == 0){
            strcpy(code_num[index].OP,"2");
        }
        else if(strcmp(code[index].OP, "LOD") == 0){
            strcpy(code_num[index].OP,"3");
        }
        else if(strcmp(code[index].OP, "STO") == 0){
            strcpy(code_num[index].OP,"4");
        }
        else if(strcmp(code[index].OP, "CAL") == 0){
            strcpy(code_num[index].OP,"5");
        }
        else if(strcmp(code[index].OP, "INC") == 0){
            strcpy(code_num[index].OP,"6");
        }
        else if(strcmp(code[index].OP, "JMP") == 0){
            strcpy(code_num[index].OP,"7");
        }
        else if(strcmp(code[index].OP, "JPC") == 0){
            strcpy(code_num[index].OP,"8");
        }
        else if(strcmp(code[index].OP, "SYS") == 0){
            strcpy(code_num[index].OP,"9");
        }
    }
    
    for(int i=0; i<CODE_INDEX/3; i++){
        printf/*("%3d*/("%s%5s%5s\n", /*i,*/ code[i].OP, code[i].L, code[i].M);
        if(outputPrint == yes){
            fprintf(fOutput,/*("%3d*/"%s%5s%5s\n", /*i,*/ code_num[i].OP, code_num[i].L, code_num[i].M);
        }
    }

    // Symbol table part (commented out, and is here for testing only) 
    /*
    printf("\nSymbol Table:\n\nKind | Name        | Value | Level | Address | Mark \n---------------------------------------------------\n");
    if(outputPrint == yes){
            fprintf(fOutput,"\nSymbol Table:\n\nKind | Name        | Value | Level | Address | Mark \n---------------------------------------------------\n");
        }
    for(int i=0; i<=symbolTableSize; i++){
        printf("%4d |%12s |%6s |%6d |%8d |%6d \n", symbol_table[i].kind, symbol_table[i].name, symbol_table[i].val, symbol_table[i].level, symbol_table[i].adr, symbol_table[i].mark);
        if(outputPrint == yes){
            fprintf(fOutput,"%4d |%12s |%6s |%6d |%8d |%6d \n", symbol_table[i].kind, symbol_table[i].name, symbol_table[i].val, symbol_table[i].level, symbol_table[i].adr, symbol_table[i].mark);
        }
    }
    
    */
    
    fclose(fOutput);
    return 0;
}

// #HELPER
int close_error(void){
    fclose(fOutput);
    fclose(fInput);
    return -1;
}
// # SYNTAX

// Emit - for printing generated commands
void emit(char* comand, char* lex, char* mode){

    int index = CODE_INDEX/3;

    strcpy(code[index].OP,comand);
    strcpy(code[index].L,lex);
    strcpy(code[index].M,mode);

    CODE_INDEX +=3;
}

// Symbol Table Lookup
int symbol_table_lookup(char* name, int level){
    int size = symbolTableSize;
    for(int i = size; i>0; i--){
        if(strcmp(symbol_table[i].name, name)== 0 && symbol_table[i].mark == 0){
            return i;
        }
    }
    return -1;
}

// Symbol Table Insert
void symbol_table_insert(int kind, char* name, char* val, int lex_lvl, int adr, int mark){
    int size = ++symbolTableSize;
    symbol_table[size].level = lex_lvl;   
    symbol_table[size].mark = mark;
    symbol_table[size].adr = adr;
    strcpy(symbol_table[size].name, name);
    strcpy(symbol_table[size].val, val);
    symbol_table[size].kind = kind;
}

// Symbol Table Delete (De-activate)
void symbol_table_delete(int index){
    symbol_table[index].mark = 1;
}

// Load Token
void load_token(void){
    TOKEN_INDEX++;
}

// Program
int program(void){
    symbol_table_insert(3, "main_scope", "0", 0, 0, 0);
    if (block(0)== -1){
        return -1;
    }
    if(type[TOKEN_INDEX] != periodsym){
        sprintf(message,"***** Error number 009, Period expected");
        return -1; 
    }
    load_token();
    if(type[TOKEN_INDEX] != 0){
        sprintf(message,"***** Error number 006, Redundant code after end of program (after period)");
        return -1; 
    }
    
    emit("SYS","0","3");
    return 0;
}

// Block
int block(int lev){
    int dx, tx0, cx0;
    dx = 3;
    tx0 = symbolTableSize;
    symbol_table[tx0].adr = CODE_INDEX; 

    emit("JMP","0","0");
    //TODO: lev > levmax thing
    if(const_declaration(lev) == -1)
    {
        return -1;
    }
    if(var_declaration(lev, &dx) == -1)
    {
        return -1;
    }
    if(procedure_declaration(lev, &dx) == -1)
    {
        return -1;
    }
    sprintf(mode,"%d",CODE_INDEX);
    strcpy(code[symbol_table[tx0].adr/3].M, mode);

    symbol_table[tx0].adr = CODE_INDEX; 

    sprintf(mode,"%d",dx);
    emit("INC","0",mode);
    int statement_res = statement(lev);
    if(statement_res == -1)
    {
        return -1;
    }
    if( statement_res != 1 && (
        (type[TOKEN_INDEX] == semicolonsym &&
         TOKEN_INDEX+1 < MAX_TOKENS && 
         type[TOKEN_INDEX+1] == periodsym) ||
        type[TOKEN_INDEX] == identsym     ||
        type[TOKEN_INDEX] == callsym      || 
        type[TOKEN_INDEX] == beginsym     || 
        type[TOKEN_INDEX] == ifsym        || 
        type[TOKEN_INDEX] == whilesym     || 
        type[TOKEN_INDEX] == readsym      || 
        type[TOKEN_INDEX] == writesym     ||
        type[TOKEN_INDEX] == procsym)
    ){
        sprintf(message,"***** Error number 008, Incorrect symbol after statement part in block");
        return -1;
    }
    return_from_block(lev);
    if(strcmp(symbol_table[tx0].name, "main_scope")!=0)
    emit("OPR","0","0");
    return 0;
}


void return_from_block(int lev){
    for(int i =1; i<= symbolTableSize; i++){
        if(symbol_table[i].level == lev)
        symbol_table_delete(i);
    }
}

// Const Delaration
int const_declaration(int lev){
    //return -1 as error 0 otherwise
    if(type[TOKEN_INDEX] == constsym){
        do{ 
            //identifier of a constant
            load_token();
            
            if(type[TOKEN_INDEX] != identsym){
                sprintf(message,"***** Error number 004, const, var, procedure, and read must be followed by identifier");
                return -1;
            }
            
            char* constant = lexeme[TOKEN_INDEX];
            //must be new
            int ind = symbol_table_lookup(constant, lev);
            if( ind != -1 && symbol_table[ind].level == lev ){
                sprintf(message, "***** Error number 019, Symbol name has already been declared: \"%s\"", constant);
                return -1;
            }

            //equals
            load_token();
            if(type[TOKEN_INDEX] != eqsym){
                if(type[TOKEN_INDEX] == becomessym){
                    sprintf(message, "***** Error number 001, Use = instead of :=");
                    return -1;
                }
                sprintf(message, "***** Error number 003, Identifier must be followed by =");
                return -1;
            }

            //number
            load_token();
            if(type[TOKEN_INDEX] != numbersym){
                sprintf(message, "***** Error number 002, = must be followed by a number: \"%s\"", constant);
                return -1;
            }
            char* val = lexeme[TOKEN_INDEX];
            symbol_table_insert(1, constant, val, lev, 0, 0);
            
            load_token();
        }while(type[TOKEN_INDEX] == commasym);
        if(type[TOKEN_INDEX] != semicolonsym)
        {
            sprintf(message,"***** Error number 005, Semicolon or comma missing");
            return -1;
        }
        load_token();
    }
    return 0;
}

int var_declaration(int lev, int* numVars){
    if(type[TOKEN_INDEX] == varsym){
        do{
            *numVars = *numVars+1;
            load_token();
            //has to be identity (at least one)
            if(type[TOKEN_INDEX] != identsym){
                sprintf(message, "***** Error number 004, const, var, procedure, and read must be followed by identifier");
                return -1;
            }

            char* variable = lexeme[TOKEN_INDEX];
            //must be new
            int ind = symbol_table_lookup(variable, lev);
            if( ind != -1 && symbol_table[ind].level == lev ){
                sprintf(message, "***** Error number 019, Symbol name has already been declared: \"%s\"", variable);
                return -1;
            }
            symbol_table_insert(2, variable, "0", lev, *numVars-1, 0);

            load_token();
        }while(type[TOKEN_INDEX] == commasym);

        if(type[TOKEN_INDEX] != semicolonsym){
            sprintf(message, "***** Error number 005, Semicolon or comma missing");
            return -1;
        }
        load_token();
    }
    return 0;
}

int procedure_declaration(int lev, int* numVars){
    while(type[TOKEN_INDEX] == procsym){
        load_token();
        if(type[TOKEN_INDEX] != identsym)
        {
            sprintf(message, "***** Error number 004, const, var, procedure, and read must be followed by identifier");
            return -1;
        }
        char* proc = lexeme[TOKEN_INDEX];
        //must be new
        int ind = symbol_table_lookup(proc, lev);
        if( ind != -1){
            sprintf(message, "***** Error number 019, Symbol name has already been declared: \"%s\"", proc);
            return -1;
        }
        symbol_table_insert(3, proc, "0", lev, 0, 0);
        load_token();

        if(type[TOKEN_INDEX] != semicolonsym){
            sprintf(message, "***** Error number 005, Semicolon or comma missing");
            return -1;
        }

        load_token();

        if (block(lev+1)== -1)
        {
            if(!(type[TOKEN_INDEX] == semicolonsym &&
                 TOKEN_INDEX+1 < MAX_TOKENS && 
                 type[TOKEN_INDEX+1] == periodsym))
            return -1;
        }

        if(type[TOKEN_INDEX] != semicolonsym)
        {
            sprintf(message, "***** Error number 005, Semicolon or comma missing");
            return -1;
        }

        load_token();
    }
    return 0;
}

int statement(int lev){
    if(type[TOKEN_INDEX] == identsym){
        int symID = symbol_table_lookup(lexeme[TOKEN_INDEX], lev);
        if(symID == -1){
            sprintf(message, "***** Error number 011, Undeclared identifier: \"%s\"", lexeme[TOKEN_INDEX]);
            return -1;  
        }
        //if it's not a var (const)
        if(symbol_table[symID].kind != 2){
            sprintf(message, "***** Error number 012, Assignment to constant or procedure is not allowed");
            return -1;
        }

        load_token();
        // := sym is a must have
        if(type[TOKEN_INDEX] != becomessym){
            sprintf(message, "***** Error number 013, Assignment operator expected");
            return -1;
        }
        load_token();

        if(expression(lev)== -1){
            return -1;
        }
        sprintf(mode,"%d",symbol_table[symID].adr);
        sprintf(mode2,"%d",lev - symbol_table[symID].level);
        emit("STO", mode2, mode);

        return 0;
    }
    if(type[TOKEN_INDEX] == callsym){
        
        load_token();
        
        if(type[TOKEN_INDEX] != identsym){
            sprintf(message,"***** Error number 014, call must be followed by an identifier");
            return -1;
        }

        int symID = symbol_table_lookup(lexeme[TOKEN_INDEX], lev);
        if(symID == -1){
            sprintf(message, "***** Error number 007, Procedure isnt in the scope or undeclared: \"%s\"", lexeme[TOKEN_INDEX]);
            return -1;
        }
        if(symbol_table[symID].kind == 3){
            sprintf(mode,"%d",symbol_table[symID].adr);
            sprintf(mode2,"%d",lev - symbol_table[symID].level);
            emit("CAL", mode2, mode);
        }
        else{
            sprintf(message, "***** Error number 015, Call of a constant or variable is meaningless: \"%s\"", lexeme[TOKEN_INDEX]);
            return -1;
        }

        load_token();
        return 0;
    }
    if(type[TOKEN_INDEX] == beginsym){
        do{
            load_token();
            if(statement(lev)==-1){
                return -1;
            }
        }while(type[TOKEN_INDEX] == semicolonsym);
        if(type[TOKEN_INDEX] != endsym){
            if(type[TOKEN_INDEX] == identsym || type[TOKEN_INDEX] == callsym || type[TOKEN_INDEX] == beginsym || type[TOKEN_INDEX] == ifsym || type[TOKEN_INDEX] == whilesym || type[TOKEN_INDEX] == readsym || type[TOKEN_INDEX] == writesym){
                sprintf(message,"***** Error number 010, Semicolon between statements missing");
                return -1;
            }
            sprintf(message,"***** Error number 017, Semicolon or End expected");
            return -1;
        }
        load_token();
        return 0;
    }
    if(type[TOKEN_INDEX] == ifsym){
        load_token();
        if(condition(lev) == -1){
            return -1;
        }
        int jpcIDx = CODE_INDEX;
        sprintf(mode, "%d", jpcIDx);
        emit("JPC","0",mode);
        if(type[TOKEN_INDEX] != thensym){
            sprintf(message, "***** Error number 016, then expected");
            return -1;
        }
        load_token();
        if(statement(lev)==-1){
            return -1;
        }

        sprintf(mode, "%d",CODE_INDEX);
        strcpy(code[jpcIDx/3].M, mode);
        return 0;
    }
    if(type[TOKEN_INDEX] == whilesym){
        load_token();
        int loopIDx = CODE_INDEX;
        if(condition(lev) == -1){
            return -1;
        }
        if(type[TOKEN_INDEX]!=dosym){
            sprintf(message, "***** Error number 018, Do expected");
            return -1;
        }
        load_token();
        int jpcIDx = CODE_INDEX;
        sprintf(mode,"%d",jpcIDx);
        emit("JPC","0",mode);
        if(statement(lev) == -1){
            return -1;
        }
        sprintf(mode,"%d",loopIDx);
        emit("JMP","0",mode);

        sprintf(mode, "%d",CODE_INDEX);
        strcpy(code[jpcIDx/3].M,mode);
        return 0;
    }
    if(type[TOKEN_INDEX] == readsym){
        load_token();

        if(type[TOKEN_INDEX] != identsym){
            sprintf(message, "***** Error number 004, const, var, procedure, and read must be followed by identifier");
            return -1;
        }
        int symIDx = symbol_table_lookup(lexeme[TOKEN_INDEX], lev);
        if(symIDx == -1){
            sprintf(message, "***** Error number 011, Undeclared identifier: \"%s\"", lexeme[TOKEN_INDEX]);
            return -1;
        }
        if(symbol_table[symIDx].kind != 2){
            sprintf(message, "***** Error number 012, Assignment to constant or procedure is not allowed");
            return -1;
        }
        load_token();
        emit("SYS", "0", "2");
        sprintf(mode,"%d",symbol_table[symIDx].adr);
        sprintf(mode2,"%d",lev - symbol_table[symIDx].level);
        emit("STO",mode2,mode);
        return 0;
    }
    if(type[TOKEN_INDEX] == writesym){
        load_token();
        if(expression(lev)== -1){
            return -1;
        }
        emit("SYS","0","1");
        return 0;
    }
    return 1;
}

int condition(int lev){
    if(type[TOKEN_INDEX] == oddsym){
        load_token();
        if(expression(lev) == -1){
            return -1;
        }
        emit("OPR","0","11");
        return 0;
    }
    else{
        if(expression(lev) == -1){
            return -1;
        }
        if(type[TOKEN_INDEX] == eqsym){
            load_token();
            if(expression(lev) == -1){
                return -1;
            }
            emit("OPR","0","5");
            return 0;
        }
        else if(type[TOKEN_INDEX] == neqsym){
            load_token();
            if(expression(lev) == -1){
                return -1;
            }
            emit("OPR","0","6");
            return 0;
        }
        else if(type[TOKEN_INDEX] == lessym){
            load_token();
            if(expression(lev) == -1){
                return -1;
            }
            emit("OPR","0","7");
            return 0;
        }
        else if(type[TOKEN_INDEX] == leqsym){
            load_token();
            if(expression(lev) == -1){
                return -1;
            }
            emit("OPR","0","8");
            return 0;
        }
        else if(type[TOKEN_INDEX] == gtrsym){
            load_token();
            if(expression(lev) == -1){
                return -1;
            }
            emit("OPR","0","9");
            return 0;
        }
        else if(type[TOKEN_INDEX] == geqsym){
            load_token();
            if(expression(lev) == -1){
                return -1;
            }
            emit("OPR","0","10");
            return 0;
        }
        else{
            sprintf(message, "***** Error number 020, Relational operator expected");
            return -1;
        }
    }
}

int expression(int lev){
        if(term(lev) == -1){
            return -1;
        }
        while(type[TOKEN_INDEX] == plussym || type[TOKEN_INDEX] == minussym){
            if(type[TOKEN_INDEX] == plussym){
                load_token();
                if(term(lev) == -1){
                    return -1;
                }
                emit("OPR","0","1");
            }
            else{
                load_token();
                if(term(lev) == -1){
                    return -1;
                }
                emit("OPR","0","2");
            }
        }
        return 0;
}

int term(int lev){
    if(factor(lev) == -1){
        return -1;
    }
    while(type[TOKEN_INDEX] == multsym || type[TOKEN_INDEX] == slashsym){
        if(type[TOKEN_INDEX] == multsym){
            load_token();
            if(factor(lev) == -1){
                return -1;
            }
            emit("OPR","0","3");
        }
        else {
            load_token();
            if(factor(lev) == -1){
                return -1;
            }
            emit("OPR","0","4");
        }
    }
    return 0;
}

int factor(int lev){
    if(type[TOKEN_INDEX] == identsym){
        int symIDx = symbol_table_lookup(lexeme[TOKEN_INDEX], lev);
        if(symIDx == -1){
            sprintf(message, "***** Error number 011, Undeclared identifier: \"%s\"", lexeme[TOKEN_INDEX]);
            return -1;
        }
        //constant
        if(symbol_table[symIDx].kind == 1){
            emit("LIT","0",symbol_table[symIDx].val);
        }
        //variable
        else if(symbol_table[symIDx].kind == 2){
            sprintf(mode,"%d",symbol_table[symIDx].adr);
            sprintf(mode2,"%d",lev - symbol_table[symIDx].level);
            emit("LOD",mode2,mode);
        }
        else{
            sprintf(message, "***** Error number 021, Expression must not contain a procedure identifier: \"%s\"", lexeme[TOKEN_INDEX]);
            return -1;
        }
        load_token();
        return 0;
    }
    else if(type[TOKEN_INDEX] == numbersym){
        emit("LIT","0",lexeme[TOKEN_INDEX]);
        load_token();
        return 0;
    }
    else if(type[TOKEN_INDEX] == lparentsym){
        load_token();
        if(expression(lev) == -1){
            return -1;
        }
        if(type[TOKEN_INDEX] != rparentsym){
            sprintf(message, "***** Error number 022, Right parenthesis missing");
            return -1;
        }
        load_token();
        return 0;
    }
    else {
        sprintf(message, "***** Error number 023, An expression cannot begin with this symbol.");
        return -1;
    }
}

//#LEX
int getType(char currChar[2], char prevChar[2]){
//this returns either a number, identifier, or a terminal symbol
//this is what I implemented as a checker of a character type.
// checking the type of each character helps understand whether it is needed
// to stop adding characters to the "current token" or not. And a corner case,
// the 2-character terminal symbols. Hence the prevChar passing.
    char ch = currChar[0];
    char pr = prevChar[0];
    //digit
    if(ch >= 48 && ch <= 57){
        return numbersym;
    }
    //alpha
    else if((ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122)){
        return identsym;
    }
    else if(ch == '>' && pr == '<'){
        return neqsym;
    }
    else if(ch == '>'){
        return gtrsym;
    }
    else if(ch == '=' && pr == '<'){
        return leqsym;
    }
    else if(ch == '=' && pr == '>'){
        return geqsym;
    }
    else if(ch == '=' && pr == ':'){
        return becomessym;
    }
    else if(ch == '='){
        return eqsym;
    }
    else if(ch == '<'){
        return lessym;
    }
    else if(ch == '+'){
        return plussym;
    }
    else if(ch == '-'){
        return minussym;
    }
    else if(ch == '*'){
        return multsym;
    }
    else if(ch == '/'){
        return slashsym;
    }
    else if(ch == '('){
        return lparentsym;
    }
    else if(ch == ')'){
        return rparentsym;
    }
    else if(ch == ','){
        return commasym;
    }
    else if(ch == ';'){
        return semicolonsym;
    }
    else if(ch == '.'){
        return periodsym;
    }
    else if(ch == ':'){
        return invalidSymbol;
    }
    else{
        return invalidSymbol;
    }
}

//#LEX
int isValidNumber(char* currLexeme){
//just integers like in the rubric.. no floaters
//Checking if number is valid according to formula.
//also if too long, the number-too-long flag is returned
    int len = strlen(currLexeme);

    char ch;
    if(len ==0){
        return 0;
    }
    for(int i = 0; i < len; i++){
        ch = currLexeme[i];
        if(!(ch >= 48 && ch <= 57)){
            return 0;
        }
    }
    if(len > 5){
        return numberTooLong;
    }
    return 1;
}

//#LEX
int isValidReserve(char currLexeme[35]){
//Checking if reserved is valid according to formula.
//this is the simplest thing in the whole programm...
// just checking if our given token is one of the reserved words
    if(!strcmp(currLexeme, "ifel")){
        /*
        return ifelsym;
        */
        return identsym;
    }
    if(!strcmp(currLexeme, "begin")){
        return beginsym;
    }
    if(!strcmp(currLexeme, "end")){
        return endsym;
    }
    if(!strcmp(currLexeme, "if")){
        return ifsym;
    }
    if(!strcmp(currLexeme, "then")){
        return thensym;
    }
    if(!strcmp(currLexeme, "while")){
        return whilesym;
    }
    if(!strcmp(currLexeme, "do")){
        return dosym;
    }
    if(!strcmp(currLexeme, "call")){
        return callsym;
    }
    if(!strcmp(currLexeme, "const")){
        return constsym;
    }
    if(!strcmp(currLexeme, "var")){
        return varsym;
    }
    if(!strcmp(currLexeme, "procedure")){
        return procsym;
    }
    if(!strcmp(currLexeme, "write")){
        return writesym;
    }
    if(!strcmp(currLexeme, "read")){
        return readsym;
    }
    if(!strcmp(currLexeme, "else")){
        /*
        return elsesym;
        */
        return identsym;
    }
    if(!strcmp(currLexeme, "odd")){
        return oddsym;
    }

    
    return 0;
}

//#LEX
int isValidIdent(char* currLexeme){
//Checking if identifier is valid according to formula.
//also if too long, the name-too-long flag is returned
    int len = strlen(currLexeme);

    char ch = currLexeme[0];
    if(!((ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122))){
            return 0;
        }
    for(int i = 1; i < len; i++){
        ch = currLexeme[i];
        if(!((ch >= 48 && ch <= 57) || (ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122))){
            return 0;
        }
    }
    if(len > 11){
        return nameTooLong;
    }
    return identsym;
}
