#! /usr/bin/env tclsh

# Copyright © 1998-1999 Scriptics Corporation.
# All rights reserved.

# You may distribute and/or modify this program under the terms of the GNU
# Affero General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.

# uniParse.tcl --
#
#	This program parses the UnicodeData file and generates the
#	corresponding tclUniData.c file with compressed character
#	data tables.  The input to this program should be the latest
#	UnicodeData file from:
#	    ftp://ftp.unicode.org/Public/UNIDATA/UnicodeData.txt


namespace eval uni {
    set shift 5;		# number of bits of data within a page
				# This value can be adjusted to find the
				# best split to minimize table size

    variable pMap;		# map from page to page index, each entry is
				# an index into the pages table, indexed by
				# page number
    variable pages;		# map from page index to page info, each
				# entry is a list of indices into the groups
				# table, the list is indexed by the offset
    variable groups;		# list of character info values, indexed by
				# group number, initialized with the
				# unassigned character group

    variable categories {
	Cn Lu Ll Lt Lm Lo Mn Me Mc Nd Nl No Zs Zl Zp
	Cc Cf Co Cs Pc Pd Ps Pe Pi Pf Po Sm Sc Sk So
    };				# Ordered list of character categories, must
				# match the enumeration in the header file.
}

proc uni::getValue {items index} {
    variable categories

    # Extract character info

    set category [lindex $items 2]
    if {[scan [lindex $items 12] %x toupper] == 1} {
	set toupper [expr {$index - $toupper}]
    } else {
	set toupper 0
    }
    if {[scan [lindex $items 13] %x tolower] == 1} {
	set tolower [expr {$tolower - $index}]
    } else {
	set tolower 0
    }
    if {[scan [lindex $items 14] %x totitle] == 1} {
	set totitle [expr {$index - $totitle}]
    } elseif {$tolower} {
	set totitle 0
    } else {
	set totitle $toupper
    }

    set categoryIndex [lsearch -exact $categories $category]
    if {$categoryIndex < 0} {
	error "Unexpected character category: $index($category)"
    }

    return [list $categoryIndex $toupper $tolower $totitle]
}

proc uni::getGroup {value} {
    variable groups

    set gIndex [lsearch -exact $groups $value]
    if {$gIndex < 0} {
	set gIndex [llength $groups]
	lappend groups $value
    }
    return $gIndex
}

proc uni::addPage {info} {
    variable pMap
    variable pages
    variable shift

    set pIndex [lsearch -exact $pages $info]
    if {$pIndex < 0} {
	set pIndex [llength $pages]
	lappend pages $info
    }
    lappend pMap [expr {$pIndex << $shift}]
    return
}

proc uni::buildTables {data} {
    variable shift

    variable pMap {}
    variable pages {}
    variable groups {{0 0 0 0}}
    variable next 0
    set info {}			;# temporary page info

    set mask [expr {(1 << $shift) - 1}]

    foreach line [split $data \n] {
	if {$line eq ""} {
	    if {!($next & $mask)} {
		# next character is already on page boundary
		continue
	    }
	    # fill remaining page
	    set line [format %X [expr {($next-1)|$mask}]]
	    append line ";;Cn;0;ON;;;;;N;;;;;\n"
	}

	set items [split $line \;]

	scan [lindex $items 0] %x index
	if {$index > 0x3FFFF} then {
	    # Ignore characters > plane 3
	    continue
	}
	set index [format %d $index]

	set gIndex [getGroup [getValue $items $index]]

	# Since the input table omits unassigned characters, these will
	# show up as gaps in the index sequence.  There are a few special cases
	# where the gaps correspond to a uniform block of assigned characters.
	# These are indicated as such in the character name.

	# Enter all unassigned characters up to the current character.
	if {($index > $next) \
		&& ![regexp "Last>$" [lindex $items 1]]} {
	    for {} {$next < $index} {incr next} {
		lappend info 0
		if {($next & $mask) == $mask} {
		    addPage $info
		    set info {}
		}
	    }
	}

	# Enter all assigned characters up to the current character
	for {set i $next} {$i <= $index} {incr i} {
	    # Add the group index to the info for the current page
	    lappend info $gIndex

	    # If this is the last entry in the page, add the page
	    if {($i & $mask) == $mask} {
		addPage $info
		set info {}
	    }
	}
	set next [expr {$index + 1}]
    }
    return
}

proc uni::main {} {
    global argc argv0 argv
    variable pMap
    variable pages
    variable groups
    variable shift
    variable next

    if {$argc != 2} {
	puts stderr "\nusage: $argv0 <datafile> <outdir>\n"
	exit 1
    }
    set f [open [lindex $argv 0] r]
    set data [read $f]
    close $f

    buildTables $data
    puts "X = [llength $pMap]  Y= [llength $pages]  A= [llength $groups]"
    set size [expr {[llength $pMap]*2 + ([llength $pages]<<$shift)}]
    puts "shift = $shift, space = $size"

    set f [open [file join [lindex $argv 1] tclUniData.c] w]
    fconfigure $f -translation lf -encoding utf-8
    puts $f "/*
 * tclUniData.c --
 *
 *	Declarations of Unicode character information tables.  This file is
 *	automatically generated by the tools/uniParse.tcl script.  Do not
 *	modify this file by hand.
 *
 * Copyright © 1998 Scriptics Corporation.
 * All rights reserved.
 */

/*
 * A 16-bit Unicode character is split into two parts in order to index
 * into the following tables.  The lower OFFSET_BITS comprise an offset
 * into a page of characters.  The upper bits comprise the page number.
 */

#define OFFSET_BITS $shift

/*
 * The pageMap is indexed by page number and returns an alternate page number
 * that identifies a unique page of characters.  Many Unicode characters map
 * to the same alternate page number.
 */

static const unsigned short pageMap\[\] = {"
    set line "    "
    set last [expr {[llength $pMap] - 1}]
    for {set i 0} {$i <= $last} {incr i} {
	if {$i == [expr {0x10000 >> $shift}]} {
	    set line [string trimright $line " \t,"]
	    puts $f $line
	    set lastpage [expr {[lindex $line end] >> $shift}]
	    puts stdout "lastpage: $lastpage"
	    set line "    ,"
	}
	append line [lindex $pMap $i]
	if {$i != $last} {
	    append line ", "
	}
	if {[string length $line] > 70} {
	    puts $f [string trimright $line]
	    set line "    "
	}
    }
    puts $f $line
    puts $f "};

/*
 * The groupMap is indexed by combining the alternate page number with
 * the page offset and returns a group number that identifies a unique
 * set of character attributes.
 */

static const unsigned char groupMap\[\] = {"
    set line "    "
    set lasti [expr {[llength $pages] - 1}]
    for {set i 0} {$i <= $lasti} {incr i} {
	set page [lindex $pages $i]
	set lastj [expr {[llength $page] - 1}]
	if {$i == ($lastpage + 1)} {
	    puts $f [string trimright $line " \t,"]
	    set line "    ,"
	}
	for {set j 0} {$j <= $lastj} {incr j} {
	    append line [lindex $page $j]
	    if {$j != $lastj || $i != $lasti} {
		append line ", "
	    }
	    if {[string length $line] > 70} {
		puts $f [string trimright $line]
		set line "    "
	    }
	}
    }
    puts $f $line
    puts $f "};

/*
 * Each group represents a unique set of character attributes.  The attributes
 * are encoded into a 32-bit value as follows:
 *
 * Bits 0-4	Character category: see the constants listed below.
 *
 * Bits 5-7	Case delta type: 000 = identity
 *				 010 = add delta for lower
 *				 011 = add delta for lower, add 1 for title
 *				 100 = subtract delta for title/upper
 *				 101 = sub delta for upper, sub 1 for title
 *				 110 = sub delta for upper, add delta for lower
 *				 111 = subtract delta for upper
 *
 * Bits 8-31	Case delta: delta for case conversions.  This should be the
 *			    highest field so we can easily sign extend.
 */

static const int groups\[\] = {"
    set line "    "
    set last [expr {[llength $groups] - 1}]
    for {set i 0} {$i <= $last} {incr i} {
	foreach {type toupper tolower totitle} [lindex $groups $i] {}

	# Compute the case conversion type and delta

	if {$totitle} {
	    if {$totitle == $toupper} {
		# subtract delta for title or upper
		set case 4
		set delta $toupper
		if {$tolower} {
		    error "New case conversion type needed: $toupper $tolower $totitle"
		}
	    } elseif {$toupper} {
		# subtract delta for upper, subtract 1 for title
		set case 5
		set delta $toupper
		if {($totitle != 1) || $tolower} {
		    error "New case conversion type needed: $toupper $tolower $totitle"
		}
	    } else {
		# add delta for lower, add 1 for title
		set case 3
		set delta $tolower
		if {$totitle != -1} {
		    error "New case conversion type needed: $toupper $tolower $totitle"
		}
	    }
	} elseif {$toupper} {
	    set delta $toupper
	    if {$tolower == $toupper} {
		# subtract delta for upper, add delta for lower
		set case 6
	    } elseif {!$tolower} {
		# subtract delta for upper
		set case 7
	    } else {
		error "New case conversion type needed: $toupper $tolower $totitle"
	    }
	} elseif {$tolower} {
	    # add delta for lower
	    set case 2
	    set delta $tolower
	} else {
	    # noop
	    set case 0
	    set delta 0
	}

	append line [expr {($delta << 8) | ($case << 5) | $type}]
	if {$i != $last} {
	    append line ", "
	}
	if {[string length $line] > 65} {
	    puts $f [string trimright $line]
	    set line "    "
	}
    }
    puts $f $line
    puts -nonewline $f "};

#define UNICODE_OUT_OF_RANGE(ch) (((ch) & 0x1FFFFF) >= [format 0x%X $next])

/*
 * The following constants are used to determine the category of a
 * Unicode character.
 */

enum {
    UNASSIGNED,
    UPPERCASE_LETTER,
    LOWERCASE_LETTER,
    TITLECASE_LETTER,
    MODIFIER_LETTER,
    OTHER_LETTER,
    NON_SPACING_MARK,
    ENCLOSING_MARK,
    COMBINING_SPACING_MARK,
    DECIMAL_DIGIT_NUMBER,
    LETTER_NUMBER,
    OTHER_NUMBER,
    SPACE_SEPARATOR,
    LINE_SEPARATOR,
    PARAGRAPH_SEPARATOR,
    CONTROL,
    FORMAT,
    PRIVATE_USE,
    SURROGATE,
    CONNECTOR_PUNCTUATION,
    DASH_PUNCTUATION,
    OPEN_PUNCTUATION,
    CLOSE_PUNCTUATION,
    INITIAL_QUOTE_PUNCTUATION,
    FINAL_QUOTE_PUNCTUATION,
    OTHER_PUNCTUATION,
    MATH_SYMBOL,
    CURRENCY_SYMBOL,
    MODIFIER_SYMBOL,
    OTHER_SYMBOL
};

/*
 * The following macros extract the fields of the character info.  The
 * GetDelta() macro is complicated because we can't rely on the C compiler
 * to do sign extension on right shifts.
 */

#define GetCaseType(info) (((info) & 0xE0) >> 5)
#define GetCategory(ch) (GetUniCharInfo(ch) & 0x1F)
#define GetDelta(info) ((info) >> 8)

/*
 * This macro extracts the information about a character from the
 * Unicode character tables.
 */

#define GetUniCharInfo(ch) (groups\[groupMap\[pageMap\[((ch) & 0x1FFFFF) >> OFFSET_BITS\] | ((ch) & ((1 << OFFSET_BITS)-1))\]\])
"

    close $f
}

uni::main

return
