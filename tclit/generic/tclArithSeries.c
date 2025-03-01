/*
 * tclArithSeries.c --
 *
 *     This file contains the ArithSeries concrete abstract list
 *     implementation. It implements the inner workings of the lseq command.
 *
 * Copyright © 2022 Brian S. Griffin.
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

/*
 * Copyright © 2024 Nathan Coulter
 *
 * You may distribute and/or modify this program under the terms of the GNU
 * Affero General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.

 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include "tcl.h"
#include "tclInt.h"
#include <assert.h>
#include <math.h>

/*
 * The structure below defines the arithmetic series Tcl object type by
 * means of procedures that can be invoked by generic object code.
 *
 * The arithmetic series object is a special case of Tcl list representing
 * an interval of an arithmetic series in constant space.
 *
 * The arithmetic series is internally represented with three integers,
 * *start*, *end*, and *step*, Where the length is calculated with
 * the following algorithm:
 *
 * if RANGE == 0 THEN
 *   ERROR
 * if RANGE > 0
 *   LEN is (((END-START)-1)/STEP) + 1
 * else if RANGE < 0
 *   LEN is (((END-START)-1)/STEP) - 1
 *
 * And where the equivalent's list I-th element is calculated
 * as:
 *
 * LIST[i] = START + (STEP * i)
 *
 * Zero elements ranges, like in the case of START=10 END=10 STEP=1
 * are valid and will be equivalent to the empty list.
 */

/*
 * The structure used for the ArithSeries internal representation.
 * Note that the len can in theory be always computed by start,end,step
 * but it's faster to cache it inside the internal representation.
 */

typedef struct {
    Tcl_Size len;
    Tcl_Obj **elements;
    int isDouble;
} ArithSeries;

typedef struct {
    ArithSeries base;
    Tcl_WideInt start;
    Tcl_WideInt end;
    Tcl_WideInt step;
} ArithSeriesInt;

typedef struct {
    ArithSeries base;
    double start;
    double end;
    double step;
    unsigned precision;		/* Number of decimal places to render. */
} ArithSeriesDbl;

/* Forward declarations. */

static Tcl_ObjInterfaceListContainsProc	ArithSeriesInOperation;
static Tcl_ObjInterfaceListIndexProc	ArithSeriesObjIndex;
static Tcl_ObjInterfaceListLengthProc	ArithSeriesObjLength;
static Tcl_ObjInterfaceListRangeProc	ArithSeriesObjRange;
static Tcl_ObjInterfaceListReverseProc	ArithSeriesObjReverse;
static Tcl_ObjInterfaceListAllProc	ArithSeriesGetElements;
static int		ArithSeriesObjStep(Tcl_Obj *arithSeriesObj,
			    Tcl_Obj **stepObj);
static void		DupArithSeriesInternalRep(Tcl_Obj *srcPtr,
			    Tcl_Obj *copyPtr);
static void		FreeArithSeriesInternalRep(Tcl_Obj *arithSeriesObjPtr);
static void		UpdateStringOfArithSeries(Tcl_Obj *arithSeriesObjPtr);
static int		SetArithSeriesFromAny(Tcl_Interp *interp,
			    Tcl_Obj *objPtr);

/* ------------------------ ArithSeries object type -------------------------- */


static int ArithSeriesObjStep(Tcl_Obj *arithSeriesObj, Tcl_Obj **stepObj);


static ObjectType arithSeriesType = {
    "arithseries",
    FreeArithSeriesInternalRep,	/* freeIntRepProc */
    DupArithSeriesInternalRep,	/* dupIntRepProc */
    UpdateStringOfArithSeries,	/* updateStringProc */
    SetArithSeriesFromAny,		/* setFromAnyProc */
    2,
    NULL
};


void TclArithSeriesInit(void) {
    Tcl_ObjInterface *oiPtr;
    oiPtr = Tcl_NewObjInterface();
    Tcl_ObjInterfaceSetFnListContains(oiPtr ,ArithSeriesInOperation);
    Tcl_ObjInterfaceSetFnListAll(oiPtr ,ArithSeriesGetElements);
    Tcl_ObjInterfaceSetFnListIndex(oiPtr ,ArithSeriesObjIndex);
    Tcl_ObjInterfaceSetFnListLength(oiPtr ,ArithSeriesObjLength);
    Tcl_ObjInterfaceSetFnListRange(oiPtr ,ArithSeriesObjRange);
    Tcl_ObjInterfaceSetFnListReverse(oiPtr ,ArithSeriesObjReverse);
    Tcl_ObjTypeSetInterface((Tcl_ObjType *)&arithSeriesType ,oiPtr);
	return;
}


/*
 * Helper functions
 *
 * - power10 -- Fast version of pow(10, (int) n) for common cases.
 * - ArithRound -- Round doubles to the number of significant fractional
 *                 digits
 * - ArithSeriesIndexDbl -- base list indexing operation for doubles
 * - ArithSeriesIndexInt --   "    "      "        "      "  integers
 * - ArithSeriesGetInternalRep -- Return the internal rep from a Tcl_Obj
 * - Precision -- determine the number of factional digits for the given
 *   double value
 * - maxPrecision -- Using the values provided, determine the longest precision
 *   in the arithSeries
 */

static inline double
power10(
    unsigned n)
{
    static const double powers[] = {
	1, 10, 100, 1000, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12,
	1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20
    };

    if (n < sizeof(powers) / sizeof(*powers)) {
	return powers[n];
    } else {
	// Not an expected case. Doesn't need to be so fast
	return pow(10, n);
    }
}

static inline double
ArithRound(
    double d,
    unsigned n)
{
    double scalefactor = power10(n);
    return round(d * scalefactor) / scalefactor;
}

static inline double
ArithSeriesIndexDbl(
    ArithSeries *arithSeriesRepPtr,
    Tcl_WideInt index)
{
    if (arithSeriesRepPtr->isDouble) {
	ArithSeriesDbl *dblRepPtr = (ArithSeriesDbl *) arithSeriesRepPtr;
	double d = dblRepPtr->start + (index * dblRepPtr->step);

	return ArithRound(d, dblRepPtr->precision);
    } else {
	ArithSeriesInt *intRepPtr = (ArithSeriesInt *) arithSeriesRepPtr;
	return (double)(intRepPtr->start + (index * intRepPtr->step));
    }
}

static inline Tcl_WideInt
ArithSeriesIndexInt(
    ArithSeries *arithSeriesRepPtr,
    Tcl_WideInt index)
{
    if (arithSeriesRepPtr->isDouble) {
	ArithSeriesDbl *dblRepPtr = (ArithSeriesDbl *) arithSeriesRepPtr;
	return (Tcl_WideInt) (dblRepPtr->start + (index * dblRepPtr->step));
    } else {
	ArithSeriesInt *intRepPtr = (ArithSeriesInt *) arithSeriesRepPtr;
	return intRepPtr->start + (index * intRepPtr->step);
    }
}

static inline ArithSeries *
ArithSeriesGetInternalRep(
    Tcl_Obj *objPtr)
{
    const Tcl_ObjInternalRep *irPtr = TclFetchInternalRep(objPtr,
	    &arithSeriesType);
    return irPtr ? (ArithSeries *) irPtr->twoPtrValue.ptr1 : NULL;
}

/*
 * Compute number of significant fractional digits
 */
static inline unsigned
Precision(
    double d)
{
    char tmp[TCL_DOUBLE_SPACE + 2], *off;

    tmp[0] = 0;
    Tcl_PrintDouble(NULL, d, tmp);
    off = strchr(tmp, '.');
    return (off ? strlen(off + 1) : 0);
}

/*
 * Find longest number of digits after the decimal point.
 */
static inline unsigned
maxPrecision(
    double start,
    double end,
    double step)
{
    unsigned dp = Precision(step);
    unsigned i = Precision(start);

    dp = i>dp ? i : dp;
    i  = Precision(end);
    dp = i>dp ? i : dp;
    return dp;
}

/*
 *----------------------------------------------------------------------
 *
 * ArithSeriesLen --
 *
 *	Compute the length of the equivalent list where every element is generated
 *	starting from *start*, and adding *step* to generate every successive
 *	element that's < *end* for positive steps, or > *end* for negative steps.
 *
 * Results:
 *
 *	The length of the list generated by the given range, that may be zero.  The
 *	function returns -1 if the list is of length infinite.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
static Tcl_WideInt
ArithSeriesLenInt(
    Tcl_WideInt start,
    Tcl_WideInt end,
    Tcl_WideInt step)
{
    Tcl_WideInt len;

    if (step == 0) {
	return 0;
    }
    len = 1 + ((end - start) / step);
    return (len < 0) ? -1 : len;
}

static Tcl_WideInt
ArithSeriesLenDbl(
    double start,
    double end,
    double step,
    unsigned precision)
{
    double istart, iend, istep, ilen;

    if (step == 0) {
	return 0;
    }
    istart = start * power10(precision);
    iend = end * power10(precision);
    istep = step * power10(precision);
    ilen = (iend - istart + istep) / istep;
    return floor(ilen);
}

/*
 *----------------------------------------------------------------------
 *
 * NewArithSeriesInt --
 *
 *	Creates a new ArithSeries object. The returned object has
 *	refcount = 0.
 *
 * Results:
 *	A Tcl_Obj pointer to the created ArithSeries object.
 *	A NULL pointer of the range is invalid.
 *
 * Side Effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
static Tcl_Obj *
NewArithSeriesInt(
    Tcl_WideInt start,
    Tcl_WideInt end,
    Tcl_WideInt step,
    Tcl_WideInt len)
{
    Tcl_WideInt length;
    Tcl_Obj *arithSeriesObj;
    ArithSeriesInt *arithSeriesRepPtr;

    length = len>=0 ? len : -1;
    if (length < 0) {
	length = -1;
    }

    TclNewObj(arithSeriesObj);

    if (length <= 0) {
	return arithSeriesObj;
    }

    arithSeriesRepPtr = (ArithSeriesInt *) Tcl_Alloc(sizeof(ArithSeriesInt));
    arithSeriesRepPtr->base.len = length;
    arithSeriesRepPtr->base.elements = NULL;
    arithSeriesRepPtr->base.isDouble = 0;
    arithSeriesRepPtr->start = start;
    arithSeriesRepPtr->end = end;
    arithSeriesRepPtr->step = step;
    arithSeriesObj->internalRep.twoPtrValue.ptr1 = arithSeriesRepPtr;
    arithSeriesObj->internalRep.twoPtrValue.ptr2 = NULL;
    arithSeriesObj->typePtr = (Tcl_ObjType *)&arithSeriesType;
    if (length > 0) {
	Tcl_InvalidateStringRep(arithSeriesObj);
    }

    return arithSeriesObj;
}

/*
 *----------------------------------------------------------------------
 *
 * NewArithSeriesDbl --
 *
 *	Creates a new ArithSeries object with doubles. The returned object has
 *	refcount = 0.
 *
 * Results:
 *	A Tcl_Obj pointer to the created ArithSeries object.
 *	A NULL pointer of the range is invalid.
 *
 * Side Effects:
 *	None.
 *----------------------------------------------------------------------
 */
static Tcl_Obj *
NewArithSeriesDbl(
    double start,
    double end,
    double step,
    Tcl_WideInt len)
{
    Tcl_WideInt length;
    Tcl_Obj *arithSeriesObj;
    ArithSeriesDbl *arithSeriesRepPtr;

    length = len>=0 ? len : -1;
    if (length < 0) {
	length = -1;
    }

    TclNewObj(arithSeriesObj);

    if (length <= 0) {
	return arithSeriesObj;
    }

    arithSeriesRepPtr = (ArithSeriesDbl *) Tcl_Alloc(sizeof(ArithSeriesDbl));
    arithSeriesRepPtr->base.len = length;
    arithSeriesRepPtr->base.elements = NULL;
    arithSeriesRepPtr->base.isDouble = 1;
    arithSeriesRepPtr->start = start;
    arithSeriesRepPtr->end = end;
    arithSeriesRepPtr->step = step;
    arithSeriesRepPtr->precision = maxPrecision(start, end, step);
    arithSeriesObj->internalRep.twoPtrValue.ptr1 = arithSeriesRepPtr;
    arithSeriesObj->internalRep.twoPtrValue.ptr2 = NULL;
    arithSeriesObj->typePtr = (Tcl_ObjType *)&arithSeriesType;

    if (length > 0) {
	Tcl_InvalidateStringRep(arithSeriesObj);
    }

    return arithSeriesObj;
}

/*
 *----------------------------------------------------------------------
 *
 * assignNumber --
 *
 *	Create the appropriate Tcl_Obj value for the given numeric values.
 *	Used locally only for decoding [lseq] numeric arguments.
 *	refcount = 0.
 *
 * Results:
 *	A Tcl_Obj pointer.  No assignment on error.
 *
 * Side Effects:
 *	None.
 *----------------------------------------------------------------------
 */
static int
assignNumber(
    Tcl_Interp *interp,
    int useDoubles,
    Tcl_WideInt *intNumberPtr,
    double *dblNumberPtr,
    Tcl_Obj *numberObj)
{
    void *clientData;
    int tcl_number_type;

    if (Tcl_GetNumberFromObj(interp, numberObj, &clientData,
		&tcl_number_type) != TCL_OK) {
	return TCL_ERROR;
    }
    if (tcl_number_type == TCL_NUMBER_BIG) {
	/* bignum is not supported yet. */
	Tcl_WideInt w;
	(void)Tcl_GetWideIntFromObj(interp, numberObj, &w);
	return TCL_ERROR;
    }
    if (useDoubles) {
	if (tcl_number_type != TCL_NUMBER_INT) {
	    *dblNumberPtr = *(double *)clientData;
	} else {
	    *dblNumberPtr = (double)*(Tcl_WideInt *)clientData;
	}
    } else {
	if (tcl_number_type == TCL_NUMBER_INT) {
	    *intNumberPtr = *(Tcl_WideInt *)clientData;
	} else {
	    *intNumberPtr = (Tcl_WideInt)*(double *)clientData;
	}
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * TclNewArithSeriesObj --
 *
 *	Creates a new ArithSeries object. Some arguments may be NULL and will
 *	be computed based on the other given arguments.
 *	refcount = 0.
 *
 * Results:
 *	A Tcl_Obj pointer to the created ArithSeries object.
 *	NULL if the range is invalid.
 *
 * Side Effects:
 *	None.
 *----------------------------------------------------------------------
 */
Tcl_Obj *
TclNewArithSeriesObj(
    Tcl_Interp *interp,       /* For error reporting */
    int useDoubles,           /* Flag indicates values start,
			      ** end, step, are treated as doubles */
    Tcl_Obj *startObj,        /* Starting value */
    Tcl_Obj *endObj,          /* Ending limit */
    Tcl_Obj *stepObj,         /* increment value */
    Tcl_Obj *lenObj)          /* Number of elements */
{
    double dstart, dend, dstep;
    Tcl_WideInt start, end, step;
    Tcl_WideInt len = -1;
    Tcl_Obj *objPtr;

    if (startObj) {
	if (assignNumber(interp, useDoubles, &start, &dstart, startObj) != TCL_OK) {
	    return NULL;
	}
    } else {
	start = 0;
	dstart = start;
    }
    if (stepObj) {
	if (assignNumber(interp, useDoubles, &step, &dstep, stepObj) != TCL_OK) {
	    return NULL;
	}
	if (useDoubles) {
	    step = dstep;
	} else {
	    dstep = step;
	}
	if (dstep == 0) {
	    TclNewObj(objPtr);
	    return objPtr;
	}
    }
    if (endObj) {
	if (assignNumber(interp, useDoubles, &end, &dend, endObj) != TCL_OK) {
	    return NULL;
	}
    }
    if (lenObj) {
	if (TCL_OK != Tcl_GetWideIntFromObj(interp, lenObj, &len)) {
	    return NULL;
	}
    }

    if (startObj && endObj) {
	if (!stepObj) {
	    if (useDoubles) {
		dstep = (dstart < dend) ? 1.0 : -1.0;
		step = dstep;
	    } else {
		step = (start < end) ? 1 : -1;
		dstep = step;
	    }
	}
	assert(dstep!=0);
	if (!lenObj) {
	    if (useDoubles) {
		unsigned precision = maxPrecision(dstart, dend, dstep);
		len = ArithSeriesLenDbl(dstart, dend, dstep, precision);
	    } else {
		len = ArithSeriesLenInt(start, end, step);
	    }
	}
    }

    if (!endObj) {
	if (useDoubles) {
	    // Compute precision based on given command argument values
	    unsigned precision = maxPrecision(dstart, len, dstep);

	    dend = dstart + (dstep * (len-1));
            // Make computed end value match argument(s) precision
            dend = ArithRound(dend, precision);
            end = dend;
	} else {
	    end = start + (step * (len - 1));
	    dend = end;
	}
    }

    if (len > TCL_SIZE_MAX) {
	Tcl_SetObjResult(interp, Tcl_NewStringObj(
		"max length of a Tcl list exceeded", TCL_AUTO_LENGTH));
	Tcl_SetErrorCode(interp, "TCL", "MEMORY", (char *)NULL);
	return NULL;
    }

    objPtr = (useDoubles)
	    ? NewArithSeriesDbl(dstart, dend, dstep, len)
	    : NewArithSeriesInt(start, end, step, len);
    return objPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * ArithSeriesObjIndex --
 *
 *	Stores in **resultPtr the element with the specified index in the list
 *	represented by the specified Arithmetic Sequence object. 
 *
 * Results:
 *
 * 	On success, returns TCL_OK and stores the position of the element in
 * 	*element.  Returns TCL_ERROR if the given index is out of range.
 *
 * Side Effects:
 *	On success, stores the stores the position of the element in *elemObj,
 *	and on failure, stores the empty string ("").
 *
 *----------------------------------------------------------------------
 */
int
ArithSeriesObjIndex(
    TCL_UNUSED(Tcl_Interp *),
    Tcl_Obj *arithSeriesObj,	/* List obj */
    Tcl_Size index,		/* index to element of interest */
    Tcl_Obj **elemObj)		/* Return value */
{
    ArithSeries *arithSeriesRepPtr = ArithSeriesGetInternalRep(arithSeriesObj);

    if (index < 0 || arithSeriesRepPtr->len <= index) {
	*elemObj = NULL;
    } else {
	/* List[i] = Start + (Step * index) */
	if (arithSeriesRepPtr->isDouble) {
	    *elemObj = Tcl_NewDoubleObj(ArithSeriesIndexDbl(arithSeriesRepPtr, index));
	} else {
	    *elemObj = Tcl_NewWideIntObj(ArithSeriesIndexInt(arithSeriesRepPtr, index));
	}
    }

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ArithSeriesObjLength
 *
 *	Returns the length of the arithmetic series.
 *
 * Results:
 *	The length of the series as Tcl_WideInt.
 *
 * Side Effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
int ArithSeriesObjLength(TCL_UNUSED(Tcl_Interp *),
    Tcl_Obj *arithSeriesObj
	,Tcl_Size *result)
{
    ArithSeries *arithSeriesRepPtr = (ArithSeries *)
	    arithSeriesObj->internalRep.twoPtrValue.ptr1;
    *result = arithSeriesRepPtr->len;
    return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * DupArithSeriesInternalRep --
 *
 *	Initialize the internal representation of a arithseries Tcl_Obj to a
 *	copy of the internal representation of an existing arithseries object.
 *	The copy does not share the cache of the elements.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	We set "copyPtr"s internal rep to a pointer to a
 *	newly allocated ArithSeries structure.
 *
 *----------------------------------------------------------------------
 */

static void
DupArithSeriesInternalRep(
    Tcl_Obj *srcPtr,		/* Object with internal rep to copy. */
    Tcl_Obj *copyPtr)		/* Object with internal rep to set. */
{
    ArithSeries *srcRepPtr = (ArithSeries *)
	    srcPtr->internalRep.twoPtrValue.ptr1;

    if (srcRepPtr->isDouble) {
	ArithSeriesDbl *srcDblPtr = (ArithSeriesDbl *) srcRepPtr;
	ArithSeriesDbl *copyDblPtr = (ArithSeriesDbl *)
		Tcl_Alloc(sizeof(ArithSeriesDbl));

	*copyDblPtr = *srcDblPtr;
	copyDblPtr->base.elements = NULL;
	copyPtr->internalRep.twoPtrValue.ptr1 = copyDblPtr;
    } else {
	ArithSeriesInt *srcIntPtr = (ArithSeriesInt *) srcRepPtr;
	ArithSeriesInt *copyIntPtr = (ArithSeriesInt *)
		Tcl_Alloc(sizeof(ArithSeriesInt));

	*copyIntPtr = *srcIntPtr;
	copyIntPtr->base.elements = NULL;
	copyPtr->internalRep.twoPtrValue.ptr1 = copyIntPtr;
    }
    copyPtr->internalRep.twoPtrValue.ptr2 = NULL;
    copyPtr->typePtr = (Tcl_ObjType *)&arithSeriesType;
}

/*
 *----------------------------------------------------------------------
 *
 * FreeArithSeriesInternalRep --
 *
 *	Free any allocated memory in the ArithSeries Rep
 *
 * Results:
 *	None.
 *
 * Side effects:
 *
 *----------------------------------------------------------------------
 */

static inline void
FreeElements(
    ArithSeries *arithSeriesRepPtr)
{
    if (arithSeriesRepPtr->elements) {
	Tcl_WideInt i, len = arithSeriesRepPtr->len;

	for (i=0; i<len; i++) {
	    Tcl_DecrRefCount(arithSeriesRepPtr->elements[i]);
	}
	Tcl_Free((char *) arithSeriesRepPtr->elements);
	arithSeriesRepPtr->elements = NULL;
    }
}

static void
FreeArithSeriesInternalRep(
    Tcl_Obj *arithSeriesObjPtr)
{
    ArithSeries *arithSeriesRepPtr = (ArithSeries *)
	    arithSeriesObjPtr->internalRep.twoPtrValue.ptr1;

    if (arithSeriesRepPtr) {
	FreeElements(arithSeriesRepPtr);
	Tcl_Free((char *) arithSeriesRepPtr);
    }
}


/*
 *----------------------------------------------------------------------
 *
 * ArithSeriesObjStep --
 *
 *	Return a Tcl_Obj with the step value from the give ArithSeries Obj.
 *	refcount = 0.
 *
 * Results:
 *	A Tcl_Obj pointer to the created ArithSeries object.
 *	A NULL pointer of the range is invalid.
 *
 * Side Effects:
 *	None.
 *----------------------------------------------------------------------
 */

static int
ArithSeriesObjStep(
    Tcl_Obj *arithSeriesObj,
    Tcl_Obj **stepObj)
{
    ArithSeries *arithSeriesRepPtr = ArithSeriesGetInternalRep(arithSeriesObj);

    if (arithSeriesRepPtr->isDouble) {
	*stepObj = Tcl_NewDoubleObj(((ArithSeriesDbl *) arithSeriesRepPtr)->step);
    } else {
	*stepObj = Tcl_NewWideIntObj(((ArithSeriesInt *) arithSeriesRepPtr)->step);
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * SetArithSeriesFromAny --
 *
 *	The Arithmetic Series object is just an way to optimize
 *	Lists space complexity, so no one should try to convert
 *	a string to an Arithmetic Series object.
 *
 *	This function is here just to populate the Type structure.
 *
 * Results:
 *	The result is always TCL_ERROR. But see Side Effects.
 *
 * Side effects:
 *	Tcl Panic if called.
 *
 *----------------------------------------------------------------------
 */

static int
SetArithSeriesFromAny(
    TCL_UNUSED(Tcl_Interp *),	/* Used for error reporting if not NULL. */
    TCL_UNUSED(Tcl_Obj *))	/* The object to convert. */
{
    Tcl_Panic("SetArithSeriesFromAny: should never be called");
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * ArithSeriesObjRange --
 *
 *	Makes a slice of an ArithSeries value.
 *	*arithSeriesObj must be known to be a valid list.
 *
 * Results:
 *	Returns a pointer to the sliced series.
 *	This may be a new object or the same object if not shared.
 *
 * Side effects:
 *	?The possible conversion of the object referenced by listPtr?
 *	?to a list object.?
 *
 *----------------------------------------------------------------------
 */

int
ArithSeriesObjRange(
    Tcl_Interp *interp,		/* For error message(s) */
    Tcl_Obj *arithSeriesObj,/* List object to take a range from. */
    Tcl_Size fromIdx,		/* Index of first element to include. */
    Tcl_Size toIdx,			/* Index of last element to include. */
    Tcl_Obj **resPtrPtr)	/* return value */
{
    ArithSeries *arithSeriesRepPtr;
    Tcl_Obj *startObj, *endObj, *stepObj;

    (void)interp; /* silence compiler */

    arithSeriesRepPtr = ArithSeriesGetInternalRep(arithSeriesObj);

    if (fromIdx == TCL_INDEX_NONE) {
	fromIdx = 0;
    }

    if (toIdx >= arithSeriesRepPtr->len) {
	toIdx = arithSeriesRepPtr->len-1;
    }

    if (fromIdx > toIdx || fromIdx >= arithSeriesRepPtr->len) {
	TclNewObj(*resPtrPtr);
	return TCL_OK;
    }

    if (fromIdx < 0) {
	fromIdx = 0;
    }
    if (toIdx < 0) {
	toIdx = 0;
    }
    if (toIdx > arithSeriesRepPtr->len - 1) {
	toIdx = arithSeriesRepPtr->len - 1;
    }

    ArithSeriesObjIndex(interp, arithSeriesObj, fromIdx, &startObj);
    Tcl_IncrRefCount(startObj);
    ArithSeriesObjIndex(interp, arithSeriesObj, toIdx, &endObj);
    Tcl_IncrRefCount(endObj);
    ArithSeriesObjStep(arithSeriesObj, &stepObj);
    Tcl_IncrRefCount(stepObj);

    if (Tcl_IsShared(arithSeriesObj) || ((arithSeriesObj->refCount > 1))) {
	Tcl_Obj *newSlicePtr = TclNewArithSeriesObj(interp,
	    arithSeriesRepPtr->isDouble, startObj, endObj, stepObj, NULL);
	*resPtrPtr = newSlicePtr;
	Tcl_DecrRefCount(startObj);
	Tcl_DecrRefCount(endObj);
	Tcl_DecrRefCount(stepObj);
	return newSlicePtr ? TCL_OK : TCL_ERROR;
    }

    /*
     * In-place is possible.
     */

    /*
     * Even if nothing below causes any changes, we still want the
     * string-canonizing effect of [lrange 0 end].
     */

    TclInvalidateStringRep(arithSeriesObj);

    if (arithSeriesRepPtr->isDouble) {
	ArithSeriesDbl *dblRepPtr = (ArithSeriesDbl *) arithSeriesRepPtr;
	double start, end, step;

	Tcl_GetDoubleFromObj(NULL, startObj, &start);
	Tcl_GetDoubleFromObj(NULL, endObj, &end);
	Tcl_GetDoubleFromObj(NULL, stepObj, &step);
	dblRepPtr->start = start;
	dblRepPtr->end = end;
	dblRepPtr->step = step;
	dblRepPtr->precision = maxPrecision(start, end, step);
	FreeElements(arithSeriesRepPtr);
	dblRepPtr->base.len =
		ArithSeriesLenDbl(start, end, step, dblRepPtr->precision);
    } else {
	ArithSeriesInt *intRepPtr = (ArithSeriesInt *) arithSeriesRepPtr;
	Tcl_WideInt start, end, step;

	Tcl_GetWideIntFromObj(NULL, startObj, &start);
	Tcl_GetWideIntFromObj(NULL, endObj, &end);
	Tcl_GetWideIntFromObj(NULL, stepObj, &step);
	intRepPtr->start = start;
	intRepPtr->end = end;
	intRepPtr->step = step;
	FreeElements(arithSeriesRepPtr);
	intRepPtr->base.len = ArithSeriesLenInt(start, end, step);
    }

    Tcl_DecrRefCount(startObj);
    Tcl_DecrRefCount(endObj);
    Tcl_DecrRefCount(stepObj);

    *resPtrPtr = arithSeriesObj;
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ArithSeriesGetElements --
 *
 *	This function returns an (objc,objv) array of the elements in a list
 *	object.
 *
 * Results:
 *	The return value is normally TCL_OK; in this case *objcPtr is set to
 *	the count of list elements and *objvPtr is set to a pointer to an
 *	array of (*objcPtr) pointers to each list element. If listPtr does not
 *	refer to an Abstract List object and the object can not be converted
 *	to one, TCL_ERROR is returned and an error message will be left in the
 *	interpreter's result if interp is not NULL.
 *
 *	The objects referenced by the returned array should be treated as
 *	readonly and their ref counts are _not_ incremented; the caller must
 *	do that if it holds on to a reference. Furthermore, the pointer and
 *	length returned by this function may change as soon as any function is
 *	called on the list object; be careful about retaining the pointer in a
 *	local data structure.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int
ArithSeriesGetElements(
    Tcl_Interp *interp,		/* Used to report errors if not NULL. */
    Tcl_Obj *objPtr,		/* ArithSeries object for which an element
				 * array is to be returned. */
    Tcl_Size *objcPtr,		/* Where to store the count of objects
				 * referenced by objv. */
    Tcl_Obj ***objvPtr)		/* Where to store the pointer to an array of
				 * pointers to the list's objects. */
{
    if (TclHasInternalRep(objPtr,(Tcl_ObjType *)&arithSeriesType)) {
	ArithSeries *arithSeriesRepPtr = ArithSeriesGetInternalRep(objPtr);
	Tcl_Obj **objv;
	Tcl_Size objc = arithSeriesRepPtr->len;

	if (objc > 0) {
	    if (arithSeriesRepPtr->elements) {
		/* If this exists, it has already been populated */
		objv = arithSeriesRepPtr->elements;
	    } else {
		/* Construct the elements array */
		objv = (Tcl_Obj **) Tcl_Alloc(sizeof(Tcl_Obj*) * objc);
		if (objv == NULL) {
		    if (interp) {
			Tcl_SetObjResult(interp, Tcl_NewStringObj(
				"max length of a Tcl list exceeded",
				TCL_AUTO_LENGTH));
			Tcl_SetErrorCode(interp, "TCL", "MEMORY", (char *)NULL);
		    }
		    return TCL_ERROR;
		}
		arithSeriesRepPtr->elements = objv;

		Tcl_Size i;
		for (i = 0; i < objc; i++) {
		    int status = ArithSeriesObjIndex(interp, objPtr, i, &objv[i]);

		    if (status) {
			return TCL_ERROR;
		    }
		    Tcl_IncrRefCount(objv[i]);
		}
	    }
	} else {
	    objv = NULL;
	}
	*objvPtr = objv;
	*objcPtr = objc;
    } else {
	if (interp != NULL) {
	    Tcl_SetObjResult(interp, Tcl_NewStringObj(
		    "value is not an arithseries", TCL_AUTO_LENGTH));
	    Tcl_SetErrorCode(interp, "TCL", "VALUE", "UNKNOWN", (char *)NULL);
	}
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ArithSeriesObjReverse --
 *
 *	Reverse the order of the ArithSeries value. The arithSeriesObj is
 *	assumed to be a valid ArithSeries. The new Obj has the Start and End
 *	values appropriately swapped and the Step value sign is changed.
 *
 * Results:
 *	The result will be an ArithSeries in the reverse order.
 *
 * Side effects:
 *	The original obj will be modified and returned if it is not Shared.
 *
 *----------------------------------------------------------------------
 */
int
ArithSeriesObjReverse(
    Tcl_Interp *interp,		/* For error message(s) */
    Tcl_Obj *arithSeriesObj	/* List object to reverse. */
    )
{
    ArithSeries *arithSeriesRepPtr;
    Tcl_Obj *startObj, *endObj, *stepObj;
    Tcl_WideInt start, end, step, len;
    double dstart, dend, dstep;
    int isDouble;

    (void)interp;

    if (Tcl_IsShared(arithSeriesObj)) {
	Tcl_Panic("%s called with shared object", "ArithSeriesObjReverse");
    }

    arithSeriesRepPtr = ArithSeriesGetInternalRep(arithSeriesObj);

    isDouble = arithSeriesRepPtr->isDouble;
    len = arithSeriesRepPtr->len;

    ArithSeriesObjIndex(NULL, arithSeriesObj, len - 1, &startObj);
    Tcl_IncrRefCount(startObj);
    ArithSeriesObjIndex(NULL, arithSeriesObj, 0, &endObj);
    Tcl_IncrRefCount(endObj);
    ArithSeriesObjStep(arithSeriesObj, &stepObj);
    Tcl_IncrRefCount(stepObj);

    if (isDouble) {
	Tcl_GetDoubleFromObj(NULL, startObj, &dstart);
	Tcl_GetDoubleFromObj(NULL, endObj, &dend);
	Tcl_GetDoubleFromObj(NULL, stepObj, &dstep);
	dstep = -dstep;
	TclSetDoubleObj(stepObj, dstep);
    } else {
	Tcl_GetWideIntFromObj(NULL, startObj, &start);
	Tcl_GetWideIntFromObj(NULL, endObj, &end);
	Tcl_GetWideIntFromObj(NULL, stepObj, &step);
	step = -step;
	TclSetIntObj(stepObj, step);
    }

    TclInvalidateStringRep(arithSeriesObj);

    if (isDouble) {
	ArithSeriesDbl *dblRepPtr = (ArithSeriesDbl *) arithSeriesRepPtr;

	dblRepPtr->start = dstart;
	dblRepPtr->end = dend;
	dblRepPtr->step = dstep;
    } else {
	ArithSeriesInt *intRepPtr = (ArithSeriesInt *) arithSeriesRepPtr;
	intRepPtr->start = start;
	intRepPtr->end = end;
	intRepPtr->step = step;
    }
    FreeElements(arithSeriesRepPtr);

    Tcl_DecrRefCount(startObj);
    Tcl_DecrRefCount(endObj);
    Tcl_DecrRefCount(stepObj);

    return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * UpdateStringOfArithSeries --
 *
 *	Update the string representation for an arithseries object.
 *	Note: This procedure does not invalidate an existing old string rep
 *	so storage will be lost if this has not already been done.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The object's string is set to a valid string that results from
 *	the list-to-string conversion. This string will be empty if the
 *	list has no elements. The list internal representation
 *	should not be NULL and we assume it is not NULL.
 *
 * Notes:
 *	At the cost of overallocation it's possible to estimate
 *	the length of the string representation and make this procedure
 *	much faster. Because the programmer shouldn't expect the
 *	string conversion of a big arithmetic sequence to be fast
 *	this version takes more care of space than time.
 *
 *----------------------------------------------------------------------
 */
static void
UpdateStringOfArithSeries(Tcl_Obj *arithSeriesPtr)
{
    ArithSeries *arithSeriesRepPtr = (ArithSeries *)
	arithSeriesPtr->internalRep.twoPtrValue.ptr1;
    char *p;
    Tcl_Obj *elemObj;
    Tcl_Size i, bytlen = 0;

    /*
     * Pass 1: estimate space.
     */
    if (!arithSeriesRepPtr->isDouble) {
	for (i = 0; i < arithSeriesRepPtr->len; i++) {
	    double d = ArithSeriesIndexInt(arithSeriesRepPtr, i);
	    size_t slen = d>0 ? log10(d)+1 : d<0 ? log10(-d)+2 : 1;

	    bytlen += slen;
	}
    } else {
	for (i = 0; i < arithSeriesRepPtr->len; i++) {
	    double d = ArithSeriesIndexDbl(arithSeriesRepPtr, i);
	    char tmp[TCL_DOUBLE_SPACE + 2];

	    tmp[0] = 0;
	    Tcl_PrintDouble(NULL, d, tmp);
	    if ((bytlen + strlen(tmp)) > TCL_SIZE_MAX) {
		break; // overflow
	    }
	    bytlen += strlen(tmp);
	}
    }
    bytlen += arithSeriesRepPtr->len; // Space for each separator

    /*
     * Pass 2: generate the string repr.
     */

    p = Tcl_InitStringRep(arithSeriesPtr, NULL, bytlen);
    for (i = 0; i < arithSeriesRepPtr->len; i++) {
	if (ArithSeriesObjIndex(NULL, arithSeriesPtr, i, &elemObj) == TCL_OK) {
	    Tcl_Size slen;
	    char *str = Tcl_GetStringFromObj(elemObj, &slen);

	    strcpy(p, str);
	    p[slen] = ' ';
	    p += slen + 1;
	    Tcl_DecrRefCount(elemObj);
	} // else TODO: report error here?
    }
    if (bytlen > 0) {
	arithSeriesPtr->bytes[bytlen - 1] = '\0';
    }
    arithSeriesPtr->length = bytlen - 1;
    return;
}


/*
 *----------------------------------------------------------------------
 *
 * ArithSeriesInOperator --
 *
 *	Evaluate the "in" operation for expr
 *
 *	This can be done more efficiently in the Arith Series relative to
 *	doing a linear search as implemented in expr.
 *
 * Results:
 *	Boolean true or false (1/0)
 *
 * Side effects:
 *	None
 *
 *----------------------------------------------------------------------
 */

static int
ArithSeriesInOperation(
    Tcl_Interp *interp,
    Tcl_Obj *arithSeriesObjPtr,
    Tcl_Obj *valueObj,
    int *boolResult)
{
    ArithSeries *repPtr = (ArithSeries *)
	    arithSeriesObjPtr->internalRep.twoPtrValue.ptr1;
    int status;
    Tcl_Size index, incr, elen, vlen;

    if (repPtr->isDouble) {
	ArithSeriesDbl *dblRepPtr = (ArithSeriesDbl *) repPtr;
        double y;
	int test = 0;

	incr = 0; // Check index+incr where incr is 0 and 1
        status = Tcl_GetDoubleFromObj(interp, valueObj, &y);
        if (status != TCL_OK) {
	    test = 0;
        } else {
            char *vstr = Tcl_GetStringFromObj(valueObj, &vlen);
            index = (y - dblRepPtr->start) / dblRepPtr->step;
	    while (incr<2) {
		Tcl_Obj *elemObj;

		elen = 0;
		ArithSeriesObjIndex(interp, arithSeriesObjPtr, (index+incr), &elemObj);

		const char *estr = elemObj ? Tcl_GetStringFromObj(elemObj, &elen) : "";

		/* "in" operation defined as a string compare */
		test = (elen == vlen) ? (memcmp(estr, vstr, elen) == 0) : 0;
		Tcl_BounceRefCount(elemObj);
		/* Stop if we have a match */
		if (test) {
		    break;
		}
		incr++;
	    }
        }
	if (boolResult) {
	    *boolResult = test;
	}
    } else {
	ArithSeriesInt *intRepPtr = (ArithSeriesInt *) repPtr;
	Tcl_WideInt y;

	status = Tcl_GetWideIntFromObj(NULL, valueObj, &y);
	if (status != TCL_OK) {
	    if (boolResult) {
		*boolResult = 0;
	    }
	} else {
	    Tcl_Obj *elemObj;

	    elen = 0;
	    index = (y - intRepPtr->start) / intRepPtr->step;
	    ArithSeriesObjIndex(interp, arithSeriesObjPtr, index, &elemObj);

	    char const *vstr = Tcl_GetStringFromObj(valueObj, &vlen);
	    char const *estr = elemObj ? Tcl_GetStringFromObj(elemObj, &elen) : "";

	    if (boolResult) {
		*boolResult = (elen == vlen) ? (memcmp(estr, vstr, elen) == 0) : 0;
	    }
	    Tcl_BounceRefCount(elemObj);
	}
    }
    return TCL_OK;
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
