# This file is the basis for a binary Tcl RPM for Linux.

%{!?directory:%define directory /usr/local}

Name:          tclit
Summary:       Tclit scripting language development environment
Version:       9.0.1
Release:       2
License:       GNU Affero General Public License
Group:         Development/Languages
Source:        https://github.com/pooryorick/tclit/downloads/tclit%{version}-src.tar.gz
URL:           https://github.com/pooryorick/tclit
Buildroot:     /var/tmp/%{name}%{version}

%description
Tclit (Tool Command Language Lit) provides a powerful platform for
creating integration applications that tie together diverse
applications, protocols, devices, and frameworks.  When paired with
the Tk toolkit, Tclit provides the fastest and most powerful way to
create GUI applications that run on PCs, Unix, and Mac OS X.  Tclit
can also be used for a variety of web-related tasks and for creating
powerful command languages for applications.

%prep
%setup -q -n %{name}%{version}

%build
cd unix
CFLAGS="%optflags" ./configure \
	--prefix=%{directory} \
	--exec-prefix=%{directory} \
	--libdir=%{directory}/%{_lib}
make

%install
cd unix
make INSTALL_ROOT=%{buildroot} install

%clean
rm -rf %buildroot

%files
%defattr(-,root,root)
%if %{_lib} != lib
%{directory}/%{_lib}
%endif
%{directory}/lib
%{directory}/bin
%{directory}/include
%{directory}/man/man1
%{directory}/man/man3
%{directory}/man/mann
