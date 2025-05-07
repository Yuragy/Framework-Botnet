set -e
handle_error() {
    echo "Error on line $1"
    exit 1
}
trap 'handle_error $LINENO' ERR
check_command() {
    command -v "$1" >/dev/null 2>&1 || { echo >&2 "$1 is required but it's not installed. Aborting."; exit 1; }
}
check_file() {
    [[ -f "$1" ]] || { echo >&2 "$1 does not exist. Aborting."; exit 1; }
}
check_command "cmake"
check_command "make"
TEST_BINARY="./build/tests/load_test"
OUTPUT_DIR="load_test_results"
LOG_FILE="$OUTPUT_DIR/load_test.log"
NUM_RUNS=1
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -t|--test-binary) TEST_BINARY="$2"; shift ;;
        -o|--output-dir) OUTPUT_DIR="$2"; shift ;;
        -l|--log-file) LOG_FILE="$2"; shift ;;
        -n|--num-runs) NUM_RUNS="$2"; shift ;;
        -h|--help)
            echo "Usage: $0 [-t|--test-binary <test_binary>] [-o|--output-dir <output_directory>] [-l|--log-file <log_file>] [-n|--num-runs <num_runs>]"
            exit 0
            ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

check_file "$TEST_BINARY"
mkdir -p "$OUTPUT_DIR"
echo "Running load test $NUM_RUNS times..."
for ((i=1; i<=NUM_RUNS; i++)); do
    echo "Run $i of $NUM_RUNS"
    "$TEST_BINARY" | tee -a "$LOG_FILE"
done
echo "Load test completed successfully! Results are logged in $LOG_FILE"

