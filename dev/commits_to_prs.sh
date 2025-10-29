#!/usr/bin/env bash

# Converts each commit on top of UPSTREAM/BASE into its own PR.
# For each commit (oldest to newest):
#   - Creates a branch from upstream base
#   - Cherry-picks the commit onto that branch
#   - Pushes the branch to origin
#   - Opens a PR against upstream/base with title/body from the commit
#
# Defaults assume a typical fork setup with remotes:
#   origin   -> your fork (push target)
#   upstream -> canonical repo (PR base)
#
# Requirements:
#   - git and gh installed
#   - gh authenticated (gh auth login)
#   - remotes configured: an 'upstream' remote pointing to the canonical repo
#
# Environment variables (optional):
#   UPSTREAM_REMOTE   Remote name for upstream (default: upstream)
#   ORIGIN_REMOTE     Remote name for origin   (default: origin)
#   BASE_BRANCH       Base branch on upstream  (default: main)
#   BRANCH_PREFIX     Prefix for created branches (default: pr)
#   DRY_RUN           If set (non-empty), only print actions
#   INCLUDE_MERGES    If set (non-empty), attempt to include merge commits (default: skip merges)
#   OVERWRITE_LOCAL   If set, overwrite existing local branch of same name
#
# Notes:
#   - Merge commits are skipped by default because cherry-picking merges
#     requires selecting a parent (-m) and can be ambiguous.
#   - On conflicts during cherry-pick, the script stops. Resolve conflicts and
#     re-run; it will skip already-created branches. If a commit was already
#     applied (empty cherry-pick), it is detected and skipped.

set -euo pipefail

UPSTREAM_REMOTE=${UPSTREAM_REMOTE:-upstream}
ORIGIN_REMOTE=${ORIGIN_REMOTE:-origin}
BASE_BRANCH=${BASE_BRANCH:-main}
BRANCH_PREFIX=${BRANCH_PREFIX:-pr}
DRY_RUN=${DRY_RUN:-}
INCLUDE_MERGES=${INCLUDE_MERGES:-}
OVERWRITE_LOCAL=${OVERWRITE_LOCAL:-}

log() { echo "[commits_to_prs] $*"; }
die() { echo "[commits_to_prs][error] $*" >&2; exit 1; }

command -v git >/dev/null 2>&1 || die "git is required"
command -v gh  >/dev/null 2>&1 || die "gh is required (https://cli.github.com/)"

# Ensure gh is authenticated early to avoid half-progress
if ! gh auth status >/dev/null 2>&1; then
  die "gh auth not configured. Run 'gh auth login' first."
fi

# Determine upstream repo owner/repo slug
determine_upstream_repo() {
  local upstream_repo=""
  # Prefer parsing from git remote URL to avoid gh needing a resolvable slug
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

UPSTREAM_REPO=$(determine_upstream_repo)
[ -n "$UPSTREAM_REPO" ] || die "Unable to determine upstream repo. Ensure remote '$UPSTREAM_REMOTE' exists."

# Determine origin owner (for detecting existing PRs as fork:branch)
determine_origin_owner() {
  local origin_owner=""
  # Prefer parsing from git remote URL
  local origin_url
  origin_url=$(git remote get-url "$ORIGIN_REMOTE" 2>/dev/null || true)
  if [ -n "$origin_url" ]; then
    origin_owner=$(echo "$origin_url" \
      | sed -E 's#^git@[^:]+:##; s#^https?://[^/]+/##; s#/.*$##')
  fi
  if [ -z "$origin_owner" ]; then
    origin_owner=$(gh repo view --json owner -q .owner.login 2>/dev/null || true)
  fi
  printf "%s" "$origin_owner"
}

ORIGIN_OWNER=$(determine_origin_owner)
[ -n "$ORIGIN_OWNER" ] || die "Unable to determine origin owner. Ensure remote '$ORIGIN_REMOTE' exists."

# Check if a PR already exists for this branch
# Tries multiple strategies to be robust across gh versions.
pr_exists_for_branch() {
  local branch_name="$1"
  local out
  # 1) Direct head filter with owner:branch
  out=$(gh pr list --repo "$UPSTREAM_REPO" --state open --head "$ORIGIN_OWNER:$branch_name" --json number --jq '.[0].number' 2>/dev/null || true)
  if [ -n "$out" ]; then return 0; fi
  # 2) Filter open PRs by headRefName (branch name)
  out=$(gh pr list --repo "$UPSTREAM_REPO" --state open --json number,headRefName --jq 'map(select(.headRefName=="'"$branch_name"'")) | .[0].number' 2>/dev/null || true)
  if [ -n "$out" ]; then return 0; fi
  # 3) As a fallback, search all PRs (in case of state mismatch)
  out=$(gh pr list --repo "$UPSTREAM_REPO" --state all --json number,headRefName --jq 'map(select(.headRefName=="'"$branch_name"'")) | .[0].number' 2>/dev/null || true)
  [ -n "$out" ]
}

# Fetch latest base
log "Fetching $UPSTREAM_REMOTE/$BASE_BRANCH"
if [ -z "$DRY_RUN" ]; then
  git fetch "$UPSTREAM_REMOTE" "$BASE_BRANCH" --tags --prune
fi

BASE_REF="$UPSTREAM_REMOTE/$BASE_BRANCH"

# Compute list of commits unique to HEAD vs base, oldest->newest
log "Computing commits on top of $BASE_REF"
if [ -n "$INCLUDE_MERGES" ]; then
  COMMITS=$(git rev-list --reverse "$BASE_REF"..HEAD)
else
  COMMITS=$(git rev-list --reverse --no-merges "$BASE_REF"..HEAD)
fi

if [ -z "$COMMITS" ]; then
  log "No commits found on top of $BASE_REF. Nothing to do."
  exit 0
fi

# Helper to slugify a string for branch names
slugify() {
  # lower, replace non-alnum with '-', collapse dashes, trim, limit length
  tr '[:upper:]' '[:lower:]' \
    | sed -E 's/[^a-z0-9]+/-/g; s/^-+|-+$//g; s/-{2,}/-/g' \
    | cut -c1-50
}

# For each commit, create branch, cherry-pick, push, and open PR
for SHA in $COMMITS; do
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

  log "Processing $SHA → branch '$BRANCH'"

  # Check local branch existence
  if git show-ref --verify --quiet "refs/heads/$BRANCH"; then
    if [ -z "$OVERWRITE_LOCAL" ]; then
      log "Local branch $BRANCH exists; skipping branch creation."
    else
      log "Overwriting local branch $BRANCH from $BASE_REF"
      if [ -z "$DRY_RUN" ]; then
        git branch -f "$BRANCH" "$BASE_REF"
      fi
    fi
  else
    log "Creating branch $BRANCH from $BASE_REF"
    if [ -z "$DRY_RUN" ]; then
      git branch "$BRANCH" "$BASE_REF"
    fi
  fi

  # Switch to branch
  if [ -z "$DRY_RUN" ]; then
    git switch "$BRANCH"
  fi

  # Cherry-pick the commit
  log "Cherry-picking $SHA onto $BRANCH"
  if [ -z "$DRY_RUN" ]; then
    if ! git cherry-pick -x "$SHA"; then
      # If the cherry-pick failed, it can be either a real conflict or an
      # empty pick because the commit is already applied to this branch.
      # Detect the latter and skip it so we can continue.
      if git rev-parse -q --verify CHERRY_PICK_HEAD >/dev/null 2>&1; then
        # If there are no unmerged files, the failure is likely an empty pick.
        if [ -z "$(git diff --name-only --diff-filter=U)" ]; then
          log "Cherry-pick produced no changes (already applied). Skipping $SHA."
          # Skip this pick in the sequencer and move on.
          git cherry-pick --skip >/dev/null 2>&1 || true
        else
          die "Cherry-pick conflicted for $SHA. Resolve conflicts and re-run."
        fi
      else
        die "Cherry-pick failed unexpectedly for $SHA."
      fi
    fi
  fi

  # Push to origin
  log "Pushing $BRANCH to $ORIGIN_REMOTE"
  if [ -z "$DRY_RUN" ]; then
    git push -u "$ORIGIN_REMOTE" "$BRANCH"
  fi

  # Create PR with title/body from commit
  log "Creating PR against $UPSTREAM_REPO:$BASE_BRANCH"
  if [ -z "$DRY_RUN" ]; then
    # If a PR for this fork branch already exists against upstream, skip creation
    if pr_exists_for_branch "$BRANCH"; then
      log "PR already exists for branch $BRANCH; skipping creation."
    else
      BODY_FILE=$(mktemp)
      cleanup() { rm -f "$BODY_FILE" 2>/dev/null || true; }
      trap cleanup EXIT
      printf '%s' "$BODY" > "$BODY_FILE"

      if ! gh pr create \
        --repo "$UPSTREAM_REPO" \
        --base "$BASE_BRANCH" \
        --title "$SUBJECT" \
        --body-file "$BODY_FILE"; then
        # Race, or 'already exists' case; treat as success if detectable
        if pr_exists_for_branch "$BRANCH"; then
          log "Detected existing PR after creation attempt; continuing."
        else
          die "Failed to create PR for $SHA (branch $BRANCH)"
        fi
      fi
    fi
  fi

  log "Done with $SHA"
done

log "All done."
