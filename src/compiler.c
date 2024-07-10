#include "token-list.h"

int token; /* store scan() */
int lupe=0; /* lupe>0:in while    lupe=0:out of while */
char procname[MAXSTRSIZE]; /* register procedure_name in parse_subprogram_dc */
char procesname[MAXSTRSIZE]; /* register procedure_name when parse_call_st */
int ispr_flag=0, dc_flag=0, subpro_flag=0, subpro_flag2=0; /*
    ispr_flag : 1 when parse_formal_prm,  0 else
    dc_flag : 1 when parse_variable_dc, 0 else
    subpro_flag : 1 when parse_subprogram_dc and parse_procedure_name, 0 else
    subpro_flag2 : 1 when parse_subprogram_dc and parse_compound_st, 0 else
 */
int *prms, num; /*
 prms : store type of actual argument
 num : store num of arguments
 */
FILE *fp2;
/* file pointer to *.csl */
int LineNum;
/* store the linenum which don't used */
int main_flag=0;
/* 1 : when parse main compound statement,  0 : else */
int breaklnum;
/* for parse_exit_st(). store the linenum for JUMP */
int exps_flag=0;
/* 1 : when parse_expressions,  0 : else */
int variableonly_flag=0;
/* 1 : when expression is only variable,  0 : else */

int main(int nc, char *np[]) {
	char *fname,*file;

	if(nc < 2) {
		printf("File name id not given.\n");
		return 0;
	}
	if(init_scan(np[1]) < 0) {
		printf("File %s can not open.\n", np[1]);
		return 0;
	}

	if((fname = (char*)malloc(sizeof(strlen(np[1])+1))) == NULL){
		printf("cannot malloc in main()\n");
		return 0;
	}
	if((file = (char*)malloc(sizeof(strlen(np[1])-3))) == NULL){
		printf("cannot malloc-2 in main()\n");
		return 0;
	}
	snprintf(file,strlen(np[1])-3,"%s",np[1]);
	snprintf(fname,strlen(np[1])+1,"%s.csl",file);
	free(file);
	if((fp2 = fopen(fname, "w")) == NULL){
		printf("file open error\n");
		return 0;
	}

	init_idtab();
	strcpy(procname,"");
	LineNum = 1;

	token = scan();
	if(parse_program() == ERROR){
		printf("Abnormal termination\n");
		free(fname);
		fclose(fp2);
		end_scan();
		return 0;
	}
	add_mem(NULL);
	outlib();
	free(fname);
	fclose(fp2);
	collect_id(globalidroot);
	release_id();
	end_scan();
	return 0;
}

/* parse program */
int parse_program(void){
	if(token != TPROGRAM){
		error("parse_program(program)");
		return ERROR;
	}

	token = scan();
	if(token != TNAME){
		error("parse_program(TNAME)");
		return ERROR;
	}

	fprintf(fp2,"$$%s  START\n"
			"    LAD   gr0,0\n"
			"    CALL  L%.4d\n"
			"    CALL  FLUSH\n"
			"    SVC   0\n",string_attr,LineNum++);

	token = scan();
	if(token != TSEMI){
		error("parse_program(TSEMI)");
		return ERROR;
	}

	token = scan();
	if(parse_block() == ERROR){
		return ERROR;
	}

	if(token != TDOT){
		error("parse_program(TDOT)");
		return ERROR;
	}
	fprintf(fp2,"    RET\n");

	token = scan();
	return NORMAL;
}

/* parse block */
int parse_block(void){
	while(1){
		if(token == TVAR){
			if(parse_variable_dc() == ERROR){
				return ERROR;
			}
		}
		else if(token == TPROCEDURE){
			if(parse_subprogram_dc() == ERROR){
				return ERROR;
			}
		}else{
			break;
		}
	}

	main_flag=1;
	if(parse_compound_st() == ERROR){
		return ERROR;
	}

	return NORMAL;
}

/* parse variable declaration */
int parse_variable_dc(void){
	int type;
	struct ID *p;

	if(token != TVAR){
		/* do not print error message */
		return ERROR;
	}
	token = scan();

	dc_flag = 1;

	if(parse_variable_names() == ERROR){
		return ERROR;
	}

	if(token != TCOLON){
		error("parse_variable_dc(TCOLON)");
		return ERROR;
	}
	token = scan();

	if((type = parse_type()) == ERROR){
		return ERROR;
	}
	if(strcmp(procname,"") == 0){
		if(type == TPINT || type == TPCHAR || type == TPBOOL){
			for(p=globalidroot;p!=NULL;p=p->nextp){
				if(p->itp == NULL){
					fprintf(fp2,"$%s  DS  1\n",p->name);
				}
			}
		}else{
			for(p=globalidroot;p!=NULL;p=p->nextp){
				if(p->itp == NULL){
					fprintf(fp2,"$%s  DS  %d\n",p->name,num_attr);
				}
			}
		}
		if((register_type(globalidroot,NULL,type,num_attr)) == ERROR)return ERROR;
	}else{
		if(type == TPINT || type == TPCHAR || type == TPBOOL){
			for(p=localidroot;p!=NULL;p=p->nextp){
				if(p->itp == NULL){
					fprintf(fp2,"$%s%%%s  DS  1\n",p->name,procname);
				}
			}
		}else{
			for(p=localidroot;p!=NULL;p=p->nextp){
				if(p->itp == NULL){
					fprintf(fp2,"$%s%%%s  DS  %d\n",p->name,procname,num_attr);
				}
			}
		}
		if((register_type(localidroot,NULL,type,num_attr)) == ERROR)return ERROR;
	}

	if(token != TSEMI){
		error("parse_variable_dc(TSEMI)");
		return ERROR;
	}
	token = scan();

	while(token == TNAME){

		if(parse_variable_names() == ERROR){
			return ERROR;
		}

		if(token != TCOLON){
			error("parse_variable_dc(TCOLON)");
			return ERROR;
		}
		token = scan();

		if((type = parse_type()) == ERROR){
			return ERROR;
		}
		if(strcmp(procname,"") == 0){
			if(type == TPINT || type == TPCHAR || type == TPBOOL){
				for(p=globalidroot;p!=NULL;p=p->nextp){
					if(p->itp == NULL){
						fprintf(fp2,"$%s  DS  1\n",p->name);
					}
				}
			}else{
				for(p=globalidroot;p!=NULL;p=p->nextp){
					if(p->itp == NULL){
						fprintf(fp2,"$%s  DS  %d\n",p->name,num_attr);
					}
				}
			}
			if((register_type(globalidroot,NULL,type,num_attr)) == ERROR)return ERROR;
		}else{
			if(type == TPINT || type == TPCHAR || type == TPBOOL){
				for(p=localidroot;p!=NULL;p=p->nextp){
					if(p->itp == NULL){
						fprintf(fp2,"$%s%%%s  DS  1\n",p->name,procname);
					}
				}
			}else{
				for(p=localidroot;p!=NULL;p=p->nextp){
					if(p->itp == NULL){
						fprintf(fp2,"$%s%%%s  DS  %d\n",p->name,procname,num_attr);
					}
				}
			}
			if((register_type(localidroot,NULL,type,num_attr)) == ERROR)return ERROR;
		}

		if(token != TSEMI){
			error("parse_variable_dc(TSEMI)");
			return ERROR;
		}
		token = scan();
	}
	dc_flag = 0;

	return NORMAL;
}

/* parse variable names */
int parse_variable_names(void){
	if(parse_variable_name() == ERROR){
		return ERROR;
	}

	while(token == TCOMMA){
		token = scan();

		if(parse_variable_name() == ERROR){
			return ERROR;
		}
	}

	return NORMAL;
}

/* parse variable name */
int parse_variable_name(void){
	int type;

	if(token != TNAME){
		error("parse_variable_name(TNAME)");
		return ERROR;
	}
	if(ispr_flag == 1 || dc_flag == 1){
		if((register_name(string_attr,procname,ispr_flag)) == ERROR)return ERROR;
		type = NORMAL;
	}else{
		if(subpro_flag2 == 1){
			if((type = register_rline(localidroot,string_attr)) == ERROR){
				if((type = register_rline(globalidroot,string_attr)) == ERROR){
					printf("not defined %s line:%d\n", string_attr, get_linenum());
					return ERROR;
				}
			}
		}else{
			if((type = register_rline(globalidroot,string_attr)) == ERROR){
				printf("not defined %s line:%d\n", string_attr, get_linenum());
				return ERROR;
			}
		}
	}
	token = scan();

	return type;
}

/* parse type */
int parse_type(void){
	int type, type2;

	if((type = parse_standard_type()) == ERROR){
		if((type2 = parse_array_type()) == ERROR){
			return ERROR;
		}else{
			type = type2;
		}
	}

	return type;
}

/* parse standard type */
int parse_standard_type(void){
	int type;

	if((token != TINTEGER) && (token != TBOOLEAN) && (token != TCHAR)){
		/* do not print error message */
		return ERROR;
	}
	switch(token){
	case TINTEGER:
		type = TPINT;
		break;
	case TBOOLEAN:
		type = TPBOOL;
		break;
	case TCHAR:
		type = TPCHAR;
		break;
	}
	token = scan();
	return type;
}

/* parse array type */
int parse_array_type(void){
	int type;

	if(token != TARRAY){
		error("parse_array_type(TARRAY)");
		return ERROR;
	}
	token = scan();

	if(token != TLSQPAREN){
		error("parse_array_type(TLSQPAREN)");
		return ERROR;
	}
	token = scan();

	if(token != TNUMBER){
		error("parse_array_type(TNUMBER)");
		return ERROR;
	}
	if(num_attr < 1){
		error("array size");
		return ERROR;
	}
	token = scan();

	if(token != TRSQPAREN){
		error("parse_array_type(TRSQPAREN)");
		return ERROR;
	}
	token = scan();

	if(token != TOF){
		error("parse_array_type(TOF)");
		return ERROR;
	}
	token = scan();

	if((type = parse_standard_type()) == ERROR){
		return ERROR;
	}

	switch(type){
	case TPINT:
		type = TPARRAYINT;
		break;
	case TPBOOL:
		type = TPARRAYBOOL;
		break;
	case TPCHAR:
		type = TPARRAYCHAR;
		break;
	}

	return type;
}

/* parse subprogram declaration */
int parse_subprogram_dc(void){
	struct ID *p;

	if(token != TPROCEDURE){
		error("parse_subprogram_dc(TPROCEDURE)");
		return ERROR;
	}
	token = scan();
	subpro_flag = 1;

	if(parse_procedure_name() == ERROR){
		return ERROR;
	}
	subpro_flag = 0;

	if(token == TLPAREN){
		if(parse_formal_prm() == ERROR){
			return ERROR;
		}
	}

	if(token != TSEMI){
		error("parse_subprogram_dc(TSEMI)");
		return ERROR;
	}
	token = scan();

	if(token == TVAR){
		if(parse_variable_dc() == ERROR){
			return ERROR;
		}
	}

	fprintf(fp2,
			"$%s  DS  0\n"
			"    POP   gr2\n",procname);
	for(p=localidroot;p!=NULL;p=p->nextp){
		if(p->ispara != 0){
			fprintf(fp2,
					"    POP   gr1\n"
					"    ST    gr1,$%s%%%s\n"
					,p->name,p->procname);
		}
	}
	fprintf(fp2,"    PUSH  0,gr2\n");

	subpro_flag2 = 1;
	if(parse_compound_st() == ERROR){
		return ERROR;
	}
	subpro_flag2 = 0;

	if(token != TSEMI){
		error("parse_subprogram_dc(TSEMI2)");
		return ERROR;
	}
	token = scan();

	fprintf(fp2,"    RET\n");

	strcpy(procname,"");
	collect_id(localidroot);
	return NORMAL;
}

/* parse procedure name */
int parse_procedure_name(void){
	if(token != TNAME){
		error("parse_procedure_name(TNAME)");
		return ERROR;
	}
	if(subpro_flag == 1){
		strcpy(procname,string_attr);
		if((register_name(procname,"",0)) == ERROR)return ERROR;
		if((register_type(globalidroot,procname,TPPROC,0)) == ERROR)return ERROR;
	}else{
		strcpy(procesname,string_attr);

		if(strcmp(procname,procesname) == 0){
			error("recursive call");
			return ERROR;
		}

		if(register_rline(globalidroot,string_attr) == ERROR){
			printf("not defined %s line:%d\n", string_attr, get_linenum());
			return ERROR;
		}
	}
	token = scan();

	return NORMAL;
}

/* parse formal parameter */
int parse_formal_prm(void){
	int type;
	struct ID *p;

	if(token != TLPAREN){
		error("parse_formal_prm(TLPAREN)");
		return ERROR;
	}
	token = scan();
	ispr_flag = 1;

	if(parse_variable_names() == ERROR){
		return ERROR;
	}

	if(token != TCOLON){
		error("parse_formal_prm(TCOLON)");
		return ERROR;
	}
	token = scan();

	type = parse_type();
	if(type != TPINT && type != TPCHAR && type != TPBOOL){
		error("type must standard in formal parameter");
		return ERROR;
	}
	for(p=localidroot;p!=NULL;p=p->nextp){
		if(p->itp == NULL){
			fprintf(fp2,"$%s%%%s  DS  1\n",p->name,p->procname);
		}
	}
	if((register_type(localidroot,NULL,type,num_attr)) == ERROR)return ERROR;
	if((register_paratype(procname)) == ERROR)return ERROR;

	while(token == TSEMI){
		token = scan();

		if(parse_variable_names() == ERROR){
			return ERROR;
		}

		if(token != TCOLON){
			error("parse_formal_prm(TCOLON2)");
			return ERROR;
		}
		token = scan();

		type = parse_type();
		if(type != TPINT && type != TPCHAR && type != TPBOOL){
			error("type must standard in formal parameter");
			return ERROR;
		}
		for(p=localidroot;p!=NULL;p=p->nextp){
			if(p->itp == NULL){
				fprintf(fp2,"$%s%%%s  DS  1\n",p->name,p->procname);
			}
		}
		if((register_type(localidroot,NULL,type,num_attr)) == ERROR)return ERROR;
		if((register_paratype(procname)) == ERROR)return ERROR;
	}

	if(token != TRPAREN){
		error("parse_formal_prm(TRPAREN)");
		return ERROR;
	}
	token = scan();
	ispr_flag = 0;

	return NORMAL;
}

/* parse compound statement */
int parse_compound_st(void){
	if(token != TBEGIN){
		error("parse_compound_st(TBEGIN)");
		return ERROR;
	}
	token = scan();
	if(main_flag == 1){
		fprintf(fp2,"L0001  DS  0\n");
		main_flag = 0;
	}

	if(parse_statement() == ERROR){
		return ERROR;
	}

	while(token == TSEMI){
		token = scan();

		if(parse_statement() == ERROR){
			return ERROR;
		}
	}

	if(token != TEND){
		error("parse_compound_st(TEND)");
		return ERROR;
	}
	token = scan();

	return NORMAL;
}

/* parse statement */
int parse_statement(void){
	switch(token){
	case TNAME:
		if(parse_assignment_st() == ERROR){
			return ERROR;
		}
		break;
	case TIF:
		if(parse_condition_st() == ERROR){
			return ERROR;
		}
		break;
	case TWHILE:
		if(parse_iteration_st() == ERROR){
			return ERROR;
		}
		break;
	case TBREAK:
		if(parse_exit_st() == ERROR){
			return ERROR;
		}
		break;
	case TCALL:
		if(parse_call_st() == ERROR){
			return ERROR;
		}
		break;
	case TRETURN:
		if(parse_return_st() == ERROR){
			return ERROR;
		}
		break;
	case TREAD:
	case TREADLN:
		if(parse_input_st() == ERROR){
			return ERROR;
		}
		break;
	case TWRITE:
	case TWRITELN:
		if(parse_output_st() == ERROR){
			return ERROR;
		}
		break;
	case TBEGIN:
		if(parse_compound_st() == ERROR){
			return ERROR;
		}
		break;
	default:
		if(parse_empty_st() == ERROR){
			return ERROR;
		}
	}

	return NORMAL;
}

/* parse condition statement */
int parse_condition_st(void){
	int linenum1, linenum2;

	if(token != TIF){
		error("parse_condition_st(TIF)");
		return ERROR;
	}
	token = scan();

	if(parse_expression() != TPBOOL){
		error("expression must boolean in condition statement");
		return ERROR;
	}
	linenum1 = LineNum;
	fprintf(fp2,"    POP   gr1\n"
			"    CPA   gr1,gr0\n"
			"    JZE   L%.4d\n",LineNum++);

	if(token != TTHEN){
		error("parse_condition_st(TTHEN)");
		return ERROR;
	}
	token = scan();

	if(parse_statement() == ERROR){
		return ERROR;
	}

	if(token == TELSE){
		token = scan();
		linenum2 = LineNum;
		fprintf(fp2,"    JUMP  L%.4d\n"
				"L%.4d  DS  0\n",LineNum++,linenum1);

		if(parse_statement() == ERROR){
			return ERROR;
		}
		fprintf(fp2,"L%.4d  DS  0\n",linenum2);
	}else{
		fprintf(fp2,"L%.4d  DS  0\n",linenum1);
	}

	return NORMAL;
}

/* parse iteration statement */
int parse_iteration_st(void){
	int linenum1, linenum2, beforebreaklnum;

	if(token != TWHILE){
		error("parse_iteration_st(TWHILE)");
		return ERROR;
	}
	lupe++;
	token = scan();

	linenum1 = LineNum;
	fprintf(fp2,"L%.4d  DS  0\n",LineNum++);

	if(parse_expression() != TPBOOL){
		error("expression must boolean in iteration statement");
		return ERROR;
	}

	if(token != TDO){
		error("parse_iteration_st(TDO)");
		return ERROR;
	}
	token = scan();

	linenum2 = LineNum;
	fprintf(fp2,"    POP   gr1\n"
			"    CPA   gr1,gr0\n"
			"    JZE   L%.4d\n",LineNum++);
	beforebreaklnum = breaklnum;
	breaklnum = linenum2;

	if(parse_statement() == ERROR){
		return ERROR;
	}

	fprintf(fp2,"    JUMP  L%.4d\n"
			"L%.4d  DS  0\n",linenum1,linenum2);

	breaklnum = beforebreaklnum;
	lupe--;
	return NORMAL;
}

/* parse_exit_statement */
int parse_exit_st(void){
	if(token != TBREAK){
		error("parse_exit_st(TBREAK)");
		return ERROR;
	}
	if(lupe < 1){
		error("parse_exit_st(lupe)");
		return ERROR;
	}
	token = scan();

	fprintf(fp2,"    JUMP  L%.4d",breaklnum);

	return NORMAL;
}

/* parse call statement */
int parse_call_st(void){
	if(token != TCALL){
		error("parse_call_st(TCALL)");
		return ERROR;
	}
	token = scan();

	if(parse_procedure_name() == ERROR){
		return ERROR;
	}

	prms = NULL;
	if((num = num_prm(procesname)) == -1){
		return ERROR;
	}

	if(token == TLPAREN){
		token = scan();

		if(num == 0){
			error("num of parameters");
			return ERROR;
		}

		if(parse_expressions() == ERROR){
			return ERROR;
		}

		if(token != TRPAREN){
			error("parse_call_st(TRPAREN)");
			return ERROR;
		}
		token = scan();
	}

	if(check_prm(procesname,prms) == ERROR){
		if(prms != NULL)free(prms);
		return ERROR;
	}

	fprintf(fp2,"    CALL  $%s\n",procesname);

	if(prms != NULL)free(prms);
	return NORMAL;
}

/* parse expressions */
int parse_expressions(void){
	int type,i;
	char memstr[MAXSTRSIZE];
	int linenum;

	exps_flag=1;

	if((prms = (int*)malloc(sizeof(int)*num)) == NULL){
		printf("can not malloc in parse_expressions\n");
		return ERROR;
	}

	if((type = parse_expression()) == ERROR){
		return ERROR;
	}
	i = 0;
	prms[i++] = type;

	if(variableonly_flag==0){
		linenum = LineNum++;
		fprintf(fp2,
				"    POP   gr1\n"
				"    LAD   gr2,L%.4d\n"
				"    ST    gr1,0,gr2\n"
				"    PUSH  0,gr2\n"
				,linenum);
		snprintf(memstr,MAXSTRSIZE,"L%.4d  DS  1\n",linenum);
		add_mem(memstr);
	}

	while(token == TCOMMA){
		token = scan();

		if((type = parse_expression()) == ERROR){
			return ERROR;
		}

		if(i+1 > num){
			error("num of parameters");
			return ERROR;
		}

		prms[i++] = type;

		if(variableonly_flag==0){
			linenum = LineNum++;
			fprintf(fp2,
					"    POP   gr1\n"
					"    LAD   gr2,L%.4d\n"
					"    ST    gr1,0,gr2\n"
					"    PUSH  0,gr2\n"
					,linenum);
			snprintf(memstr,MAXSTRSIZE,"L%.4d  DS  1\n",linenum);
			add_mem(memstr);
		}
	}

	if(num != i){
		error("num of parameters-2");
		return ERROR;
	}

	exps_flag=0;

	return NORMAL;
}

/* parse return statement */
int parse_return_st(void){
	if(token != TRETURN){
		error("parse_return_st(TRETURN)");
		return ERROR;
	}
	token = scan();

	fprintf(fp2,"    RET\n");

	return NORMAL;
}

/* parse assignment statement */
int parse_assignment_st(void){
	int ltype, rtype;

	if((ltype = parse_left_part()) == ERROR){
		return ERROR;
	}

	if(token != TASSIGN){
		error("parse_assignment_st(TASSIGN)");
		return ERROR;
	}
	token = scan();

	if((rtype = parse_expression()) == ERROR){
		return ERROR;
	}

	if(ltype != rtype){
		error("left part and expression must same type");
		return ERROR;
	}
	if(ltype != TPINT && ltype != TPCHAR && ltype != TPBOOL){
		error("left part and expression must standard type");
		return ERROR;
	}

	fprintf(fp2,
			"    POP   gr2\n"
			"    POP   gr1\n"
			"    ST    gr2,0,gr1\n");

	return NORMAL;
}

/* parse left part */
int parse_left_part(void){
	return parse_variable();
}

/* parse variable */
int parse_variable(void){
	int type,type2;
	struct ID *p;
	char name[MAXSTRSIZE];

	strcpy(name,string_attr);

	if((type = parse_variable_name()) == ERROR){
		return ERROR;
	}

	if(token == TLSQPAREN){
		token = scan();

		if(type!=TPARRAYINT && type!=TPARRAYCHAR && type!=TPARRAYBOOL){
			error("variable name must array type");
			return ERROR;
		}

		if((type2 = parse_expression()) == ERROR){
			return ERROR;
		}
		if(type2 != TPINT){
			error("expression must integer");
			return ERROR;
		}

		if(token != TRSQPAREN){
			error("parse_variable(TRSQPAREN)");
			return ERROR;
		}
		token = scan();

		switch(type){
		case TPARRAYINT:
			type = TPINT;
			break;
		case TPARRAYCHAR:
			type = TPCHAR;
			break;
		case TPARRAYBOOL:
			type = TPBOOL;
			break;
		}

		if((p=search_idtab(localidroot,name)) == NULL){
			p=search_idtab(globalidroot,name);
			fprintf(fp2,
					"    LAD   gr1,$%s\n"
					"    POP   gr2\n"
					"    ADDA  gr1,gr2\n"
					"    PUSH  0,gr1\n",p->name);
		}else{
			if(p->ispara == 0){
				fprintf(fp2,"    LAD   gr1,$%s%%%s\n",p->name,p->procname);
			}else{
				fprintf(fp2,"    LD    gr1,$%s%%%s\n",p->name,p->procname);
			}
			fprintf(fp2,
					"    POP   gr2\n"
					"    ADDA  gr1,gr2\n"
					"    PUSH  0,gr1\n");
		}
	}else{
		if((p=search_idtab(localidroot,name)) == NULL){
			p=search_idtab(globalidroot,name);
			fprintf(fp2,
					"    LAD   gr1,$%s\n"
					"    PUSH  0,gr1\n",p->name);
		}else{
			if(p->ispara == 0){
				fprintf(fp2,
						"    LAD   gr1,$%s%%%s\n"
						"    PUSH  0,gr1\n",p->name,p->procname);
			}else{
				fprintf(fp2,
						"    LD    gr1,$%s%%%s\n"
						"    PUSH  0,gr1\n",p->name,p->procname);
			}
		}
	}

	return type;
}

/* parse expression */
int parse_expression(void){
	int type,type2,opr,line1,line2;

	if((type = parse_simple_exp()) == ERROR){
		return ERROR;
	}

	while(token>=TEQUAL && token<=TGREQ){
		opr = token;
		parse_relational_op();

		if(variableonly_flag == 1){
			fprintf(fp2,
					"    POP   gr1\n"
					"    LD    gr1,0,gr1\n"
					"    PUSH  0,gr1\n");
			variableonly_flag = 0;
		}

		if((type2 = parse_simple_exp()) == ERROR){
			return ERROR;
		}
		if(type != type2){
			error("both of simple_exp must same type in parse_expression");
			return ERROR;
		}
		type = TPBOOL;

		if(variableonly_flag == 1){
			fprintf(fp2,
					"    POP   gr1\n"
					"    LD    gr1,0,gr1\n"
					"    PUSH  0,gr1\n");
			variableonly_flag = 0;
		}

		line1 = LineNum++;
		line2 = LineNum++;
		fprintf(fp2,
				"    POP   gr2\n"
				"    POP   gr1\n"
				"    CPA   gr1,gr2\n");
		switch(opr){
		case TEQUAL:
			fprintf(fp2,
					"    JZE   L%.4d\n"
					"    LD    gr1,gr0\n"
					"    JUMP  L%.4d\n"
					"L%.4d  DS  0\n"
					"    LAD   gr1,1\n"
					"L%.4d  DS  0\n"
					,line1,line2,line1,line2);
			break;
		case TNOTEQ:
			fprintf(fp2,
					"    JNZ   L%.4d\n"
					"    LD    gr1,gr0\n"
					"    JUMP  L%.4d\n"
					"L%.4d  DS  0\n"
					"    LAD   gr1,1\n"
					"L%.4d  DS  0\n"
					,line1,line2,line1,line2);
			break;
		case TLE:
			fprintf(fp2,
					"    JMI   L%.4d\n"
					"    LD    gr1,gr0\n"
					"    JUMP  L%.4d\n"
					"L%.4d  DS  0\n"
					"    LAD   gr1,1\n"
					"L%.4d  DS  0\n"
					,line1,line2,line1,line2);
			break;
		case TLEEQ:
			fprintf(fp2,
					"    JPL   L%.4d\n"
					"    LAD   gr1,1\n"
					"    JUMP  L%.4d\n"
					"L%.4d  DS  0\n"
					"    LD    gr1,gr0\n"
					"L%.4d  DS  0\n"
					,line1,line2,line1,line2);
			break;
		case TGR:
			fprintf(fp2,
					"    JPL   L%.4d\n"
					"    LD    gr1,gr0\n"
					"    JUMP  L%.4d\n"
					"L%.4d  DS  0\n"
					"    LAD   gr1,1\n"
					"L%.4d  DS  0\n"
					,line1,line2,line1,line2);
			break;
		case TGREQ:
			fprintf(fp2,
					"    JMI   L%.4d\n"
					"    LAD   gr1,1\n"
					"    JUMP  L%.4d\n"
					"L%.4d  DS  0\n"
					"    LD    gr1,gr0\n"
					"L%.4d  DS  0\n"
					,line1,line2,line1,line2);
			break;
		}
		fprintf(fp2,"    PUSH  0,gr1\n");
	}

	if(variableonly_flag == 1 && exps_flag == 0){
		fprintf(fp2,
				"    POP   gr1\n"
				"    LD    gr1,0,gr1\n"
				"    PUSH  0,gr1\n");
		variableonly_flag = 0;
	}

	return type;
}

/* parse simple expression */
int parse_simple_exp(void){
	int type, type2, type3, pm_flag=0, m_flag=0, opr;

	if(token == TPLUS){
		pm_flag=1;
		token = scan();
	}else if(token == TMINUS){
		pm_flag=1;
		m_flag=1;
		token = scan();
	}

	if((type = parse_term()) == ERROR){
		return ERROR;
	}

	if(pm_flag == 1 && type != TPINT){
		error("term must integer when have + or -");
		return ERROR;
	}
	pm_flag=0;

	if(m_flag == 1){
		if(variableonly_flag == 1){
			fprintf(fp2,
					"    POP   gr1\n"
					"    LD    gr1,0,gr1\n"
					"    PUSH  0,gr1\n");
			variableonly_flag=0;
		}
		fprintf(fp2,
				"    POP   gr1\n"
				"    LD    gr2,gr0\n"
				"    SUBA  gr2,gr1\n"
				"    JOV   EOVF\n"
				"    PUSH  0,gr2\n");
		m_flag=0;
	}

	while(token == TPLUS || token == TMINUS || token == TOR){
		opr = token;
		type2 = parse_additive_op();

		if(variableonly_flag == 1){
			fprintf(fp2,
					"    POP   gr1\n"
					"    LD    gr1,0,gr1\n"
					"    PUSH  0,gr1\n");
			variableonly_flag=0;
		}

		if((type3 = parse_term()) == ERROR){
			return ERROR;
		}
		if(type == type2 && type2 == type3){
			type = type3;
		}else{
			error("type error in simple_exp");
			return ERROR;
		}

		if(variableonly_flag == 1){
			fprintf(fp2,
					"    POP   gr1\n"
					"    LD    gr1,0,gr1\n"
					"    PUSH  0,gr1\n");
			variableonly_flag=0;
		}

		fprintf(fp2,
				"    POP   gr2\n"
				"    POP   gr1\n");
		switch(opr){
		case TPLUS:
			fprintf(fp2,
					"    ADDA  gr1,gr2\n"
					"    JOV   EOVF\n");
			break;
		case TMINUS:
			fprintf(fp2,
					"    SUBA  gr1,gr2\n"
					"    JOV   EOVF\n");
			break;
		case TOR:
			fprintf(fp2,"    OR    gr1,gr2\n");
			break;
		}
		fprintf(fp2,"    PUSH  0,gr1\n");
	}

	return type;
}

/* parse term */
int parse_term(void){
	int type, type2, type3, opr;

	if((type = parse_factor()) == ERROR){
		return ERROR;
	}

	while(token == TSTAR || token == TDIV || token == TAND){
		opr = token;
		type2 = parse_multiple_op();

		if(variableonly_flag == 1){
			fprintf(fp2,
					"    POP   gr1\n"
					"    LD    gr1,0,gr1\n"
					"    PUSH  0,gr1\n");
			variableonly_flag=0;
		}

		if((type3 = parse_factor()) == ERROR){
			return ERROR;
		}
		if(type == type2 && type2 == type3){
			type = type3;
		}else{
			error("type error in parse_term");
			return ERROR;
		}

		if(variableonly_flag == 1){
			fprintf(fp2,
					"    POP   gr1\n"
					"    LD    gr1,0,gr1\n"
					"    PUSH  0,gr1\n");
			variableonly_flag=0;
		}

		fprintf(fp2,
				"    POP   gr2\n"
				"    POP   gr1\n");
		switch(opr){
		case TSTAR:
			fprintf(fp2,
					"    MULA  gr1,gr2\n"
					"    JOV   EOVF\n");
			break;
		case TDIV:
			fprintf(fp2,
					"    DIVA  gr1,gr2\n"
					"    JOV   EOVF\n");
			break;
		case TAND:
			fprintf(fp2,
					"    AND   gr1,gr2\n");
			break;
		}
		fprintf(fp2,"    PUSH  0,gr1\n");
	}

	return type;
}

/* parse factor */
int parse_factor(void){
	int type,type2,linenum;

	switch(token){
	case TNAME:
		if((type = parse_variable()) == ERROR){
			return ERROR;
		}
		variableonly_flag=1;
		break;
	case TNUMBER:
	case TFALSE:
	case TTRUE:
	case TSTRING:
		if((type = parse_constant()) == ERROR){
			return ERROR;
		}
		variableonly_flag=0;
		break;
	case TLPAREN:
		token = scan();

		if((type = parse_expression()) == ERROR){
			return ERROR;
		}

		if(token != TRPAREN){
			error("parse_factor(TRPAREN)");
			return ERROR;
		}
		token = scan();
		break;
	case TNOT:
		token = scan();

		if((type = parse_factor()) == ERROR){
			return ERROR;
		}
		if(type != TPBOOL){
			error("factor must boolean when have not");
			return ERROR;
		}

		if(variableonly_flag==1){
			fprintf(fp2,
					"    POP   gr1\n"
					"    LD    gr1,0,gr1\n"
					"    PUSH  0,gr1\n");
		}
		fprintf(fp2,
				"    POP   gr1\n"
				"    LAD   gr2,1\n"
				"    XOR   gr1,gr2\n"
				"    PUSH  0,gr1\n");
		variableonly_flag=0;

		break;
	case TINTEGER:
	case TBOOLEAN:
	case TCHAR:
		if((type = parse_standard_type()) == ERROR){
			return ERROR;
		}

		if(token != TLPAREN){
			error("parse_factor(TLPAREN)");
			return ERROR;
		}
		token = scan();

		if((type2 = parse_expression()) == ERROR){
			return ERROR;
		}
		if(type2 != TPINT && type2 != TPBOOL && type2 != TPCHAR){
			error("expression must standard type in parse_factor");
			return ERROR;
		}

		if(token != TRPAREN){
			error("parse_factor(TRPAREN2)");
			return ERROR;
		}
		token = scan();

		if(variableonly_flag == 1){
			fprintf(fp2,
					"    POP   gr1\n"
					"    LD    gr1,0,gr1\n"
					"    PUSH  0,gr1\n");
		}
		switch(type){
		case TPINT:
			break;
		case TPCHAR:
			fprintf(fp2,
					"    POP   gr1\n"
					"    LAD   gr2,127\n"
					"    AND   gr1,gr2\n"
					"    PUSH  0,gr1\n");
			break;
		case TPBOOL:
			linenum = LineNum++;
			fprintf(fp2,
					"    POP   gr1\n"
					"    CPA   gr1,gr0\n"
					"    JZE   L%.4d\n"
					"    LAD   gr1,1\n"
					"L%.4d  DS  0\n"
					"    PUSH  0,gr1\n",linenum,linenum);
			break;
		}
		variableonly_flag=0;

		break;
		default:
			error("parse_factor");
			return ERROR;
	}

	return type;
}

/* parse constant */
int parse_constant(void){
	int type;

	switch(token){
	case TNUMBER:
		type = TPINT;

		fprintf(fp2,"    LAD   gr1,%d\n",num_attr);

		break;
	case TFALSE:
		type = TPBOOL;

		fprintf(fp2,"    LD    gr1,gr0\n");

		break;
	case TTRUE:
		type = TPBOOL;

		fprintf(fp2,"    LAD   gr1,1\n");

		break;
	case TSTRING:
		if(strlen(string_attr) != 1){
			error("length of TSTRING must 1 in parse_constant");
			return ERROR;
		}

		type = TPCHAR;

		fprintf(fp2,"    LAD   gr1,%d\n",string_attr[0]);

		break;
	default:
		error("parse_constant");
		return ERROR;
	}
	token = scan();

	fprintf(fp2,"    PUSH  0,gr1\n");

	return type;
}

/* parse multiplicative operator */
int parse_multiple_op(void){
	int type;

	switch(token){
	case TSTAR:
		type = TPINT;
		break;
	case TDIV:
		type = TPINT;
		break;
	case TAND:
		type = TPBOOL;
		break;
	default:
		return ERROR;
	}
	token = scan();

	return type;
}

/* parse additive operator */
int parse_additive_op(void){
	int type;

	switch(token){
	case TPLUS:
		type = TPINT;
		break;
	case TMINUS:
		type = TPINT;
		break;
	case TOR:
		type = TPBOOL;
		break;
	default:
		return ERROR;
	}
	token = scan();

	return type;
}

/* parse relational operator */
int parse_relational_op(void){
	switch(token){
	case TEQUAL:
		break;
	case TNOTEQ:
		break;
	case TLE:
		break;
	case TLEEQ:
		break;
	case TGR:
		break;
	case TGREQ:
		break;
	default:
		return ERROR;
	}
	token = scan();

	return NORMAL;
}

/* parse input statement */
int parse_input_st(void){
	int type, flag=0;

	switch(token){
	case TREAD:
		break;
	case TREADLN:
		flag=1;
		break;
	default:
		error("parse_input_st(read)");
		return ERROR;
	}
	token = scan();

	if(token == TLPAREN){
		token = scan();

		if((type = parse_variable()) == ERROR){
			return ERROR;
		}
		if(type != TPINT && type != TPCHAR){
			error("variable must integer or char in parse_input_st");
			return ERROR;
		}

		fprintf(fp2,"    POP   gr1\n");
		switch(type){
		case TPINT:
			fprintf(fp2,"    CALL  READINT\n");
			break;
		case TPCHAR:
			fprintf(fp2,"    CALL  READCHAR\n");
			break;
		}

		while(token == TCOMMA){
			token = scan();

			if((type = parse_variable()) == ERROR){
				return ERROR;
			}
			if(type != TPINT && type != TPCHAR){
				error("variable must integer or char in parse_input_st");
				return ERROR;
			}

			fprintf(fp2,"    POP   gr1\n");
			switch(type){
			case TPINT:
				fprintf(fp2,"    CALL  READINT\n");
				break;
			case TPCHAR:
				fprintf(fp2,"    CALL  READCHAR\n");
				break;
			}
		}

		if(token != TRPAREN){
			error("parse_input_st(TRPAREN)");
			return ERROR;
		}
		token = scan();
	}

	if(flag == 1){
		fprintf(fp2,"    CALL  READLINE\n");
		flag=0;
	}

	return NORMAL;
}

/* parse output statement */
int parse_output_st(void){
	int flag=0;

	switch(token){
	case TWRITE:
		break;
	case TWRITELN:
		flag=1;
		break;
	default:
		error("parse_output_st(write)");
		return ERROR;
	}
	token = scan();

	if(token == TLPAREN){
		token = scan();

		if(parse_output_format() == ERROR){
			return ERROR;
		}

		while(token == TCOMMA){
			token = scan();

			if(parse_output_format() == ERROR){
				return ERROR;
			}
		}

		if(token != TRPAREN){
			error("parse_output_st(TRPAREN)");
			return ERROR;
		}
		token = scan();
	}

	if(flag == 1){
		fprintf(fp2,"    CALL  WRITELINE\n");
		flag=0;
	}

	return NORMAL;
}

/* parse output format */
int parse_output_format(void){
	int type, linenum;
	char memstr[MAXSTRSIZE];

	if(token == TSTRING){
		if(strlen(string_attr) == 1){
			/* do nothing */
		}else{
			linenum = LineNum++;
			fprintf(fp2,
					"    LAD   gr1,L%.4d\n"
					"    LD    gr2,gr0\n"
					"    CALL  WRITESTR\n",linenum);
			snprintf(memstr,MAXSTRSIZE,"L%.4d  DC  '%s'\n",linenum,string_attr);
			add_mem(memstr);

			token = scan();
			return NORMAL;
		}
	}

	if((type = parse_expression()) == ERROR){
		return ERROR;
	}
	if(type != TPINT && type != TPCHAR && type != TPBOOL){
		error("expression must standard type in parse_output_format");
		return ERROR;
	}

	fprintf(fp2,
			"    POP   gr1\n"
			"    LD    gr2,gr0\n");

	if(token == TCOLON){
		token = scan();

		if(token != TNUMBER){
			error("parse_output_format(TNUMBER)");
			return ERROR;
		}
		fprintf(fp2,"    LAD   gr2,%d\n",num_attr);

		token = scan();
	}

	switch(type){
	case TPINT:
		fprintf(fp2,"    CALL  WRITEINT\n");
		break;
	case TPBOOL:
		fprintf(fp2,"    CALL  WRITEBOOL\n");
		break;
	case TPCHAR:
		fprintf(fp2,"    CALL  WRITECHAR\n");
		break;
	}

	return NORMAL;
}

/* parse empty statement */
int parse_empty_st(void){
	return NORMAL;
}

void outlib(void) {
	fprintf(fp2,""
			"; ------------------------\n"
			"; Utility functions\n"
			"; ------------------------\n"
			"EOVF            CALL    WRITELINE\n"
			"                LAD     gr1, EOVF1\n"
			"                LD      gr2, gr0\n"
			"                CALL    WRITESTR\n"
			"                CALL    WRITELINE\n"
			"                SVC     1  ;  overflow error stop\n"
			"EOVF1           DC      '***** Run-Time Error : Overflow *****'\n"
			"E0DIV           JNZ     EOVF\n"
			"                CALL    WRITELINE\n"
			"                LAD     gr1, E0DIV1\n"
			"                LD      gr2, gr0\n"
			"                CALL    WRITESTR\n"
			"                CALL    WRITELINE\n"
			"                SVC     2  ;  0-divide error stop\n"
			"E0DIV1          DC      '***** Run-Time Error : Zero-Divide *****'\n"
			"EROV            CALL    WRITELINE\n"
			"                LAD     gr1, EROV1\n"
			"                LD      gr2, gr0\n"
			"                CALL    WRITESTR\n"
			"                CALL    WRITELINE\n"
			"                SVC     3  ;  range-over error stop\n"
			"EROV1           DC      '***** Run-Time Error : Range-Over in Array Index *****'\n"
			"; gr1の値（文字）をgr2のけた数で出力する．\n"
			"; gr2が0なら必要最小限の桁数で出力する\n"
			"WRITECHAR       RPUSH\n"
			"                LD      gr6, SPACE\n"
			"                LD      gr7, OBUFSIZE\n"
			"WC1             SUBA    gr2, ONE  ; while(--c > 0) {\n"
			"                JZE     WC2\n"
			"                JMI     WC2\n"
			"                ST      gr6, OBUF,gr7  ;  *p++ = ' ';\n"
			"                CALL    BOVFCHECK\n"
			"                JUMP    WC1  ; }\n"
			"WC2             ST      gr1, OBUF,gr7  ; *p++ = gr1;\n"
			"                CALL    BOVFCHECK\n"
			"                ST      gr7, OBUFSIZE\n"
			"                RPOP\n"
			"                RET\n"
			"; gr1が指す文字列をgr2のけた数で出力する．\n"
			"; gr2が0なら必要最小限の桁数で出力する\n"
			"WRITESTR        RPUSH\n"
			"                LD      gr6, gr1  ; p = gr1;\n"
			"WS1             LD      gr4, 0,gr6  ; while(*p != 0) {\n"
			"                JZE     WS2\n"
			"                ADDA    gr6, ONE  ;  p++;\n"
			"                SUBA    gr2, ONE  ;  c--;\n"
			"                JUMP    WS1  ; }\n"
			"WS2             LD      gr7, OBUFSIZE  ; q = OBUFSIZE;\n"
			"                LD      gr5, SPACE\n"
			"WS3             SUBA    gr2, ONE  ; while(--c >= 0) {\n"
			"                JMI     WS4\n"
			"                ST      gr5, OBUF,gr7  ;  *q++ = ' ';\n"
			"                CALL    BOVFCHECK\n"
			"                JUMP    WS3  ; }\n"
			"WS4             LD      gr4, 0,gr1  ; while(*gr1 != 0) {\n"
			"                JZE     WS5\n"
			"                ST      gr4, OBUF,gr7  ;  *q++ = *gr1++;\n"
			"                ADDA    gr1, ONE\n"
			"                CALL    BOVFCHECK\n"
			"                JUMP    WS4  ; }\n"
			"WS5             ST      gr7, OBUFSIZE  ; OBUFSIZE = q;\n"
			"                RPOP\n"
			"                RET\n"
			"BOVFCHECK       ADDA    gr7, ONE\n"
			"                CPA     gr7, BOVFLEVEL\n"
			"                JMI     BOVF1\n"
			"                CALL    WRITELINE\n"
			"                LD      gr7, OBUFSIZE\n"
			"BOVF1           RET\n"
			"BOVFLEVEL       DC      256\n"
			"; gr1の値（整数）をgr2のけた数で出力する．\n"
			"; gr2が0なら必要最小限の桁数で出力する\n"
			"WRITEINT        RPUSH\n"
			"                LD      gr7, gr0  ; flag = 0;\n"
			"                CPA     gr1, gr0  ; if(gr1>=0) goto WI1;\n"
			"                JPL     WI1\n"
			"                JZE     WI1\n"
			"                LD      gr4, gr0  ; gr1= - gr1;\n"
			"                SUBA    gr4, gr1\n"
			"                CPA     gr4, gr1\n"
			"                JZE     WI6\n"
			"                LD      gr1, gr4\n"
			"                LD      gr7, ONE  ; flag = 1;\n"
			"WI1             LD      gr6, SIX  ; p = INTBUF+6;\n"
			"                ST      gr0, INTBUF,gr6  ; *p = 0;\n"
			"                SUBA    gr6, ONE  ; p--;\n"
			"                CPA     gr1, gr0  ; if(gr1 == 0)\n"
			"                JNZ     WI2\n"
			"                LD      gr4, ZERO  ;  *p = '0';\n"
			"                ST      gr4, INTBUF,gr6\n"
			"                JUMP    WI5  ; }\n"
			"; else {\n"
			"WI2             CPA     gr1, gr0  ;  while(gr1 != 0) {\n"
			"                JZE     WI3\n"
			"                LD      gr5, gr1  ;   gr5 = gr1 - (gr1 / 10) * 10;\n"
			"                DIVA    gr1, TEN  ;   gr1 /= 10;\n"
			"                LD      gr4, gr1\n"
			"                MULA    gr4, TEN\n"
			"                SUBA    gr5, gr4\n"
			"                ADDA    gr5, ZERO  ;   gr5 += '0';\n"
			"                ST      gr5, INTBUF,gr6  ;   *p = gr5;\n"
			"                SUBA    gr6, ONE  ;   p--;\n"
			"                JUMP    WI2  ;  }\n"
			"WI3             CPA     gr7, gr0  ;  if(flag != 0) {\n"
			"                JZE     WI4\n"
			"                LD      gr4, MINUS  ;   *p = '-';\n"
			"                ST      gr4, INTBUF,gr6\n"
			"                JUMP    WI5  ;  }\n"
			"WI4             ADDA    gr6, ONE  ;  else p++;\n"
			"; }\n"
			"WI5             LAD     gr1, INTBUF,gr6  ; gr1 = p;\n"
			"                CALL    WRITESTR  ; WRITESTR();\n"
			"                RPOP\n"
			"                RET\n"
			"WI6             LAD     gr1, MMINT\n"
			"                CALL    WRITESTR  ; WRITESTR();\n"
			"                RPOP\n"
			"                RET\n"
			"MMINT           DC      '-32768'\n"
			"; gr1の値（真理値）が0なら'FALSE'を\n"
			"; 0以外なら'TRUE'をgr2のけた数で出力する．\n"
			"; gr2が0なら必要最小限の桁数で出力する\n"
			"WRITEBOOL       RPUSH\n"
			"                CPA     gr1, gr0  ; if(gr1 != 0)\n"
			"                JZE     WB1\n"
			"                LAD     gr1, WBTRUE  ;  gr1 = TRUE;\n"
			"                JUMP    WB2\n"
			"; else\n"
			"WB1             LAD     gr1, WBFALSE  ;  gr1 = FALSE;\n"
			"WB2             CALL    WRITESTR  ; WRITESTR();\n"
			"                RPOP\n"
			"                RET\n"
			"WBTRUE          DC      'TRUE'\n"
			"WBFALSE         DC      'FALSE'\n"
			"; 改行を出力する\n"
			"WRITELINE       RPUSH\n"
			"                LD      gr7, OBUFSIZE\n"
			"                LD      gr6, NEWLINE\n"
			"                ST      gr6, OBUF,gr7\n"
			"                ADDA    gr7, ONE\n"
			"                ST      gr7, OBUFSIZE\n"
			"                OUT     OBUF, OBUFSIZE\n"
			"                ST      gr0, OBUFSIZE\n"
			"                RPOP\n"
			"                RET\n"
			"FLUSH           RPUSH\n"
			"                LD      gr7, OBUFSIZE\n"
			"                JZE     FL1\n"
			"                CALL    WRITELINE\n"
			"FL1             RPOP\n"
			"                RET\n"
			"; gr1が指す番地に文字一つを読み込む\n"
			"READCHAR        RPUSH\n"
			"                LD      gr5, RPBBUF  ; if(RPBBUF != 0) {\n"
			"                JZE     RC0\n"
			"                ST      gr5, 0,gr1  ;  *gr1 = RPBBUF;\n"
			"                ST      gr0, RPBBUF  ;  RPBBUF = 0\n"
			"                JUMP    RC3  ;  return; }\n"
			"RC0             LD      gr7, INP  ; inp = INP;\n"
			"                LD      gr6, IBUFSIZE  ; if(IBUFSIZE == 0) {\n"
			"                JNZ     RC1\n"
			"                IN      IBUF, IBUFSIZE  ;  IN();\n"
			"                LD      gr7, gr0  ;  inp = 0;\n"
			"; }\n"
			"RC1             CPA     gr7, IBUFSIZE  ; if(inp == IBUFSIZE) {\n"
			"                JNZ     RC2\n"
			"                LD      gr5, NEWLINE  ;  *gr1 = '\\n';\n"
			"                ST      gr5, 0,gr1\n"
			"                ST      gr0, IBUFSIZE  ;  IBUFSIZE = INP = 0;\n"
			"                ST      gr0, INP\n"
			"                JUMP    RC3  ; }\n"
			"; else {\n"
			"RC2             LD      gr5, IBUF,gr7  ;  *gr1 = *inp++;\n"
			"                ADDA    gr7, ONE\n"
			"                ST      gr5, 0,gr1\n"
			"                ST      gr7, INP  ;  INP = inp;\n"
			"; }\n"
			"RC3             RPOP\n"
			"                RET\n"
			"; gr1が指す番地に整数値一つを読み込む\n"
			"READINT         RPUSH\n"
			"; do {\n"
			"RI1             CALL    READCHAR  ;  ch = READCHAR();\n"
			"                LD      gr7, 0,gr1\n"
			"                CPA     gr7, SPACE  ; } while(ch==' ' || ch=='\\t' || ch=='\\n');\n"
			"                JZE     RI1\n"
			"                CPA     gr7, TAB\n"
			"                JZE     RI1\n"
			"                CPA     gr7, NEWLINE\n"
			"                JZE     RI1\n"
			"                LD      gr5, ONE  ; flag = 1\n"
			"                CPA     gr7, MINUS  ; if(ch == '-') {\n"
			"                JNZ     RI4\n"
			"                LD      gr5, gr0  ;  flag = 0;\n"
			"                CALL    READCHAR  ;  ch = READCHAR();\n"
			"                LD      gr7, 0,gr1\n"
			"RI4             LD      gr6, gr0  ; v = 0;     ; }\n"
			"RI2             CPA     gr7, ZERO  ; while('0' <= ch && ch <= '9') {\n"
			"                JMI     RI3\n"
			"                CPA     gr7, NINE\n"
			"                JPL     RI3\n"
			"                MULA    gr6, TEN  ;  v = v*10+ch-'0';\n"
			"                ADDA    gr6, gr7\n"
			"                SUBA    gr6, ZERO\n"
			"                CALL    READCHAR  ;  ch = READSCHAR();\n"
			"                LD      gr7, 0,gr1\n"
			"                JUMP    RI2  ; }\n"
			"RI3             ST      gr7, RPBBUF  ; ReadPushBack();\n"
			"                ST      gr6, 0,gr1  ; *gr1 = v;\n"
			"                CPA     gr5, gr0  ; if(flag == 0) {\n"
			"                JNZ     RI5\n"
			"                SUBA    gr5, gr6  ;  *gr1 = -v;\n"
			"                ST      gr5, 0,gr1\n"
			"; }\n"
			"RI5             RPOP\n"
			"                RET\n"
			"; 入力を改行コードまで（改行コードも含む）読み飛ばす\n"
			"READLINE        ST      gr0, IBUFSIZE\n"
			"                ST      gr0, INP\n"
			"                ST      gr0, RPBBUF\n"
			"                RET\n"
			"ONE             DC      1\n"
			"SIX             DC      6\n"
			"TEN             DC      10\n"
			"SPACE           DC      #0020  ; ' '\n"
			"MINUS           DC      #002D  ; '-'\n"
			"TAB             DC      #0009  ; '\\t'\n"
			"ZERO            DC      #0030  ; '0'\n"
			"NINE            DC      #0039  ; '9'\n"
			"NEWLINE         DC      #000A  ; '\\n'\n"
			"INTBUF          DS      8\n"
			"OBUFSIZE        DC      0\n"
			"IBUFSIZE        DC      0\n"
			"INP             DC      0\n"
			"OBUF            DS      257\n"
			"IBUF            DS      257\n"
			"RPBBUF          DC      0\n"
			"    END\n"
	);
}

void add_mem(char *str2){
	static char str[MAXSTRSIZE];

	if(str2 == NULL){
		fprintf(fp2,"%s",str);
	}else{
		strcat(str,str2);
	}
}
