#!/bin/bash
set -euo pipefail
DEVICE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
# ROOT is the HackiOS tree this package is applied to. By default the repository that
# contains this directory; set THYME_ROOT to build against another checkout, e.g.
#   THYME_ROOT=../hackios-xnu-kernel-umi ./build-kernel.sh
ROOT=${THYME_ROOT:-$(cd "$DEVICE_DIR/../.." && pwd)}
ROOT=$(cd "$ROOT" && pwd)
export XNU_SRC="$ROOT/out/thyme/xnu"
python3 "$DEVICE_DIR/tools/stage-wifi.py"
exec "$ROOT/out/thyme/wifi-build/tools/mk-ath11k.sh" "$@"
