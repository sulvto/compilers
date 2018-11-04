# 一但有任何一个语句返回非真的值，则退出bash
set -o errexit

cd ../

make clean
make
cp ./diksam ./test
cd -
./diksam class02.dkm
