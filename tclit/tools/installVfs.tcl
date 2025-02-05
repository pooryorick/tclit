#! /usr/bin/env tclsh

# Copyright © 2018 Sean Woods.  All rights reserved.
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.

# You may distribute and/or modify this program under the terms of the GNU
# Affero General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.

#----------------------------------------------------------------------
#
# installVfs.tcl --
#
#        This file wraps the /library file system around a binary
#
#----------------------------------------------------------------------

proc mapDir {resultvar prefix filepath} {
    upvar 1 $resultvar result
    if {![info exists result]} {
      set result {}
    }
    set queue [list $prefix $filepath]
    while {[llength $queue]} {
      set queue [lassign $queue qprefix qpath]
      foreach ftail [glob -directory $qpath -nocomplain -tails *] {
	  set f [file join $qpath $ftail]
	  if {[file isdirectory $f]} {
	    if {$ftail eq "CVS"} continue
	    lappend queue [file join $qprefix $ftail] $f
	  } elseif {[file isfile $f]} {
	      if {$ftail eq "pkgIndex.tcl"} continue
	      if {$ftail eq "manifest.txt"} {
		lappend result $f [file join $qprefix pkgIndex.tcl]
	      } else {
		lappend result $f [file join $qprefix $ftail]
	      }
	  }
       }
    }
}
if {[llength $argv]<4} {
  error "Usage: [file tail [info script]] IMG_OUTPUT IMG_INPUT PREFIX FILE_SYSTEM ?PREFIX FILE_SYSTEM?..."
}

set paths [lassign $argv DLL_OUTPUT DLL_INPUT]
foreach {prefix fpath} $paths {
  mapDir files $prefix [file normalize $fpath]
}
if {$DLL_INPUT != {}} {
  zipfs lmkzip $DLL_OUTPUT $files
} else {
  zipfs lmkimg $DLL_OUTPUT $files {} $DLL_INPUT
}
