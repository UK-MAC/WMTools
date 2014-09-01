
/*  -----------------------------------------------------------------------------------
 *
 *  Interim wrappers for the Fortran to C interface.
 *
 *  These are needed to convert case-insensitive Fortran routine calls into
 *  case-sensitive C interface
 *
 *  Pre-processor macros are also used on the wrapper names to ensure they are
 *  correctly resolved dependent on the platform.
 *
 *  -----------------------------------------------------------------------------------
 *  -----------------------------------------------------------------------------------
 */

/*
 * Pre-processor macros to process subroutine names called by Fortran90.
 *
 * The name required is platform-dependent, so the pre-processor can be used at
 * compile-time to select which is required.
 *
 *   Define:                    Action:
 *   -------                    -------
 *
 *   F2C_NOCHANGE		Leave C interfaces called by F90 unchanged, ie. name -> name
 *				[AIX platform requirement]
 *   F2C_CAPS			Capitalize C interfaces, ie. name -> NAME
 *				[CRAY platform requirement]
 *   F2C_DOUBLEUNDERSCORE	Append a double underscore "__", ie. name -> name__
 *				[some Linux platforms requirement]
 *   F2C_UNDERSCORE		Append a single underscore "_", ie. name -> name_
 *				[some Linux and Solaris platform requirement]
 *   default			No change
 *
 *
 * (NB: ## is the append operator in cpp definitions
 *      ie. name##thing resolves to blahthing when name is blah)
 */


#if defined (F2C_NOCHANGE)
#  define F2C(name,NAME) name

#elif defined (F2C_UNDERSCORE)

#  define F2C(name,NAME) name##_

#elif defined (F2C_DOUBLEUNDERSCORE)
#  define F2C(name,NAME) name##__

#elif defined (F2C_CAPS)
#  define F2C(name,NAME) NAME

#elif defined (F2C_CAPSUNDERSCORE)
#  define F2C(name,NAME) NAME##_

#elif F2C_CAPSDOUBLEUNDERSCORE
#  define F2C(name,NAME) NAME##__

#else
#  define F2C(name,NAME) name

#endif


#include <string.h>

#define F2C_strcpy(cstr, fstr, flen) {(void)memcpy((void *)cstr, (const void *)fstr, (size_t)flen);cstr[flen]='\0';}
#define C2F_strcpy(fstr, flen, cstr) {(void)memset((void *)fstr, (int)' ', (size_t)flen);(void)memcpy((void *)fstr, (const void *)cstr, (size_t)strlen(cstr));}

/* 
 * EOF
 */
