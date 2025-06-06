/*
 * Copyright (c) 1998-2000 Ajuba Solutions
 * Copyright (c) 1995-1997 Sun Microsystems, Inc.
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
 * tclIO.h --
 *
 *	This file provides the generic portions (those that are the same on
 *	all platforms and for all channel types) of Tcl's IO facilities.
 */

/*
 * Make sure that both EAGAIN and EWOULDBLOCK are defined. This does not
 * compile on systems where neither is defined. We want both defined so that
 * we can test safely for both. In the code we still have to test for both
 * because there may be systems on which both are defined and have different
 * values.
 */

#if ((!defined(EWOULDBLOCK)) && (defined(EAGAIN)))
#   define EWOULDBLOCK EAGAIN
#endif
#if ((!defined(EAGAIN)) && (defined(EWOULDBLOCK)))
#   define EAGAIN EWOULDBLOCK
#endif
#if ((!defined(EAGAIN)) && (!defined(EWOULDBLOCK)))
#error one of EWOULDBLOCK or EAGAIN must be defined
#endif

/*
 * struct ChannelBuffer:
 *
 * Buffers data being sent to or from a channel.
 */

typedef struct ChannelBuffer {
    Tcl_Size refCount;		/* Current uses count */
    Tcl_Size nextAdded;		/* The next position into which a character
				 * will be put in the buffer. */
    Tcl_Size nextRemoved;	/* Position of next byte to be removed from
				 * the buffer. */
    Tcl_Size bufLength;		/* How big is the buffer? */
    struct ChannelBuffer *nextPtr;
				/* Next buffer in chain. */
    char buf[TCLFLEXARRAY];	/* Placeholder for real buffer. The real
				 * buffer occupies this space + bufSize-1
				 * bytes. This must be the last field in the
				 * structure. */
} ChannelBuffer;

#define CHANNELBUFFER_HEADER_SIZE	offsetof(ChannelBuffer, buf)

/*
 * How much extra space to allocate in buffer to hold bytes from previous
 * buffer (when converting to UTF-8) or to hold bytes that will go to next
 * buffer (when converting from UTF-8).
 */

#define BUFFER_PADDING		16

/*
 * The following defines the *default* buffer size for channels.
 */

#define CHANNELBUFFER_DEFAULT_SIZE	(1024 * 4)

/*
 * The following structure describes the information saved from a call to
 * "fileevent". This is used later when the event being waited for to invoke
 * the saved script in the interpreter designed in this record.
 */

typedef struct EventScriptRecord {
    struct Channel *chanPtr;	/* The channel for which this script is
				 * registered. This is used only when an error
				 * occurs during evaluation of the script, to
				 * delete the handler. */
    Tcl_Obj *scriptPtr;		/* Script to invoke. */
    Tcl_Interp *interp;		/* In what interpreter to invoke script? */
    int mask;			/* Events must overlap current mask for the
				 * stored script to be invoked. */
    struct EventScriptRecord *nextPtr;
				/* Next in chain of records. */
} EventScriptRecord;

/*
 * struct Channel:
 *
 * One of these structures is allocated for each open channel. It contains
 * data specific to the channel but which belongs to the generic part of the
 * Tcl channel mechanism, and it points at an instance specific (and type
 * specific) instance data, and at a channel type structure.
 */

typedef struct Channel {
    struct ChannelState *state; /* Split out state information */
    void *instanceData;		/* Instance-specific data provided by creator
				 * of channel. */
    const Tcl_ChannelType *typePtr;
				/* Pointer to channel type structure. */
    struct Channel *downChanPtr;/* Refers to channel this one was stacked
				 * upon. This reference is NULL for normal
				 * channels. See Tcl_StackChannel. */
    struct Channel *upChanPtr;	/* Refers to the channel above stacked this
				 * one. NULL for the top most channel. */

    /*
     * Intermediate buffers to hold pre-read data for consumption by a newly
     * stacked transformation. See 'Tcl_StackChannel'.
     */

    ChannelBuffer *inQueueHead;	/* Points at first buffer in input queue. */
    ChannelBuffer *inQueueTail;	/* Points at last buffer in input queue. */

    Tcl_Size refCount;
} Channel;

/*
 * struct ChannelState:
 *
 * One of these structures is allocated for each open channel. It contains
 * data specific to the channel but which belongs to the generic part of the
 * Tcl channel mechanism, and it points at an instance specific (and type
 * specific) instance data, and at a channel type structure.
 */

typedef struct ChannelState {
    char *channelName;		/* The name of the channel instance in Tcl
				 * commands. Storage is owned by the generic
				 * IO code, is dynamically allocated. */
    int	flags;			/* OR'ed combination of the flags defined
				 * below. */
    Tcl_Encoding encoding;	/* Encoding to apply when reading or writing
				 * data on this channel. NULL means no
				 * encoding is applied to data. */
    Tcl_EncodingState inputEncodingState;
				/* Current encoding state, used when
				 * converting input data bytes to UTF-8. */
    int inputEncodingFlags;	/* Encoding flags to pass to conversion
				 * routine when converting input data bytes to
				 * UTF-8. May be TCL_ENCODING_START before
				 * converting first byte and TCL_ENCODING_END
				 * when EOF is seen. */
    Tcl_EncodingState outputEncodingState;
				/* Current encoding state, used when
				 * converting UTF-8 to output data bytes. */
    int outputEncodingFlags;	/* Encoding flags to pass to conversion
				 * routine when converting UTF-8 to output
				 * data bytes. May be TCL_ENCODING_START
				 * before converting first byte and
				 * TCL_ENCODING_END when EOF is seen. */
    TclEolTranslation inputTranslation;
				/* What translation to apply for end of line
				 * sequences on input? */
    TclEolTranslation outputTranslation;
				/* What translation to use for generating end
				 * of line sequences in output? */
    int inEofChar;		/* If nonzero, use this as a signal of EOF on
				 * input. */
    int unreportedError;	/* Non-zero if an error report was deferred
				 * because it happened in the background. The
				 * value is the POSIX error code. */
    Tcl_Size refCount;		/* How many interpreters hold references to
				 * this IO channel? */
    struct CloseCallback *closeCbPtr;
				/* Callbacks registered to be called when the
				 * channel is closed. */
    char *outputStage;		/* Temporary staging buffer used when
				 * translating EOL before converting from
				 * UTF-8 to external form. */
    ChannelBuffer *curOutPtr;	/* Current output buffer being filled. */
    ChannelBuffer *outQueueHead;/* Points at first buffer in output queue. */
    ChannelBuffer *outQueueTail;/* Points at last buffer in output queue. */
    ChannelBuffer *saveInBufPtr;/* Buffer saved for input queue - eliminates
				 * need to allocate a new buffer for "gets"
				 * that crosses buffer boundaries. */
    ChannelBuffer *inQueueHead;	/* Points at first buffer in input queue. */
    ChannelBuffer *inQueueTail;	/* Points at last buffer in input queue. */
    struct ChannelHandler *chPtr;/* List of channel handlers registered for
				  * this channel. */
    int interestMask;		/* Mask of all events this channel has
				 * handlers for. */
    EventScriptRecord *scriptRecordPtr;
				/* Chain of all scripts registered for event
				 * handlers ("fileevent") on this channel. */
    Tcl_Size bufSize;		/* What size buffers to allocate? */
    int eventQueued;
    struct CopyState *csPtrR;	/* State of background copy for which channel
				 * is input, or NULL. */
    struct CopyState *csPtrW;	/* State of background copy for which channel
				 * is output, or NULL. */
    Channel *topChanPtr;	/* Refers to topmost channel in a stack. Never
				 * NULL. */
    Channel *bottomChanPtr;	/* Refers to bottommost channel in a stack.
				 * This channel can be relied on to live as
				 * long as the channel state. Never NULL. */
    struct ChannelState *nextCSPtr;
				/* Next in list of channels currently open. */
    Tcl_ThreadId managingThread;/* TIP #10: Id of the thread managing this
				 * stack of channels. */

    /*
     * TIP #219 ... Info for the I/O system ...
     * Error message set by channel drivers, for the propagation of arbitrary
     * Tcl errors. This information, if present (chanMsg not NULL), takes
     * precedence over a Posix error code returned by a channel operation.
     */

    Tcl_Obj *chanMsg;
    Tcl_Obj *unreportedMsg;	/* Non-NULL if an error report was deferred
				 * because it happened in the background. The
				 * value is the chanMg, if any. #219's
				 * companion to 'unreportedError'. */
    Tcl_Size epoch;		/* Used to test validity of stored channelname
				 * lookup results. */
    int maxPerms;		/* TIP #220: Max access privileges
				 * the channel was created with. */
} ChannelState;

/*
 * Values for the flags field in Channel. Any OR'ed combination of the
 * following flags can be stored in the field. These flags record various
 * options and state bits about the channel. In addition to the flags below,
 * the channel can also have TCL_READABLE (1<<1) and TCL_WRITABLE (1<<2) set.
 */

#define CHANNEL_NONBLOCKING	(1<<6)	/* Channel is currently in nonblocking
					 * mode. */
#define BG_FLUSH_SCHEDULED	(1<<7)	/* A background flush of the queued
					 * output buffers has been
					 * scheduled. */
#define CHANNEL_CLOSED		(1<<8)	/* Channel has been closed. No further
					 * Tcl-level IO on the channel is
					 * allowed. */
#define CHANNEL_EOF		(1<<9)	/* EOF occurred on this channel. This
					 * bit is cleared before every input
					 * operation. */
#define CHANNEL_STICKY_EOF	(1<<10)	/* EOF occurred on this channel
					 * because we saw the input
					 * eofChar. This bit prevents clearing
					 * of the EOF bit before every input
					 * operation. */
#define CHANNEL_BLOCKED		(1<<11)	/* EWOULDBLOCK or EAGAIN occurred on
					 * this channel. This bit is cleared
					 * before every input or output
					 * operation. */
#define INPUT_SAW_CR		(1<<12)	/* Channel is in CRLF eol input
					 * translation mode and the last byte
					 * seen was a "\r". */
#define CHANNEL_DEAD		(1<<13)	/* The channel has been closed by the
					 * exit handler (on exit) but not
					 * deallocated. When any IO operation
					 * sees this flag on a channel, it
					 * does not call driver level
					 * functions to avoid referring to
					 * deallocated data. */
#define CHANNEL_NEED_MORE_DATA	(1<<14)	/* The last input operation failed
					 * because there was not enough data
					 * to complete the operation. This
					 * flag is set when gets fails to get
					 * a complete line or when read fails
					 * to get a complete character. When
					 * set, file events will not be
					 * delivered for buffered data until
					 * the state of the channel
					 * changes. */
#define CHANNEL_ENCODING_ERROR	(1<<15)	/* set if channel
					 * encountered an encoding error */
#define CHANNEL_RAW_MODE	(1<<16)	/* When set, notes that the Raw API is
					 * being used. */
#define CHANNEL_LINEBUFFERED	(1<<17)	/* Output to the channel must be
					 * flushed after every newline. */
#define CHANNEL_UNBUFFERED	(1<<18)	/* Output to the channel must always
					 * be flushed immediately. */
#define CHANNEL_INCLOSE		(1<<19)	/* Channel is currently being closed.
					 * Its structures are still live and
					 * usable, but it may not be closed
					 * again from within the close
					 * handler. */
#define CHANNEL_CLOSEDWRITE	(1<<21)	/* Channel write side has been closed.
					 * No further Tcl-level write IO on
					 * the channel is allowed. */

/*
 * The length of time to wait between synthetic timer events. Must be zero or
 * bad things tend to happen.
 */

#define SYNTHETIC_EVENT_TIME	0

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
