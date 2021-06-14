# Author:   Valeriy Onuchin

# Default startup script for bash. Invoked at login, before ".bashrc".

# ----------------------------------------------------------------------
# Setup shell variables and flags for use in login shell only.
# ----------------------------------------------------------------------

set -o ignoreeof

# ----------------------------------------------------------------------
# Define umask.
# ----------------------------------------------------------------------

umask 022

# ----------------------------------------------------------------------
# Setup main environment variables.
# ----------------------------------------------------------------------

export ENV=$HOME/.bashrc
export PATH=~/bin:/usr/local/bin:/opt/bin:/bin:/usr/bin:/sbin:/usr/sbin:./bin:.
export MANPATH=~/man:/usr/local/man:/usr/man/preformat:/usr/man:/var/man:
export HISTFILE=$HOME/.bash_history
export HISTSIZE=128
export USER=$LOGNAME
export EDITOR=$(type -path nano)
export PAGER=$(type -path nano)
export LESS=Mqd
export CDPATH=""
export VISUAL=$(type -path jfe)
export EXINIT='set sm ic redraw magic noeb opt report=2'
export TZ=MET
#
# WinNT path
#
export PATH=$PATH:/cygdrive/c/WINDOWS/system32:/cygdrive/c/WINDOWS:/cygdrive/c/WINDOWS/System32/Wbem:/home/onuchin/bin/pfd/bin/bin/x86


#
# Set VisualStudio, DirectShow, DDK enviroment variables
#
. vsvars10.sh

# ROOT - must be set individually on each particular machine
#
export ROOTSYS='D:\Cygwin\home\onuchin\root_v5.34.36.win32.vc10'   # must be in DOS format

export PATH=`/usr/bin/cygpath -u $ROOTSYS`/bin:$PATH
export PATH=`/usr/bin/cygpath -u $ROOTSYS`/macros:$PATH
export LIB=`/usr/bin/cygpath -u $ROOTSYS`/lib:$LIB


export DISPLAY=localhost:0.0
export PATH=/lib/gcc/i686-pc-cygwin/3.4.4:$PATH


# ----------------------------------------------------------------------
# Specify termcap file to find the terminal entry.
# ----------------------------------------------------------------------

if [ -r $HOME/.termcap ]
then
   export TERMCAP=$HOME/.termcap
else
   export TERMCAP=/etc/termcap
fi

# ----------------------------------------------------------------------
# Initialize terminal.
# And set the command line editors.
# ----------------------------------------------------------------------

export TERM
case $TERM in
"" | dialup | network)
   PS3='Please enter number or terminal name: '
   select TERM in apollo mac cito vt100 sun
   do TERM=${TERM:=$REPLY}
   done
   ;;
esac

#set -o nano

# ----------------------------------------------------------------------
# Define shell prompts.
# The main prompt is redefined in ".kshrc" at new shell invokations.
# Use highlighting for some terminals that support this option.
# ----------------------------------------------------------------------


if [ "X$MYPROMPT" != X ]
then
   export PROMPT="$MYPROMPT"
else
   export PROMPT="[\!]"
fi

case $TERM in
cit* | mac* | uni* | ts1* | vt1* | vt2* | *xterm* | dtterm | ansi | cygwin)
   export PROMPT="\[[1m\]$PROMPT\[[0m\]"
   ;;
esac

#if [ "$TERM" = cygwin ]
#then
#   export TERM=ansi
#fi

if [ "$TERM" = dtterm ]
then
   export TERM=xterm
fi


#--------------------------------------------------------------
# Change at shell startup the xmodmap.
#--------------------------------------------------------------

if [ -r $HOME/.modmap ]; then
   if [ "$DISPLAY" = ":0.0" ] || [ "$DISPLAY" = ":0" ]; then
      xmodmap $HOME/.modmap
   fi
fi

#--------------------------------------------------------------
# Source .bashrc
#--------------------------------------------------------------

if [ "$ENV" ]; then . $ENV; fi

