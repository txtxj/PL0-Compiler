#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pl0.h"
#include "set.h"

void print_error(int error_type)
{
	int i;

	printf("      ");
	for (i = 1; i <= character_count - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", error_type, err_msg[error_type]);
	err_count++;
}

void getch(void)
{
	if (character_count == line_length)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		line_length = character_count = 0;
		printf("%5d  ", current_inst_index);
		while ( (!feof(infile)) && ((next_char = (char)getc(infile)) != '\n'))
		{
			printf("%c", next_char);
			line[++line_length] = next_char;
		}
		printf("\n");
		line[++line_length] = ' ';
	}
	next_char = line[++character_count];
}

void look_ahead(void)
{
	last_symbol = next_symbol;
	last_num = next_num;
	strcpy(last_id, next_id);
	get_symbol();
	roll_back_flag = 1;
}

void accept_look_ahead()
{
	roll_back_flag = 0;
	get_symbol();
}

void roll_back(void)
{
	char id[IDENTIFIER_MAX_LENGTH + 1];
	int temp = next_symbol;
	next_symbol = last_symbol;
	last_symbol = temp;

	temp = next_num;
	next_num = last_num;
	last_num = temp;

	strcpy(id, last_id);
	strcpy(last_id, next_id);
	strcpy(next_id, id);
}

void inst_pop(void)
{
	current_inst_index--;
}

void get_symbol(void)
{
	int i, k;
	char id[IDENTIFIER_MAX_LENGTH + 1];

	if (roll_back_flag)
	{
		next_symbol = last_symbol;
		next_num = last_num;
		strcpy(next_id, last_id);
		roll_back_flag = 0;
		return;
	}

	while (next_char == ' ' || next_char == '\t')
		getch();

	if (isalpha(next_char) || next_char == '_')  /* symbol is a reserved word or an identifier. */
	{
		k = 0;
		do
		{
			if (k < IDENTIFIER_MAX_LENGTH)
				id[k++] = next_char;
			getch();
		} while (isalpha(next_char) || isdigit(next_char) || next_char == '_');
		id[k] = '\0';
		strcpy(next_id, id);
		reserve_word[0] = next_id;
		i = RESERVE_WORD_TABLE_MAX_LENGTH;
		while (strcmp(next_id, reserve_word[i--]) != 0);
		if (++i)
			next_symbol = reserve_word_symbol[i];  /* symbol is a reserve_word. */
		else
			next_symbol = SYM_IDENTIFIER;  /* symbol is an identifier. */
	}
	else if (isdigit(next_char))  /* symbol is a number. */
	{
		k = next_num = 0;
		next_symbol = SYM_NUMBER;
		do
		{
			next_num = next_num * 10 + next_char - '0';
			k++;
			getch();
		}
		while (isdigit(next_char));
		if (k > DIGIT_MAX_LENGTH)
			print_error(25);  /* The number is too great. */
	}
	else if (next_char == ':')
	{
		getch();
		if (next_char == '=')
		{
			next_symbol = SYM_BECOMES;
			getch();
		}
		else if (next_char == '(')
		{
			next_symbol = SYM_LOOP_INIT;
			getch();
		}
		else
		{
			next_symbol = SYM_NULL;
		}
	}
	else if (next_char == '>')
	{
		getch();
		if (next_char == '=')
		{
			next_symbol = SYM_GEQ;
			getch();
		}
		else
		{
			next_symbol = SYM_GTR;
		}
	}
	else if (next_char == '<')
	{
		getch();
		if (next_char == '=')
		{
			next_symbol = SYM_LEQ;
			getch();
		}
		else if (next_char == '>')
		{
			next_symbol = SYM_NEQ;
			getch();
		}
		else
		{
			next_symbol = SYM_LES;
		}
	}
	else
	{
		i = REVERSE_CHAR_TABLE_MAX_LENGTH;
		reserve_char[0] = next_char;
		while (reserve_char[i--] != next_char);
		if (++i)
		{
			next_symbol = reserve_char_symbol[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
}

void gen_inst(int inst_op_code, int inst_level, int inst_address)
{
	if (current_inst_index > INST_MAX_COUNT)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[current_inst_index].func_code = inst_op_code;
	code[current_inst_index].level = inst_level;
	code[current_inst_index].address = inst_address;
	current_inst_index++;
}

void test(symbol_set s1, symbol_set s2, int n)
{
	symbol_set s;

	if (!in_set(next_symbol, s1))
	{
		print_error(n);
		s = unite_set(s1, s2);
		while(!in_set(next_symbol, s))
			get_symbol();
		destroy_set(s);
	}
}

void enter(int kind)
{
	id_mask* mask;
	int array_size = 1;
	identifier *array_dimension;

	current_table_index++;
	strcpy(id_table[current_table_index].name, next_id);
	id_table[current_table_index].kind = kind;
	switch (kind)
	{
		case ID_CONSTANT:
			if (next_num > MAX_ADDRESS)
			{
				print_error(25); // The number is too great.
				next_num = 0;
			}
			id_table[current_table_index].value = next_num;
			break;
		case ID_VARIABLE:
			mask = (id_mask*) &id_table[current_table_index];
			mask->level = (short)current_level;
			mask->address = (short)data_alloc_index++;
			break;
		case ID_PROCEDURE:
			mask = (id_mask*) &id_table[current_table_index];
			mask->level = (short)current_level;
			break;
		case ID_ARRAY:
			get_symbol();
			mask = (id_mask*) &id_table[current_table_index];
			mask->level = (short)current_level;
			mask->address = (short)data_alloc_index;
			if (next_symbol != SYM_LBRACKET)
				print_error(31);
			current_table_index++;
			array_dimension = &id_table[current_table_index];
			array_dimension->kind = ID_CONSTANT;
			array_dimension->value = 0;
			strcpy(array_dimension->name, "0");
			while (next_symbol == SYM_LBRACKET)
			{
				get_symbol();
				if (next_symbol == SYM_NUMBER)
				{
					if (next_num <= 0)
						print_error(34);
					array_size *= next_num;
					current_table_index++;
					id_table[current_table_index].kind = ID_CONSTANT;
					id_table[current_table_index].value = next_num;
					strcpy(id_table[current_table_index].name, "0");
					array_dimension->value++;
					get_symbol();
				}
				else
					print_error(37);
				if (next_symbol == SYM_RBRACKET)
					get_symbol();
				else
					print_error(33);
			}
			data_alloc_index += array_size;
			break;
		default:
			break;
	}
}

int position(char* id)
{
	int i;
	strcpy(id_table[0].name, id);
	i = current_table_index + 1;
	while (strcmp(id_table[--i].name, id) != 0);
	return i;
}

void const_declaration(void)
{
	if (next_symbol == SYM_IDENTIFIER)
	{
		get_symbol();
		if (next_symbol == SYM_EQU || next_symbol == SYM_BECOMES)
		{
			if (next_symbol == SYM_BECOMES)
				print_error(1); // Found ':=' when expecting '='.
			get_symbol();
			if (next_symbol == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				get_symbol();
			}
			else
			{
				print_error(2); // There must be address number to follow '='.
			}
		}
		else
		{
			print_error(3); // There must be an '=' to follow the identifier.
		}
	} else print_error(4);
}

void var_declaration(void)
{
	if (next_symbol == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);
		get_symbol();
	}
	else
	{
		print_error(4);
	}
}

void array_declaration(void)
{
	if (next_symbol == SYM_IDENTIFIER)
	{
		enter(ID_ARRAY);
	}
	else
	{
		print_error(4);
	}
}

void list_code(int from, int to)
{
	int i;
	
	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].func_code], code[i].level, code[i].address);
	}
	printf("\n");
}

void array_element(symbol_set sym_set, id_mask* mk)
{
	symbol_set set, set1;
	int i, dimension = ((identifier*)(mk + 1))->value;

	gen_inst(LEA, current_level - mk->level, mk->address);
	set1 = create_set(SYM_RBRACKET, SYM_NULL);
	set = unite_set(sym_set, set1);

	gen_inst(LIT, 0, 0);
	for (i = 1; i < dimension; i++)
	{
		if (next_symbol == SYM_LBRACKET)
		{
			get_symbol();
			if (! in_set(next_symbol, factor_begin_symbol_set))
				print_error(36);
			expression(set);
			gen_inst(OPR, 0, OPR_ADD);
			gen_inst(LIT, 0, ((identifier*)(mk + i + 2))->value);
			gen_inst(OPR, 0, OPR_MUL);
		}
		else
			print_error(31);
		if (next_symbol == SYM_RBRACKET)
			get_symbol();
		else
			print_error(33);
	}
	if (next_symbol == SYM_LBRACKET)
	{
		get_symbol();
		if (! in_set(next_symbol, factor_begin_symbol_set))
			print_error(36);
		expression(set);
		gen_inst(OPR, 0, OPR_ADD);
		gen_inst(OPR, 0, OPR_ADD);
	}
	else
		print_error(31);
	if (next_symbol == SYM_RBRACKET)
		get_symbol();
	else
		print_error(33);
	destroy_set(set);
	destroy_set(set1);
}

void factor(symbol_set sym_set)
{
	int i;
	symbol_set set, set1;
	id_mask* mk;

	if (factor_in_stack_flag)
	{
		factor_in_stack_flag = 0;
		return;
	}
	test(factor_begin_symbol_set, sym_set, 24);

	if (in_set(next_symbol, factor_begin_symbol_set))
	{
		if (next_symbol == SYM_IDENTIFIER)
		{
			if ((i = position(next_id)) == 0)
			{
				get_symbol();
				print_error(11); // Undeclared identifier.
			}
			else
			{
				get_symbol();
				switch (id_table[i].kind)
				{
					case ID_CONSTANT:
						gen_inst(LIT, 0, id_table[i].value);
						break;
					case ID_VARIABLE:
						mk = (id_mask*) &id_table[i];
							gen_inst(LOD, current_level - mk->level, mk->address);
						break;
					case ID_PROCEDURE:
						print_error(21); // Procedure identifier can not be in an expression.
						break;
					case ID_ARRAY:
						array_element(sym_set, (id_mask*) &id_table[i]);
						gen_inst(LOA, 0, 0);
						break;
				}
			}
		}
		else if (next_symbol == SYM_NUMBER)
		{
			if (next_num > MAX_ADDRESS)
			{
				print_error(25); // The number is too great.
				next_num = 0;
			}
			gen_inst(LIT, 0, next_num);
			get_symbol();
		}
		else if (next_symbol == SYM_LPAREN)
		{
			get_symbol();
			set = unite_set(create_set(SYM_RPAREN, SYM_NULL), sym_set);
			assign_expression(set);
			destroy_set(set);
			if (next_symbol == SYM_RPAREN)
			{
				get_symbol();
			}
			else
			{
				print_error(22);
			}
		}
		else if(next_symbol == SYM_MINUS)
		{
			get_symbol();
			factor(sym_set);
			gen_inst(OPR, 0, OPR_NEG);
		}
		else if (next_symbol == SYM_SET_JUMP)
		{
			get_symbol();
			if (next_symbol == SYM_LPAREN)
				get_symbol();
			else
				print_error(27);
			set1 = create_set(SYM_RPAREN, SYM_NULL);
			set = unite_set(sym_set, set1);
			if (in_set(next_symbol, factor_begin_symbol_set))
				expression(set);
			else
				print_error(26);
			if (next_symbol == SYM_RPAREN)
				get_symbol();
			else
				print_error(23);
			gen_inst(SJP, 0, 0);
			destroy_set(set1);
			destroy_set(set);
		}
		set = create_set(SYM_LPAREN, SYM_NULL);
		test(sym_set, set, 23);
		destroy_set(set);
	}
}

void term(symbol_set sym_set)
{
	int mul_op;
	symbol_set set;
	
	set = unite_set(sym_set, create_set(SYM_TIMES, SYM_SLASH, SYM_NULL));
	factor(set);
	while (next_symbol == SYM_TIMES || next_symbol == SYM_SLASH)
	{
		mul_op = next_symbol;
		get_symbol();
		factor(set);
		if (mul_op == SYM_TIMES)
		{
			gen_inst(OPR, 0, OPR_MUL);
		}
		else
		{
			gen_inst(OPR, 0, OPR_DIV);
		}
	}
	destroy_set(set);
}

void expression(symbol_set sym_set)
{
	int add_op;
	symbol_set set;

	set = unite_set(sym_set, create_set(SYM_PLUS, SYM_MINUS, SYM_NULL));
	
	term(set);
	while (next_symbol == SYM_PLUS || next_symbol == SYM_MINUS)
	{
		add_op = next_symbol;
		get_symbol();
		term(set);
		if (add_op == SYM_PLUS)
		{
			gen_inst(OPR, 0, OPR_ADD);
		}
		else
		{
			gen_inst(OPR, 0, OPR_MIN);
		}
	}
	destroy_set(set);
}

void assign_expression(symbol_set sym_set)
{
	int i;
	id_mask* mk;

	/* Read a factor. */
	test(factor_begin_symbol_set, sym_set, 24);

	if (in_set(next_symbol, factor_begin_symbol_set))
	{
		if (next_symbol == SYM_IDENTIFIER)
		{
			if ((i = position(next_id)) == 0)
				print_error(11);
			else
			{
				mk = (id_mask*) &id_table[i];
				if (mk->kind == ID_VARIABLE || mk->kind == ID_CONSTANT)
				{
					look_ahead();
					if (next_symbol == SYM_BECOMES)
					{
						accept_look_ahead();
						switch (id_table[i].kind)
						{
							case ID_CONSTANT:
								print_error(12);
								break;
							case ID_VARIABLE:
								assign_expression(sym_set);
								gen_inst(STO, current_level - mk->level, mk->address);
								gen_inst(LOD, current_level - mk->level, mk->address);
								break;
							default:
								print_error(30);
						}
					}
					else
					{
						roll_back();
						expression(sym_set);
					}
				}
				else if (mk->kind == ID_ARRAY)
				{
					get_symbol();
					array_element(sym_set, mk);
					if (next_symbol == SYM_BECOMES)
					{
						get_symbol();
						gen_inst(CPY, 0, 0);
						assign_expression(sym_set);
						gen_inst(STA, 0, 0);
						gen_inst(LOA, 0, 0);
					}
					else if (next_symbol == SYM_PLUS || next_symbol == SYM_MINUS || next_symbol == SYM_TIMES || next_symbol == SYM_SLASH)
					{
						gen_inst(LOA, 0, 0);
						roll_back_factor();
						expression(sym_set);
					}
				}
			}
		}
		else if (in_set(next_symbol, factor_begin_symbol_set))
		{
			expression(sym_set);
		}
		test(sym_set, create_set(SYM_LPAREN, SYM_NULL), 23);
	}
}

void roll_back_factor(void)
{
	factor_in_stack_flag = 1;
}

void condition(symbol_set sym_set)
{
	int relation_op;
	symbol_set set;

	if (next_symbol == SYM_ODD)
	{
		get_symbol();
		expression(sym_set);
		gen_inst(OPR, 0, 6);
	}
	else
	{
		set = unite_set(relation_symbol_set, sym_set);
		expression(set);
		destroy_set(set);
		if (!in_set(next_symbol, relation_symbol_set))
		{
			print_error(20);
		}
		else
		{
			relation_op = next_symbol;
			get_symbol();
			expression(sym_set);
			switch (relation_op)
			{
				case SYM_EQU:
					gen_inst(OPR, 0, OPR_EQU);
					break;
				case SYM_NEQ:
					gen_inst(OPR, 0, OPR_NEQ);
					break;
				case SYM_LES:
					gen_inst(OPR, 0, OPR_LES);
					break;
				case SYM_GEQ:
					gen_inst(OPR, 0, OPR_GEQ);
					break;
				case SYM_GTR:
					gen_inst(OPR, 0, OPR_GTR);
					break;
				case SYM_LEQ:
					gen_inst(OPR, 0, OPR_LEQ);
					break;
				default:
					break;
			}
		}
	}
}

void statement(symbol_set sym_set)
{
	int i, cx1, cx2, cx3, cx4, loop_var_flag = 0;
	symbol_set set1, set;

	if (next_symbol == SYM_IDENTIFIER)
	{
		id_mask* mk;
		if (! (i = position(next_id)))
		{
			print_error(11);
		}
		mk = (id_mask*) &id_table[i];
		if (i && (mk->kind == ID_VARIABLE || mk->kind == ID_ARRAY))
		{
			assign_expression(sym_set);
			inst_pop();
		}
		else
		{
			get_symbol();
			print_error(12);
		}
	}
	else if (next_symbol == SYM_CALL)
	{
		get_symbol();
		if (next_symbol != SYM_IDENTIFIER)
		{
			print_error(14);
		}
		else
		{
			if (! (i = position(next_id)))
			{
				print_error(11);
			}
			else if (id_table[i].kind == ID_PROCEDURE)
			{
				id_mask* mk;
				mk = (id_mask*) &id_table[i];
				gen_inst(CAL, current_level - mk->level, mk->address);
			}
			else
			{
				print_error(15);
			}
			get_symbol();
		}
	} 
	else if (next_symbol == SYM_IF)
	{
		get_symbol();
		set1 = create_set(SYM_THEN, SYM_NULL);
		set = unite_set(set1, sym_set);
		condition(set);
		destroy_set(set1);
		destroy_set(set);
		if (next_symbol == SYM_THEN)
			get_symbol();
		else
			print_error(16);
		cx1 = current_inst_index;
		gen_inst(JPC, 0, 0);
		statement(sym_set);
		if (next_symbol == SYM_SEMICOLON)
		{
			look_ahead();
			if (next_symbol == SYM_ELSE)
			{
				accept_look_ahead();
				cx2 = current_inst_index;
				gen_inst(JMP, 0, 0);
				code[cx1].address = current_inst_index;
				statement(sym_set);
				code[cx2].address = current_inst_index;
			}
			else
			{
				roll_back();
				code[cx1].address = current_inst_index;
			}
		}
	}
	else if (next_symbol == SYM_BEGIN)
	{
		get_symbol();
		set1 = create_set(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = unite_set(set1, sym_set);
		statement(set);
		while (next_symbol == SYM_SEMICOLON || in_set(next_symbol, state_begin_symbol_set))
		{
			if (next_symbol == SYM_SEMICOLON)
			{
				get_symbol();
			}
			else
			{
				print_error(10);
			}
			statement(set);
		}
		destroy_set(set1);
		destroy_set(set);
		if (next_symbol == SYM_END)
		{
			get_symbol();
		}
		else
		{
			print_error(17);
		}
	}
	else if (next_symbol == SYM_WHILE)
	{
		cx1 = current_inst_index;
		get_symbol();
		set1 = create_set(SYM_DO, SYM_NULL);
		set = unite_set(set1, sym_set);
		condition(set);
		destroy_set(set1);
		destroy_set(set);
		cx2 = current_inst_index;
		gen_inst(JPC, 0, 0);
		if (next_symbol == SYM_DO)
		{
			get_symbol();
		}
		else
		{
			print_error(18);
		}
		statement(sym_set);
		gen_inst(JMP, 0, cx1);
		code[cx2].address = current_inst_index;
	}
	else if (next_symbol == SYM_PRINT)
	{
		get_symbol();
		if (next_symbol == SYM_LPAREN)
			get_symbol();
		else
			print_error(27);
		set1 = create_set(SYM_COMMA, SYM_RPAREN, SYM_NULL);
		set = unite_set(sym_set, set1);
		while (next_symbol != SYM_RPAREN)
		{
			if (in_set(next_symbol, factor_begin_symbol_set))
			{
				expression(set);
			}
			else
				print_error(26);
			gen_inst(PRT, 0, 0);
			if (next_symbol == SYM_COMMA)
				get_symbol();
			else if (next_symbol != SYM_RPAREN)
				print_error(23);
		}
		destroy_set(set1);
		destroy_set(set);
		gen_inst(PRT, 0, 1);
		get_symbol();
	}
	else if (next_symbol == SYM_FOR)
	{
		get_symbol();
		if (next_symbol == SYM_LPAREN)
			get_symbol();
		else
			print_error(27);
		if (next_symbol == SYM_VAR)
		{
			get_symbol();
			if (next_symbol == SYM_IDENTIFIER)
			{
				enter(ID_VARIABLE);
				loop_var_flag = 1;
				gen_inst(INT, 0, 1);
			}
			else
				print_error(4);
		}
		else if (next_symbol != SYM_IDENTIFIER)
			print_error(28);
		if (next_symbol == SYM_IDENTIFIER)
		{
			if (! (i = position(next_id)))
			{
				print_error(11);
			}
			else if (id_table[i].kind != ID_VARIABLE)
			{
				print_error(12);
				i = 0;
			}
			get_symbol();
			if (next_symbol != SYM_LOOP_INIT)
			{
				print_error(29);
			}
			get_symbol(); /* Match ':(' */

			/* Match low */
			set1 = create_set(SYM_COMMA, SYM_RPAREN, SYM_NULL);
			set = unite_set(sym_set, set1);
			if (in_set(next_symbol, factor_begin_symbol_set))
				expression(set);
			else
				print_error(26);
			id_mask* mk_i = (id_mask*) &id_table[i];
			gen_inst(STO, current_level - mk_i->level, mk_i->address);
			if (next_symbol == SYM_COMMA)
				get_symbol();
			else
				print_error(38);

			/* Match high */
			cx1 = current_inst_index;
			if (in_set(next_symbol, factor_begin_symbol_set))
				expression(set);
			else
				print_error(26);

			gen_inst(LOD, current_level - mk_i->level, mk_i->address);
			gen_inst(OPR, 0, OPR_LEQ);
			cx2 = current_inst_index;
			gen_inst(JPC, 0, 0);
			cx3 = current_inst_index;
			gen_inst(JMP, 0, 0);

			cx4 = current_inst_index;
			if (next_symbol == SYM_COMMA)
			{
				get_symbol();
				expression(set);
			}
			else
				gen_inst(LIT, 0, 1);

			/* Match two ')' */
			if (next_symbol == SYM_RPAREN)
				get_symbol();
			else
				print_error(26);
			if (next_symbol == SYM_RPAREN)
				get_symbol();
			else
				print_error(26);

			gen_inst(LOD, current_level - mk_i->level, mk_i->address);
			gen_inst(OPR, 0, OPR_ADD);
			gen_inst(STO, current_level - mk_i->level, mk_i->address);
			gen_inst(JMP, 0, cx1);

			if (! in_set(next_symbol, state_begin_symbol_set))
			{
				print_error(7);
			}
			code[cx2].address = current_inst_index;
			statement(sym_set);

			gen_inst(JMP, 0, cx4);
			code[cx3].address = current_inst_index;

			destroy_set(set1);
			destroy_set(set);
		}
		if (loop_var_flag == 1)
		{
			current_table_index--;
			gen_inst(INT, 0, -1);
		}
	}
	else if (next_symbol == SYM_LONG_JUMP)
	{
		get_symbol();
		if (next_symbol == SYM_LPAREN)
			get_symbol();
		else
			print_error(27);
		set1 = create_set(SYM_COMMA, SYM_NULL);
		set = unite_set(sym_set, set1);
		if (in_set(next_symbol, factor_begin_symbol_set))
			expression(set);
		else
			print_error(26);
		if (next_symbol == SYM_COMMA)
			get_symbol();
		else
			print_error(38);
		destroy_set(set1);
		destroy_set(set);
		set1 = create_set(SYM_RPAREN, SYM_NULL);
		set = unite_set(sym_set, set1);
		if (in_set(next_symbol, factor_begin_symbol_set))
			expression(set);
		else
			print_error(26);
		gen_inst(LJP, 0, 0);
		if (next_symbol == SYM_RPAREN)
			get_symbol();
		else
			print_error(23);
		destroy_set(set1);
		destroy_set(set);
	}
	test(sym_set, phi, 19);
}

void block(symbol_set sym_set)
{
	int code_index, i, j;
	id_mask* mk;
	int block_data_alloc_index;
	int block_table_index;
	int saved_table_index;
	symbol_set set1, set;

	data_alloc_index = 3;
	block_data_alloc_index = data_alloc_index;
	block_table_index = current_table_index;
	mk = (id_mask*) &id_table[current_table_index];
	mk->address = (short)current_inst_index;
	gen_inst(JMP, 0, 0);
	if (current_level > MAX_LEVEL)
	{
		print_error(32); // There are too many levels.
	}
	do
	{
		if (next_symbol == SYM_CONST)
		{
			get_symbol();
			do
			{
				const_declaration();
				while (next_symbol == SYM_COMMA)
				{
					get_symbol();
					const_declaration();
				}
				if (next_symbol == SYM_SEMICOLON)
				{
					get_symbol();
				}
				else
				{
					print_error(5); // Missing ',' or ';'.
				}
			}
			while (next_symbol == SYM_IDENTIFIER);
		}

		if (next_symbol == SYM_VAR)
		{
			get_symbol();
			do
			{
				var_declaration();
				while (next_symbol == SYM_COMMA)
				{
					get_symbol();
					var_declaration();
				}
				if (next_symbol == SYM_SEMICOLON)
				{
					get_symbol();
				}
				else
				{
					print_error(5); // Missing ',' or ';'.
				}
			}
			while (next_symbol == SYM_IDENTIFIER);
		}

		if (next_symbol == SYM_ARRAY)
		{
			get_symbol();
			do
			{
				array_declaration();
				while (next_symbol == SYM_COMMA)
				{
					get_symbol();
					array_declaration();
				}
				if (next_symbol == SYM_SEMICOLON)
				{
					get_symbol();
				}
				else
				{
					print_error(5); // Missing ',' or ';'.
				}
			}
			while (next_symbol == SYM_IDENTIFIER);
		}

		block_data_alloc_index = data_alloc_index; // Save data_alloc_index before handling procedure call!
		while (next_symbol == SYM_PROCEDURE)
		{
			get_symbol();
			if (next_symbol == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				get_symbol();
			}
			else
			{
				print_error(4);
			}


			if (next_symbol == SYM_SEMICOLON)
			{
				get_symbol();
			}
			else
			{
				print_error(5); // Missing ',' or ';'.
			}

			current_level++;
			saved_table_index = current_table_index;
			set1 = create_set(SYM_SEMICOLON, SYM_NULL);
			set = unite_set(set1, sym_set);
			block(set);
			destroy_set(set1);
			destroy_set(set);
			current_table_index = saved_table_index;
			current_level--;

			if (next_symbol == SYM_SEMICOLON)
			{
				get_symbol();
				set1 = create_set(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = unite_set(state_begin_symbol_set, set1);
				test(set, sym_set, 6);
				destroy_set(set1);
				destroy_set(set);
			}
			else
			{
				print_error(5); // Missing ',' or ';'.
			}
		}
		data_alloc_index = block_data_alloc_index; // Restore data_alloc_index after handling procedure call!
		set1 = create_set(SYM_IDENTIFIER, SYM_NULL);
		set = unite_set(state_begin_symbol_set, set1);
		test(set, declare_begin_symbol_set, 7);
		destroy_set(set1);
		destroy_set(set);
	}
	while (in_set(next_symbol, declare_begin_symbol_set));

	code[mk->address].address = current_inst_index;
	mk->address = (short)current_inst_index;
	code_index = current_inst_index;
	gen_inst(INT, 0, block_data_alloc_index);
	set1 = create_set(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = unite_set(set1, sym_set);
	statement(set);
	destroy_set(set1);
	destroy_set(set);
	gen_inst(OPR, 0, OPR_RET); // return
	test(sym_set, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	list_code(code_index, current_inst_index);
}

int base(const int stack[], int now_level, int level_diff)
{
	int b = now_level;
	
	while (level_diff--)
		b = stack[b];
	return b;
}

envir_buf create_environment(int index, int pc, int stack[STACK_SIZE], int top, int b)
{
	int i;

	envir_buf en = malloc(sizeof (struct environment));
	en->index = index;
	en->b = b;
	en->pc = pc;
	en->top = top;
	en->next = NULL;
	for (i = 0; i <= top; i++)
		en->stack[i] = stack[i];
	return en;
}

void save_environment(envir_buf en)
{
	envir_buf p, pre;

	if (envir == NULL)
		envir = en;
	else if (envir->index > en->index)
	{
		en->next = envir;
		envir = en;
	}
	else if (envir->index == en->index)
	{
		en->next = envir->next;
		free(envir);
		envir = en;
	}
	else
	{
		pre = envir;
		for (p = envir->next; p != NULL; p = p->next)
		{
			if (p->index == en->index)
			{
				en->next = p->next;
				pre->next = en;
				free(p);
				return;
			}
			else if (p->index > en->index)
			{
				pre->next = en;
				en->next = p;
				return;
			}
		}
		pre->next = en;
	}
}

envir_buf load_environment(int index)
{
	envir_buf p;

	for (p = envir; p != NULL; p = p->next)
		if (p->index == index)
			return p;

	return NULL;
}

void interpret()
{
	int pc;        			/**< program counter */
	int stack[STACK_SIZE];
	int top;       			/**< top of stack */
	int b;         			/**< program, base, and top-stack register */
	instruction i; 			/**< instruction register */

	int j;
	envir_buf en;			/**< for load environment */

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.func_code)
		{
			case LIT:
				stack[++top] = i.address;
				break;
			case OPR:
				switch (i.address) // operator
				{
				case OPR_RET:
					top = b - 1;
					pc = stack[top + 3];
					b = stack[top + 2];
					break;
				case OPR_NEG:
					stack[top] = -stack[top];
					break;
				case OPR_ADD:
					top--;
					stack[top] += stack[top + 1];
					break;
				case OPR_MIN:
					top--;
					stack[top] -= stack[top + 1];
					break;
				case OPR_MUL:
					top--;
					stack[top] *= stack[top + 1];
					break;
				case OPR_DIV:
					top--;
					if (stack[top + 1] == 0)
					{
						fprintf(stderr, "Runtime Error: Divided by zero.\n");
						fprintf(stderr, "Program terminated.\n");
						continue;
					}
					stack[top] /= stack[top + 1];
					break;
				case OPR_ODD:
					stack[top] %= 2;
					break;
				case OPR_EQU:
					top--;
					stack[top] = stack[top] == stack[top + 1];
					break;
				case OPR_NEQ:
					top--;
					stack[top] = stack[top] != stack[top + 1];
					break;
				case OPR_LES:
					top--;
					stack[top] = stack[top] < stack[top + 1];
					break;
				case OPR_GEQ:
					top--;
					stack[top] = stack[top] >= stack[top + 1];
					break;
				case OPR_GTR:
					top--;
					stack[top] = stack[top] > stack[top + 1];
					break;
				case OPR_LEQ:
					top--;
					stack[top] = stack[top] <= stack[top + 1];
					break;
				}
				break;
			case LOD:
				stack[++top] = stack[base(stack, b, i.level) + i.address];
				break;
			case STO:
				stack[base(stack, b, i.level) + i.address] = stack[top];
//				printf("Assign: %d\n", stack[top]);
				top--;
				break;
			case CAL:
				stack[top + 1] = base(stack, b, i.level);
				// generate new block mark
				stack[top + 2] = b;
				stack[top + 3] = pc;
				b = top + 1;
				pc = i.address;
				break;
			case INT:
				top += i.address;
				break;
			case JMP:
				pc = i.address;
				break;
			case JPC:
				if (stack[top] == 0)
					pc = i.address;
				top--;
				break;
			case PRT:
				if (i.address == 0)
					printf("%d ", stack[top--]);
				else
					printf("\n");
				break;
			case LOA:
				stack[top] = stack[stack[top]];
				break;
			case STA:
				stack[stack[top - 1]] = stack[top];
				top -= 2;
				break;
			case LEA:
				stack[++top] = base(stack, b, i.level) + i.address;
				break;
			case CPY:
				top++;
				stack[top] = stack[top - 1];
				break;
			case SJP:
				top--;
				save_environment(create_environment(
						stack[top + 1], pc, stack, top, b
						));
				break;
			case LJP:
				en = load_environment(stack[top - 1]);
				en->top++;
				en->stack[en->top] = stack[top];
				pc = en->pc;
				top = en->top;
				b = en->b;
				for (j = 0; j <= en->top; j++)
					stack[j] = en->stack[j];
				break;
		}
	}
	while (pc);

	printf("End executing PL/0 program.\n");
}

int main()
{
	FILE* bin_output;
	char s[80];
	int i;
	symbol_set set, set1, set2;

	printf("Please input source file name:\n");
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = create_set(SYM_NULL);
	relation_symbol_set = create_set(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
	
	// create begin symbol sets
	declare_begin_symbol_set = create_set(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	state_begin_symbol_set = create_set(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_IDENTIFIER, SYM_PRINT, SYM_FOR, SYM_NULL);
	factor_begin_symbol_set = create_set(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_SET_JUMP, SYM_NULL);

	err_count = character_count = current_inst_index = line_length = 0;
	next_char = ' ';

	get_symbol();

	set1 = create_set(SYM_PERIOD, SYM_NULL);
	set2 = unite_set(declare_begin_symbol_set, state_begin_symbol_set);
	set = unite_set(set1, set2);
	block(set);
	destroy_set(set1);
	destroy_set(set2);
	destroy_set(set);
	destroy_set(phi);
	destroy_set(relation_symbol_set);
	destroy_set(declare_begin_symbol_set);
	destroy_set(state_begin_symbol_set);
	destroy_set(factor_begin_symbol_set);

	if (next_symbol != SYM_PERIOD)
		print_error(9);
	if (err_count == 0)
	{
		bin_output = fopen("bin.txt", "w");
		for (i = 0; i < current_inst_index; i++)
			fwrite(&code[i], sizeof(instruction), 1, bin_output);
		fclose(bin_output);
	}
	if (err_count == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err_count);
	list_code(0, current_inst_index);
	return 0;
}