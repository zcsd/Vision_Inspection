### How to build external Qt modules?

Take qtmqtt as example:

Download the source code, extract and put it in /home/xxx/Qt5.11.2/5.11.2/Src/qtmqtt,

cd /home/xxx/Qt5.11.2/5.11.2/Src/qtmqtt

mkdir .git (if there is no .git directory, Qt module's headers auto-generation depends on a git_build feature. So either clone git repo or otherwise mkdir .git in the module-root directory (it is where you'll find the qmqtt.pro and .qmake.conf files))

mkdir build & cd build

/home/xxx/Qt5.11.2/5.11.2/gcc_64/bin/qmake -r ..

make install

Done, you can use QtMqtt as usual.