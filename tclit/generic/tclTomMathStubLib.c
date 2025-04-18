/*
 * Copyright © 1998-1999 Scriptics Corporation.
 * Copyright © 1998 Paul Duffin.
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

/*
 * You may distribute and/or modify this program under the terms of the GNU
 * Affero General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.

 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

/*
 * tclTomMathStubLib.c --
 *
 *	Stub object that will be statically linked into extensions that want
 *	to access Tcl.
 */

#include "tclInt.h"
#include "tclTomMath.h"

MODULE_SCOPE const TclTomMathStubs *tclTomMathStubsPtr;

const TclTomMathStubs *tclTomMathStubsPtr = NULL;

/*
 *----------------------------------------------------------------------
 *
 * TclTomMathInitStubs --
 *
 *	Initializes the Stubs table for Tcl's subset of libtommath
 *
 * Results:
 *	Returns a standard Tcl result.
 *
 * This procedure should not be called directly, but rather through
 * the TclTomMath_InitStubs macro, to insure that the Stubs table
 * matches the header files used in compilation.
 *
 *----------------------------------------------------------------------
 */

MODULE_SCOPE const char *
TclTomMathInitializeStubs(
    Tcl_Interp *interp,		/* Tcl interpreter */
    const char *version,	/* Tcl version needed */
    int epoch,			/* Stubs table epoch from the header files */
    int revision)		/* Stubs table revision number from the
				 * header files */
{
    int exact = 0;
    const char *packageName = "tcl::tommath";
    const char *errMsg = NULL;
    TclTomMathStubs *stubsPtr = NULL;
    const char *actualVersion = tclStubsPtr->tcl_PkgRequireEx(interp,
	    packageName, version, exact, &stubsPtr);

    if (actualVersion == NULL) {
	return NULL;
    }
    if (stubsPtr == NULL) {
	errMsg = "missing stub table pointer";
    } else if (stubsPtr->tclBN_epoch() != epoch) {
	errMsg = "epoch number mismatch";
    } else if (stubsPtr->tclBN_revision() != revision) {
	errMsg = "requires a later revision";
    } else {
	tclTomMathStubsPtr = stubsPtr;
	return actualVersion;
    }
    tclStubsPtr->tcl_ResetResult(interp);
    tclStubsPtr->tcl_AppendResult(interp, "Error loading ", packageName,
	    " (requested version ", version, ", actual version ",
	    actualVersion, "): ", errMsg, NULL);
    return NULL;
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
