#!/bin/bash

# Compile all ADS1220 examples locally with arduino-cli.
# Usage: ./compile_examples.sh [FQBN]
#   Default board: arduino:avr:uno

BOARD_FQBN="${1:-arduino:avr:uno}"
EXAMPLES_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=============================================="
echo "Compiling ADS1220 examples"
echo "Board: $BOARD_FQBN"
echo "=============================================="
echo ""

cd "$EXAMPLES_DIR"

examples=(
    "01-Basic-SingleEnded-Read"
    "02-Differential-Continuous-Stream"
    "03-Internal-Temperature-Sensor"
    "04-High-Resolution-PGA"
    "05-RTD-PT100-3Wire"
    "06-Loadcell-Wheatstone-Bridge"
    "07-Register-Dump-Diagnostics"
    "08-ESP32-MultiSPI"
)

success_count=0
total_count=${#examples[@]}

for example in "${examples[@]}"; do
    echo "----------------------------------------------"
    echo "Compiling: $example"
    echo "----------------------------------------------"

    if [ -d "$example" ]; then
        if arduino-cli compile --fqbn "$BOARD_FQBN" "$example"; then
            echo "✅ SUCCESS: $example"
            ((success_count++))
        else
            echo "❌ FAILED: $example"
        fi
    else
        echo "⚠️  SKIP: $example not found"
    fi
    echo ""
done

echo "=============================================="
echo "Compilation Summary"
echo "Board: $BOARD_FQBN"
echo "Successful: $success_count/$total_count examples"
echo "=============================================="
