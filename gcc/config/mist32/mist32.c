/* Target Code for mist32
   Copyright (C) 2012, 2013 Free Software Foundation, Inc.
   Contributed by Hirotaka Kawata <hirotaka@techno-st.net>

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "hash-set.h"
#include "machmode.h"
#include "vec.h"
#include "double-int.h"
#include "input.h"
#include "alias.h"
#include "symtab.h"
#include "wide-int.h"
#include "inchash.h"
#include "tree.h"
#include "stor-layout.h"
#include "varasm.h"
#include "stringpool.h"
#include "calls.h"
#include "rtl.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "insn-config.h"
#include "conditions.h"
#include "output.h"
#include "dbxout.h"
#include "insn-attr.h"
#include "flags.h"
#include "hashtab.h"
#include "function.h"
#include "statistics.h"
#include "real.h"
#include "fixed-value.h"
#include "expmed.h"
#include "dojump.h"
#include "explow.h"
#include "emit-rtl.h"
#include "stmt.h"
#include "expr.h"
#include "recog.h"
#include "diagnostic-core.h"
#include "ggc.h"
#include "dominance.h"
#include "cfg.h"
#include "cfgrtl.h"
#include "cfganal.h"
#include "lcm.h"
#include "cfgbuild.h"
#include "cfgcleanup.h"
#include "predict.h"
#include "basic-block.h"
#include "df.h"
#include "tm_p.h"
#include "target.h"
#include "target-def.h"
#include "tm-constrs.h"
#include "opts.h"
#include "builtins.h"

/* The value of TARGET_ATTRIBUTE_TABLE.  */
static const struct attribute_spec mist32_attribute_table[] = {
  /* Name, min_len, max_len, decl_req, type_req, fn_type_req, handler,
     affects_type_identity.  */
  { "interrupt", 0, 0, true,  false, false, NULL, false },
  { NULL,        0, 0, false, false, false, NULL, false }
};

/* Initialize the GCC target structure.  */
#undef  TARGET_ATTRIBUTE_TABLE
#define TARGET_ATTRIBUTE_TABLE mist32_attribute_table

/* function value */

static rtx
mist32_function_value (const_tree valtype,
		       const_tree fntype_or_decli ATTRIBUTE_UNUSED,
		       bool outgoing ATTRIBUTE_UNUSED)
{
  return gen_rtx_REG (TYPE_MODE (valtype), GP_RETURN);
}

static rtx
mist32_libcall_value (machine_mode mode,
		      const_rtx fun ATTRIBUTE_UNUSED)
{
  return gen_rtx_REG (mode, GP_RETURN);
}

static bool
mist32_function_value_regno_p (const unsigned int regno)
{
  return (regno == GP_RETURN);
}

#undef TARGET_FUNCTION_VALUE
#define TARGET_FUNCTION_VALUE mist32_function_value
#undef TARGET_LIBCALL_VALUE
#define TARGET_LIBCALL_VALUE mist32_libcall_value
#undef TARGET_FUNCTION_VALUE_REGNO_P
#define TARGET_FUNCTION_VALUE_REGNO_P mist32_function_value_regno_p

/* print operand */

void
mist32_print_operand (FILE *file, rtx x, int code)
{
  rtx x0;

  switch (code)
    {
    case 'b':
      /* Convert GCC's comparison operators into mist32 comparison codes.  */
      switch (GET_CODE (x))
	{
	case EQ:  fprintf (file, "eq"); break;
	case NE:  fprintf (file, "ne"); break;
	case LT:  fprintf (file, "lt"); break;
	case LE:  fprintf (file, "le"); break;
	case GT:  fprintf (file, "gt"); break;
	case GE:  fprintf (file, "ge"); break;
	case LTU: fprintf (file, "ltu"); break;
	case LEU: fprintf (file, "leu"); break;
	case GTU: fprintf (file, "gtu"); break;
	case GEU: fprintf (file, "geu");  break;
	default:
	  output_operand_lossage ("mist32_print_operand: unrecognized %%b code");
	  break;
	}
      return;
      
    case 'B':
      /* Convert GCC's comparison operators into the complimentary mist32
	 comparison codes.  */
      switch (GET_CODE (x))
	{
	case EQ:  fprintf (file, "ne"); break;
	case NE:  fprintf (file, "eq"); break;
	case LT:  fprintf (file, "ge"); break;
	case LE:  fprintf (file, "gt"); break;
	case GT:  fprintf (file, "le"); break;
	case GE:  fprintf (file, "lt"); break;
	case LTU: fprintf (file, "geu"); break;
	case LEU: fprintf (file, "gtu"); break;
	case GTU: fprintf (file, "leu"); break;
	case GEU: fprintf (file, "ltu"); break;
	default:
	  output_operand_lossage ("mist32_print_operand: unrecognized %%B code");
	  break;
	}
      return;

    case 'A':
      /* Print a signed byte value as an unsigned value.  */
      if (GET_CODE (x) != CONST_INT)
	output_operand_lossage ("mist32_print_operand: invalid operand to %%A code");
      else
	{
	  HOST_WIDE_INT val;
	  
	  val = INTVAL (x);
	  val &= 0xff;

	  fprintf (file, HOST_WIDE_INT_PRINT_DEC, val);
	}
      return;
      
    case 'x':
      if (GET_CODE (x) != CONST_INT
	  || INTVAL (x) < 16
	  || INTVAL (x) > 32)
	output_operand_lossage ("mist32_print_operand: invalid %%x code");
      else
	fprintf (file, HOST_WIDE_INT_PRINT_DEC, INTVAL (x) - 16);
      return;

    case 'F':
      if (GET_CODE (x) != CONST_DOUBLE)
	output_operand_lossage ("mist32_print_operand: invalid %%F code");
      else
	{
	  char str[30];

	  real_to_decimal (str, CONST_DOUBLE_REAL_VALUE (x),
			   sizeof (str), 0, 1);
	  fputs (str, file);
	}
      return;
      
    case 0:
      /* Handled below.  */
      break;
      
    default:
      fprintf (stderr, "unknown code = %x\n", code);
      output_operand_lossage ("mist32_print_operand: unknown code");
      return;
    }

  switch (GET_CODE (x))
    {
    case REG:
      fputs (reg_names [REGNO (x)], file);
      break;

    case MEM:
      x0 = XEXP (x,0);
      
      switch (GET_CODE (x0))
	{
	case REG:
	  gcc_assert ((unsigned) REGNO (x0) < ARRAY_SIZE (reg_names));
	  fprintf (file, "%s", reg_names [REGNO (x0)]);
	  break;
	  
	case SYMBOL_REF:
	  output_address (x0);
	  break;
	  
	default:
	  fprintf (stderr, "bad MEM code = %x\n", GET_CODE (x0));
	  debug_rtx (x);
	  output_operand_lossage ("mist32_print_operand: unhandled MEM");
	  break;
	}
      break;
      
    case CONST_DOUBLE :
      /* We handle SFmode constants here as output_addr_const doesn't.  */
      if (GET_MODE (x) == SFmode)
	{
	  REAL_VALUE_TYPE d;
	  long l;

	  REAL_VALUE_FROM_CONST_DOUBLE (d, x);
	  REAL_VALUE_TO_TARGET_SINGLE (d, l);
	  fprintf (file, "0x%08lx", l);
	  break;
	}

      /* Fall through.  Let output_addr_const deal with it.  */
    default:
      output_addr_const (file, x);
      break;
    }

  return;
}

void
mist32_print_operand_address (FILE *stream, rtx address)
{
  switch (GET_CODE (address))
    {
    case SYMBOL_REF:
      output_addr_const (stream, address);
      break;
      
    default:
      fprintf (stderr, "code = %x\n", GET_CODE (address));
      debug_rtx (address);
      output_operand_lossage ("mist32_print_operand_address: unhandled address");
      break;
    }
}

#undef  TARGET_PRINT_OPERAND
#define TARGET_PRINT_OPERAND mist32_print_operand
#undef  TARGET_PRINT_OPERAND_ADDRESS
#define TARGET_PRINT_OPERAND_ADDRESS mist32_print_operand_address

static bool
mist32_legitimate_address_p (machine_mode mode ATTRIBUTE_UNUSED, rtx x, bool strict)
{
  if (! REG_P (x))
    return false;

  if (strict)
    {
      if (GPR_P (REGNO (x)))
	return true;
    }
  else
    {
      if (GPR_P (REGNO (x))
	  || REGNO (x) == ARG_POINTER_REGNUM
	  || ! HARD_REGISTER_P (x))
	return true;
    }

  /*
  switch (GET_CODE (x)) {
  case PLUS:
    if (REG_P (XEXP (x, 0))
	&& GPR_P (REGNO (XEXP (x, 0)))
	&& CONSTANT_ADDRESS_P (XEXP (x, 1))
	&& CONST_INT_P (XEXP (x, 1))) {
      if (mode == SImode
	  && INTVAL (XEXP (x, 1)) <= 127
	  && INTVAL (XEXP (x, 1)) >= -128)
	return true;
      if (mode == QImode
	  && INTVAL (XEXP (x, 1)) <= 31
	  && INTVAL (XEXP (x, 1)) >= -32)
	return true;
    }
    break;
  case REG:
    if (strict)
      return GPR_P (REGNO (x));
    else
      return (GPR_P (REGNO (x))
	      || ! HARD_REGISTER_P (x));
    break;
  case SYMBOL_REF:
  case LABEL_REF:
    return true;
    break;
  default:
    break;
  }
  */

  return false;
}

static bool
mist32_legitimate_constant_p (machine_mode mode ATTRIBUTE_UNUSED, rtx x)
{
  return (! (GET_CODE (x) == CONST
	     && GET_CODE (XEXP (x, 0)) == PLUS
	     && (GET_CODE (XEXP (XEXP (x, 0), 0)) == SYMBOL_REF || GET_CODE (XEXP (XEXP (x, 0), 0)) == LABEL_REF)
	     && CONST_INT_P (XEXP (XEXP (x, 0), 1))
	     && (unsigned HOST_WIDE_INT) INTVAL (XEXP (XEXP (x, 0), 1)) > 0x3ff));

  /*
  switch (GET_CODE (x)) {
  case CONST:
    break;
  case SYMBOL_REF:
  case LABEL_REF:
  case CONST_INT:
    return true;
    break;
  default:
    break;
  }

  return false;
  */
}

#undef TARGET_LEGITIMATE_ADDRESS_P
#define TARGET_LEGITIMATE_ADDRESS_P mist32_legitimate_address_p
#undef TARGET_LEGITIMATE_CONSTANT_P
#define TARGET_LEGITIMATE_CONSTANT_P mist32_legitimate_constant_p

/* Return non-zero if the function argument described by TYPE is to be
   passed by reference.  */

static bool
mist32_pass_by_reference (cumulative_args_t cum_v ATTRIBUTE_UNUSED,
			  machine_mode mode, const_tree type,
			  bool named ATTRIBUTE_UNUSED)
{
  int size;

  if (type)
    size = int_size_in_bytes (type);
  else
    size = GET_MODE_SIZE (mode);

  return (size < 0 || size > 8);
}

static bool
mist32_return_in_memory (const_tree type, const_tree fntype ATTRIBUTE_UNUSED)
{
  cumulative_args_t dummy = pack_cumulative_args (NULL);

  return mist32_pass_by_reference (dummy, TYPE_MODE (type), type, false);
}

/* Forward declaration.  */
static rtx mist32_function_arg (cumulative_args_t, machine_mode,
				const_tree, bool);
static void mist32_function_arg_advance (cumulative_args_t, machine_mode,
					 const_tree, bool);

/* Some function arguments will only partially fit in the registers
   that hold arguments.  Given a new arg, return the number of bytes
   that fit in argument passing registers.  */

static int
mist32_arg_partial_bytes (cumulative_args_t cum_v, machine_mode mode,
			  tree type, bool named)
{
  CUMULATIVE_ARGS *cum = get_cumulative_args (cum_v);
  int bytes_left, size;

  if (*cum > GP_ARG_LAST)
    return 0;

  if (mist32_pass_by_reference (cum_v, mode, type, named))
    size = 4;
  else if (type)
    {
      if (AGGREGATE_TYPE_P (type))
	return 0;
      size = int_size_in_bytes (type);
    }
  else
    size = GET_MODE_SIZE (mode);

  bytes_left = (UNITS_PER_WORD * MAX_ARGS_IN_REGISTERS) - ((*cum - GP_ARG_FIRST) * UNITS_PER_WORD);

  if (size > bytes_left)
    return bytes_left;
  else
    return 0;
}

/* Return the next register to be used to hold a function argument or
   NULL_RTX if there's no more space.  */

static rtx
mist32_function_arg (cumulative_args_t cum_v, machine_mode mode,
		     const_tree type ATTRIBUTE_UNUSED,
		     bool named ATTRIBUTE_UNUSED)
{
  CUMULATIVE_ARGS *cum = get_cumulative_args (cum_v);

  return (*cum <= GP_ARG_LAST
	  ? gen_rtx_REG (mode, *cum)
	  : NULL_RTX);
}

#define MIST32_FUNCTION_ARG_SIZE(MODE, TYPE)	\
  ((MODE) != BLKmode ? GET_MODE_SIZE (MODE)	\
   : (unsigned) int_size_in_bytes (TYPE))

/* Update the data in CUM to advance over an argument
   of mode MODE and data type TYPE.
   (TYPE is null for libcalls where that information may not be available.)  */

static void
mist32_function_arg_advance (cumulative_args_t cum_v, machine_mode mode,
			     const_tree type, bool named ATTRIBUTE_UNUSED)
{
  CUMULATIVE_ARGS *cum = get_cumulative_args (cum_v);

  *cum = (*cum <= GP_ARG_LAST
	  ? *cum + ((UNITS_PER_WORD - 1 + MIST32_FUNCTION_ARG_SIZE (mode, type)) / UNITS_PER_WORD)
	  : *cum);
}

#undef  TARGET_PROMOTE_PROTOTYPES
#define TARGET_PROMOTE_PROTOTYPES hook_bool_const_tree_true

#undef  TARGET_RETURN_IN_MEMORY
#define TARGET_RETURN_IN_MEMORY mist32_return_in_memory

#undef  TARGET_MUST_PASS_IN_STACK
#define TARGET_MUST_PASS_IN_STACK must_pass_in_stack_var_size
#undef  TARGET_PASS_BY_REFERENCE
#define TARGET_PASS_BY_REFERENCE mist32_pass_by_reference
#undef  TARGET_ARG_PARTIAL_BYTES
#define TARGET_ARG_PARTIAL_BYTES mist32_arg_partial_bytes
#undef  TARGET_FUNCTION_ARG
#define TARGET_FUNCTION_ARG mist32_function_arg
#undef  TARGET_FUNCTION_ARG_ADVANCE
#define TARGET_FUNCTION_ARG_ADVANCE mist32_function_arg_advance

static void
mist32_setup_incoming_varargs (cumulative_args_t cum_v, machine_mode mode,
			       tree type ATTRIBUTE_UNUSED, int *pretend_size, int no_rtl)
{
  CUMULATIVE_ARGS *cum = get_cumulative_args (cum_v);
  int first_anon_arg;

  if (no_rtl)
    return;

  /* All BLKmode values are passed by reference.  */
  gcc_assert (mode != BLKmode);

  first_anon_arg = *cum + ((GET_MODE_SIZE (mode) + UNITS_PER_WORD - 1) / UNITS_PER_WORD);

  if (first_anon_arg <= GP_ARG_LAST)
    {
      /* Note that first_reg_offset <= GP_ARG_LAST.  */
      int first_reg_offset = first_anon_arg;
      /* Size in words to "pretend" allocate.  */
      int size = (GP_ARG_LAST + 1) - first_reg_offset;
      rtx regblock;

      regblock = gen_frame_mem (BLKmode,
				plus_constant (Pmode, arg_pointer_rtx,
					       FIRST_PARM_OFFSET (0)));
      set_mem_alias_set (regblock, get_varargs_alias_set ());
      move_block_from_reg (first_reg_offset, regblock, size);

      *pretend_size = (size * UNITS_PER_WORD);
    }
}

#undef  TARGET_SETUP_INCOMING_VARARGS
#define TARGET_SETUP_INCOMING_VARARGS mist32_setup_incoming_varargs

/* Worker function for TARGET_CAN_ELIMINATE.  */

bool
mist32_can_eliminate (const int from ATTRIBUTE_UNUSED, const int to)
{
  return (from == ARG_POINTER_REGNUM && to == STACK_POINTER_REGNUM
          ? ! frame_pointer_needed
          : true);
}

#undef TARGET_CAN_ELIMINATE
#define TARGET_CAN_ELIMINATE mist32_can_eliminate

/* Type of function DECL.

   The result is cached.  To reset the cache at the end of a function,
   call with DECL = NULL_TREE.  */

enum mist32_function_type
mist32_compute_function_type (tree decl)
{
  /* Cached value.  */
  static enum mist32_function_type fn_type = MIST32_FUNCTION_UNKNOWN;
  /* Last function we were called for.  */
  static tree last_fn = NULL_TREE;

  /* Resetting the cached value?  */
  if (decl == NULL_TREE)
    {
      fn_type = MIST32_FUNCTION_UNKNOWN;
      last_fn = NULL_TREE;
      return fn_type;
    }

  if (decl == last_fn && fn_type != MIST32_FUNCTION_UNKNOWN)
    return fn_type;

  /* Compute function type.  */
  fn_type = (lookup_attribute ("interrupt", DECL_ATTRIBUTES (current_function_decl)) != NULL_TREE
	     ? MIST32_FUNCTION_INTERRUPT
	     : MIST32_FUNCTION_NORMAL);

  last_fn = decl;
  return fn_type;
}

/* Structure to be filled in by mist32_compute_frame_size() with register
   save masks, and offsets for the current function.  */
struct mist32_frame_info
{
  unsigned int total_size;	/* # Bytes that the entire frame takes up.  */
  unsigned int extra_size;	/* # bytes of extra stuff.  */
  unsigned int pretend_size;	/* # Bytes we push and pretend caller did.  */
  unsigned int args_size;	/* # Bytes that outgoing arguments take up.  */
  unsigned int reg_size;	/* # Bytes needed to store regs.  */
  unsigned int var_size;	/* # Bytes that variables take up.  */
  unsigned int frame_size;      /* # Bytes in current frame.  */
  unsigned int gmask;		/* Mask of saved registers.  */
  unsigned int save_fp;		/* Nonzero if frame pointer must be saved.  */
  unsigned int save_rp;		/* Nonzero if return pointer must be saved.  */
  int          initialized;	/* Nonzero if frame size already calculated.  */
};

/* Current frame information calculated by mist32_compute_frame_size().  */
static struct mist32_frame_info 	current_frame_info;

/* Zero structure to initialize current_frame_info.  */
static struct mist32_frame_info 	zero_frame_info;

#define FRAME_POINTER_MASK 	(1 << (FRAME_POINTER_REGNUM))
#define RETURN_POINTER_MASK 	(1 << (RETURN_POINTER_REGNUM))

/* Tell prologue and epilogue if register REGNO should be saved / restored.
   The return address and frame pointer are treated separately.
   Don't consider them here.  */
#define MUST_SAVE_REGISTER(regno, interrupt_p)				\
  ((regno) != RETURN_POINTER_REGNUM && (regno) != FRAME_POINTER_REGNUM	\
   && ((df_regs_ever_live_p (regno) && !call_really_used_regs[regno])	\
       || (interrupt_p && call_really_used_regs[regno])))

#define MUST_SAVE_FRAME_POINTER	 (df_regs_ever_live_p (FRAME_POINTER_REGNUM))
#define MUST_SAVE_RETURN_POINTER (df_regs_ever_live_p (RETURN_POINTER_REGNUM) \
				  || crtl->profile)

/* Returns the number of bytes offset between FROM_REG and TO_REG
   for the current function.  As a side effect it fills in the 
   current_frame_info structure, if the data is available.  */
unsigned int
mist32_compute_frame_size (int size)	/* # of var. bytes allocated.  */
{
  unsigned int regno;
  unsigned int total_size, var_size, args_size, pretend_size, extra_size;
  unsigned int reg_size;
  unsigned int gmask;
  enum mist32_function_type fn_type;
  int interrupt_p;

  var_size	= MIST32_STACK_ALIGN (size);
  args_size	= MIST32_STACK_ALIGN (crtl->outgoing_args_size);
  pretend_size	= crtl->args.pretend_args_size;
  extra_size	= FIRST_PARM_OFFSET (0);
  total_size	= extra_size + pretend_size + args_size + var_size;
  reg_size	= 0;
  gmask		= 0;

  /* See if this is an interrupt handler.  Call used registers must be saved
     for them too.  */
  fn_type = mist32_compute_function_type (current_function_decl);
  interrupt_p = MIST32_INTERRUPT_P (fn_type);

  /* Calculate space needed for registers.  */
  for (regno = GP_REG_FIRST; regno <= GP_REG_LAST; regno++)
    {
      if (MUST_SAVE_REGISTER (regno, interrupt_p))
	{
	  reg_size += UNITS_PER_WORD;
	  gmask |= 1 << regno;
	}
    }

  current_frame_info.save_fp = MUST_SAVE_FRAME_POINTER;
  current_frame_info.save_rp = MUST_SAVE_RETURN_POINTER;

  reg_size += ((current_frame_info.save_fp + current_frame_info.save_rp)
	       * UNITS_PER_WORD);
  total_size += reg_size;

  /* ??? Not sure this is necessary, and I don't think the epilogue
     handler will do the right thing if this changes total_size.  */
  total_size = MIST32_STACK_ALIGN (total_size);

  /* frame_size = total_size - (pretend_size + reg_size); */

  /* Save computed information.  */
  current_frame_info.total_size   = total_size;
  current_frame_info.extra_size   = extra_size;
  current_frame_info.pretend_size = pretend_size;
  current_frame_info.var_size     = var_size;
  current_frame_info.args_size    = args_size;
  current_frame_info.reg_size	  = reg_size;
  current_frame_info.gmask	  = gmask;
  current_frame_info.initialized  = reload_completed;

  /* Ok, we're done.  */
  return total_size;
}

/* Expand the mist32 prologue as a series of insns.  */

void
mist32_expand_prologue (void)
{
  int regno;
  int frame_size;
  unsigned int gmask;

  if (! current_frame_info.initialized)
    mist32_compute_frame_size (get_frame_size ());

  gmask = current_frame_info.gmask;

  /* These cases shouldn't happen.  Catch them now.  */
  gcc_assert (current_frame_info.total_size || !gmask);

  /* Allocate space for register arguments if this is a variadic function.  */
  if (current_frame_info.pretend_size != 0)
    {
      /* Use a HOST_WIDE_INT temporary, since negating an unsigned int gives
	 the wrong result on a 64-bit host.  */
      HOST_WIDE_INT pretend_size = current_frame_info.pretend_size;
      emit_insn (gen_sub_stack_pointer (GEN_INT (pretend_size)));
    }

  /* Save any registers we need to and set up fp.  */
  if (current_frame_info.save_fp)
    emit_insn (gen_pushsi1 (frame_pointer_rtx));

  gmask &= ~(FRAME_POINTER_MASK | RETURN_POINTER_MASK);

  /* Save any needed call-saved regs (and call-used if this is an
     interrupt handler).  */
  for (regno = GP_REG_FIRST; regno <= GP_REG_LAST; regno++)
    {
      if ((gmask & (1 << regno)) != 0)
	emit_insn (gen_pushsi1 (gen_rtx_REG (Pmode, regno)));
    }

  if (current_frame_info.save_rp)
    emit_insn (gen_pushsi1 (gen_rtx_REG (Pmode, RETURN_POINTER_REGNUM)));

  /* Allocate the stack frame.  */
  frame_size = (current_frame_info.total_size
		- (current_frame_info.pretend_size
		   + current_frame_info.reg_size));

  if (frame_size == 0)
    ; /* Nothing to do.  */
  else if (frame_size <= 0x3ff)
    emit_insn (gen_sub_stack_pointer (GEN_INT (frame_size)));
  else
    {
      int remaining = frame_size;

      emit_insn (gen_movsi (gen_rtx_REG (Pmode, TMP_REGNUM), stack_pointer_rtx));
      while(remaining > 0)
	{
	  emit_insn (gen_subsi3 (gen_rtx_REG (Pmode, TMP_REGNUM),
				 gen_rtx_REG (Pmode, TMP_REGNUM),
				 GEN_INT ((remaining > 0x3ff) ? 0x3ff : remaining)));
	  remaining -= 0x3ff;
	}
      emit_insn (gen_movsi (stack_pointer_rtx, gen_rtx_REG (Pmode, TMP_REGNUM)));
    }

  if (frame_pointer_needed)
    emit_insn (gen_movsi (frame_pointer_rtx, stack_pointer_rtx));

  if (crtl->profile)
    /* Push rret for mcount (form_pc, x).  */
    emit_insn (gen_pushsi1 (gen_rtx_REG (Pmode, RETURN_POINTER_REGNUM)));

  if (crtl->profile)
    emit_insn (gen_blockage ());
}

/* Set up the stack and frame pointer (if desired) for the function.
   Note, if this is changed, you need to mirror the changes in
   mist32_compute_frame_size which calculates the prolog size.  */

static void
mist32_output_function_prologue (FILE * file, HOST_WIDE_INT size)
{
  enum mist32_function_type fn_type = mist32_compute_function_type (current_function_decl);

  /* If this is an interrupt handler, mark it as such.  */
  if (MIST32_INTERRUPT_P (fn_type))
    fprintf (file, "\t%s interrupt handler\n", ASM_COMMENT_START);

  if (! current_frame_info.initialized)
    mist32_compute_frame_size (size);

  /* This is only for the human reader.  */
  fprintf (file,
	   "\t%s PROLOGUE, vars= %d, regs= %d, args= %d, extra= %d\n",
	   ASM_COMMENT_START,
	   current_frame_info.var_size,
	   current_frame_info.reg_size / 4,
	   current_frame_info.args_size,
	   current_frame_info.extra_size);
}

/* Expand the mist32 epilogue as a series of insns.  */

void
mist32_expand_epilogue (void)
{
  int regno;
  int noepilogue = FALSE;
  int total_size;

  gcc_assert (current_frame_info.initialized);
  total_size = current_frame_info.total_size;

  if (total_size == 0)
    {
      rtx insn = get_last_insn ();

      /* If the last insn was a BARRIER, we don't have to write any code
	 because a jump (aka return) was put there.  */
      if (insn && NOTE_P (insn))
	insn = prev_nonnote_insn (insn);
      if (insn && BARRIER_P (insn))
	noepilogue = TRUE;
    }

  if (!noepilogue)
    {
      unsigned int var_size = current_frame_info.var_size;
      unsigned int args_size = current_frame_info.args_size;
      unsigned int gmask = current_frame_info.gmask;
      int can_trust_sp_p = !cfun->calls_alloca;

      if (flag_exceptions)
        emit_insn (gen_blockage ());

      /* The first thing to do is point the sp at the bottom of the register
	 save area.  */
      if (can_trust_sp_p || frame_pointer_needed)
	{
	  unsigned int reg_offset = var_size + args_size;

	  if (frame_pointer_needed)
	    emit_insn (gen_movsi (stack_pointer_rtx, frame_pointer_rtx));

	  if (reg_offset == 0)
	    ; /* Nothing to do.  */
	  else if (reg_offset <= 0x3ff)
	    emit_insn (gen_add_stack_pointer (GEN_INT (reg_offset)));
	  else
	    {
	      int remaining = reg_offset;

	      emit_insn (gen_movsi (gen_rtx_REG (Pmode, TMP_REGNUM), stack_pointer_rtx));
	      while(remaining > 0)
		{
		  emit_insn (gen_addsi3 (gen_rtx_REG (Pmode, TMP_REGNUM),
					 gen_rtx_REG (Pmode, TMP_REGNUM),
					 GEN_INT ((remaining > 0x3ff) ? 0x3ff : remaining)));
		  remaining -= 0x3ff;
		}
	      emit_insn (gen_movsi (stack_pointer_rtx, gen_rtx_REG (Pmode, TMP_REGNUM)));
	    }
	}
      else
	gcc_unreachable ();

      if (current_frame_info.save_rp)
	emit_insn (gen_popsi1 (gen_rtx_REG (Pmode, RETURN_POINTER_REGNUM)));

      gmask &= ~(FRAME_POINTER_MASK | RETURN_POINTER_MASK);

      /* Restore any saved registers, in reverse order of course.  */
      for (regno = GP_REG_LAST; regno >= 0; --regno)
	{
	  if ((gmask & (1 << regno)) != 0)
	    emit_insn (gen_popsi1 (gen_rtx_REG (Pmode, regno)));
	}

      if (current_frame_info.save_fp)
	emit_insn (gen_popsi1 (gen_rtx_REG (Pmode, FRAME_POINTER_REGNUM)));

      /* Remove varargs area if present.  */
      if (current_frame_info.pretend_size != 0)
	emit_insn (gen_add_stack_pointer (GEN_INT (current_frame_info.pretend_size)));

      emit_insn (gen_blockage ());
    }
}

/* Do any necessary cleanup after a function to restore stack, frame,
   and regs.  */

static void
mist32_output_function_epilogue (FILE * file,
				 HOST_WIDE_INT size ATTRIBUTE_UNUSED)
{
  /* This is only for the human reader.  */
  fprintf (file, "\t%s EPILOGUE\n", ASM_COMMENT_START);

  /* Reset state info for each function.  */
  current_frame_info = zero_frame_info;
  mist32_compute_function_type (NULL_TREE);
}

#undef  TARGET_ASM_FUNCTION_PROLOGUE
#define TARGET_ASM_FUNCTION_PROLOGUE mist32_output_function_prologue
#undef  TARGET_ASM_FUNCTION_EPILOGUE
#define TARGET_ASM_FUNCTION_EPILOGUE mist32_output_function_epilogue

#undef  TARGET_ASM_ALIGNED_HI_OP
#define TARGET_ASM_ALIGNED_HI_OP "\t.word\t"
#undef  TARGET_ASM_ALIGNED_SI_OP
#define TARGET_ASM_ALIGNED_SI_OP "\t.long\t"

/* Return nonzero if this function is known to have a null or 1 instruction
   epilogue.  */

int
direct_return (void)
{
  if (!reload_completed)
    return FALSE;

  if (MIST32_INTERRUPT_P (mist32_compute_function_type (current_function_decl)))
    return FALSE;

  if (! current_frame_info.initialized)
    mist32_compute_frame_size (get_frame_size ());

  return current_frame_info.total_size == 0;
}

/*
  Condition Code
*/

/* FIXME: This hooks seems not to be used */

static bool
mist32_fixed_condition_code_regs (unsigned int *p1, unsigned int *p2)
{
  *p1 = CONDITION_CODE_REGNUM;
  *p2 = INVALID_REGNUM;
  return true;
}

#undef	TARGET_FIXED_CONDITION_CODE_REGS
#define	TARGET_FIXED_CONDITION_CODE_REGS mist32_fixed_condition_code_regs

/*
  Trampoline
*/

static void
mist32_asm_trampoline_template (FILE *f)
{
  /* static chain pointer */
  fprintf (f, "\twl16\t%s, 0\n", reg_names [STATIC_CHAIN_REGNUM]);
  fprintf (f, "\tlih\t%s, 0\n", reg_names [STATIC_CHAIN_REGNUM]);

  /* jump to real nested function */
  fprintf (f, "\twl16\t%s, 0\n", reg_names [TMP_REGNUM]);
  fprintf (f, "\tlih\t%s, 0\n", reg_names [TMP_REGNUM]);
  fprintf (f, "\tb\t%s, #al\n", reg_names [TMP_REGNUM]);
}

static void
mist32_trampoline_init (rtx m_tramp, tree fndecl, rtx chain_value)
{
  rtx mem, fnaddr = XEXP (DECL_RTL (fndecl), 0);

  emit_block_move (m_tramp, assemble_trampoline_template (),
		   GEN_INT (TRAMPOLINE_SIZE), BLOCK_OP_NORMAL);

  /* store static chain pointer */
  mem = adjust_address (m_tramp, HImode, 2);
  emit_move_insn (mem, chain_value);
  mem = adjust_address (m_tramp, HImode, 6);
  emit_move_insn (mem, gen_rtx_LSHIFTRT (SImode, chain_value, GEN_INT (16)));

  /* store real function address */
  mem = adjust_address (m_tramp, HImode, 10);
  emit_move_insn (mem, fnaddr);
  mem = adjust_address (m_tramp, HImode, 14);
  emit_move_insn (mem, gen_rtx_LSHIFTRT (SImode, fnaddr, GEN_INT (16)));
}

#undef TARGET_ASM_TRAMPOLINE_TEMPLATE
#define TARGET_ASM_TRAMPOLINE_TEMPLATE mist32_asm_trampoline_template
#undef TARGET_TRAMPOLINE_INIT
#define TARGET_TRAMPOLINE_INIT mist32_trampoline_init

/*
  Instruction Scheduling
*/

static int
mist32_issue_rate (void)
{
  /* FIXME: MIST1032ISA (In-order) core is single issue. */
  return 2;
}

#undef TARGET_SCHED_ISSUE_RATE
#define TARGET_SCHED_ISSUE_RATE mist32_issue_rate

/* Compare insn and branch insn must be adjacent in mist32 */
/* FIXME: This hooks seems not to be used */

static bool
mist32_macro_fusion_pair_p (rtx_insn* prev, rtx_insn* curr)
{
  rtx prev_set = single_set(prev);

  if(!prev_set)
    return false;

  if (any_condjump_p (curr)) {
    rtx cc_reg_1 = gen_rtx_REG (CCmode, CONDITION_CODE_REGNUM);
    if (modified_in_p (cc_reg_1, prev_set))
      return true;
  }

  return false;
}

#undef TARGET_SCHED_MACRO_FUSION_P
#define TARGET_SCHED_MACRO_FUSION_P hook_bool_void_true
#undef TARGET_SCHED_MACRO_FUSION_PAIR_P
#define TARGET_SCHED_MACRO_FUSION_PAIR_P mist32_macro_fusion_pair_p

struct gcc_target targetm = TARGET_INITIALIZER;
