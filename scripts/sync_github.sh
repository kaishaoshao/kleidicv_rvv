#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
#
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

SCRIPT_PATH="$(realpath "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)")"
REPO_ROOT="$(realpath "${SCRIPT_PATH}/..")"

DEFAULT_REMOTE_NAME="origin"
DEFAULT_UPSTREAM_NAME="upstream"
DEFAULT_BRANCH="main"
DEFAULT_REMOTE_URL="https://github.com/kaishaoshao/kleidicv_rvv.git"

REMOTE_NAME="${1:-${DEFAULT_REMOTE_NAME}}"
REMOTE_URL="${2:-${DEFAULT_REMOTE_URL}}"
BRANCH_NAME="${3:-${DEFAULT_BRANCH}}"
UPSTREAM_NAME="${UPSTREAM_NAME:-${DEFAULT_UPSTREAM_NAME}}"

cd "${REPO_ROOT}"

current_branch="$(git branch --show-current)"
if [[ -n "${current_branch}" && "${current_branch}" != "${BRANCH_NAME}" ]]; then
  git branch -M "${BRANCH_NAME}"
fi

if git remote get-url "${REMOTE_NAME}" >/dev/null 2>&1; then
  existing_url="$(git remote get-url "${REMOTE_NAME}")"
  if [[ "${existing_url}" != "${REMOTE_URL}" ]]; then
    if git remote get-url "${UPSTREAM_NAME}" >/dev/null 2>&1; then
      git remote set-url "${REMOTE_NAME}" "${REMOTE_URL}"
    else
      git remote rename "${REMOTE_NAME}" "${UPSTREAM_NAME}"
      git remote add "${REMOTE_NAME}" "${REMOTE_URL}"
    fi
  fi
else
  git remote add "${REMOTE_NAME}" "${REMOTE_URL}"
fi

git push -u "${REMOTE_NAME}" "${BRANCH_NAME}"
