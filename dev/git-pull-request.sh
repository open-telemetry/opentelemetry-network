#!/bin/bash

# Create a draft pull request against the upstream repo's main branch.
# - Infers the head branch from the current checked-out branch.
# - Optionally accepts a PR title as arguments (supports spaces).
# - Starts with an empty PR body.
#
# Usage:
#   ./git-pull-request.sh [optional title]

set -o pipefail

# Collect all args as the title (allows spaces)
TITLE="$*"

# Determine current branch
BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || true)
if [ -z "$BRANCH" ] || [ "$BRANCH" = "HEAD" ]; then
  echo "Unable to infer current branch (detached HEAD?). Checkout a branch and retry."
  exit 1
fi

# Default title falls back to Draft: <branch> if none provided
if [ -z "$TITLE" ]; then
  TITLE="Draft: $BRANCH"
fi

# Determine upstream repo slug (owner/repo)
determine_upstream_repo() {
  local upstream_repo=""
  if command -v gh >/dev/null 2>&1; then
    upstream_repo=$(gh repo view upstream --json nameWithOwner -q .nameWithOwner 2>/dev/null || true)
  fi
  if [ -z "$upstream_repo" ]; then
    local upstream_url
    upstream_url=$(git remote get-url upstream 2>/dev/null || true)
    if [ -n "$upstream_url" ]; then
      upstream_repo=$(echo "$upstream_url" \
        | sed -E 's#^git@[^:]+:##; s#^https?://[^/]+/##; s#\.git$##')
    fi
  fi
  echo "$upstream_repo"
}

UPSTREAM_REPO=$(determine_upstream_repo)
if [ -z "$UPSTREAM_REPO" ]; then
  echo "Skipping PR creation: unable to determine upstream repo. Ensure 'upstream' remote exists."
  exit 0
fi

if ! command -v gh >/dev/null 2>&1; then
  echo "gh not found; skipping draft PR creation. Install GitHub CLI: https://cli.github.com/"
  echo "You can create it manually with:"
  echo "  gh pr create --repo $UPSTREAM_REPO --base main --title \"$TITLE\" --body \"\" --draft"
  exit 0
fi

# Ensure gh is authenticated
if ! gh auth status >/dev/null 2>&1; then
  echo "gh is not authenticated; skipping draft PR creation. Run 'gh auth login' and retry:"
  echo "  gh pr create --repo $UPSTREAM_REPO --base main --title \"$TITLE\" --body \"\" --draft"
  exit 0
fi

# Create the draft PR. This may fail if there are no changes vs base.
if gh pr create \
    --repo "$UPSTREAM_REPO" \
    --base main \
    --title "$TITLE" \
    --body "" \
    --draft; then
  echo "Draft PR created against $UPSTREAM_REPO (base: main, head: ${ORIGIN_OWNER:-}:$BRANCH)."
else
  echo "Could not create draft PR (possibly no changes vs base). You can try manually with:"
  echo "  gh pr create --repo $UPSTREAM_REPO --base main --head ${ORIGIN_OWNER:-<your-username>}:$BRANCH --title \"$TITLE\" --body \"\" --draft"
fi
