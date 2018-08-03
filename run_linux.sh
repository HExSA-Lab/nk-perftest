#!/bin/bash
set -o pipefail -o noclobber -o xtrace -e errexit -o nounset

# pass arguments like
# version=12 ./run_linux.sh
build_host="${build_host:-tinker-2}"
build_path="${build_path:-perftest}"
run_host="${run_host:-tinker-2}"
run_target="${run_target:-main}"
delete=${delete:-}
if [ -z "${version:-}" ]
then
	echo must pass a version, like
	echo $ version=12 ./run_linux.sh
	exit 1
fi

# clear out log
log=data/linux_${version}.log
if [ -n "${delete}" ]
then
	rm -f "${log}"
fi
echo > "${log}"

# Add version information to log
commit=$(git log -1 --pretty=format:"%h")
echo Git commit ${commit} >> "${log}"
git status >> "${log}"

# sync with build_host
make clean
rsync ./ "${build_host}:${build_path}/" \
      --archive  \
      --compress  \
      --copy-links \
      --delete      \
      --exclude=".git/*"

# build on build host
ssh "${build_host}" make V=1 -C "${build_path}" "${run_target}" >> "${log}"

# move from build host to run host
remote_tmp_path=/tmp
local_tmp_path=/tmp
ssh "${run_host}" rm -f "${remote_tmp_path}/${run_target}"
if [ "${build_host}" = "${run_host}" ]
then
	ssh "${build_host}" mv "${build_path}/${run_target}" "${remote_tmp_path}/${run_target}"
else
	# scp build_host -> local
	scp "${build_host}:${build_path}/${run_target}" "${local_tmp_path}/${run_target}"
	# scp local -> run_host
	scp "${local_tmp_path}/${run_target}" "${run_host}:${remote_tmp_path}/${run_target}"
fi

echo running
unbuffer ssh "${run_host}" "unbuffer perf stat -e cycles -e L1-icache-load-misses -e dTLB-load-misses ${remote_tmp_path}/${run_target}" | unbuffer -p tee -a "${log}"

# send a notification
if which ntfy
then
	ntfy send 'Linux done'
fi


