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
# Run a specific test file included in this directory among all the tests
# created by Orange.
#
# Version:     1.0
# Created:     2017-04-03 by Julien Déramond
#

QMLJSREFORMATTER_BIN="../build/qmljsreformatter"

function usage() {
    echo "Usage: $0 [-h|--help] TEST_FILE_BASENAME"
    exit 1
}

function test_file() {
    if [[ ! -f $1 ]] ; then
        echo "[ERROR] $1 is missing"
        exit 1
    fi
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

## Besides the help options, the script must be executed with one parameter
if [[ $# != 1 ]]; then
    usage
fi

## Verify the files
test_file $1.test.qml
test_file $1.reference.qml

$QMLJSREFORMATTER_BIN $1.test.qml $1.qml.generated

if ! diff $file $1.reference.qml $1.qml.generated >/dev/null ; then
     echo "[ERROR] $1.test.qml"
     echo "$(diff $1.reference.qml $1.qml.generated)"
     rm $1.qml.generated
     exit 2
fi

echo "[OK] $1.test.qml"
rm $1.qml.generated
exit 0
