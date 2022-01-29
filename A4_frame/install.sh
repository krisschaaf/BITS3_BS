#!/bin/sh

# This script installs a new version of the trans

module="translate"
device="trans"
mode="666"
bufsize_val=40
shift_val=3

# remove old version of the module out of OS
erg=$(cat /proc/modules | awk " \$1==\"$module\" { print \$1 }")
if [ "$erg" == "$module" ]; then
   # remove old module
   /sbin/rmmod $module
   echo "Module $module removed"
fi

# remove device node
rm -f /dev/trans?

# invoke insmod with all arguments we got
/sin/insmod ./$module.ko translate_bufsize=$bufsize_val translate_shift=$shift_val
if [ $? -ne 0 ]; then
   echo "Could not load module ${module}."
   exit 1
fi
echo "Module $module inserted"

# make dev node
major=$(cat /proc/devices | awk " \$2==\"$module\" { print \$1 }") #hier vielleicht mit grep stat awk suchen?
if [ "$major" == "" ]; then
   echo "Module $module does not have a device id"
   exit
fi

mknod /dev/${device} c $major 0
if [ $? -ne 0 ]; then
   echo "Could not generate node /dev/$device"
   exit 1
fi
echo "Node /dev/$device generated"
#echo "Major Number: $(cd /dev/ | ls -l | grep $device | awk '{ print $5 }' | cut -f1 -d",")"
echo "Major Number: $(cat /proc/devices | grep $device | cut -f1 -d" ")"

mknod /dev/${device} c $major 1
if [ $? -ne 0 ]; then
   echo "Could not generate node /dev/$device"
   exit 1
fi
echo "Node /dev/$device generated"
#echo "Major Number: $(cd /dev/ | ls -l | grep $device | awk '{ print $5 }' | cut -f1 -d",")"
echo "Major Number: $(cat /proc/devices | grep $device | cut -f1 -d" ")"

# give appropriate group/permissions, and change the group.
# Not all distributions have staff, some have "wheel" instead.
group="staff"
grep -q '^staff:' /etc/group || group="wheel"
chgrp $group /dev/${device}
chmod $mode /dev/${device}

# EOF
