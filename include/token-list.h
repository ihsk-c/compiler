/* token-list.h */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXSTRSIZE 1024

/* Token */
#define TNAME 1 /* Name : Alphabet { Alphabet | Digit } */
#define TPROGRAM 2 /* program : Keyword */
#define TVAR 3 /* var : Keyword */
#define TARRAY 4 /* array : Keyword */
#define TOF 5 /* of : Keyword */
#define TBEGIN 6 /* begin : Keyword */
#define TEND 7 /* end : Keyword */
#define TIF 8 /* if : Keyword */
#define TTHEN 9 /* then : Keyword */
#define TELSE 10 /* else : Keyword */
#define TPROCEDURE 11 /* procedure : Keyword */
#define TRETURN 12 /* return : Keyword */
#define TCALL 13 /* call : Keyword */
#define TWHILE 14 /* while : Keyword */
#define TDO 15 /* do : Keyword */
#define TNOT 16 /* not : Keyword */
#define TOR 17 /* or : Keyword */
#define TDIV 18 /* div : Keyword */
#define TAND 19 /* and : Keyword */
#define TCHAR 20 /* char : Keyword */
#define TINTEGER 21 /* integer : Keyword */
#define TBOOLEAN 22 /* boolean : Keyword */
#define TREADLN 23 /* readln : Keyword */
#define TWRITELN 24 /* writeln : Keyword */
#define TTRUE 25 /* true : Keyword */
#define TFALSE 26 /* false : Keyword */
#define TNUMBER 27 /* unsigned integer */
#define TSTRING 28 /* String */
#define TPLUS 29 /* + : symbol */
#define TMINUS 30 /* - : symbol */
#define TSTAR 31 /* * : symbol */
#define TEQUAL 32 /* = : symbol */
#define TNOTEQ 33 /* <> : symbol */
#define TLE 34 /* < : symbol */
#define TLEEQ 35 /* <= : symbol */
#define TGR 36 /* > : symbol */
#define TGREQ 37 /* >= : symbol */
#define TLPAREN 38 /* ( : symbol */
#define TRPAREN 39 /* ) : symbol */
#define TLSQPAREN 40 /* [ : symbol */
#define TRSQPAREN 41 /* ] : symbol */
#define TASSIGN 42 /* := : symbol */
#define TDOT 43 /* . : symbol */
#define TCOMMA 44 /* , : symbol */
#define TCOLON 45 /* : : symbol */
#define TSEMI 46 /* ; : symbol */
#define TREAD 47 /* read : Keyword */
#define TWRITE 48 /* write : Keyword */
#define TBREAK 49 /* break : Keyword */

#define NUMOFTOKEN 49

/* scan.c */

#define KEYWORDSIZE 28

extern struct KEY {
	char * keyword;
	int keytoken;
} key[KEYWORDSIZE];

extern int init_scan(char *filename);
extern int scan(void);
extern int num_attr;
extern char string_attr[MAXSTRSIZE];
extern char *tokenstr[NUMOFTOKEN+1];
extern int get_linenum(void);
extern void end_scan(void);
extern void error(char *mes);
extern int name_key(void);
extern int un_int(void);
extern int str(void);
extern int tle(void);
extern int tgr(void);
extern int colon(void);
extern int enter(void);
extern int comment(void);

/* compiler.c */

#define NORMAL 0
#define ERROR 1

extern int parse_program(void);
extern int parse_block(void);
extern int parse_variable_dc(void);
extern int parse_variable_names(void);
extern int parse_variable_name(void);
extern int parse_type(void);
extern int parse_standard_type(void);
extern int parse_array_type(void);
extern int parse_subprogram_dc(void);
extern int parse_procedure_name(void);
extern int parse_formal_prm(void);
extern int parse_compound_st(void);
extern int parse_statement(void);
extern int parse_condition_st(void);
extern int parse_iteration_st(void);
extern int parse_exit_st(void);
extern int parse_call_st(void);
extern int parse_expressions(void);
extern int parse_return_st(void);
extern int parse_assignment_st(void);
extern int parse_left_part(void);
extern int parse_variable(void);
extern int parse_expression(void);
extern int parse_simple_exp(void);
extern int parse_term(void);
extern int parse_factor(void);
extern int parse_constant(void);
extern int parse_multiple_op(void);
extern int parse_additive_op(void);
extern int parse_relational_op(void);
extern int parse_input_st(void);
extern int parse_output_st(void);
extern int parse_output_format(void);
extern int parse_empty_st(void);
extern void outlib(void);
extern void add_mem(char*);

/* id_list.c */

#define TPINT 2
#define TPCHAR 3
#define TPBOOL 4
#define TPARRAY 5
#define TPARRAYINT 6
#define TPARRAYCHAR 7
#define TPARRAYBOOL 8
#define TPPROC 9

struct TYPE {
	int ttype; /* TPINT TPCHAR TPBOOL TPARRAY TPARRAYINT TPARRAYCHAR
TPARRAYBOOL TPPROC */
	int arraysize; /* size of array, if TPARRAY */
	struct TYPE *etp; /* pointer to element type if TPARRAY */
	struct TYPE *paratp; /* pointer to parameter’s type list if ttype is TPPROC
	 */
};

struct LINE {
	int reflinenum;
	struct LINE *nextlinep;
};

struct ID {
	char *name;
	char *procname; /* procedure name within which this name is defined.
NULL if global name */
	struct TYPE *itp;
	int ispara; /* 2:formal parameter(registered), 1:formal parameter, 0:else(variable) */
	int deflinenum;
	struct LINE *irefp;
	struct ID *nextp;
};

extern struct ID *globalidroot, *localidroot, *allidroot; /* Pointers to
root of global & local symbol tables */

extern void init_idtab(void);
extern struct ID *search_idtab(struct ID*, char*);
extern int register_name(char*, char*, int);
extern int register_type(struct ID*, char*, int, int);
extern int register_paratype(char*);
extern int register_rline(struct ID*, char*);
extern int num_prm(char*);
extern int check_prm(char*, int*);
extern void collect_id(struct ID*);
extern void release_id(void);
extern void release_type(struct TYPE*);
