#!/usr/bin/env bash
set -e

out_file="${HOME}/perftest/data/nautk6.csv"
nautilus="${HOME}/nk"
run_host="${run_host-tinker-1}"
run_host_dns="${run_host_dns-tinker-1.cs.iit.edu}"
run_host_ipmi="${run_host_ipmi-10.47.152.28}"

rm -rf "${nautilus}/src/app"     "${nautilus}/src/database"
rm -rf "${nautilus}/include/app" "${nautilus}/include/database"

ln -s "$(realpath ./src/app)"     "${nautilus}/src/app"
ln -s "$(realpath ./include/app)" "${nautilus}/include/app"

ln -s "$(realpath ./src/database)"     "${nautilus}/src/database"
ln -s "$(realpath ./include/database)" "${nautilus}/include/database"

echo 'obj-$(NAUT_CONFIG_APP_MAIN) += database/' >> "${nautilus}/src/Makefile"

cd "${nautilus}"
#"${nautilus}/run_remote_qemu.sh"
run_host="${run_host}" run_host_dns="${run_host_dns}" run_host_ipmi="${run_host_ipmi}" "${nautilus}/run_remote.py"
strings "${nautilus}/output.txt" | tr '\n' '\0' | grep -o -a -P '(?<=begin_data\0).*(?=\0end_data)' | tr '\0' '\n' > "${out_file}"
