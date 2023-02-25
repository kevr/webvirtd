#!/bin/bash
# Copyright 2023 Kevin Morris
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
sources=$(find "$1" -type f -name '*.*pp')

read -r -d '' license_comment << EOF
 \* Copyright [0-9]\+ .*_\
 \*_\
 \* Licensed under the Apache License, Version 2.0 (the "License");_\
 \* you may not use this file except in compliance with the License._\
 \* You may obtain a copy of the License at_\
 \*_\
 \* http://www.apache.org/licenses/LICENSE-2.0_\
 \*_\
 \* Unless required by applicable law or agreed to in writing, software_\
 \* distributed under the License is distributed on an "AS IS" BASIS,_\
 \* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or_\
 \* implied. See the License for the specific language governing_\
 \* permissions and limitations under the License._\
 \*/
EOF

failed=0
for src in $sources; do
    if ! (cat "$src" | sed -z 's|\n|_|g' | grep -q "$license_comment"); then
        echo "$src: license notice is missing"
        failed=1
    fi
done

exit $failed
