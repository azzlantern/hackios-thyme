#!/bin/bash
set -euo pipefail
DEVICE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
# ROOT is the HackiOS tree this package is applied to. By default the repository that
# contains this directory; set THYME_ROOT to build against another checkout, e.g.
#   THYME_ROOT=../hackios-xnu-kernel-umi ./build-kernel.sh
ROOT=${THYME_ROOT:-$(cd "$DEVICE_DIR/../.." && pwd)}
ROOT=$(cd "$ROOT" && pwd)
python3 "$DEVICE_DIR/prepare.py" userland
export DEVICE=umi BLUETOOTH=1 GL=1
export RC_JOBS=rc.jobs.umi-boot
export ADSP= REST=${REST:-1}
exec "$ROOT/out/thyme/userland-build/userland/build.sh" "$@"
