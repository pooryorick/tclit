/*
 * Copyright © 1998 Scriptics Corporation.
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
 * tclTestProcBodyObj.c --
 *
 *	Implements the "procbodytest" package, which contains commands to test
 *	creation of Tcl procedures whose body argument is a Tcl_Obj of type
 *	"procbody" rather than a string.
 */

#undef BUILD_tcl
#undef STATIC_BUILD
#ifndef USE_TCL_STUBS
#   define USE_TCL_STUBS
#endif
#include "tclInt.h"

/*
 * name and version of this package
 */

static const char packageName[] = "tcl::procbodytest";
static const char packageVersion[] = "1.1";

/*
 * Name of the commands exported by this package
 */

static const char procCommand[] = "proc";
static const char checkCommand[] = "check";

/*
 * this struct describes an entry in the table of command names and command
 * procs
 */

typedef struct {
    const char *cmdName;		/* command name */
    Tcl_ObjCmdProc *proc;	/* command proc */
    int exportIt;		/* if 1, export the command */
} CmdTable;

/*
 * Declarations for functions defined in this file.
 */

static Tcl_ObjCmdProc ProcBodyTestProcObjCmd;
static Tcl_ObjCmdProc ProcBodyTestCheckObjCmd;
static int	ProcBodyTestInitInternal(Tcl_Interp *interp, int isSafe);
static int	RegisterCommand(Tcl_Interp* interp,
			const char *namesp, const CmdTable *cmdTablePtr);

/*
 * List of commands to create when the package is loaded; must go after the
 * declarations of the enable command procedure.
 */

static const CmdTable commands[] = {
    { procCommand,	ProcBodyTestProcObjCmd,	1 },
    { checkCommand,	ProcBodyTestCheckObjCmd,	1 },
    { 0, 0, 0 }
};

static const CmdTable safeCommands[] = {
    { procCommand,	ProcBodyTestProcObjCmd,	1 },
    { checkCommand,	ProcBodyTestCheckObjCmd,	1 },
    { 0, 0, 0 }
};

/*
 *----------------------------------------------------------------------
 *
 * Procbodytest_Init --
 *
 *	This function initializes the "tcl::procbodytest" package.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Procbodytest_Init(
    Tcl_Interp *interp)		/* the Tcl interpreter for which the package
				 * is initialized */
{
    return ProcBodyTestInitInternal(interp, 0);
}

/*
 *----------------------------------------------------------------------
 *
 * Procbodytest_SafeInit --
 *
 *	This function initializes the "tcl::procbodytest" package.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
Procbodytest_SafeInit(
    Tcl_Interp *interp)		/* the Tcl interpreter for which the package
				 * is initialized */
{
    return ProcBodyTestInitInternal(interp, 1);
}

/*
 *----------------------------------------------------------------------
 *
 * RegisterCommand --
 *
 *	This function registers a command in the context of the given
 *	namespace.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
RegisterCommand(
    Tcl_Interp* interp,		/* the Tcl interpreter for which the operation
				 * is performed */
    const char *namesp,		/* the namespace in which the command is
				 * registered */
    const CmdTable *cmdTablePtr)/* the command to register */
{
    char buf[128];

    if (cmdTablePtr->exportIt) {
	snprintf(buf, sizeof(buf), "namespace eval %s { namespace export %s }",
		namesp, cmdTablePtr->cmdName);
	if (Tcl_EvalEx(interp, buf, TCL_INDEX_NONE, 0) != TCL_OK) {
	    return TCL_ERROR;
	}
    }

    snprintf(buf, sizeof(buf), "%s::%s", namesp, cmdTablePtr->cmdName);
    Tcl_CreateObjCommand(interp, buf, cmdTablePtr->proc, 0, 0);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ProcBodyTestInitInternal --
 *
 *  This function initializes the Loader package.
 *  The isSafe flag is 1 if the interpreter is safe, 0 otherwise.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  None.
 *
 *----------------------------------------------------------------------
 */

static int
ProcBodyTestInitInternal(
    Tcl_Interp *interp,		/* the Tcl interpreter for which the package
				 * is initialized */
    int isSafe)			/* 1 if this is a safe interpreter */
{
    const CmdTable *cmdTablePtr;

    cmdTablePtr = (isSafe) ? &safeCommands[0] : &commands[0];
    for ( ; cmdTablePtr->cmdName ; cmdTablePtr++) {
	if (RegisterCommand(interp, packageName, cmdTablePtr) != TCL_OK) {
	    return TCL_ERROR;
	}
    }

    return Tcl_PkgProvideEx(interp, packageName, packageVersion, NULL);
}

/*
 *----------------------------------------------------------------------
 *
 * ProcBodyTestProcObjCmd --
 *
 *  Implements the "procbodytest::proc" command. Here is the command
 *  description:
 *	procbodytest::proc newName argList bodyName
 *  Looks up a procedure called $bodyName and, if the procedure exists,
 *  constructs a Tcl_Obj of type "procbody" and calls Tcl_ProcObjCmd.
 *  Arguments:
 *    newName		the name of the procedure to be created
 *    argList		the argument list for the procedure
 *    bodyName		the name of an existing procedure from which the
 *			body is to be copied.
 *  This command can be used to trigger the branches in Tcl_ProcObjCmd that
 *  construct a proc from a "procbody", for example:
 *	proc a {x} {return $x}
 *	a 123
 *	procbodytest::proc b {x} a
 *  Note the call to "a 123", which is necessary so that the Proc pointer
 *  for "a" is filled in by the internal compiler; this is a hack.
 *
 * Results:
 *  Returns a standard Tcl code.
 *
 * Side effects:
 *  A new procedure is created.
 *  Leaves an error message in the interp's result on error.
 *
 *----------------------------------------------------------------------
 */

static int
ProcBodyTestProcObjCmd(
    TCL_UNUSED(void *),
    Tcl_Interp *interp,		/* the current interpreter */
    int objc,			/* argument count */
    Tcl_Obj *const objv[])	/* arguments */
{
    const char *fullName;
    Tcl_Command procCmd;
    Command *cmdPtr;
    Proc *procPtr = NULL;
    Tcl_Obj *bodyObjPtr;
    Tcl_Obj *myobjv[5];
    int result;

    if (objc != 4) {
	Tcl_WrongNumArgs(interp, 1, objv, "newName argsList bodyName");
	return TCL_ERROR;
    }

    /*
     * Find the Command pointer to this procedure
     */

    fullName = Tcl_GetString(objv[3]);
    procCmd = Tcl_FindCommand(interp, fullName, NULL, TCL_LEAVE_ERR_MSG);
    if (procCmd == NULL) {
	return TCL_ERROR;
    }

    cmdPtr = (Command *) procCmd;

    /*
     * check that this is a procedure and not a builtin command:
     * If a procedure, cmdPtr->objClientData is TclIsProc(cmdPtr).
     */

    if (cmdPtr->objClientData != TclIsProc(cmdPtr)) {
	Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
		"command \"", fullName, "\" is not a Tcl procedure", (char *)NULL);
	return TCL_ERROR;
    }

    /*
     * it is a Tcl procedure: the client data is the Proc structure
     */

    procPtr = (Proc *) cmdPtr->objClientData;
    if (procPtr == NULL) {
	Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "procedure \"",
		fullName, "\" does not have a Proc struct!", (char *)NULL);
	return TCL_ERROR;
    }

    /*
     * create a new object, initialize our argument vector, call into Tcl
     */

    bodyObjPtr = TclNewProcBodyObj(procPtr);
    if (bodyObjPtr == NULL) {
	Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
		"failed to create a procbody object for procedure \"",
		fullName, "\"", (char *)NULL);
	return TCL_ERROR;
    }
    Tcl_IncrRefCount(bodyObjPtr);

    myobjv[0] = objv[0];
    myobjv[1] = objv[1];
    myobjv[2] = objv[2];
    myobjv[3] = bodyObjPtr;
    myobjv[4] = NULL;

    result = Tcl_ProcObjCmd(NULL, interp, objc, myobjv);
    Tcl_DecrRefCount(bodyObjPtr);

    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * ProcBodyTestCheckObjCmd --
 *
 *  Implements the "procbodytest::check" command. Here is the command
 *  description:
 *	procbodytest::check
 *
 *  Performs an internal check that the Tcl_PkgPresent() command returns
 *  the same version number as was registered when the tcl::procbodytest package
 *  was provided.  Places a boolean in the interp result indicating the
 *  test outcome.
 *
 * Results:
 *  Returns a standard Tcl code.
 *
 *----------------------------------------------------------------------
 */

static int
ProcBodyTestCheckObjCmd(
    TCL_UNUSED(void *),
    Tcl_Interp *interp,		/* the current interpreter */
    int objc,			/* argument count */
    Tcl_Obj *const objv[])	/* arguments */
{
    const char *version;

    if (objc != 1) {
	Tcl_WrongNumArgs(interp, 1, objv, "");
	return TCL_ERROR;
    }

    version = Tcl_PkgPresentEx(interp, packageName, packageVersion, 1, NULL);
    Tcl_SetObjResult(interp, Tcl_NewBooleanObj(
	    strcmp(version, packageVersion) == 0));
    return TCL_OK;
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
