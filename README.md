NucPack Generator
==================

"NucPackGen", is a NucWriter PACK generation utility

Description
==================

<br>
Usage: ./nucpackgen [options]<br>
<br>
Options:<br>
-h | --help           print this message<br>
-d | --items-dir      directory with u-boot.bin, environment.img, uImage, rootfs.jffs2<br>
-i | --ddr-ini-file   DDR ini file from sys_cfg (by def: "sys_cfg/NUC976DK62Y.ini")<br>
-o | --output-file    output pack name<br>

Example:<br>
	./nucpackgen -i images/ -o vutpack.bin<br>

Directories of utility:
===================
src/                   - source files;<br>
sys_cfg/               - ini files for initialization the internal DDR;  <br> 


Building:
================================
Use CMake tool for build the project. For example:

	cmake .
	make 