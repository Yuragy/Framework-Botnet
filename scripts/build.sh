set -e

handle_error() {
    echo "Error on line $1"
    exit 1
}

trap 'handle_error $LINENO' ERR

check_command() {
    command -v "$1" >/dev/null 2>&1 || { echo >&2 "$1 is required but it's not installed. Aborting."; exit 1; }
}

check_command "cmake"
check_command "make"
check_command "openssl"
check_command "curl"

BUILD_DIR="build"
CMAKE_PARAMS=".."
NUM_CORES=$(nproc)
LOG_FILE="build.log"
CLEAN_BUILD=false

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -b|--build-dir) BUILD_DIR="$2"; shift ;;
        -c|--cmake-params) CMAKE_PARAMS="$2"; shift ;;
        -j|--jobs) NUM_CORES="$2"; shift ;;
        -l|--log-file) LOG_FILE="$2"; shift ;;
        -C|--clean) CLEAN_BUILD=true ;;
        -h|--help)
            echo "Usage: $0 [-b|--build-dir <build_directory>] [-c|--cmake-params <cmake_parameters>] [-j|--jobs <num_jobs>] [-l|--log-file <log_file>] [-C|--clean]"
            exit 0
            ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

exec > >(tee -i $LOG_FILE) 2>&1

echo "Build directory: $BUILD_DIR"
echo "CMake parameters: $CMAKE_PARAMS"
echo "Number of parallel jobs: $NUM_CORES"
echo "Log file: $LOG_FILE"
echo "Clean build: $CLEAN_BUILD"

if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
echo "Running CMake..."
cmake $CMAKE_PARAMS
echo "Running Make..."
make -j"$NUM_CORES"
echo "Build completed successfully!"
