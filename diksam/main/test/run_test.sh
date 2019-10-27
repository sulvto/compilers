# 一但有任何一个语句返回非真的值，则退出bash
set -o errexit

cd ../

make clean
make
cp ./diksam ./test
cd -

echo 'test simple.dkm'
./diksam simple.dkm

echo 'test sum.dkm'
./diksam sum.dkm

echo 'test hoge.dkm'
./diksam hoge.dkm

echo 'test class01.dkm'
./diksam class01.dkm

echo 'test class02.dkm'
./diksam class02.dkm

echo 'test class03.dkm'
./diksam class03.dkm

echo 'test array.dkm'
./diksam array.dkm

echo 'test for.dkm'
./diksam for.dkm

echo 'test throw.dkm'
./diksam throw.dkm

echo 'test trycatch.dkm'
./diksam trycatch.dkm
