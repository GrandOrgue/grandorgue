#!/bin/bash

# $@ - optional list of actions: functional, perf, tests, coverage, all (default: all)
# Actions:
#   functional - run functional tests (-R GOTestFunctional)
#   perf       - run performance tests (-R GOTestPerf)
#   tests      - run all tests without filter (no coverage)
#   coverage   - run coverage steps (ctest -T coverage + gcovr)
#   all        - run all tests without filter + coverage (default when no args)

set -e

if [ "$1" = "--help" ] || [ "$1" = "-?" ]; then
    echo "Usage: $(basename $0) [action...]"
    echo ""
    echo "Actions (can be combined):"
    echo "  functional  Run functional tests (-R GOTestFunctional)"
    echo "  perf        Run performance tests (-R GOTestPerf)"
    echo "  tests       Run all tests without filter (no coverage)"
    echo "  coverage    Run coverage steps (ctest -T coverage + gcovr)"
    echo "  all         Run all tests + coverage (default when no args)"
    exit 0
fi

ACTIONS="${*:-all}"
IS_ALL=false
IS_TESTS=false
IS_FUNCTIONAL=false
IS_PERF=false
IS_COVERAGE=false

for ACTION in $ACTIONS; do
    case $ACTION in
        all)        IS_ALL=true ;;
        tests)      IS_TESTS=true ;;
        functional) IS_FUNCTIONAL=true ;;
        perf)       IS_PERF=true ;;
        coverage)   IS_COVERAGE=true ;;
    esac
done

# Print system information for performance test comparison
echo "=========================================="
echo "System Information for Performance Tests"
echo "=========================================="
echo ""

# CPU Model
echo "=== CPU Model ==="
cat /proc/cpuinfo | grep "model name" | head -1 || echo "CPU model: N/A"
echo ""

# CPU Architecture and Cores
echo "=== CPU Architecture & Cores ==="
lscpu | grep -E "Architecture:|CPU\(s\):|Thread\(s\) per core:|Core\(s\) per socket:" || echo "CPU info: N/A"
echo ""

# CPU Frequency
echo "=== CPU Frequency ==="
lscpu | grep -E "MHz|max MHz|min MHz" || echo "CPU frequency: N/A"
echo ""

# CPU Cache
echo "=== CPU Cache ==="
lscpu | grep -E "L1d cache:|L1i cache:|L2 cache:|L3 cache:" || echo "Cache info: N/A"
echo ""

# CPU Flags (AVX support)
echo "=== CPU SIMD Support ==="
grep -o "avx[^ ]*\|sse[^ ]*" /proc/cpuinfo | head -20 | sort -u | tr '\n' ' ' || echo "SIMD flags: N/A"
echo ""
echo ""

# GCC Version
echo "=== GCC Version ==="
gcc --version | head -1 || echo "GCC: N/A"
echo ""

# CPU Governor
echo "=== CPU Governor ==="
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor 2>/dev/null | uniq || echo "Governor: N/A (not available)"
echo ""

# RAM
echo "=== Memory ==="
free -h | grep Mem || echo "Memory info: N/A"
echo ""

# OS Information
echo "=== Operating System ==="
if [ -f /etc/os-release ]; then
    grep -E "PRETTY_NAME|VERSION_ID" /etc/os-release || echo "OS: N/A"
else
    echo "OS: N/A"
fi
echo ""

# Hostname
echo "=== Hostname ==="
hostname || echo "Hostname: N/A"
echo ""

echo "=========================================="
echo "Starting Tests"
echo "=========================================="
echo ""

if $IS_ALL || $IS_TESTS; then
    ctest -T test --verbose
else
    if $IS_FUNCTIONAL; then
        ctest -T test --verbose -R GOTestFunctional
    fi
    if $IS_PERF; then
        ctest -T test --verbose -R GOTestPerf
    fi
fi

if $IS_ALL || $IS_COVERAGE; then
    ctest -T coverage
    gcovr -e 'submodules/*|usr/*'
fi
