#!/bin/sh

set -e

if [ ! -f "build/env.sh" ]; then
    echo "$0 must be run from the root of the repository."
    exit 2
fi

# Create fake Go workspace if it doesn't exist yet.
workspace="$PWD/build/_workspace"
root="$PWD"
dogxdir="$workspace/src/github.com/dogx"
if [ ! -L "$dogxdir/go-dogx" ]; then
    mkdir -p "$dogxdir"
    cd "$dogxdir"
    ln -s ../../../../../. go-dogx
    cd "$root"
fi

# Set up the environment to use the workspace.
GOPATH="$workspace"
export GOPATH

# Run the command inside the workspace.
cd "$dogxdir/go-dogx"
PWD="$dogxdir/go-dogx"

# Launch the arguments with the configured environment.
exec "$@"
