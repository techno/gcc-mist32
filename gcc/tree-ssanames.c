/* Generic routines for manipulating SSA_NAME expressions
   Copyright (C) 2003 Free Software Foundation, Inc.
                                                                                
This file is part of GCC.
                                                                                
GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.
                                                                                
GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
                                                                                
You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */
                                                                                
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "varray.h"
#include "ggc.h"

/* Rewriting a function into SSA form can create a huge number of SSA_NAMEs,
   many of which may be thrown away shortly after their creation if jumps
   were threaded through PHI nodes.  

   While our garbage collection mechanisms will handle this situation, it
   is extremely wasteful to create nodes and throw them away, especially
   when the nodes can be reused.

   For PR 8361, we can significantly reduce the number of nodes allocated
   and thus the total amount of memory allocated by managing SSA_NAMEs a
   little.  This additionally helps reduce the amount of work done by the
   garbage collector.  Similar results have been seen on a wider variety
   of tests (such as the compiler itself).

   Right now we maintain our free list on a per-function basis.  It may
   or may not make sense to maintain the free list for the duration of
   a compilation unit. 

   External code should rely solely upon HIGHEST_SSA_VERSION and the
   externally defined functions.  External code should not know about
   the details of the free list management.

   External code should also not assume the version number on nodes is
   monotonically increasing.  We reuse the version number when we
   reuse an SSA_NAME expression.  This helps keep arrays and bitmaps
   more compact.

   We could also use a zone allocator for these objects since they have
   a very well defined lifetime.  If someone wants to experiment with that
   this is the place to try it.  */
   
/* Next SSA version number to be allocated.  */
unsigned int highest_ssa_version;
                                                                                
/* Free list of SSA_NAMEs.  This list is wiped at the end of each function
   after we leave SSA form.  */
static GTY (()) tree free_ssanames;

/* Version numbers with special meanings.  We start allocating new version
   numbers after the special ones.  */
#define UNUSED_NAME_VERSION 0

#ifdef GATHER_STATISTICS
unsigned int ssa_name_nodes_reused;
unsigned int ssa_name_nodes_created;
#endif

/* Initialize management of SSA_NAMEs.  */

void
init_ssanames (void)
{
  highest_ssa_version = UNUSED_NAME_VERSION + 1;
  free_ssanames = NULL;
}

/* Finalize management of SSA_NAMEs.  */

void
fini_ssanames (void)
{
  free_ssanames = NULL;
}

/* Dump some simple statistics regarding the re-use of SSA_NAME nodes.  */

#ifdef GATHER_STATISTICS
void
ssanames_print_statistics (void)
{
  fprintf (stderr, "SSA_NAME nodes allocated: %u\n", ssa_name_nodes_created);
  fprintf (stderr, "SSA_NAME nodes reused: %u\n", ssa_name_nodes_reused);
}
#endif

/* Return an SSA_NAME node for variable VAR defined in statement STMT.
   STMT may be an empty statement for artificial references (e.g., default
   definitions created when a variable is used without a preceding
   definition).  */

tree
make_ssa_name (tree var, tree stmt)
{
  tree t;

#if defined ENABLE_CHECKING
  if ((!DECL_P (var)
       && TREE_CODE (var) != INDIRECT_REF)
      || (!IS_EXPR_CODE_CLASS (TREE_CODE_CLASS (TREE_CODE (stmt)))
	  && TREE_CODE (stmt) != PHI_NODE))
    abort ();
#endif

  /* If our free list has an element, then use it.  Also reuse the
     SSA version number of the element on the free list which helps
     keep sbitmaps and arrays sized HIGHEST_SSA_VERSION smaller.  */
  if (free_ssanames)
    {
      unsigned int save_version;

      t = free_ssanames;
      free_ssanames = TREE_CHAIN (free_ssanames);
#ifdef GATHER_STATISTICS
      ssa_name_nodes_reused++;
#endif

      /* Clear the node so that it looks just like one we would have
	 received from make_node.  */
      save_version = SSA_NAME_VERSION (t);
      memset (t, 0, tree_size (t));
      TREE_SET_CODE (t, SSA_NAME);
      SSA_NAME_VERSION (t) = save_version;
    }
  else
    {
      t = make_node (SSA_NAME);
      SSA_NAME_VERSION (t) = highest_ssa_version++;
#ifdef GATHER_STATISTICS
      ssa_name_nodes_created++;
#endif
    }

  TREE_TYPE (t) = TREE_TYPE (var);
  SSA_NAME_VAR (t) = var;
  SSA_NAME_DEF_STMT (t) = stmt;

  return t;
}

/* We no longer need the SSA_NAME expression VAR, release it so that
   it may be reused. 

   Note it is assumed that no calls to make_ssa_name will be made
   until all uses of the ssa name are released and that the only
   use of the SSA_NAME expression is to check its SSA_NAME_VAR.  All
   other fields must be assumed clobbered.  */

void
release_ssa_name (tree var)
{
  /* release_ssa_name can be called multiple times on a single SSA_NAME.
     However, it should only end up on our free list one time.   We
     keep a status bit in the SSA_NAME node itself to indicate it has
     been put on the free list. 

     Note that once on the freelist you can not reference the SSA_NAME's
     defining statement.  */
  if (! SSA_NAME_IN_FREE_LIST (var))
    {
      SSA_NAME_IN_FREE_LIST (var) = 1;
      TREE_CHAIN (var) = free_ssanames;
      free_ssanames = var;
    }
}

#include "gt-tree-ssanames.h"
