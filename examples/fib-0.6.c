/*
 * jtc ver 0.6.1
 * source: examples/fib.jt
 */
#include "rt.h"

obj *_fib_1 (obj * n);
obj *_main_1 ();

obj *
_fib_1 (obj * n)
{
  st *_ctx = NULL;
  store (&_ctx, 4, 2, n);
  if (o_lval (8, o_op (5, LE, retrieve (&_ctx, 5, 2), o_int (5, 1))))
    {
      return o_return (&_ctx, retrieve (&_ctx, 6, 2));
    }

  return o_return (&_ctx,
		   o_op (8, ADD,
			 _fib_1 (o_op
				 (8, SUB, retrieve (&_ctx, 8, 2),
				  o_int (8, 1))), _fib_1 (o_op (8, SUB,
								retrieve
								(&_ctx, 8, 2),
								o_int (8,
								       2)))));

  return o_return (&_ctx, o_int (0, 0));
}

obj *
_main_1 ()
{
  st *_ctx = NULL;
  println (1, _fib_1 (o_int (11, 10)));
  return o_return (&_ctx, o_int (12, 0));

  return o_return (&_ctx, o_int (0, 0));
}

int
_ep ()
{
  obj *o = _main_1 ();
  return o_lval (0, o);
}

/* EOF */
