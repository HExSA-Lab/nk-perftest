Restart tinker-2 (boots into Fedora by default):

    ssh subutai ./ipmitool power cycle

Attach serial console (to boot into and control Nautilus)

    ssh subutai ./ipmitool sol activate -e '@'
    # uses '@' as your escape char.
    # Send   '\n'  @  .   to exit


