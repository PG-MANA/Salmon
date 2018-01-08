# spec file for Sardine 
#
#Copyright 2017 PG_MANA
#
#Licensed under the Apache License, Version 2.0 (the "License");  
#you may not use this file except in compliance with the License.  
#You may obtain a copy of the License at  
#
#https://www.apache.org/licenses/LICENSE-2.0
#
#Unless required by applicable law or agreed to in writing, software  
#distributed under the License is distributed on an "AS IS" BASIS,  
#WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
#See the License for the specific language governing permissions and  
#limitations under the License.

%define APP_NAME              salmon
%define APP_VERSION         0.0.2
%define APP_HOMEPAGE    https://mnas.info/soft/salmon./linux/
%define APP_LICENCE         Apache License, Version 2.0

Summary: Salmon for Linux
Summary(ja):Twitter Client for Linux
Name: %{APP_NAME}
Source0: %{APP_NAME}-%{APP_VERSION}.tar.gz
Version: %{APP_VERSION}
Release: 1
License: %{APP_LICENCE}
URL: %{APP_HOMEPAGE}
Group: Applications/Internet
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Prefix: %{_prefix}

#ビルド時に必要なもの
BuildRequires:cmake >= 3.1.0

%define INSTALLDIR %{buildroot}/usr/local/bin

%description
Linux/X11を対象としたTwitter Clientです。
と言ってもQtを使用しているので環境さえ整えばMacやWindowsでも動作すると思います。 
openSUSE Tumbleweed で開発してます。

%prep
%setup -q

%build
%cmake -DCMAKE_CXX_FLAGS="-s" ..
make %{?_smp_mflags}

%install
mkdir -p %{buildroot}/usr/share/applications %{buildroot}/usr/share/pixmaps
cp src/Resources/icon/icon-normal.png %{buildroot}/usr/share/pixmaps/%{APP_NAME}.png
cp %{APP_NAME}.desktop %{buildroot}/usr/share/applications/%{APP_NAME}.desktop
cd build
make install DESTDIR=%{buildroot}

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/bin/%{APP_NAME}
/usr/share/pixmaps/%{APP_NAME}.png
/usr/share/applications/%{APP_NAME}.desktop
