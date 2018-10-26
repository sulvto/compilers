cd ../
make clean
make
cp ./diksam ./test
cd -
./diksam test_throw.dkm
