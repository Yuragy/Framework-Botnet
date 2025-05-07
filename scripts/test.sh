set -e
handle_error() {
    echo "Error on line $1"
    exit 1
}
trap 'handle_error $LINENO' ERR
check_file() {
    [[ -f "$1" ]] || { echo >&2 "$1 does not exist. Aborting."; exit 1; }
}
TESTS=(
    "./build/tests/BotTest.cpp"
    "./build/tests/BotManagerTest.cpp"
    "./build/tests/NetWorkManagerTest.cpp"
    "./build/tests/SecurityTest.cpp"
    "./build/tests/MessagingTest.cpp"
    "./build/tests/DataBaseManagerTest.cpp"
    "./build/tests/MonitoringServicetest.cpp"
    "./build/tests/ReportGeneratortTest.cpp"
    "./build/tests/CommandLineinterfacetest.cpp"
    "./build/tests/LoadTester.cpp"
)
LOG_FILE="test_results.log"
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -l|--log-file) LOG_FILE="$2"; shift ;;
        -h|--help)
            echo "Usage: $0 [-l|--log-file <log_file>]"
            exit 0
            ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done
for test in "${TESTS[@]}"; do
    check_file "$test"
done
echo "Running tests..."
for test in "${TESTS[@]}"; do
    echo "Running $test..."
    "$test" | tee -a "$LOG_FILE"
done

echo "All tests completed successfully! Results are logged in $LOG_FILE"
