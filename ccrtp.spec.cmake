%{!?release: %define release 0}
#%{!?version: %define version @VERSION@}

#%define _libname libccrtp1-@VERSION@
#%define _devname libccrtp-devel

Summary: "ccrtp" - a Common C++ class framework for RTP/RTCP
Name: libccrtp1
Version: %{version}
Release: %{release}%{?dist}
License: LGPL v2 or later
Group: System/Libraries
URL: http://www.gnu.org/software/commoncpp/commoncpp.html
Source0: ftp://ftp.gnu.org/gnu/cccrtp/libccrtp1-%{PACKAGE_VERSION}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root 
Provides: %{name} = %{version}-%{release}
BuildRequires: commoncpp2-devel >= 1.4.0
BuildRequires: pkgconfig
BuildRequires: libstdc++-devel
BuildRequires: libgcrypt-devel

%define srcdirname %{name}-%{version}

%description
This package contains the runtime library needed by applications that use 
the GNU RTP stack.

ccRTP is a generic, extensible and efficient C++ framework for
developing applications based on the Real-Time Transport Protocol
(RTP) from the IETF. It is based on Common C++ and provides a full
RTP/RTCP stack for sending and receiving of realtime data by the use
of send and receive packet queues. ccRTP supports unicast,
multi-unicast and multicast, manages multiple sources, handles RTCP
automatically, supports different threading models and is generic as
for underlying network and transport protocols.

%package devel
Group: Development/Libraries
Summary: Headers and static link library for ccrtp.
Requires: %{_libname} = %{version} 
Requires: commoncpp2-devel >= 1.4.0
Requires: libgcrypt-devel
Provides: %{name}-devel = %{version}-%{release}

%description devel
This package provides the header files, link libraries, and 
documentation for building applications that use GNU ccrtp. 

%prep
%setup -q

%build
cd ..
%{__rm} -rf build_tree
%{__mkdir} build_tree
cd build_tree
cmake -DCMAKE_INSTALL_PREFIX=%{buildroot}%{_prefix} ../%{srcdirname}
%{__make}

%install 
cd ../build_tree
%{__make} install

#%makeinstall
#rm -rf %{buildroot}/%{_infodir} 

%clean
%{__rm} -rf %{buildroot}
%{__rm} -rf build_tree

%files
%defattr(-,root,root,0755)
%doc AUTHORS COPYING ChangeLog README COPYING.addendum
%{_libdir}/*.so.*

%files devel
%defattr(-,root,root,0755)
#%{_libdir}/*.a
%{_libdir}/*.so
#%{_libdir}/*.la   
%{_libdir}/pkgconfig/*.pc
%dir %{_includedir}/ccrtp
%{_includedir}/ccrtp/*.h

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig  
