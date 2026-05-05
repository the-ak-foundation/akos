PRJ_DIR=$(git rev-parse --show-toplevel)
echo PRJ_DIR=$PRJ_DIR.

if [[ ! -d "$PRJ_DIR/ext" ]]; then
    mkdir -p "$PRJ_DIR/ext"
fi

if [[ ! -d "$PRJ_DIR/ext/Unity/.git" ]]; then
    cd "$PRJ_DIR/ext"
    git clone https://github.com/ThrowTheSwitch/Unity.git
    cd -
fi

if [[ ! -d "$PRJ_DIR/ext/CMock/.git" ]]; then
    cd "$PRJ_DIR/ext"
    git clone https://github.com/ThrowTheSwitch/CMock.git
    cd -
fi
