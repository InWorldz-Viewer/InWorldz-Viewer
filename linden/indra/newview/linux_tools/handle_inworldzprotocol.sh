#!/bin/bash

# Send a URL of the form inworldz://... to InWorldz.
#

URL="$1"

if [ -z "$URL" ]; then
    echo Usage: $0 inworldz://...
    exit
fi

RUN_PATH=`dirname "$0" || echo .`
cd "${RUN_PATH}"

exec ./inworldz -url \'"${URL}"\'

