# Copyright © 2009 Pat Thoyts <patthoyts@users.sourceforge.net>
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.

# You may distribute and/or modify this program under the terms of the GNU
# Affero General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.

# httpd11.tcl --                                                -*- tcl -*-
#
#	A simple httpd for testing HTTP/1.1 client features.
#	Not suitable for use on a internet connected port.

package require Tcl

proc ::tcl::dict::get? {dict key} {
    if {[dict exists $dict $key]} {
	return [dict get $dict $key]
    }
    return
}
namespace ensemble configure dict \
    -map [linsert [namespace ensemble configure dict -map] end get? ::tcl::dict::get?]

proc make-chunk-generator {data {size 4096}} {
    variable _chunk_gen_uid
    if {![info exists _chunk_gen_uid]} {set _chunk_gen_uid 0}
    set lambda {{data size} {
	set pos 0
	yield
	while {1} {
	    set payload [string range $data $pos [expr {$pos + $size - 1}]]
	    incr pos $size
	    set chunk [format %x [string length $payload]]\r\n$payload\r\n
	    yield $chunk
	    if {![string length $payload]} {return}
	}
    }}
    set name chunker[incr _chunk_gen_uid]
    coroutine $name ::apply $lambda $data $size
    return $name
}

proc get-chunks {data {compression gzip}} {
    switch -exact -- $compression {
	gzip     { set data [zlib gzip $data] }
	deflate  { set data [zlib deflate $data] }
	compress { set data [zlib compress $data] }
    }

    set data ""
    set chunker [make-chunk-generator $data 671]
    while {[string length [set chunk [$chunker]]]} {
	append data $chunk
    }
    return $data
}

proc blow-chunks {data {ochan stdout} {compression gzip}} {
    switch -exact -- $compression {
	gzip     { set data [zlib gzip $data] }
	deflate  { set data [zlib deflate $data] }
	compress { set data [zlib compress $data] }
    }

    set chunker [make-chunk-generator $data 671]
    while {[string length [set chunk [$chunker]]]} {
	puts -nonewline $ochan $chunk
    }
    return
}

proc mime-type {filename} {
    switch -exact -- [file extension $filename] {
	.htm - .html { return {text text/html}}
	.png { return {binary image/png} }
	.jpg { return {binary image/jpeg} }
	.gif { return {binary image/gif} }
	.css { return {text   text/css} }
	.xml { return {text   text/xml} }
	.xhtml {return {text  application/xml+html} }
	.svg { return {text image/svg+xml} }
	.txt - .tcl - .c - .h { return {text text/plain}}
    }
    return {binary text/plain}
}

proc Puts {chan s} {puts $chan $s; puts $s}

proc Service {chan addr port} {
    chan event $chan readable [info coroutine]
    while {1} {
	set meta {}
	chan configure $chan -buffering line -encoding iso8859-1 -translation crlf
	chan configure $chan -blocking 0
	yield
	while {[gets $chan line] < 0} {
	    if {[eof $chan]} {chan event $chan readable {}; close $chan; return}
	    yield
	}
	if {[eof $chan]} {chan event $chan readable {}; close $chan; return}
	foreach {req url protocol} {GET {} HTTP/1.1} break
	regexp {^(\S+)\s+(.*)\s(\S+)?$} $line -> req url protocol

	puts $line
	while {[gets $chan line] > 0} {
	    if {[regexp {^([^:]+):(.*)$} $line -> key val]} {
		puts [list $key [string trim $val]]
		lappend meta [string tolower $key] [string trim $val]
	    }
	    yield
	}

	set encoding identity
	set transfer ""
	set close 1
	set type text/html
	set code "404 Not Found"
	set data "<html><head><title>Error 404</title></head>"
	append data "<body><h1>Not Found</h1><p>Try again.</p></body></html>"

	if {[scan $url {%[^?]?%s} path query] < 2} {
	    set query ""
	}

	switch -exact -- $req {
	    GET - HEAD {
	    }
	    POST {
		# Read the query.
		set qlen [dict get? $meta content-length]
		if {[string is integer -strict $qlen]} {
		    chan configure $chan -buffering none -translation binary
		    while {[string length $query] < $qlen} {
			append query [read $chan $qlen]
			if {[string length $query] < $qlen} {yield}
		    }
		    # Check for excess query bytes [Bug 2715421]
		    if {[dict get? $meta x-check-query] eq "yes"} {
			chan configure $chan -blocking 0
			append query [read $chan]
		    }
		}
	    }
	    default {
		# invalid request error 5??
	    }
	}
	if {$query ne ""} {puts $query}

	set path [string trimleft $path /]
	set path [file join [pwd] $path]
	if {[file exists $path] && [file isfile $path]} {
	    foreach {what type} [mime-type $path] break
	    set f [open $path r]
	    if {$what eq "binary"} {
		chan configure $f -translation binary
	    } else {
		chan configure $f -encoding utf-8
	    }
	    set data [read $f]
	    close $f
	    set code "200 OK"
	    set close [expr {[dict get? $meta connection] eq "close"}]
	}

	if {$protocol eq "HTTP/1.1"} {
	    foreach enc [split [dict get? $meta accept-encoding] ,] {
		set enc [string trim $enc]
		# The current implementation of "compress" appears to be
		# incorrect (bug [a13b9d0ce1]).  Keep it here for
		# experimentation only.  The tests that use it have the
		# constraint "badCompress".  The client code in http has
		# been removed, but can be restored from comments if
		# experimentation is desired.
		if {$enc in {deflate gzip compress}} {
		    set encoding $enc
		    break
		}
	    }
	    set transfer chunked
	} else {
	    set close 1
	}

	set nosendclose 0
	set msdeflate 0
	foreach pair [split $query &] {
	    if {[scan $pair {%[^=]=%s} key val] != 2} {set val ""}
	    switch -exact -- $key {
		nosendclose  {set nosendclose 1}
		close        {set close 1 ; set transfer 0}
		transfer     {set transfer $val}
		content-type {set type $val}
		msdeflate    {set msdeflate $val}
	    }
	}
	if {$protocol eq "HTTP/1.1"} {
	    set nosendclose 0
	}

	chan configure $chan -buffering line -encoding iso8859-1 -translation crlf
	Puts $chan "$protocol $code"
	Puts $chan "content-type: $type"
	Puts $chan [format "x-crc32: %08x" [zlib crc32 $data]]
	if {$req eq "POST"} {
	    Puts $chan [format "x-query-length: %d" [string length $query]]
	}
	if {$close && (!$nosendclose)} {
	    Puts $chan "connection: close"
	}
	Puts $chan "x-requested-encodings: [dict get? $meta accept-encoding]"
	if {$encoding eq "identity" && (!$nosendclose)} {
	    Puts $chan "content-length: [string length $data]"
	} elseif {$encoding eq "identity"} {
	    # This is a blatant attempt to confuse the client by sending neither
	    # "Connection: close" nor "Content-Length" when in non-chunked mode.
	    # See test http11-3.4.
	} else {
	    Puts $chan "content-encoding: $encoding"
	}
	if {$transfer eq "chunked"} {
	    Puts $chan "transfer-encoding: chunked"
	}
	puts $chan ""
	flush $chan

	chan configure $chan -buffering full -translation binary
	if {$encoding eq {deflate}} {
	    # When http.tcl uses the correct decoder (bug [a13b9d0ce1]) for
	    # "accept-encoding deflate", i.e. "zlib decompress", this choice of
	    # encoding2 allows the tests to pass.  It appears to do "deflate"
	    # correctly, but this has not been verified with a non-Tcl client.
	    set encoding2 compress
	} else {
	    set encoding2 $encoding
	}
	if {$transfer eq "chunked"} {
	    blow-chunks $data $chan $encoding2
	} elseif {$encoding2 ne "identity" && $msdeflate eq {1}} {
	    puts -nonewline $chan [string range [zlib $encoding2 $data] 2 end-4]
	    # Used in some tests of "deflate" to produce the non-RFC-compliant
	    # Microsoft version of "deflate".
	} elseif {$encoding2 ne "identity"} {
	    puts -nonewline $chan [zlib $encoding2 $data]
	} else {
	    puts -nonewline $chan $data
	}

	if {$close} {
	    chan event $chan readable {}
	    close $chan
	    puts "close $chan"
	    return
	} else {
	    flush $chan
	}
	puts "pipeline $chan"
    }
}

proc Accept {chan addr port} {
    coroutine client$chan Service $chan $addr $port
    return
}

proc Control {chan} {
    if {[gets $chan line] >= 0} {
	if {[string trim $line] eq "quit"} {
	    set ::forever 1
	}
    }
    if {[eof $chan]} {
	chan event $chan readable {}
    }
}

proc Main {{port 0}} {
    set server [socket -server Accept -myaddr localhost $port]
    puts [chan configure $server -sockname]
    flush stdout
    chan event stdin readable [list Control stdin]
    vwait ::forever
    close $server
    return "done"
}

if {!$tcl_interactive} {
    set r [catch [linsert $argv 0 Main] err]
    if {$r} {puts stderr $errorInfo} elseif {[string length $err]} {puts $err}
    exit $r
}
