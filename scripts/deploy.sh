set -e

handle_error() {
    echo "Error on line $1"
    exit 1
}

trap 'handle_error $LINENO' ERR
check_command() {
    command -v "$1" >/dev/null 2>&1 || { echo >&2 "$1 is required but it's not installed. Aborting."; exit 1; }
}

check_command "rsync"
check_command "ssh"
DEPLOY_DIR="deploy"
REMOTE_USER="user"
REMOTE_HOST="host"
REMOTE_DIR="/path/to/remote/dir"
SSH_KEY="$HOME/.ssh/id_rsa"
RSYNC_OPTIONS="-avz --delete"
LOG_FILE="/var/log/deploy.log"
POST_DEPLOY_COMMANDS=""
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -d|--deploy-dir) DEPLOY_DIR="$2"; shift ;;
        -u|--user) REMOTE_USER="$2"; shift ;;
        -h|--host) REMOTE_HOST="$2"; shift ;;
        -r|--remote-dir) REMOTE_DIR="$2"; shift ;;
        -k|--ssh-key) SSH_KEY="$2"; shift ;;
        -o|--rsync-options) RSYNC_OPTIONS="$2"; shift ;;
        -l|--log-file) LOG_FILE="$2"; shift ;;
        -p|--post-deploy-commands) POST_DEPLOY_COMMANDS="$2"; shift ;;
        --help)
            echo "Usage: $0 [-d|--deploy-dir <deploy_directory>] [-u|--user <remote_user>] [-h|--host <remote_host>] [-r|--remote-dir <remote_directory>] [-k|--ssh-key <ssh_key_path>] [-o|--rsync-options <rsync_options>] [-l|--log-file <log_file>] [-p|--post-deploy-commands <commands>]"
            exit 0
            ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

echo "Deploy directory: $DEPLOY_DIR"
echo "Remote user: $REMOTE_USER"
echo "Remote host: $REMOTE_HOST"
echo "Remote directory: $REMOTE_DIR"
echo "SSH key: $SSH_KEY"
echo "Rsync options: $RSYNC_OPTIONS"
echo "Log file: $LOG_FILE"
echo "Post-deploy commands: $POST_DEPLOY_COMMANDS"
if ! ping -c 1 -W 1 "$REMOTE_HOST" > /dev/null 2>&1; then
    echo "Remote host $REMOTE_HOST is not reachable. Aborting."
    exit 1
fi

echo "Deploying project..."
rsync $RSYNC_OPTIONS -e "ssh -i $SSH_KEY" "$DEPLOY_DIR/" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR"
ssh -i "$SSH_KEY" "$REMOTE_USER@$REMOTE_HOST" << EOF
    cd "$REMOTE_DIR"
    # ./restart_server.sh
    echo "Deployment completed successfully!"
    $POST_DEPLOY_COMMANDS
EOF
echo "$(date): Deployment completed successfully!" >> "$LOG_FILE"

