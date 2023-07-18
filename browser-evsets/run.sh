#!/bin/env /bin/bash

set -x

google-chrome --user-data-dir=/tmp/tmp.u9lo18kaTh --js-flags='--allow-natives-syntax' http://localhost:8000/perfnow.html | ./verify_addr.sh
