#!/usr/bin/env bash
#
# Regression test runner for zcc.
#
# For each test/cases/<name>.c:
#   1. compile to LLVM IR with the zcc compiler (-llvm mode)
#   2. build a native binary with the host clang (libc printf/scanf as the oracle)
#   3. run it and compare stdout to test/cases/<name>.expected
#
# A case whose first line contains "XFAIL" documents a known-broken feature:
# it is allowed to fail (reported as "xfail"), and if it unexpectedly passes it
# is reported as "XPASS" (a hint to drop the marker). The suite's exit status is
# non-zero only when a non-XFAIL case fails.
#
# Override the host compiler with CC=... (default: clang).

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
COMPILER="$ROOT/build/compiler"
CASES_DIR="$ROOT/test/cases"
CC="${CC:-clang}"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

if [ ! -x "$COMPILER" ]; then
    echo "error: $COMPILER not found - run 'make' first" >&2
    exit 1
fi

pass=0 fail=0 xfail=0 xpass=0

for src in "$CASES_DIR"/*.c; do
    [ -e "$src" ] || continue
    name="$(basename "$src" .c)"
    exp="$CASES_DIR/$name.expected"

    if [ ! -f "$exp" ]; then
        echo "FAIL  $name (no .expected file)"
        fail=$((fail + 1))
        continue
    fi

    is_xfail=0
    if head -n 1 "$src" | grep -q "XFAIL"; then
        is_xfail=1
    fi

    ll="$WORK/$name.ll"
    bin="$WORK/$name.bin"
    ok=1
    "$COMPILER" -llvm "$src" -o "$ll" >/dev/null 2>"$WORK/$name.cc.log" || ok=0
    if [ $ok -eq 1 ]; then
        "$CC" "$ll" -o "$bin" >/dev/null 2>"$WORK/$name.ld.log" || ok=0
    fi

    got=""
    if [ $ok -eq 1 ]; then
        got="$("$bin" 2>/dev/null)"
    fi
    want="$(cat "$exp")"

    if [ $ok -eq 1 ] && [ "$got" = "$want" ]; then
        if [ $is_xfail -eq 1 ]; then
            echo "XPASS $name (now passes - remove the XFAIL marker)"
            xpass=$((xpass + 1))
        else
            echo "PASS  $name"
            pass=$((pass + 1))
        fi
    else
        if [ $is_xfail -eq 1 ]; then
            echo "xfail $name (known broken)"
            xfail=$((xfail + 1))
        else
            echo "FAIL  $name"
            echo "      expected: $(printf '%q' "$want")"
            echo "      got:      $(printf '%q' "$got")"
            fail=$((fail + 1))
        fi
    fi
done

echo "----"
echo "pass=$pass fail=$fail xfail=$xfail xpass=$xpass"
[ $fail -eq 0 ]
