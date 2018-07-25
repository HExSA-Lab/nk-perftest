#!/usr/bin/env sh
set -e

build_host=tinker-1
build_path=memtest2
run_host=tinker-1
data_out=~/perftest/data/linux8.csv

make clean
rsync ./ "${build_host}:${build_path}/" \
      --archive  \
      --compress  \
      --copy-links \
      --delete      \
      --exclude=".git/*"

ssh "${build_host}" -- make -C "${build_path}" main

remote_tmp_path=/tmp/main
local_tmp_path=/tmp/main
ssh "${run_host}" -- rm -f "${remote_tmp_path}"
if [ "${build_host}" = "${run_host}" ]
then
	ssh "${build_host}" -- mv "${build_path}/main" "${remote_tmp_path}"
else
	scp "${build_host}:${build_path}/main" "${local_tmp_path}"
	scp "${local_tmp_path}" "${run_host}:${remote_tmp_path}"
fi

echo running
tmp_path=/tmp/my_log
ssh "${run_host}" "\"${remote_tmp_path}\" | tee \"${tmp_path}\""
scp "${run_host}:${tmp_path}" "${data_out}"

#ln -sf $(realpath "${data_out}") ~/data/data/linux_run.out

ntfy send 'Linux done'
