#!/bin/sh

echo 23 > /sys/class/gpio/unexport
echo 24 > /sys/class/gpio/unexport
echo 13 > /sys/class/gpio/unexport
echo 19 > /sys/class/gpio/unexport
echo 26 > /sys/class/gpio/unexport
echo 16 > /sys/class/gpio/unexport
echo 20 > /sys/class/gpio/unexport
echo 21 > /sys/class/gpio/unexport
echo 12 > /sys/class/gpio/unexport

cd `dirname $0`

# set path to eclipse folder. If local folder, use '.'; otherwise, use /path/to/eclipse/
eclipsehome="server";

# set ports for HTTP(S) server
HTTP_PORT=8080
HTTPS_PORT=8443

# get path to equinox jar inside $eclipsehome folder
cp=$(find $eclipsehome -name "org.eclipse.equinox.launcher_*.jar" | sort | tail -1);

echo Launching the openHAB runtime...
java -Dosgi.clean=true -Declipse.ignoreApp=true -Dosgi.noShutdown=true -Djetty.port=$HTTP_PORT -Djetty.port.ssl=$HTTPS_PORT -Djetty.home=. -Dlogback.configurationFile=configurations/logback.xml -Dfelix.fileinstall.dir=addons -Djava.library.path=lib -Djava.security.auth.login.config=./etc/login.conf -Dorg.quartz.properties=./etc/quartz.properties -Dequinox.ds.block_timeout=240000 -Dequinox.scr.waitTimeOnBlock=60000 -Dfelix.fileinstall.active.level=4 -Djava.awt.headless=true -jar $cp $* -console 
