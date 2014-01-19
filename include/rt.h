/* ex: set softtabstop=4 shiftwidth=4 expandtab: */
#ifndef __RT_H__
#define __RT_H__

#define _GNU_SOURCE

#ifdef RTE_DEBUG
#include <mcheck.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>

#include "uthash.h"

#define T_INTEGER 0
#define T_FLOAT   1
#define T_STRING  2
#define T_DICT    3
#define T_ID      10
#define T_FUNC    11

enum openum { ADD=0, SUB, MUL, DIV, EQ, NE, GT, LT, GE, LE, AND, OR, MOD, NOT };

#define RT_ERR(args...) { printf("RT_ERROR: " args); exit(1); }

#undef uthash_fatal
#define uthash_fatal(msg) RT_ERR(msg);

/* object system */

typedef struct dict {
    char *id;
    struct obj *o;
    UT_hash_handle hh;
} dict;

typedef struct obj {
    int type;
    int64_t ival;
    float fval;
    char *sval;
    dict *dval;
    int ref;
} obj;

obj *o_new(int lineno) {
    obj *o = (obj *)calloc(sizeof(obj), 1);
    if(!o)
        RT_ERR("line %d: failed to allocate memory\n", lineno);
    return o;
}

void o_dict_del(dict *d);

void o_del(obj **o) {
    if((*o)->ref < 1) {
        if((*o)->type == T_STRING) {
            free((*o)->sval);
        }
        if((*o)->type == T_DICT) {
            o_dict_del((*o)->dval);
        }
        free(*o);
        *o = NULL;
    }
}

void o_dict_del(dict *d) {
    dict *s = NULL, *tmp = NULL;
    HASH_ITER(hh, d, s, tmp) {
        HASH_DEL(d, s);
        s->o->ref--;
        o_del(&(s->o));
        free(s->id);
        free(s);
    }
}

obj *o_int(int lineno, int64_t val) {
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

obj *o_dict(int lineno) {
    obj *o = o_new(lineno);
    o->type = T_DICT;
    return o;
}

obj *o_dict_set(int lineno, obj *od, obj *i, obj *o) {
    dict *s = NULL, **d = &(od->dval);

    if(od->type != T_DICT)
        RT_ERR("line %d: not a dictionary\n", lineno);

    HASH_FIND_STR(*d, i->sval, s);
    if(!s) {
        s = (dict *)calloc(sizeof(dict), 1);
        if(!s)
            RT_ERR("line %d: failed to allocate memory\n", lineno);
        s->id = strdup(i->sval);
        HASH_ADD_KEYPTR(hh, *d, s->id, strlen(s->id), s);
    } else {
        s->o->ref--;
        o_del(&(s->o));
    }
    s->o = o;
    s->o->ref++;

    o_del(&i);
    return o;
}

obj *o_dict_get(int lineno, obj *od, obj *i) {
    dict *s = NULL;

    if(od->type != T_DICT)
        RT_ERR("line %d: not a dictionary\n", lineno);

    HASH_FIND_STR(od->dval, i->sval, s);
    if(!s)
        RT_ERR("line %d: key not found\n", lineno);

    o_del(&i);
    return s->o;
}

obj *o_dict_test(int lineno, obj *od, obj *i) {
    dict *s = NULL;
    obj *o = o_int(lineno, 1);

    if(od->type != T_DICT)
        RT_ERR("line %d: not a dictionary\n", lineno);

    HASH_FIND_STR(od->dval, i->sval, s);
    if(!s)
        o->ival = 0;

    o_del(&i);
    return o;
}

obj *o_dict_index(int lineno, obj *o) {
    obj *n = NULL;

    switch(o->type) {
        case T_STRING: /* shouldn't happen! */
            return o;
            break;
        case T_INTEGER:
            n = o_new(lineno);
            /* FIXME */
            n->sval = (char *)calloc(sizeof(char), 256);
            if(!n->sval)
                RT_ERR("line %d: failed to allocate memory", lineno);
            snprintf(n->sval, 256, "%" PRId64, o->ival);
            n->type = T_STRING;
            break;
        case T_FLOAT:
            n = o_new(lineno);
            /* FIXME */
            n->sval = (char *)calloc(sizeof(char), 256);
            if(!n->sval)
                RT_ERR("line %d: failed to allocate memory", lineno);
            snprintf(n->sval, 256, "%f", o->fval);
            n->type = T_STRING;
            break;
        default:
            RT_ERR("line %d: invalid dictionary key\n", lineno);
            break;
    }

    o_del(&o);
    return n;
}

obj *o_clone(int lineno, obj *o) {
    obj *n = NULL;
    dict *s = NULL, *nd = NULL, *tmp = NULL;

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
        case T_DICT:
            n = o_dict(lineno);
            n->ref++;
            for(s=o->dval; s; s=s->hh.next) {
                nd = (dict *)calloc(sizeof(dict), 1);
                if(!nd)
                    RT_ERR("line %d: failed to allocate memory\n", lineno);
                nd->id = strdup(s->id);
                nd->o = o_clone(lineno, s->o);
                nd->o->ref++;
                HASH_ADD_KEYPTR(hh, tmp, nd->id, strlen(nd->id), nd);
            }
            n->dval = tmp;
            break;
    }
    o_del(&o);
    return n;
}

obj *o_typeof(int lineno, obj *o) {
    char *types[] = { "<integer>", "<float>", "<string>", "<dictionary>" };
    obj *ret = o_string(lineno, types[o->type]);
    o_del(&o);
    return ret;
}

int o_lval(int lineno, obj *o) {
    int ret = 0;
    switch(o->type) {
        case T_INTEGER:
            ret = o->ival;
            break;
        case T_FLOAT:
            ret = (int)o->fval;
            break;
        case T_STRING:
            ret = o->sval && strlen(o->sval);
            break;
        case T_DICT:
            ret = HASH_COUNT(o->dval);
            break;
        default:
            RT_ERR("line %d: undefined logic value\n", lineno);
            break;
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
                        RT_ERR("line %d: unsupported type for unary '-'\n", lineno);
                        break;
                }
                break;
            case NOT:
                switch(l->type) {
                    case T_INTEGER:
                        o->ival = !l->ival;
                        break;
                    case T_STRING:
                        o->ival = !(l->sval && strlen(l->sval));
                        o->type = T_INTEGER;
                        break;
                    case T_DICT:
                        o->ival = !HASH_COUNT(l->dval);
                        o->type = T_INTEGER;
                        break;
                    default:
                        RT_ERR("line %d: unsupported type for unary 'not'\n", lineno);
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
            case T_INTEGER:
                if(l->type == T_STRING) {
                    tmp = o_new(lineno);
                    /* FIXME */
                    tmp->sval = (char *)calloc(sizeof(char), 256);
                    if(!tmp->sval)
                        RT_ERR("line %d: failed to allocate memory", lineno);
                    snprintf(tmp->sval, 256, "%" PRId64, r->ival);
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
            default:
                RT_ERR("line %d: unsupported conversion\n", lineno);
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
                default:
                    RT_ERR("line %d: unsupported type for '+'\n", lineno);
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
                    RT_ERR("line %d: unsupported type for '-'\n", lineno);
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
                    RT_ERR("line %d: unsupported type for '*'\n", lineno);
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
                    RT_ERR("line %d: unsupported type for '/'\n", lineno);
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
                case T_STRING:
                    o->ival = strcmp(l->sval, r->sval) > 0;
                    break;
                case T_DICT:
                    o->ival = HASH_COUNT(l->dval) > HASH_COUNT(r->dval);
                    break;
                default:
                    RT_ERR("line %d: unsupported type for '>'\n", lineno);
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
                case T_STRING:
                    o->ival = strcmp(l->sval, r->sval) >= 0;
                    break;
                case T_DICT:
                    o->ival = HASH_COUNT(l->dval) >= HASH_COUNT(r->dval);
                    break;
                default:
                    RT_ERR("line %d: unsupported type for '>='\n", lineno);
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
                case T_STRING:
                    o->ival = strcmp(l->sval, r->sval) < 0;
                    break;
                case T_DICT:
                    o->ival = HASH_COUNT(l->dval) < HASH_COUNT(r->dval);
                    break;
                default:
                    RT_ERR("line %d: unsupported type for '<'\n", lineno);
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
                case T_STRING:
                    o->ival = strcmp(l->sval, r->sval) <= 0;
                    break;
                case T_DICT:
                    o->ival = HASH_COUNT(l->dval) <= HASH_COUNT(r->dval);
                    break;
                default:
                    RT_ERR("line %d: unsupported type for '<='\n", lineno);
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
                case T_STRING:
                    o->ival = strcmp(l->sval, r->sval) == 0;
                    break;
                case T_DICT:
                    /* FIXME? */
                    o->ival = HASH_COUNT(l->dval) == HASH_COUNT(r->dval);
                    break;
                default:
                    RT_ERR("line %d: unsupported type for '='\n", lineno);
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
                case T_STRING:
                    o->ival = strcmp(l->sval, r->sval) != 0;
                    break;
                case T_DICT:
                    /* FIXME? */
                    o->ival = HASH_COUNT(l->dval) != HASH_COUNT(r->dval);
                    break;
                default:
                    RT_ERR("line %d: unsupported type for '<>'\n", lineno);
                    break;
            }
            break;
        case AND:
            o->type = T_INTEGER;
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival && r->ival;
                    break;
                case T_DICT:
                    o->ival = HASH_COUNT(l->dval) && HASH_COUNT(r->dval);
                    break;
                default:
                    RT_ERR("line %d: unsupported type for 'and'\n", lineno);
                    break;
            }
            break;
        case OR:
            o->type = T_INTEGER;
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival || r->ival;
                    break;
                case T_DICT:
                    o->ival = HASH_COUNT(l->dval) || HASH_COUNT(r->dval);
                    break;
                default:
                    RT_ERR("line %d: unsupported type for 'or'\n", lineno);
                    break;
            }
            break;
        case MOD:
            o->type = T_INTEGER;
            switch(l->type) {
                case T_INTEGER:
                    o->ival = l->ival % r->ival;
                    break;
                default:
                    RT_ERR("line %d: unsupported type for 'mod'\n", lineno);
                    break;
            }
            break;
    } /* switch(op) */

    o_del(&l);
    o_del(&r);

    return o;
}

void o_print(obj *o) {
    dict *s = NULL;

    switch(o->type) {
        case T_INTEGER:
            printf("%" PRId64, o->ival);
            break;
        case T_FLOAT:
            printf("%f", o->fval);
            break;
        case T_STRING:
            printf("%s", o->sval);
            break;
        case T_DICT:
            if(o->dval) {
                printf("{ ");
                for(s=o->dval; s; s=s->hh.next) {
                    printf("%s: ", s->id);
                    o_print(s->o);
                    if(s->hh.next) {
                        printf(", ");
                    }
                }
                printf(" }");
            } else {
                printf("{}");
            }
            break;
    }
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
        /* this is just to avoid freeing this object because
         * it is associated to a stored variable */
        s->o->ref++;
    } else {
        if(o == s->o)
            /* in fact we can */
            RT_ERR("line %d: can't assign to itself\n", lineno);

        /* replace the existing object, it already has a reference */
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
            case T_DICT:
                s->o->dval = o->dval;
                break;
        }
        s->o->type = o->type;
    }
    s->lineno = lineno;

    o_del(&o);
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

    o->ref++;
    HASH_ITER(hh, *ctx, s, tmp) {
        HASH_DEL(*ctx, s);
        s->o->ref--;
        o_del(&s->o);
        free(s);
    }
    o->ref--;

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
        o = va_arg(argv, obj *);
        o_print(o);
        o_del(&o);
    }
    va_end(argv);
    printf("\n");
}

/* C entry point */
int main() {
#ifdef RTE_DEBUG
    printf("** RTE_DEBUG ON **\n");
    mtrace();
    return _ep();
    muntrace();
#else
    return _ep();
#endif
}

#endif

