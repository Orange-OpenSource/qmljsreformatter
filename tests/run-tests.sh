#!/bin/bash

#
# Copyright (C) 2017 - 2019 Orange
#
# This software is distributed under the terms and conditions of the GNU
# General Public Licence version 3 as published by the Free Software
# Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
# included in the packaging of this file. Please review the following
# information to ensure the GNU General Public License requirements will
# be met: https://www.gnu.org/licenses/gpl-3.0.html.
#

#
# Orange version of the Qt Creator's qmljsreformatter
#
# Run all the tests files created by Orange in this directory.
#
# Version:     1.0
# Created:     2017-04-03 by Julien Déramond
#

QMLJSREFORMATTER_BIN="../build/qmljsreformatter"
ERRORS=0

function usage() {
    echo "Usage: $0 [-h|--help]"
    exit 1
}

## Support options
OPTS=`getopt -o h -l help -- "$@"`
if [ $? != 0 ] ; then
    exit 1
fi

eval set -- "$OPTS"

while true; do
    case "$1" in
        -h | --help) usage; shift;;
        --) shift; break;;
    esac
done

## Besides the help options, the script must be executed without parameters
if [[ $1 != "" ]]; then
    usage
fi

for file in `find . -name "*.reference.qml"` ; do
    FILE_BASENAME=.`echo $file | cut -f 2 -d '.'`

    $QMLJSREFORMATTER_BIN $FILE_BASENAME.test.qml $FILE_BASENAME.qml.generated

    echo $FILE_BASENAME.test.qml

    if ! diff $file $FILE_BASENAME.qml.generated >/dev/null ; then
         ERRORS=$[ERRORS + 1]
         echo "[FAILED] $file"
         echo "$(diff $file $FILE_BASENAME.qml.generated)"
    fi

    rm $FILE_BASENAME.qml.generated
done

if [[ $ERRORS > 0 ]] ; then
    echo "[FAILURE] -- Total errors = $ERRORS"
    exit 2
fi

echo "[SUCCESS]"
exit 0
