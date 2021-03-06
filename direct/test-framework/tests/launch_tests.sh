#! /bin/sh
set -x


iterations=100000

[ -z $1 ]  || iterations=$1


LAUNCHDIR=`pwd`
RUNDIR=`pwd`/tmp
mkdir $RUNDIR

# Check if RUNDIR is mounted, if not, mount as tmpfs
# because we don't want to hammer the disk.
if [ -z "$(mount | grep $RUNDIR)" ]; then
sudo mount -t tmpfs -osize=7G none $RUNDIR
sudo chmod a+wr $RUNDIR
fi




xterm  -e "$LAUNCHDIR/manage_nor_test.sh  0 $iterations"&
xterm  -e "$LAUNCHDIR/manage_nor_test.sh  1 $iterations"&
xterm  -e "$LAUNCHDIR/manage_nor_test.sh  2 $iterations"&

xterm  -e "$LAUNCHDIR/manage_m18_test.sh  0 $iterations"&
xterm  -e "$LAUNCHDIR/manage_m18_test.sh  1 $iterations"&
xterm  -e "$LAUNCHDIR/manage_m18_test.sh  2 $iterations"&

xterm  -e "$LAUNCHDIR/manage_nand_test.sh  0 $iterations"&
xterm  -e "$LAUNCHDIR/manage_nand_test.sh  1 $iterations"&
xterm  -e "$LAUNCHDIR/manage_nand_test.sh  2 $iterations"&
xterm  -e "$LAUNCHDIR/manage_nand_test.sh  3 $iterations"&
