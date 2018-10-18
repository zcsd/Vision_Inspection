### How to build extra Qt opcua modules?


#### Install open62541 as opcua backend

sudo apt install python-pip (for python2, python3 can not)

pip install six (for open62541)

git clone https://github.com/open62541/open62541.git

cd open62541

git checkout 0.3 

mkdir build && cd build

cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DUA_ENABLE_AMALGAMATION=ON ..

make

sudo make install


Download the source code, extract and put it in /home/xxx/Qt5.11.2/5.11.2/Src/qtopcua,

cd /home/xxx/Qt5.11.2/5.11.2/Src/qtopcua

mkdir .git (if there is no .git directory, Qt module's headers auto-generation depends on a git_build feature. So either clone git repo or otherwise mkdir .git in the module-root directory (it is where you'll find the qmqtt.pro and .qmake.conf files))

mkdir build & cd build

/home/xxx/Qt5.11.2/5.11.2/gcc_64/bin/qmake ..

make

sudo make install

Done, you can use QtOpcUa as usual.