#ifndef __RT_H__
#define __RT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "uthash.h"

#define T_INTEGER 0
#define T_FLOAT  1
#define T_STRING  2
#define T_ID      10
#define T_FUNC    11

enum openum { ADD=0, SUB, MUL, DIV, EQ, NE, GT, LT, GE, LE, AND, OR, MOD, NOT };


#define RT_ERR(args...) { printf("RT_ERROR: " args); exit(1); }

#undef uthash_fatal
#define uthash_fatal(msg) RT_ERR(msg);

/* object system */
typedef struct obj {
    int type;
    int ival;
    float fval;
    char *sval;
    int ref;
} obj;

obj *o_new(int lineno) {
    obj *o = (obj *)calloc(sizeof(obj), 1);
    if(!o)
        RT_ERR("line %d: failed to allocate memory\n", lineno);
    return o;
}

void o_del(obj **o) {
    if((*o)->ref < 1) {
        if((*o)->type == T_STRING) {
            free((*o)->sval);
        }
        free(*o);
    }
    *o = NULL;
}

obj *o_int(int lineno, int val) {
    obj *o = o_new(lineno);
    o->type = T_INTEGER;
    o->ival = val;
    return o;
}

obj *o_float(int lineno, float val) {
    obj *o = o_new(lineno);
    o->type = T_FLOAT;
    o->fval = val;
    return o;
}

obj *o_string(int lineno, const char *val) {
    obj *o = o_new(lineno);
    o->type = T_STRING;
    o->sval = strdup(val);
    return o;
}

obj *o_clone(int lineno, obj *o) {
    obj *n = NULL;
    switch(o->type) {
        case T_INTEGER:
            n = o_int(lineno, o->ival);
        break;
        case T_FLOAT:
            n = o_float(lineno, o->fval);
        break;
        case T_STRING:
            n = o_string(lineno, o->sval);
        break;
    }
    o_del(&o);
    return n;
}

int o_lval(int lineno, obj *o) {
    int ret = 0;
    switch(o->type) {
        case T_INTEGER:
            ret = o->ival;
            break;
        case T_FLOAT:
            ret = o->fval;
            break;
        case T_STRING:
            ret = o->sval && strlen(o->sval);
            break;
        default:
            RT_ERR("line %d: undefined logic value\n", lineno);
    }
    o_del(&o);
    return ret;
}

obj *o_op(int lineno, enum openum op, obj *l, obj *r) {
    obj *o = o_new(lineno), *tmp = NULL;
    o->type = l->type;

    /* unary operator */
    if(r == NULL) {
        switch(op) {
            default:
            break;
            case SUB: /* minus */
                switch(l->type) {
                    case T_INTEGER:
                        o->ival = -l->ival;
                    break;
                    case T_FLOAT:
                        o->fval = -l->fval;
                    break;
                    default:
                        RT_ERR("line %d: unsuppored type for unary '-'\n", lineno);
                    break;
                }
            break;
            case NOT:
                switch(l->type) {
                    case T_INTEGER:
                        o->ival = !l->ival;
                    break;
                    case T_STRING:
                        o->ival = o->sval && strlen(o->sval);
                        o->type = T_INTEGER;
                    break;
                    default:
                        RT_ERR("line %d: unsuppored type for unary 'not'\n", lineno);
                    break;
                }
            break;
        }

        o_del(&l);
        return o;
    }

    /* type conversions */
    if(l->type != r->type) {
        switch(r->type) {
            case T_STRING:
                RT_ERR("line %d: unsuported conversion\n", lineno);
            break;
            case T_INTEGER:
                if(l->type == T_STRING) {
                    tmp = o_new(lineno);
                    /* FIXME */
                    tmp->sval = (char *)calloc(sizeof(char), 256);
                    if(!tmp->sval)
                        RT_ERR("line %d: failed to allocate memory", lineno);
                    snprintf(tmp->sval, 256, "%i", r->ival);
                    tmp->type = T_STRING;
                } else { /* to float */
                    tmp = o_float(lineno, (float)r->ival);
                }
            break;
            case T_FLOAT:
                if(l->type == T_STRING) {
                    tmp = o_new(lineno);
                    /* FIXME */
                    tmp->sval = (char *)calloc(sizeof(char), 256);
                    if(!tmp->sval)
                        RT_ERR("line %d: failed to allocate memory", lineno);
                    snprintf(tmp->sval, 256, "%f", r->fval);
                    tmp->type = T_STRING;
                } else { /* to integer */
                    tmp = o_int(lineno, (int)r->fval);
                }
            break;
        }
        o_del(&r);
        r = tmp;
    }

    switch(op) {
        default:
        break;
        case ADD:
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival + r->ival;
                break;
                case T_FLOAT:
                    o->fval = l->fval + r->fval;
                break;
                case T_STRING:
                    o->sval = (char *)calloc(sizeof(char), strlen(l->sval)+strlen(r->sval)+1);
                    strncpy(o->sval, l->sval, strlen(l->sval));
                    strncpy(o->sval+strlen(l->sval), r->sval, strlen(r->sval));
                break;
            }
        break;
        case SUB:
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival - r->ival;
                break;
                case T_FLOAT:
                    o->fval = l->fval - r->fval;
                break;
                default:
                    RT_ERR("line %d: unsuppored type for '-'\n", lineno);
                break;
            }
        break;
        case MUL:
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival * r->ival;
                break;
                case T_FLOAT:
                    o->fval = l->fval * r->fval;
                break;
                default:
                    RT_ERR("line %d: unsuppored type for '*'\n", lineno);
                break;
            }
        break;
        case DIV:
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival / r->ival;
                break;
                case T_FLOAT:
                    o->fval = l->fval / r->fval;
                break;
                default:
                    RT_ERR("line %d: unsuppored type for '/'\n", lineno);
                break;
            }
        break;
        case GT:
            o->type = T_INTEGER;
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival > r->ival;
                break;
                case T_FLOAT:
                    o->ival = l->fval > r->fval;
                break;
                default:
                    o->ival = strcmp(l->sval, r->sval) > 0;
                break;
            }
        break;
        case GE:
            o->type = T_INTEGER;
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival >= r->ival;
                break;
                case T_FLOAT:
                    o->ival = l->fval >= r->fval;
                break;
                default:
                    o->ival = strcmp(l->sval, r->sval) >= 0;
                break;
            }
        break;
        case LT:
            o->type = T_INTEGER;
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival < r->ival;
                break;
                case T_FLOAT:
                    o->ival = l->fval < r->fval;
                break;
                default:
                    o->ival = strcmp(l->sval, r->sval) < 0;
                break;
            }
        break;
        case LE:
            o->type = T_INTEGER;
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival <= r->ival;
                break;
                case T_FLOAT:
                    o->ival = l->fval <= r->fval;
                break;
                default:
                    o->ival = strcmp(l->sval, r->sval) <= 0;
                break;
            }
        break;
        case EQ:
            o->type = T_INTEGER;
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival == r->ival;
                break;
                case T_FLOAT:
                    o->ival = l->fval == r->fval;
                break;
                default:
                    o->ival = strcmp(l->sval, r->sval) == 0;
                break;
            }
        break;
        case NE:
            o->type = T_INTEGER;
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival != r->ival;
                break;
                case T_FLOAT:
                    o->ival = l->fval != r->fval;
                break;
                default:
                    o->ival = strcmp(l->sval, r->sval) != 0;
                break;
            }
        break;
        case AND:
            o->type = T_INTEGER;
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival && r->ival;
                break;
                default:
                    RT_ERR("line %d: unsuppored type for 'and'\n", lineno);
            }
        break;
        case OR:
            o->type = T_INTEGER;
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival || r->ival;
                break;
                default:
                    RT_ERR("line %d: unsuppored type for 'or'\n", lineno);
            }
        break;
        case MOD:
            o->type = T_INTEGER;
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival % r->ival;
                break;
                default:
                    RT_ERR("line %d: unsuppored type for 'mod'\n", lineno);
            }
        break;
    } /* switch(op) */

    o_del(&l);
    o_del(&r);

    return o;
}

/* object storage */
typedef struct st {
	int index;
    int lineno;
    obj *o;
	UT_hash_handle hh;
} st;

obj *store(st **ctx, int lineno, int id, obj *o) {
    st *s = NULL;

    HASH_FIND_INT(*ctx, &id, s);
    if(!s) {
        s = (st *)calloc(sizeof(st), 1);
        if(!s)
            RT_ERR("line %d: failed to allocate memory\n", lineno);
        s->index = id;
		HASH_ADD_INT(*ctx, index, s);
        s->o = o;
    } else {
        if(s->o->type == T_STRING) {
            free(s->o->sval);
        }
        switch(o->type) {
            case T_INTEGER:
                s->o->ival = o->ival;
            break;
            case T_FLOAT:
                s->o->fval = o->fval;
            break;
            case T_STRING:
                s->o->sval = strdup(o->sval);
            break;
        }
        s->o->type = o->type;
        o_del(&o);
    }
    s->lineno = lineno;
    /* this is just to avoid freeing this object because
     * it is associated to a stored variable */
    s->o->ref++;

    return s->o;
}

obj *retrieve(st **ctx, int lineno, int id) {
    st *s = NULL;

    HASH_FIND_INT(*ctx, &id, s);
	if(!s)
        RT_ERR("line %d: undefined identifier\n", lineno);
    return s->o;
}

obj *o_return(st **ctx, obj *o) {
    st *s = NULL, *tmp = NULL;

    HASH_ITER(hh, *ctx, s, tmp) {
        HASH_DEL(*ctx, s);
		s->o->ref--;
		if(s->o != o) {
			o_del(&s->o);
		}
        free(s);
    }

    return o;
}

int _ep();

/* high level language functions */
void println(int argc, ...) {
    va_list argv;
    int i;
    obj *o;

    va_start(argv, argc);
    for(i=0; i<argc; i++) {
        o = (obj *)va_arg(argv, obj *);
        switch(o->type) {
            case T_INTEGER:
                printf("%d", o->ival);
            break;
            case T_FLOAT:
                printf("%f", o->fval);
            break;
            case T_STRING:
                printf("%s", o->sval);
            break;
        }
        o_del(&o);
    }
    va_end(argv);
    printf("\n");
}

obj *o_typeof(int lineno, obj *o) {
    char *types[] = { "<integer>", "<float>", "<string>" };
    obj *ret = o_string(lineno, types[o->type]);
    o_del(&o);
    return ret;
}

/* C entry point */
int main() {
    return _ep();
}

#endif

