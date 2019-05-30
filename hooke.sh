#! /bin/sh


die () {
cat > hooke.c << EOF
/*
 * Copyright (C) 2006 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 *
 * Plug-in to add to 'Eldo', 'HSPICE', 'LTspice', 'Spectre' and 'Qucs' circuit simulator optimization capabilities
 *
 */




#include "stdlib.h"




int HJ(int argc, char *argv[])
{
	return(EXIT_SUCCESS);
}
EOF
}


if [ "$1" = "get" ]; then
  rm -f hooke.c
  wget http://www.netlib.org/opt/hooke.c
  patch -i hooke.diff hooke.c
else
  echo Generating clean file
  die
fi
