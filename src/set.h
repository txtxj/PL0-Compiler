#ifndef SET_H
#define SET_H

/**
 * @brief Use list to store the set.
 *      The elements of the set are in ascending order.
 */
typedef struct set_node
{
	int elem;
	struct set_node* next;
} set_node, *symbol_set;

static symbol_set phi;						/**< null */
static symbol_set declare_begin_symbol_set;	/**< declare begin(const, var, procedure) */
static symbol_set state_begin_symbol_set;	/**< statement begin(begin, call, if, while, id) */
static symbol_set factor_begin_symbol_set;	/**< factor begin(id, number, lparen, minus) */
static symbol_set relation_symbol_set;		/**< relation begin(>, <, ...) */

/**
 * @brief Create a symbol_set including params.
 *      Warn that the data list must end with SYM_NULL.
 *
 * @param data A series of symbols have enum type sym_type.
 * @return Generated symbol set
 */
symbol_set create_set(int data, ...);

/**
 * @brief Delete and free the symbol set.
 *
 * @param s Symbol set to delete.
 */
void destroy_set(symbol_set s);

/**
 * @brief Unite two symbol set into one.
 *      The two param won't change after calling this function.
 *
 * @param s1 First set.
 * @param s2 Second Set.
 * @return The united set.
 */
symbol_set unite_set(symbol_set s1, symbol_set s2);

/**
 * @brief Check if elem is in symbol set s.
 *
 * @param elem Element to be searched.
 * @param s Symbol set to be searched in.
 * @return 1 if elem in s, else 0.
 */
int in_set(int elem, symbol_set s);

#endif
