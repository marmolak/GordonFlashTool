Name: gordon-flash-tool
Version: 9
Release: 0%{?dist}
Summary: Toolset for Gotek SFR1M44-U100 formatted usb flash drives.


License: BSD
URL: https://github.com/marmolak/GordonFlashTool/
Source0: https://github.com/marmolak/GordonFlashTool/archive/refs/tags/release-%{version}.tar.gz

BuildRequires: nasm
BuildRequires: gcc
BuildRequires: make

%description
Toolset for Gotek SFR1M44-U100 formatted usb flash drives.

%prep
%setup -q -n GordonFlashTool-release-%{version}


%build
%make_build


%install
%make_install


%files
%doc README.md
%license LICENSE
%{_bindir}/gordon


%changelog
