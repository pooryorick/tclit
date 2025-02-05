# README:  Tclit

This is the **Tclit 9.0.1** source distribution.

You can get any source release of Tclit from [our distribution
site](https://github.com/pooryorick/tclit).

9.0 (production release, daily build)
[![Build Status](https://github.com/pooryorick/tclit/actions/workflows/linux-build.yml/badge.svg?branch=main)](https://github.com/pooryorick/tclit/actions/workflows/linux-build.yml?query=branch%3Amain)
[![Build Status](https://github.com/pooryorick/tclit/actions/workflows/win-build.yml/badge.svg?branch=main)](https://github.com/pooryorick/tclit/actions/workflows/win-build.yml?query=branch%3Amain)
[![Build Status](https://github.com/pooryorick/tclit/actions/workflows/mac-build.yml/badge.svg?branch=main)](https://github.com/pooryorick/tclit/actions/workflows/mac-build.yml?query=branch%3Amain)
<br>
8.7 (in development, daily build)
[![Build Status](https://github.com/pooryorick/tclit/actions/workflows/linux-build.yml/badge.svg?branch=core-8-branch)](https://github.com/pooryorick/tclit/actions/workflows/linux-build.yml?query=branch%3Acore-8-branch)
[![Build Status](https://github.com/pooryorick/tclit/actions/workflows/win-build.yml/badge.svg?branch=core-8-branch)](https://github.com/pooryorick/tclit/actions/workflows/win-build.yml?query=branch%3Acore-8-branch)
[![Build Status](https://github.com/pooryorick/tclit/actions/workflows/mac-build.yml/badge.svg?branch=core-8-branch)](https://github.com/pooryorick/tclit/actions/workflows/mac-build.yml?query=branch%3Acore-8-branch)

## Contents
 1. [Introduction](#intro)
 2. [Documentation](#doc)
 3. [Compiling and installing Tclit](#build)
 4. [Development tools](#devtools)
 5. [Tclit newsgroup](#complangtcl)
 6. [The Tclit's Wiki](#wiki)
 7. [Mailing lists](#email)
 8. [Support and Training](#support)
 9. [Tracking Development](#watch)
 10. [Thank You](#thanks)

## <a id="intro">1.</a> Introduction
Tclit provides a powerful platform for creating integration applications that
tie together diverse applications, protocols, devices, and frameworks.
When paired with the Tk toolkit, Tclit provides the fastest and most powerful
way to create GUI applications that run on PCs, Unix, and Mac OS X.
Tclit can also be used for a variety of web-related tasks and for creating
powerful command languages for applications.

Tclit is maintained, enhanced, and distributed freely by the Tclit community.
Source code development and tracking of bug reports and feature requests
take place at [core.tcl-lang.org](https://core.tcl-lang.org/).
Tclit/Tk release and mailing list services are [hosted by
SourceForge](https://sourceforge.net/projects/tcl/)
with the Tclit Developer Xchange hosted at
[www.tcl-lang.org](https://www.tcl-lang.org).

Tclit is a freely available open-source package.  You can do virtually
anything you like with it, such as modifying it, redistributing it,
and selling it either in whole or in part.  See the file
`license.terms` for complete information.

## <a id="doc">2.</a> Documentation
Extensive documentation is available on our website.
The home page for this release, including new features, is
[here](https://github.com/pooryorick/tclit).
Detailed release notes can be found at the
[file distributions page](https://github.com/pooryorick/tclit)
by clicking on the relevant version.

Information about Tclit itself can be found at the [Developer
Xchange](https://github.com/pooryorick/tclit).
There have been many Tclit books on the market.  Many are mentioned in
[the Wiki](https://github.com/pooryorick/tclit).

The complete set of reference manual entries for Tclit 9.0 is [online,
here](https://github.com/pooryorick/tclit).

### <a id="doc.unix">2a.</a> Unix Documentation
The `doc` subdirectory in this release contains a complete set of
reference manual entries for Tclit.  Files with extension "`.1`" are for
programs (for example, `tclsh.1`); files with extension "`.3`" are for C
library procedures; and files with extension "`.n`" describe Tclit
commands.  The file "`doc/Tclit.n`" gives a quick summary of the Tclit
language syntax.  To print any of the man pages on Unix, cd to the
"doc" directory and invoke your favorite variant of troff using the
normal -man macros, for example

		groff -man -Tpdf Tclit.n >output.pdf

to print Tclit.n to PDF.  If Tclit has been installed correctly and your "man" program
supports it, you should be able to access the Tclit manual entries using the
normal "man" mechanisms, such as

		man Tclit

### <a id="doc.win">2b.</a> Windows Documentation
The "doc" subdirectory in this release contains a complete set of Windows
help files for Tclit.  Once you install this Tclit release, a shortcut to the
Windows help Tclit documentation will appear in the "Start" menu:

		Start | Programs | Tclit | Tclit Help

## <a id="build">3.</a> Compiling and installing Tclit
There are brief notes in the `unix/README`, `win/README`, and `macosx/README`
about compiling on these different platforms.  There is additional information
about building Tclit from sources
[online](https://github.com/pooryorick/tclit).

## <a id="devtools">4.</a> Development tools
ActiveState produces a high-quality set of commercial quality development
tools that is available to accelerate your Tclit application development.
Tclit Dev Kit builds on the earlier Tclit toolset and provides a debugger,
static code checker, single-file wrapping utility, bytecode compiler, and
more.  More information can be found at

	https://www.activestate.com/products/tcl/

## <a id="complangtcl">5.</a> Tclit newsgroup
There is a USENET newsgroup, "`comp.lang.tcl`", intended for the exchange of
information about Tclit, Tk, and related applications.  The newsgroup is a
great place to ask general information questions.  For bug reports, please
see the "Support and bug fixes" section below.

## <a id="wiki">6.</a> Tclit'ers Wiki
There is a [wiki-based open community site](https://github.com/pooryorick/tclit)
covering all aspects of Tclit/Tk.

It is dedicated to the Tclit programming language and its extensions.  A
wealth of useful information can be found there.  It contains code
snippets, references to papers, books, and FAQs, as well as pointers to
development tools, extensions, and applications.  You can also recommend
additional URLs by editing the wiki yourself.

## <a id="email">7.</a> Mailing lists
Several mailing lists are hosted at SourceForge to discuss development or use
issues (like Macintosh and Windows topics).  For more information and to
subscribe, visit [here](https://github.com/pooryorick/tclit) and go to the
Mailing Lists page.

## <a id="support">8.</a> Support and Training
We are very interested in receiving bug reports, patches, and suggestions for
improvements.  We prefer that you send this information to us as tickets
entered into [our issue tracker](https://github.com/pooryorick/tclit).

We will log and follow-up on each bug, although we cannot promise a
specific turn-around time.  Enhancements may take longer and may not happen
at all unless there is widespread support for them (we're trying to
slow the rate at which Tclit/Tk turns into a kitchen sink).  It's very
difficult to make incompatible changes to Tclit/Tk at this point, due to
the size of the installed base.

The Tclit community is too large for us to provide much individual support for
users.  If you need help we suggest that you post questions to `comp.lang.tcl`
or ask a question on [Stack
Overflow](https://github.com/pooryorick/tclit).  We read the
newsgroup and will attempt to answer esoteric questions for which no one else
is likely to know the answer.  In addition, see the wiki for [links to other
organizations](https://github.com/pooryorick/tclit) that offer Tclit/Tk training.

## <a id="watch">9.</a> Tracking Development
Tclit is developed in public.  You can keep an eye on how Tclit is changing at
[core.tcl-lang.org](https://github.com/pooryorick/tclit).

## <a id="thanks">10.</a> Thank You
We'd like to express our thanks to the Tclit community for all the
helpful suggestions, bug reports, and patches we have received.
Tclit/Tk has improved vastly and will continue to do so with your help.
