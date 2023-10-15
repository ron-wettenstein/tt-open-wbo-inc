#! /bin/csh -f

set fromdir = /nfs/iil/proj/dt/flames_git02/prover/nadela/prover_10.1.0_Nov_8_2022/prover/src/
set myfullpath = `readlink -f "$0"`
set todir = `dirname $myfullpath`

set t = $fromdir/topor
foreach f ($t/*.cpp)
	set fnoext = `echo $f | sed 's/\.[^.]*$//'`
	set fcc = ${fnoext}.cc
	set fccbasename = `basename $fcc`
	set ftarget = ${todir}/${fccbasename}
	echo "cp $f $ftarget"
	cp $f $ftarget
end

echo "cp $t/*.h* $todir"
cp $t/*.h* $todir

set u = $fromdir/utilities

echo "cp $u/BasicMemoryUsage.h $u/ColorPrint.h $u/SetInScope.h $u/TimeMeasure.h $todir"
cp $u/BasicMemoryUsage.h $u/ColorPrint.h $u/SetInScope.h $u/TimeMeasure.h $todir

echo "cp $fromdir/topor_tool/topor_tool.cpp Main.cc"
cp $fromdir/topor_tool/topor_tool.cpp Main.cc



