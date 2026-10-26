#!/bin/bash
#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# session-start.sh
#
# Created on: Oct 18, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# Cloud session setup: the toolchain for `make` and `make check` (docs/toolchain.md)
set -euo pipefail

if [ "${CLAUDE_CODE_REMOTE:-}" != "true" ]; then
	exit 0
fi

packages="gcc-14 qemu-user clang-format"
missing=""
for package in ${packages}; do
	dpkg -s "${package}" > /dev/null 2>&1 || missing="${missing} ${package}"
done

if [ -n "${missing}" ]; then
	export DEBIAN_FRONTEND=noninteractive
	apt-get update -q
	apt-get install -y -q ${missing}
fi

# GNU make 4.4 or newer (.NOTINTERMEDIATE, PLAN D117); Ubuntu 24.04 ships 4.3, so build 4.4.1 into /usr/local
if ! make -p -f /dev/null 2> /dev/null | grep -q '^\.FEATURES.*notintermediate'; then
	work="$(mktemp -d)"
	curl -sSfL -o "${work}/make.tar.gz" https://ftp.gnu.org/gnu/make/make-4.4.1.tar.gz
	echo "dd16fb1d67bfab79a72f5e8390735c49e3e8e70b4945a15ab1f81ddb78658fb3  ${work}/make.tar.gz" | sha256sum -c -
	tar -C "${work}" -xzf "${work}/make.tar.gz"
	(cd "${work}/make-4.4.1" && ./configure --prefix=/usr/local --quiet && make -s -j"$(nproc)" && make -s install)
	rm -rf "${work}"
fi

cd "${CLAUDE_PROJECT_DIR:-$(dirname "$0")/../..}"
git submodule update --init third-party/libc-test
