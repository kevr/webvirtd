#!/bin/bash
# SPDX-License-Identifier: Apache 2.0
sources=$(find "$1" -type f -name '*.*pp')

failed=0
for src in $sources; do
    if ! grep -q "SPDX-License-Identifier: Apache 2.0" $src; then
        echo "$src: SPDX-License-Identifier missing"
        failed=1
    fi
done

exit $failed
