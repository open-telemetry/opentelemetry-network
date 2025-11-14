#!/usr/bin/env bash

# Create a stack of PRs from a linear sequence of commits.
#
# Given:
#   1) a starting base branch (e.g. upstream/main or the head of a prior train)
#   2) a starting commit (bottom of the stack)
#   3) a finish commit (top of the stack)
#
# The script will:
#   - Create one branch per commit from start..finish (inclusive), oldest → newest
#   - For the first commit, branch from START_BASE (e.g. upstream/main) and cherry-pick that commit
#   - For subsequent commits, branch from the previous PR branch and cherry-pick the next commit
#   - Push each branch to the configured push remote
#   - Open a PR for each branch using the commit title as the PR title and
#     the commit body as the PR description
#   - The base for PR #1 is START_BASE; the base for PR #N>1 is the branch for PR #(N-1)
#
# This produces a "stack" of PRs, where each PR depends on the one before it.
#
# Important notes about forks vs upstream:
#   - For GitHub to accept a PR with base = "previous PR branch", that base branch
#     must exist in the base repository (the upstream repo you are opening PRs against).
#   - If you push branches only to your fork, base branches will not exist in upstream,
#     and PR creation for PR #2+ will fail. To get true stacked PRs:
#       a) Set PUSH_REMOTE to your upstream remote (often 'upstream'), or
#       b) Grant permission and push branches to upstream as well.
#   - If you cannot push branches to upstream, this script cannot create true stacked
#     PRs because GitHub does not support a fork branch as the base of a PR in upstream.
#
# Usage:
#   dev/commits_to_stacked_prs.sh <START_BASE_BRANCH> <START_COMMIT> <FINISH_COMMIT>
# Example:
#   dev/commits_to_stacked_prs.sh main abc123 def456
#
# Requirements:
#   - git and gh installed
#   - gh authenticated (gh auth login)
#   - remotes configured: an 'upstream' remote pointing to the canonical repo
#
# Environment variables (optional):
#   UPSTREAM_REMOTE   Remote name for upstream (default: upstream)
#   ORIGIN_REMOTE     Remote name for origin   (default: origin)
#   PUSH_REMOTE       Remote to push branches to (default: $UPSTREAM_REMOTE)
#   BRANCH_PREFIX     Prefix for created branches (default: stack)
#   DRY_RUN           If set (non-empty), only print actions
#   INCLUDE_MERGES    If set (non-empty), include merge commits (default: skip merges)
#   OVERWRITE_LOCAL   If set, overwrite existing local branch of same name
#   STACK_PUSH_UPSTREAM If set, also push created branches to $UPSTREAM_REMOTE (for stacking)
#
# Conflict handling:
#   - On conflicts during cherry-pick, the script stops. Resolve conflicts and
#     re-run; it will skip already-created branches. If a commit was already
#     applied (empty cherry-pick), it is detected and skipped.

set -euo pipefail

UPSTREAM_REMOTE=${UPSTREAM_REMOTE:-upstream}
ORIGIN_REMOTE=${ORIGIN_REMOTE:-origin}
PUSH_REMOTE=${PUSH_REMOTE:-$UPSTREAM_REMOTE}
BRANCH_PREFIX=${BRANCH_PREFIX:-stack}
DRY_RUN=${DRY_RUN:-}
INCLUDE_MERGES=${INCLUDE_MERGES:-}
OVERWRITE_LOCAL=${OVERWRITE_LOCAL:-}
STACK_PUSH_UPSTREAM=${STACK_PUSH_UPSTREAM:-}

log() { echo "[commits_to_stacked_prs] $*"; }
die() { echo "[commits_to_stacked_prs][error] $*" >&2; exit 1; }

command -v git >/dev/null 2>&1 || die "git is required"
command -v gh  >/dev/null 2>&1 || die "gh is required (https://cli.github.com/)"

# Ensure gh is authenticated early to avoid half-progress
if ! gh auth status >/dev/null 2>&1; then
  die "gh auth not configured. Run 'gh auth login' first."
fi

usage() {
  cat <<EOF
Usage: dev/commits_to_stacked_prs.sh <START_BASE_BRANCH> <START_COMMIT> <FINISH_COMMIT>

Creates stacked PRs where each PR is based on the previous PR branch.

Args:
  START_BASE_BRANCH  The base branch to start the stack from (e.g. main)
  START_COMMIT       The bottom commit of the stack (included)
  FINISH_COMMIT      The top commit of the stack (included)

Environment:
  UPSTREAM_REMOTE, ORIGIN_REMOTE, PUSH_REMOTE, BRANCH_PREFIX,
  DRY_RUN, INCLUDE_MERGES, OVERWRITE_LOCAL, STACK_PUSH_UPSTREAM
EOF
}

if [ "$#" -ne 3 ]; then
  usage
  exit 1
fi

BASE_BRANCH="$1"
START_COMMIT="$2"
END_COMMIT="$3"

# Determine upstream repo owner/repo slug
determine_upstream_repo() {
  local upstream_repo=""
  local upstream_url
  upstream_url=$(git remote get-url "$UPSTREAM_REMOTE" 2>/dev/null || true)
  if [ -n "$upstream_url" ]; then
    upstream_repo=$(echo "$upstream_url" \
      | sed -E 's#^git@[^:]+:##; s#^https?://[^/]+/##; s#\.git$##')
  fi
  if [ -z "$upstream_repo" ]; then
    upstream_repo=$(gh repo view --json nameWithOwner -q .nameWithOwner 2>/dev/null || true)
  fi
  printf "%s" "$upstream_repo"
}

# Determine the owner of a given remote (for PR head detection)
determine_remote_owner() {
  local remote_name="$1"
  local owner=""
  local url
  url=$(git remote get-url "$remote_name" 2>/dev/null || true)
  if [ -n "$url" ]; then
    owner=$(echo "$url" \
      | sed -E 's#^git@[^:]+:##; s#^https?://[^/]+/##; s#/.*$##')
  fi
  if [ -z "$owner" ]; then
    owner=$(gh repo view --json owner -q .owner.login 2>/dev/null || true)
  fi
  printf "%s" "$owner"
}

UPSTREAM_REPO=$(determine_upstream_repo)
[ -n "$UPSTREAM_REPO" ] || die "Unable to determine upstream repo. Ensure remote '$UPSTREAM_REMOTE' exists."

HEAD_OWNER=$(determine_remote_owner "$PUSH_REMOTE")
[ -n "$HEAD_OWNER" ] || die "Unable to determine owner for remote '$PUSH_REMOTE'. Ensure it exists."

# Check if a PR already exists for this branch
pr_exists_for_branch() {
  local branch_name="$1"
  local out
  # Direct head filter with owner:branch
  out=$(gh pr list --repo "$UPSTREAM_REPO" --state open --head "$HEAD_OWNER:$branch_name" --json number --jq '.[0].number' 2>/dev/null || true)
  if [ -n "$out" ]; then return 0; fi
  # Fallback by headRefName
  out=$(gh pr list --repo "$UPSTREAM_REPO" --state all --json number,headRefName --jq 'map(select(.headRefName=="'"$branch_name"'")) | .[0].number' 2>/dev/null || true)
  [ -n "$out" ]
}

# Validate inputs
git rev-parse --verify "$START_COMMIT^{commit}" >/dev/null 2>&1 || die "START_COMMIT not found: $START_COMMIT"
git rev-parse --verify "$END_COMMIT^{commit}" >/dev/null 2>&1 || die "FINISH_COMMIT not found: $END_COMMIT"

if ! git merge-base --is-ancestor "$START_COMMIT" "$END_COMMIT"; then
  die "START_COMMIT must be an ancestor of FINISH_COMMIT"
fi

# Fetch base branch
log "Fetching $UPSTREAM_REMOTE/$BASE_BRANCH"
if [ -z "$DRY_RUN" ]; then
  git fetch "$UPSTREAM_REMOTE" "$BASE_BRANCH" --tags --prune
fi

BASE_REF="$UPSTREAM_REMOTE/$BASE_BRANCH"

# Build commit list: START_COMMIT, then commits along ancestry path up to END_COMMIT
REV_OPTS=(--reverse --ancestry-path)
if [ -z "$INCLUDE_MERGES" ]; then
  REV_OPTS+=(--no-merges)
fi

COMMITS_AFTER=$(git rev-list "${REV_OPTS[@]}" "$START_COMMIT..$END_COMMIT")
COMMITS=("$START_COMMIT")
if [ -n "$COMMITS_AFTER" ]; then
  # shellcheck disable=SC2206
  COMMITS+=( $COMMITS_AFTER )
fi

if [ "${#COMMITS[@]}" -eq 0 ]; then
  log "No commits found between $START_COMMIT and $END_COMMIT. Nothing to do."
  exit 0
fi

# Helper to slugify a string for branch names
slugify() {
  tr '[:upper:]' '[:lower:]' \
    | sed -E 's/[^a-z0-9]+/-/g; s/^-+|-+$//g; s/-{2,}/-/g' \
    | cut -c1-50
}

CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "")
restore_branch() {
  if [ -n "$CURRENT_BRANCH" ] && [ "$CURRENT_BRANCH" != "HEAD" ]; then
    git switch "$CURRENT_BRANCH" >/dev/null 2>&1 || true
  fi
}
trap restore_branch EXIT

PREV_BRANCH=""
for SHA in "${COMMITS[@]}"; do
  PARENTS=$(git show -s --format=%P "$SHA")
  if [ -z "$INCLUDE_MERGES" ] && [ "$(wc -w <<<"$PARENTS")" -gt 1 ]; then
    log "Skipping merge commit $SHA (set INCLUDE_MERGES=1 to include)"
    continue
  fi

  SUBJECT=$(git show -s --format=%s "$SHA")
  BODY=$(git show -s --format=%b "$SHA")
  SLUG=$(printf '%s' "$SUBJECT" | slugify)
  SHORTSHA=$(printf '%.7s' "$SHA")
  BRANCH="$BRANCH_PREFIX/$SHORTSHA-$SLUG"

  # Determine base ref for creating this branch
  CREATE_FROM_REF="$BASE_REF"
  PR_BASE_BRANCH="$BASE_BRANCH"
  if [ -n "$PREV_BRANCH" ]; then
    CREATE_FROM_REF="$PREV_BRANCH"
    PR_BASE_BRANCH="$PREV_BRANCH"
  fi

  log "Processing $SHA → branch '$BRANCH' (base: $PR_BASE_BRANCH)"

  # Create or reset local branch
  if git show-ref --verify --quiet "refs/heads/$BRANCH"; then
    if [ -z "$OVERWRITE_LOCAL" ]; then
      log "Local branch $BRANCH exists; skipping branch creation."
    else
      log "Overwriting local branch $BRANCH from $CREATE_FROM_REF"
      if [ -z "$DRY_RUN" ]; then
        git branch -f "$BRANCH" "$CREATE_FROM_REF"
      fi
    fi
  else
    log "Creating branch $BRANCH from $CREATE_FROM_REF"
    if [ -z "$DRY_RUN" ]; then
      git branch "$BRANCH" "$CREATE_FROM_REF"
    fi
  fi

  # Switch to branch
  if [ -z "$DRY_RUN" ]; then
    git switch "$BRANCH"
  fi

  # Cherry-pick the single commit
  log "Cherry-picking $SHA onto $BRANCH"
  if [ -z "$DRY_RUN" ]; then
    if ! git cherry-pick -x "$SHA"; then
      if git rev-parse -q --verify CHERRY_PICK_HEAD >/dev/null 2>&1; then
        if [ -z "$(git diff --name-only --diff-filter=U)" ]; then
          log "Cherry-pick produced no changes (already applied). Skipping $SHA."
          git cherry-pick --skip >/dev/null 2>&1 || true
        else
          die "Cherry-pick conflicted for $SHA. Resolve conflicts and re-run."
        fi
      else
        die "Cherry-pick failed unexpectedly for $SHA."
      fi
    fi
  fi

  # Push to configured push remote
  log "Pushing $BRANCH to $PUSH_REMOTE"
  if [ -z "$DRY_RUN" ]; then
    git push -u "$PUSH_REMOTE" "$BRANCH"
  fi

  # Optionally ensure base branch exists in upstream for stacking
  if [ -n "$STACK_PUSH_UPSTREAM" ]; then
    # Push the branch we just created to upstream as well (so future PRs can base on it)
    log "STACK_PUSH_UPSTREAM=1: pushing $BRANCH to $UPSTREAM_REMOTE"
    if [ -z "$DRY_RUN" ]; then
      git push -u "$UPSTREAM_REMOTE" "$BRANCH" || true
    fi
  fi

  # Create PR with title/body from commit
  log "Creating PR against $UPSTREAM_REPO with base '$PR_BASE_BRANCH'"
  if [ -z "$DRY_RUN" ]; then
    if pr_exists_for_branch "$BRANCH"; then
      log "PR already exists for branch $BRANCH; skipping creation."
    else
      BODY_FILE=$(mktemp)
      printf '%s' "$BODY" > "$BODY_FILE"

      if ! gh pr create \
        --repo "$UPSTREAM_REPO" \
        --base "$PR_BASE_BRANCH" \
        --title "$SUBJECT" \
        --body-file "$BODY_FILE"; then
        # If creation fails and a PR exists, continue; otherwise error
        if pr_exists_for_branch "$BRANCH"; then
          log "Detected existing PR after creation attempt; continuing."
        else
          die "Failed to create PR for $SHA (branch $BRANCH, base $PR_BASE_BRANCH). If stacking across a fork, ensure base branch exists in upstream or push to upstream (see header notes)."
        fi
      fi
      rm -f "$BODY_FILE" 2>/dev/null || true
    fi
  fi

  PREV_BRANCH="$BRANCH"
  log "Done with $SHA"
done

log "All done."
