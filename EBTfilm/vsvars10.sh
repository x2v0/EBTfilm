# $Id :$
# Author: Valeriy Onuchin   25.10.2010
#
# This file is a unix shell version of
# C:\Program Files\Microsoft Visual Studio 10.0\VC\bin\VCVARS32.BAT
#

VSINSTALLDIR="e:\Program Files\Microsoft Visual Studio 10.0"
VCINSTALLDIR="e:\Program Files\Microsoft Visual Studio 10.0\VC"
FrameworkDir="c:\WINDOWS\Microsoft.NET\Framework"
FrameworkVersion="v4.0.30319"

echo "Setting environment for using Microsoft Visual Studio 2010 x86 tools."

#
# Root of Visual Studio IDE installed files.
#

DevEnvDir="e:\Program Files\Microsoft Visual Studio 10.0\Common7\IDE"


WINSDK_DIR="C:\Program Files\Microsoft SDKs\Windows\v7.0A"
export WINSDK_DIR

OLD_PATH="$PATH"

TMPATH="e:\Program Files\Microsoft Visual Studio 10.0\VC\VCPackages"
TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
PATH="$TMPATH"

TMPATH="e:\Program Files\Microsoft Visual Studio 10.0\SDK\v3.5\bin"
TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
PATH="$PATH:$TMPATH"

TMPATH="e:\Program Files\Microsoft Visual Studio 10.0\VC\include"
TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
PATH="$PATH:$TMPATH"

TMPATH="e:\Program Files\Microsoft Visual Studio 10.0\VC\bin"
TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
PATH="$PATH:$TMPATH"

TMPATH="e:\Program Files\Microsoft Visual Studio 10.0\Common7\Tools\bin"
TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
PATH="$PATH:$TMPATH"

TMPATH="e:\Program Files\Microsoft Visual Studio 10.0\Common7\Tools"
TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
PATH="$PATH:$TMPATH"

TMPATH="c:\WINDOWS\Microsoft.NET\Framework\v4.0.30319"
TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
PATH="$PATH:$TMPATH"

TMPATH="e:\Program Files\Microsoft Visual Studio 10.0\Common7\IDE"
TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
PATH="$PATH:$TMPATH"

TMPATH="e:\Program Files\Microsoft Visual Studio 10.0\VC\BIN"
TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
PATH="$PATH:$TMPATH"

TMPATH="C:\Program Files\Microsoft SDKs\Windows\v7.0A\bin"
TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
PATH="$PATH;$TMPATH"

PATH=`/usr/bin/cygpath --path -u "$PATH"`
PATH="$PATH:$OLD_PATH"


OLD_INCLUDE="$INCLUDE"

TMPATH="e:\Program Files\Microsoft Visual Studio 10.0\VC\ATLMFC\INCLUDE"
#TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
INCLUDE="$INCLUDE;$TMPATH"

TMPATH="e:\Program Files\Microsoft Visual Studio 10.0\VC\INCLUDE"
#TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
INCLUDE="$INCLUDE;$TMPATH"

TMPATH="C:\Program Files\Microsoft SDKs\Windows\v7.0A\Include"
#TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
INCLUDE="$INCLUDE;$TMPATH"



if test x"$OLD_INCLUDE" != x; then
    INCLUDE="$INCLUDE;$OLD_INCLUDE"
fi
export INCLUDE

OLD_LIB="$LIB"

TMPATH="e:\Program Files\Microsoft Visual Studio 10.0\VC\ATLMFC\LIB"
#TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
LIB="$TMPATH"

TMPATH="e:\Program Files\Microsoft Visual Studio 10.0\VC\LIB"
#TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
LIB="$LIB;$TMPATH"

TMPATH="c:\Program Files\Microsoft SDKs\Windows\v7.0A\lib"
#TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
LIB="$LIB;$TMPATH"


if test x"$OLD_LIB" != x; then
    LIB="$LIB;$OLD_LIB"
fi
export LIB


OLD_LIBPATH="$LIBPATH"

TMPATH="c:\WINDOWS\Microsoft.NET\Framework\v4.0.30319"
#TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
LIBPATH="$TMPATH"


TMPATH="e:\Program Files\Microsoft Visual Studio 10.0\VC\ATLMFC\LIB"
#TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
LIBPATH="$LIBPATH;$TMPATH"

TMPATH="C:\Program Files\Microsoft SDKs\Windows\v7.0A\Lib"
#TMPATH=`/usr/bin/cygpath --path -u "$TMPATH"`
LIBPATH="$LIBPATH;$TMPATH"

if test x"$OLD_LIBPATH" != x; then
    LIBPATH="$LIBPATH;$OLD_LIBPATH"
fi
export LIBPATH

unset VSINSTALLDIR
unset FrameworkDir
unset FrameworkVersion
unset FrameworkSDKDir
unset DevEnvDir
