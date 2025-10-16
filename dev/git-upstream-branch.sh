#!/bin/bash

# This script creates a new branch from the upstream main branch and pushes it to origin.

BRANCH="$1"
if [ -z "$BRANCH" ]; then
    echo "Usage: $0 <branch-name>"
    exit 1
fi
git fetch upstream && git checkout -b "$BRANCH" upstream/main && git push -u origin "$BRANCH"
echo "Branch '$BRANCH' created and pushed to origin."