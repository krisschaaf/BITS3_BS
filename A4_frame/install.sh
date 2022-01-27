#!/bin/sh

# This script installs a new version of the trans

module="trans"
device="trans"
mode="666"

# remove old version of the module out of OS
erg=$(cat /proc/modules | awk " \$1==\"$module\" { print \$1 }")
if [ "$erg" == "$module" ] ; then
   # remove old module 
   /sbin/rmmod $module
   echo "Module $module removed"
fi

# remove device node
rm -f /dev/${device}

# invoke insmod with all arguments we got
/sbin/insmod ./$module.ko $*
if [ $? -ne 0 ] ; then
   echo "Could not load module ${module}."
   exit 1
fi
echo "Module $module inserted"

# make dev node 
major=$(cat /proc/devices | awk " \$2==\"$module\" { print \$1 }")
if [ "$major" == "" ] ; then
   echo "Module $module does not have a device id"
   exit
fi

mknod /dev/${device} c $major 0
if [ $? -ne 0 ] ; then
   echo "Could not generate node /dev/$device"
   exit 1
fi
echo "Node /dev/$device generated"

# give appropriate group/permissions, and change the group.
# Not all distributions have staff, some have "wheel" instead.
group="staff"
grep -q '^staff:' /etc/group || group="wheel"
chgrp $group /dev/${device}
chmod $mode /dev/${device}

# EOF
