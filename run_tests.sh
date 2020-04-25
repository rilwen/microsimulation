#!/bin/bash

# Script which runs unit tests under Linux (SCons build) and Windows (Visual Studio build).

arch=$1
if [ "$arch" == "" ]; then
    arch="x64"
fi

test_projects="tests microsim-core-tests microsim-simulator-tests microsim-calibrator-tests microsim-uk-tests"

failed=""
for tp in $test_projects; do
    if [ "$OSTYPE" = "linux-gnu" ]; then
        ./${tp}/build_${arch}/Debug/${tp} || (failed="$failed $tp")
    else
        ./$arch/Debug/${tp}.exe || (failed="$failed $tp")
    fi
done
if [ "$failed" != "" ]; then
    echo "Failed projects: $failed"
fi
