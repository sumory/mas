mkdir -p build
cd build

case $1 in 
    with-update) 
    rm -rf * && cmake ../ && make -j8 && ./bin/mas_unittest
    ;; 
    *) 
    make -j8 && ./bin/mas_unittest
    ;; 
esac 

cd ../