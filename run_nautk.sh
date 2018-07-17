#!/usr/bin/env bash
set -e

out_file="${HOME}/perftest/data/nautk4.csv"
nautilus="${HOME}/nk"

rm -rf "${nautilus}/src/app"     "${nautilus}/src/database"
rm -rf "${nautilus}/include/app" "${nautilus}/include/database"

ln -s "$(realpath ./src/app)"     "${nautilus}/src/app"
ln -s "$(realpath ./include/app)" "${nautilus}/include/app"

ln -s "$(realpath ./src/database)"     "${nautilus}/src/database"
ln -s "$(realpath ./include/database)" "${nautilus}/include/database"

cd "${nautilus}"
#"${nautilus}/run_remote_qemu.sh"
"${nautilus}/run_remote.py"
strings "${nautilus}/output.txt" | tr '\n' '\0' | grep -o -a -P '(?<=begin_data\0).*(?=\0end_data)' | tr '\0' '\n' > "${out_file}"
