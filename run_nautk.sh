#!/usr/bin/env bash
set -e

out_file="${HOME}/data/data/nautk_r4.out"
nautilus="${HOME}/nk"
src_dest="${nautilus}/src/app"
hdr_dest="${nautilus}/include/app"

rm -rf "${src_dest}"
rm -rf "${hdr_dest}"

srcs=($(find . -name '*.c' -printf '%p\n'))
hdrs=($(find . -name '*.h' -printf '%p\n'))

mkdir "${src_dest}"
mkdir "${hdr_dest}"

for file in "${srcs[@]}"
do
    name="$(basename ${file})"
	ln -s "$(realpath "${file}")" "${src_dest}/${name}"
done

for file in "${hdrs[@]}"
do
    name="$(basename ${file})"
	ln -s "$(realpath "${file}")" "${hdr_dest}/${name}"
done

ln -s "$(realpath nk-Makefile)" "${src_dest}/Makefile"

cd "${nautilus}"
#"${nautilus}/run_remote_qemu.sh"
"${nautilus}/run_remote.py"
strings "${nautilus}/output.txt" > "${out_file}"

ln -sf "$(realpath ${out_file})" ~/data/data/nautk_run.out
