#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

case "${1:-build}" in
    build)
        if [ ! -e alc ] || [ -n "$(find *.c *.h -newer alc 2>/dev/null)" ]; then
            ./build.sh
        else
            echo nothing to build
        fi
        ;;
    clean)
        rm -f tests/*.s tests/*_bin tests/*.ok
        ;;
    *)
        echo "usage: $0 [build|clean]" >&2
        exit 1
        ;;
esac
