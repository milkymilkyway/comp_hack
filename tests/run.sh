#!/bin/bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

python3 "${SCRIPT_DIR}/run.py" -o "${SCRIPT_DIR}/results.xml" -m "${SCRIPT_DIR}/../build/bin/comp_manager" -c "${SCRIPT_DIR}/../build/bin/comp_client" -s "${SCRIPT_DIR}/../build/testing" "$@"
