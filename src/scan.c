#include "token-list.h"

/* keyword list */
struct KEY key[KEYWORDSIZE] = {
		{"and", TAND },
		{"array", TARRAY },
		{"begin", TBEGIN },
		{"boolean", TBOOLEAN},
		{"break", TBREAK },
		{"call", TCALL },
		{"char", TCHAR },
		{"div", TDIV },
		{"do", TDO },
		{"else", TELSE },
		{"end", TEND },
		{"false", TFALSE },
		{"if", TIF },
		{"integer", TINTEGER},
		{"not", TNOT },
		{"of", TOF },
		{"or", TOR },
		{"procedure", TPROCEDURE},
		{"program", TPROGRAM},
		{"read", TREAD },
		{"readln", TREADLN},
		{"return", TRETURN},
		{"then", TTHEN },
		{"true", TTRUE },
		{"var", TVAR },
		{"while", TWHILE },
		{"write", TWRITE },
		{"writeln", TWRITELN}
};

/* string of each token */
char *tokenstr[NUMOFTOKEN+1] = {
		"",
		"NAME", "program", "var", "array", "of", "begin", "end", "if", "then",
		"else", "procedure", "return", "call", "while", "do", "not", "or",
		"div", "and", "char", "integer", "boolean", "readln", "writeln", "true",
		"false", "NUMBER", "STRING", "+", "-", "*", "=", "<>", "<", "<=", ">",
		">=", "(", ")", "[", "]", ":=", ".", ",", ":", ";", "read","write",
		"break"
};

/* cbuf:character lookahead  line_num:line number */
int cbuf,line_num=0;
/* string buffer */
char string_attr[MAXSTRSIZE];
/* num buffer */
int num_attr;
FILE *fp;

/* output error message */
void error(char *mes) {
	printf("\nERROR: %s line(%d)\n", mes, get_linenum());
}

/* open file and get first character */
int init_scan(char *filename){
	if((fp = fopen(filename,"r")) == NULL){
		return -1;
	}

	cbuf = fgetc(fp);
	return 0;
}

/* scan(return token) */
int scan(){
	if(line_num == 0){
		line_num++;
	}

	switch(cbuf){
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
		return name_key();
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return un_int();
	case '\'':
		return str();
	case '+':
		cbuf = fgetc(fp);
		return TPLUS;
	case '-':
		cbuf = fgetc(fp);
		return TMINUS;
	case '*':
		cbuf = fgetc(fp);
		return TSTAR;
	case '=':
		cbuf = fgetc(fp);
		return TEQUAL;
	case '<':
		return tle();
	case '>':
		return tgr();
	case '(':
		cbuf = fgetc(fp);
		return TLPAREN;
	case ')':
		cbuf = fgetc(fp);
		return TRPAREN;
	case '[':
		cbuf = fgetc(fp);
		return TLSQPAREN;
	case ']':
		cbuf = fgetc(fp);
		return TRSQPAREN;
	case ':':
		return colon();
	case '.':
		cbuf = fgetc(fp);
		return TDOT;
	case ',':
		cbuf = fgetc(fp);
		return TCOMMA;
	case ';':
		cbuf = fgetc(fp);
		return TSEMI;
	case '{':
	case '/':
		return comment();
	case ' ':
		cbuf = fgetc(fp);
		return scan();
	case '\t':
		cbuf = fgetc(fp);
		return scan();
	case '\n':
	case '\r':
		return enter();
	case '!':
	case '"':
	case '#':
	case '$':
	case '%':
	case '&':
	case '?':
	case '@':
	case '\\':
	case '^':
	case '_':
	case '`':
	case '|':
	case '~':
		error("unknown character");
		return -1;
	case EOF:
		return -1;
	}
	/* ignore non-display character */
	cbuf = fgetc(fp);
	return scan();
}

int get_linenum(){
	return line_num;
}

void end_scan(){
	fclose(fp);
}

/* name or keyword function */
int name_key(){
	int i;

	for(i=0; i<MAXSTRSIZE; i++){
		/* check size */
		if(i == MAXSTRSIZE-1){
			error("STR size.");
			return -1;
		}

		string_attr[i] = cbuf;
		cbuf = fgetc(fp);

		if(!((cbuf >= '0' && cbuf <= '9') || (cbuf >= 'A' && cbuf <= 'Z') || (cbuf >= 'a' && cbuf <= 'z'))){
			i++;
			break;
		}
	}
	string_attr[i] = '\0';

	/* keyword check */
	for(i=0; i<KEYWORDSIZE; i++){
		if(strcmp(key[i].keyword,string_attr) == 0){
			return key[i].keytoken;
		}
	}

	return TNAME;
}

/* TNUMBER function */
int un_int(){
	int i;

	for(i=0; i<5; i++){
		string_attr[i] = cbuf;
		cbuf = fgetc(fp);

		if(!(cbuf >= '0' && cbuf <= '9')){
			i++;
			break;
		}
	}
	string_attr[i] = '\0';

	num_attr = atoi(string_attr);
	/* size check */
	if(num_attr > 32767){
		error("INT size.");
		return -1;
	}

	return TNUMBER;
}

/* TSTRING function */
int str(){
	int i;

	for(i=0; i<MAXSTRSIZE; i++){
		/* size check */
		if(i == MAXSTRSIZE-1){
			error("STR size.");
			return -1;
		}

		cbuf = fgetc(fp);
		if((cbuf >= 0x20 && cbuf <= 0x7e) || cbuf == '\t'){
			if(cbuf == '\''){ /* if '*' */
				if((cbuf = fgetc(fp)) == '\''){ /* if '*'' */
					string_attr[i++] = cbuf;

					if(i == MAXSTRSIZE-1){
						error("STR size.(2)");
						return -1;
					}
				}else{ /* string = '*' */
					string_attr[i] = '\0';
					return TSTRING;
				}
			}else{ /* string = '* */
				string_attr[i] = cbuf;
			}
		}else if(cbuf == '\n' || cbuf == '\r'){
			error("string error");
			return -1;
		}
	}
	/* never reach */
	return -1;
}

int tle(){ /*  < : symbol  */
	cbuf = fgetc(fp);

	if(cbuf == '>'){ /* <> */
		cbuf = fgetc(fp);
		return TNOTEQ;
	}else if(cbuf == '='){ /* <= */
		cbuf = fgetc(fp);
		return TLEEQ;
	}

	/* < */
	return TLE;
}

int tgr(){ /*  > : symbol  */
	cbuf = fgetc(fp);

	if(cbuf == '='){ /* >= */
		cbuf = fgetc(fp);
		return TGREQ;
	}
	/* > */
	return TGR;
}

int colon(){ /*  : : symbol  */
	cbuf = fgetc(fp);

	if(cbuf == '='){ /* := */
		cbuf = fgetc(fp);
		return TASSIGN;
	}
	/* : */
	return TCOLON;
}

int comment(){
	if(cbuf == '{'){ /* {*} */
		while((cbuf = fgetc(fp)) != '}'){
			if(cbuf == EOF){
				error("comment error.");
				return -1;
			}
		}
		cbuf = fgetc(fp);
		return scan();
	}

	//   /***/
	if((cbuf = fgetc(fp)) != '*'){
		error("comment error.(2)");
		return -1;
	}

	cbuf = fgetc(fp);
	while(1){
		if(cbuf == '*'){
			cbuf = fgetc(fp);
			if(cbuf == '/'){
				cbuf = fgetc(fp);
				return scan();
			}else if(cbuf == EOF){
				error("comment error.(3)");
				return -1;
			}
		}else if(cbuf == EOF){
			error("comment error.(4)");
			return -1;
		}else{
			cbuf = fgetc(fp);
		}
	}
}

/* indention function */
int enter(){
	if(cbuf == '\n'){ /* \n */
		cbuf = fgetc(fp);
		if(cbuf == '\r'){ /* \n\r */
			cbuf = fgetc(fp);
			line_num++;
			return scan();
		}
		line_num++;
		return scan();
	}

	/* \r */
	cbuf = fgetc(fp);
	if(cbuf == '\n'){ /* \r\n */
		cbuf = fgetc(fp);
		line_num++;
		return scan();
	}
	line_num++;
	return scan();
}
