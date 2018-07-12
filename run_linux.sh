#!/usr/bin/env sh
set -e -x

build_host=qemu
build_path=memtest2
run_host=tinker-1
data_out=~/data/memtest2/data/linux_reverse.csv

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
ssh "${run_host}" -- rm "${remote_tmp_path}"
if [ "${build_host}" = "${run_host}" ]
then
	ssh "${build_host}" -- mv "${build_path}/main" "${remote_tmp_path}"
else
	scp "${build_host}:${build_path}/main" "${local_tmp_path}"
	scp "${local_tmp_path}" "${run_host}:${remote_tmp_path}"
fi

ssh "${run_host}" -- "${remote_tmp_path}" | tee "${data_out}"
#ln -sf $(realpath "${data_out}") ~/data/data/linux_run.out
