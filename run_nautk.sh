#!/bin/bash
set -o pipefail -o noclobber -o xtrace -e errexit -o nounset

# pass arguments like
# version=12 ./run_nautk.sh
nautilus=~/nk
run_host="${run_host-tinker-2}"
run_host_dns="${run_host_dns-tinker-2.cs.iit.edu}"
run_host_ipmi="${run_host_ipmi-10.47.142.34}"
delete=${delete:-}
if [ -z "${version:-}" ]
then
	echo must pass a version, like
	echo $ version=12 ./run_linux.sh
	exit 1
fi

log=data/nautk_${version}.log
if [ -n "${delete}" ]
then
	rm -f "${log}"
fi
echo > "${log}"

# Add version information to log
commit=$(git log -1 --pretty=format:"%h")
echo Git commit ${commit} >> "${log}"
git status >> "${log}"

rm -rf "${nautilus}/src/app"      "${nautilus}/include/app"
rm -rf "${nautilus}/src/database" "${nautilus}/include/database"
rm -rf "${nautilus}/src/perf"     "${nautilus}/include/perf"

ln -s "$(realpath ./src/app)"     "${nautilus}/src/app"
ln -s "$(realpath ./include/app)" "${nautilus}/include/app"

ln -s "$(realpath ./src/database)"     "${nautilus}/src/database"
ln -s "$(realpath ./include/database)" "${nautilus}/include/database"

ln -s "$(realpath ./src/perf)"     "${nautilus}/src/perf"
ln -s "$(realpath ./include/perf)" "${nautilus}/include/perf"

if ! grep database "${nautilus}/src/Makefile" > /dev/null
then
	echo 'obj-$(NAUT_CONFIG_APP_MAIN) += database/' >> "${nautilus}/src/Makefile"
fi
if ! grep perf "${nautilus}/src/Makefile" > /dev/null
then
	echo 'obj-$(NAUT_CONFIG_APP_MAIN) += perf/' >> "${nautilus}/src/Makefile"
fi

cd "${nautilus}"
run_host="${run_host}" run_host_dns="${run_host_dns}" run_host_ipmi="${run_host_ipmi}" unbuffer "${nautilus}/run_remote.py" | unbuffer -p tee -a "${log}"

ssh -t "${ipmi_host}" -- env ipmi_host='"'"${run_host_ipmi}"'"' ./ipmitool power cycle

if [ -n "${run_host_dns}" ]
then
	run_host_dns="${run_host_dns}" ./wait_for.sh
fi
