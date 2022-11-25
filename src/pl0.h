#ifndef PL0_H
#define PL0_H

#include <stdio.h>
#include "set.h"

#define RESERVE_WORD_TABLE_MAX_LENGTH 11
#define IDENTIFIER_TABLE_MAX_LENGTH 500
#define DIGIT_MAX_LENGTH 14
#define REVERSE_CHAR_TABLE_MAX_LENGTH 10
#define IDENTIFIER_MAX_LENGTH 10

#define MAX_ADDRESS 32767
#define MAX_LEVEL 32
#define INST_MAX_COUNT 500

#define STACK_SIZE 1000

typedef enum
{
	SYM_NULL,
	SYM_IDENTIFIER,
	SYM_NUMBER,
	SYM_PLUS,
	SYM_MINUS,
	SYM_TIMES,
	SYM_SLASH,
	SYM_ODD,
	SYM_EQU,
	SYM_NEQ,
	SYM_LES,
	SYM_LEQ,
	SYM_GTR,
	SYM_GEQ,
	SYM_LPAREN,
	SYM_RPAREN,
	SYM_COMMA,
	SYM_SEMICOLON,
	SYM_PERIOD,
	SYM_BECOMES,
    SYM_BEGIN,
	SYM_END,
	SYM_IF,
	SYM_THEN,
	SYM_WHILE,
	SYM_DO,
	SYM_CALL,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE
} sym_type;

typedef enum
{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE
} id_type;

/**
 * @brief Include 7 non-operational operations
 * and an arithmetic operator identifier 'OPR'
 */
typedef enum
{
	LIT,	/**< Instructions for loading constants onto the top of the stack. */
	OPR,	/**< Includes a set of instructions for arithmetic and relational operations. */
	LOD,	/**< Instruction for loading the value of a variable onto the top of the stack. */
	STO,	/**< Storage instructions corresponding to assignment statements. */
	CAL,	/**< Instructions corresponding to procedure calls. */
	INT,	/**< Instruction to increase the value of the top-of-stack register and complete the allocation of storage for local variables. */
	JMP,	/**< Control transfer instructions for unconditional transfers. */
	JPC 	/**< Control transfer instructions for conditional transfers. */
} op_code;

/**
 * @brief Include a set of instructions for arithmetic and relational operations.
 */
typedef enum
{
	OPR_RET,	/**< Stop the foo and return. */
	OPR_NEG,	/**< Monomial operator '-'. */
	OPR_ADD,	/**< Binomial operator '+'. */
	OPR_MIN,	/**< Binomial operator '-'. */
	OPR_MUL,	/**< Binomial operator '*'. */
	OPR_DIV,	/**< Binomial operator '/'. */
	OPR_ODD,	/**< Monomial operator '%2', indicate if the number is an odd number. */
	OPR_EQU,	/**< Binomial operator '='. */
	OPR_NEQ,	/**< Binomial operator '!='. */
	OPR_LES,	/**< Binomial operator '<'. */
	OPR_LEQ,	/**< Binomial operator '<='. */
	OPR_GTR,	/**< Binomial operator '>'. */
	OPR_GEQ		/**< Binomial operator '>='. */
} opr_code;

/**
 * @brief A PL/0 instruction.
 */
typedef struct
{
	op_code func_code;/**< Function code. It should be an enum type of op_code. */
	int level; 		 /**< Nesting depth. This field is used for accessing instructions and call instructions. */
	int address; 	 /**< Displacement address. It should be an enum type of opr_code if func_code is OPR. */
} instruction;

const char* err_msg[] =
{
    "",
    "Found ':=' when expecting '='.",
    "There must be address number to follow '='.",
    "There must be an '=' to follow the identifier.",
    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
    "Missing ',' or ';'.",
    "Incorrect procedure name.",
    "Statement expected.",
    "Follow the statement is an incorrect symbol.",
    "'.' expected.",
    "';' expected.",
    "Undeclared identifier.",
    "Illegal assignment.",
    "':=' expected.",
    "There must be an identifier to follow the 'call'.",
    "A constant or variable can not be called.",
    "'then' expected.",
    "';' or 'end' expected.",
    "'do' expected.",
    "Incorrect symbol.",
    "Relative operators expected.",
    "Procedure identifier can not be in an expression.",
    "Missing ')'.",
    "The symbol can not be followed by a factor.",
    "The symbol can not be as the beginning of an expression.",
    "The number is too great.",
    "",
    "",
    "",
    "",
    "",
    "",
    "There are too many levels."
};

char next_char;
int  next_symbol;
char next_id[IDENTIFIER_MAX_LENGTH + 1];
int  next_num;
int  character_count;
int  line_length;
int  err_count;
int  current_inst_index;
int  current_level = 0;
int  current_table_index = 0;

char line[80];

instruction code[INST_MAX_COUNT];

/**
 * @brief Reserved reserve_word table.
 */
char* reserve_word[RESERVE_WORD_TABLE_MAX_LENGTH + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while"
};

/**
 * @brief The symbol type corresponding to each reserved reserve_word in the reserved reserve_word table.
 */
int reserve_word_symbol[RESERVE_WORD_TABLE_MAX_LENGTH + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE
};

/**
 * @brief Table of some one character symbols.
 */
char reserve_char[REVERSE_CHAR_TABLE_MAX_LENGTH + 1] =
{
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';'
};

/**
 * @brief Table of symbol types for some symbols.
 */
int reserve_char_symbol[REVERSE_CHAR_TABLE_MAX_LENGTH + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON
};

#define MAX_INS 8
/**
 * @brief Table of op_code name string.
 */
char* mnemonic[MAX_INS] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC"
};

/**
 * @brief Identifier table.
 */
typedef struct
{
	char name[IDENTIFIER_MAX_LENGTH + 1];
	int  kind;
	int  value;
} identifier;

identifier id_table[IDENTIFIER_TABLE_MAX_LENGTH];

typedef struct
{
	char  name[IDENTIFIER_MAX_LENGTH + 1];
	int   kind;
	short level;
	short address;
} id_mask;

FILE* infile;

int data_alloc_index;

/**
 * @brief Print error message.
 *
 * @param error_type Error type number. See err_msg.
 */
void print_error(int error_type);

/**
 * @brief Read next character from input stream.
 *
 * @details The func doesn't return the char it reads.
 * The char is stored in next_char.
 */
void getch(void);

/**
 * @brief gets a symbol from input stream.
 *
 * @details The func doesn't return the symbol it reads.
 * The symbol is stored in next_symbol.
 */
void get_symbol(void);

/**
 * @brief Generates (assembles) an instruction.
 * Store the instruction into array code.
 *
 * @param inst_op_code The func_code field of the inst. Should be type of op_code.
 * @param inst_level The level field of the inst.
 * @param inst_address The address field of the inst.
 *      If inst_op_code is OPR, this param should be type of opr_code,
 *      else this param should be an address.
 */
void gen_inst(int inst_op_code, int inst_level, int inst_address);

/**
 * @brief Tests if error occurs and skips all symbols that do not belongs to s1 or s2.
 *
 * @details This function is to ensure next_symbol is in s1.
 *      If next_symbol is not in s1, it will skip all the symbol in s1 or s2
 *      to avoid more errors in this block.
 *
 * @param s1 First symbol_set to be tested.(Usually start symbol set)
 * @param s2 Second symbol_set to be tested.(Usually content symbol set)
 * @param n Error type. Used for printing error message if next_symbol is not in s1.
 */
void test(symbol_set s1, symbol_set s2, int n);

/**
 * @brief Enter object(constant, variable or procedure) into table.
 *
 * @param kind object type, should be type of enum id_type
 */
void enter(int kind);

/**
 * @brief Locates identifier in symbol table.
 *
 * @param id Identifier name string.
 * @return index of the giving id. Return 0 if id is not in symbol table.
 */
int position(char* id);

/**
 * @brief Declare a const identifier.
 */
void const_declaration(void);

/**
 * @brief Declare a var identifier.
 */
void var_declaration(void);

/**
 * @brief Print out generated code in the area [from, to).
 *
 * @param from Print start from this index.
 * @param to Print end to this index(won't print code[to]).
 */
void list_code(int from, int to);

/**
 * @brief Handle a factor.
 *
 * @bug Function expression should be declared before this function.
 * @details A factor includes situations listed below:\n
 * ident,\n number,\n -factor,\n (expression)
 *
 * @param sym_set Factor content symbol set.
 */
void factor(symbol_set sym_set);

/**
 * @brief Handle a term.
 *
 * @details A term is like a * b or a / b, while a and b are factors.
 *
 * @param sym_set Term content symbol set.
 */
void term(symbol_set sym_set);

/**
 * @brief Handle an expression.
 *
 * @details An expression is like a + b or a - b, while a and b are terms.
 *
 * @param sym_set Expression content symbol set.
 */
void expression(symbol_set sym_set);

/**
 * @brief Handle a condition.
 *
 * @Details A condition is like a < b, a > b, ..., while a and b are expressions.
 *
 * @param sym_set Condition content symbol set.
 */
void condition(symbol_set sym_set);

/**
 * @brief Handle a statement.
 *
 * @details The textbook usually uses stmt to represent statements.
 *
 * @param sym_set Statement content symbol set.
 */
void statement(symbol_set sym_set);

/**
 * @brief Handle a block.
 *
 * @details A block is the program body.
 *
 * @param sym_set Statement content symbol set.
 */
void block(symbol_set sym_set);

/**
 * @brief Get the base address of another level on the stack.
 *
 * @param stack The stack itself.
 * @param current_level Usually program-, base-, top_stack-register.
 * @param level_diff Nesting depth.
 * @return The base address of another level.
 */
int base(const int stack[], int current_level, int level_diff);

/**
 * @brief Interprets and executes code.
 */
void interpret();

#endif