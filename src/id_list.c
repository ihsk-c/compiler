#include "token-list.h"

struct ID *globalidroot, *localidroot, *allidroot;

void init_idtab(){ /* Initialise the table */
	allidroot = NULL;
	globalidroot = NULL;
	localidroot = NULL;
}

struct ID *search_idtab(struct ID *root, char *np) { /* search the name pointed by np */
	struct ID *p;

	for(p = root; p != NULL; p = p->nextp) {
		if(strcmp(np, p->name) == 0) return(p);
	}
	return(NULL);
}

int register_name(char *namep, char *procnp, int ispr){ /*
    register name(namep), procesname(procnp), is_parameter(ispr)
*/
	struct ID *p;

	if(strcmp(procnp,"") == 0){
		if(search_idtab(globalidroot, namep) != NULL){
			printf("double define %s(%d)\n", namep, get_linenum());
			return ERROR;
		}
	}else{
		if(search_idtab(localidroot, namep) != NULL){
			printf("double define %s(%d)\n", namep, get_linenum());
			return ERROR;
		}
	}

	if ((p = (struct ID *)malloc(sizeof(struct ID))) == NULL) {
		printf("can not malloc in resister_id\n");
		return ERROR;
	}
	if ((p->name = (char*)malloc(sizeof(strlen(namep)+1))) == NULL){
		printf("can not malloc-2 in resister_id\n");
		return ERROR;
	}
	if ((p->procname = (char*)malloc(sizeof(strlen(procnp)+1))) == NULL){
		printf("can not malloc-3 in resister_id\n");
		return ERROR;
	}

	strcpy(p->name, namep);
	strcpy(p->procname, procnp);

	p->itp = NULL;
	p->ispara = ispr;
	p->deflinenum = get_linenum();
	p->irefp = NULL;

	if(strcmp(procnp,"") == 0){
		p->nextp = globalidroot;
		globalidroot = p;
	}else{
		p->nextp = localidroot;
		localidroot = p;
	}

	return NORMAL;
}

int register_type(struct ID *root, char *namep, int type, int arsize){/*
    register type and arsize
*/
	struct ID *p;

	if(namep != NULL){
		if((p = search_idtab(root, namep)) == NULL){
			printf("not defined %s(%d) in register_type\n", namep, get_linenum());
			return ERROR;
		}
		if ((p->itp = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
			printf("can not malloc in resister_type\n");
			return ERROR;
		}

		switch(type){
		case TPARRAYINT:
		case TPARRAYCHAR:
		case TPARRAYBOOL:
			p->itp->ttype = TPARRAY;
			p->itp->arraysize = arsize;
			if ((p->itp->etp = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
				printf("can not malloc-2 in resister_type\n");
				return ERROR;
			}
			p->itp->paratp = NULL;
			p->itp->etp->arraysize = 0;
			p->itp->etp->etp = NULL;
			p->itp->etp->paratp = NULL;
			switch(type){
			case TPARRAYINT:
				p->itp->etp->ttype = TPINT;
				break;
			case TPARRAYCHAR:
				p->itp->etp->ttype = TPCHAR;
				break;
			case TPARRAYBOOL:
				p->itp->etp->ttype = TPBOOL;
				break;
			}
			break;
			default:
				p->itp->ttype = type;
				p->itp->arraysize = 0;
				p->itp->etp = NULL;
				p->itp->paratp = NULL;
				break;
		}
	}else{
		for(p=root; p!=NULL; p=p->nextp){
			if(p->itp == NULL){
				if ((p->itp = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
					printf("can not malloc in resister_type\n");
					return ERROR;
				}

				switch(type){
				case TPARRAYINT:
				case TPARRAYCHAR:
				case TPARRAYBOOL:
					p->itp->ttype = TPARRAY;
					p->itp->arraysize = arsize;
					if ((p->itp->etp = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
						printf("can not malloc-2 in resister_type\n");
						return ERROR;
					}
					p->itp->paratp = NULL;
					p->itp->etp->arraysize = 0;
					p->itp->etp->etp = NULL;
					p->itp->etp->paratp = NULL;
					switch(type){
					case TPARRAYINT:
						p->itp->etp->ttype = TPINT;
						break;
					case TPARRAYCHAR:
						p->itp->etp->ttype = TPCHAR;
						break;
					case TPARRAYBOOL:
						p->itp->etp->ttype = TPBOOL;
						break;
					}
					break;
					default:
						p->itp->ttype = type;
						p->itp->arraysize = 0;
						p->itp->etp = NULL;
						p->itp->paratp = NULL;
						break;
				}
			}
		}
	}

	return NORMAL;
}

int register_paratype(char *namep){/*
    register parameter type
*/
	struct ID *p;
	struct TYPE **q;

	if((p = search_idtab(globalidroot, namep)) == NULL){
		printf("not defined %s(%d)\n", namep, get_linenum());
		return ERROR;
	}
	for(q=&p->itp->paratp; *q!=NULL; q=&(*q)->paratp){}
	for(p=localidroot; p!=NULL; p=p->nextp){
		if(p->ispara == 1){
			if ((*q = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
				printf("can not malloc in resister_paratype\n");
				return ERROR;
			}
			if(((*q)->ttype = p->itp->ttype) == TPARRAY){
				if (((*q)->etp = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
					printf("can not malloc-2 in resister_paratype\n");
					return ERROR;
				}
				(*q)->arraysize = p->itp->arraysize;
				(*q)->etp->ttype = p->itp->etp->ttype;
				(*q)->etp->arraysize = 0;
				(*q)->etp->etp = NULL;
				(*q)->etp->paratp = NULL;
			}else{
				(*q)->etp = NULL;
			}
			(*q)->paratp = NULL;
			q = &(*q)->paratp;
			p->ispara = 2;
		}
	}

	return NORMAL;
}

int register_rline(struct ID *root, char *namep){/*
    register refline
*/
	struct ID *p;
	struct LINE **q;

	if((p = search_idtab(root, namep)) == NULL){
		return ERROR;
	}
	for(q=&(p->irefp); *q!=NULL; q=&((*q)->nextlinep)){}
	if ((*q = (struct LINE *)malloc(sizeof(struct LINE))) == NULL) {
		printf("can not malloc in resister_rline\n");
		return ERROR;
	}
	(*q)->reflinenum = get_linenum();
	(*q)->nextlinep = NULL;

	if(p->itp->ttype == TPARRAY){
		switch(p->itp->etp->ttype){
		case TPINT:
			return TPARRAYINT;
		case TPCHAR:
			return TPARRAYCHAR;
		case TPBOOL:
			return TPARRAYBOOL;
		}
	}
	return p->itp->ttype;
}

int num_prm(char *prcn){/*
   return num of parameters
*/
	struct ID *p;
	struct TYPE *q;
	int i=0;

	if((p = search_idtab(globalidroot,prcn)) == NULL){
		printf("%s didn't defined\n",prcn);
		return -1;
	}

	for(q=p->itp->paratp; q!=NULL; q=q->paratp){
		i++;
	}

	return i;
}

int check_prm(char *prcn, int *prm){/*
    check parameter's type and num of parameter's
*/
	struct ID *p;
	struct TYPE *q;
	int i=0;

	if((p = search_idtab(globalidroot,prcn)) == NULL){
		printf("%s don't defined\n",prcn);
		return ERROR;
	}

	for(q=p->itp->paratp; q!=NULL; q=q->paratp){
		if(q->ttype != prm[i++]){
			error("parameter error");
			return ERROR;
		}
	}

	return NORMAL;
}

void collect_id(struct ID *root){/*
     synthesis root and allidroot
*/
	struct ID *p;

	if(root == NULL){
		return;
	}

	for(p=root; p->nextp!=NULL; p=p->nextp){}

	p->nextp = allidroot;
	allidroot = root;

	if(root == localidroot){
		localidroot = NULL;
	}else{
		globalidroot = NULL;
	}

	return;
}

void release_id(){/*
    free all
*/
	struct ID *p;
	struct LINE *q, *r;

	p = allidroot;
	while(allidroot != NULL){
		allidroot = p->nextp;

		free(p->name);
		free(p->procname);
		if(p->itp != NULL)release_type(p->itp);
		q = p->irefp;
		while(q != NULL){
			r = q->nextlinep;
			free(q);
			q = r;
		}
		free(p);

		p = allidroot;
	}
}

void release_type(struct TYPE *p){/*
  free p
*/
	if(p->etp != NULL)release_type(p->etp);
	if(p->paratp != NULL)release_type(p->paratp);
	free(p->etp);
	free(p->paratp);
}
