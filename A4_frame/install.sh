#!/bin/bash

# This script installs a new version of the trans

module="trans"
device="trans"
mode="0664"

# remove old version of the module out of OS
erg=$(cat /proc/modules | awk " \$1==\"$module\" { print \$1 }")
echo "$module,$erg"
if [ "$erg" == "$module" ]; then
   # remove old module
   /sbin/rmmod $module
   echo "Module $module removed"
fi
# remove device node
rm -f /dev/${device}0
rm -f /dev/${device}1

# invoke insmod with all arguments we got
/sbin/insmod ./$module.ko $* #HIER SONST PARAMETER MIT $*
if [ $? -ne 0 ]; then
   echo "Could not load module ${module}."
   exit 1
fi
echo "Module $module inserted"

# make dev node
major=$(cat /proc/devices | awk " \$2==\"$module\" { print \$1 }") #hier vielleicht mit grep stat awk suchen?
if [ "$major" = "" ]; then
   echo "Module $module does not have a device id"
   exit
fi

mknod /dev/${device}0 c $major 0
if [ $? -ne 0 ]; then
   echo "Could not generate node /dev/$device"
   exit 1
fi
echo "Node /dev/$device generated"
#echo "Major Number: $(cd /dev/ | ls -l | grep $device | awk '{ print $5 }' | cut -f1 -d",")"
echo "Major Number: $(cat /proc/devices | grep $device | cut -f1 -d" ")"

mknod /dev/${device}1 c $major 1
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
chgrp $group /dev/${device}0
chmod $mode /dev/${device}0
chgrp $group /dev/${device}1
chmod $mode /dev/${device}1

# EOF
