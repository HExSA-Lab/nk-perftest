#!/usr/bin/env sh
set -e

build_host=qemu
build_path=memtest2
run_host=tinker-1
data_out=~/data/data/linux_run_r4.out

make clean
rsync ./ "${build_host}:${build_path}/" \
      --archive  \
      --compress  \
      --copy-links \
      --delete      \
      --exclude=".git/*"

OPT_FLAGS="-O2 -fno-omit-frame-pointer -fno-stack-protector -fno-strict-aliasing -fno-strict-overflow -fno-delete-null-pointer-checks -fno-common -fgnu89-inline -mno-red-zone -mno-sse2 -mcmodel=large -m64"

ssh "${build_host}" -- env CC=gcc OPT_FLAGS="\"${OPT_FLAGS}\"" make -C "${build_path}" main

remote_tmp_path=/tmp/main
local_tmp_path=/tmp/main
if [ "${build_host}" = "${run_host}" ]
then
	ssh "${build_host}" -- mv "${build_path}/main" "${remote_tmp_path}"
else
	scp "${build_host}:${build_path}/main" "${local_tmp_path}"
	scp "${local_tmp_path}" "${run_host}:${remote_tmp_path}"
fi

ssh "${run_host}" -- "${remote_tmp_path}" | tee "${data_out}"
ln -sf $(realpath "${data_out}") ~/data/data/linux_run.out
