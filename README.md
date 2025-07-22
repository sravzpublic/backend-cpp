### To upgrade Boost
```bash
curl -O https://sourceforge.net/projects/boost/files/boost/1.81.0/boost_1_81_0.tar.bz2/download
cd boost_1_81_0
./bootstrap.sh
./bjam install
```
### To install mongodb client
```bash
RUN cd ~ \
    && wget https://github.com/mongodb/mongo-c-driver/releases/download/1.23.2/mongo-c-driver-1.23.2.tar.gz \
    && tar xzf mongo-c-driver-1.23.2.tar.gz \
    && cd mongo-c-driver-1.23.2 \
    && mkdir build \
    && cd build \
    && cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF .. \
    && cmake --build . --target install \
    && cd ~ \
    && rm mongo-c-driver-1.23.2.tar.gz \
    && rm -rf mongo-c-driver-1.23.2

# installing mongocxx driver - connects c++ to mongo
RUN cd ~ \
    && wget https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.7.1/mongo-cxx-driver-r3.7.1.tar.gz \
    && tar -xzf mongo-cxx-driver-r3.7.1.tar.gz \
    && cd mongo-cxx-driver-r3.7.1/build \
    && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local .. \
    && make EP_mnmlstc_core \
    && make \
    && make install \
    && cd ~ \
    && rm mongo-cxx-driver-r3.7.1.tar.gz \
    && rm -rf mongo-cxx-driver-r3.7.1

# Remove libhiredis
root@db0bf31be228:/usr/local/lib# mv /usr/lib/x86_64-linux-gnu/libhiredis.a /tmp
root@db0bf31be228:/usr/local/lib# mv /usr/lib/x86_64-linux-gnu/libhiredis.so /tmp
root@db0bf31be228:/usr/local/lib# mv /usr/lib/x86_64-linux-gnu/libhiredis.so.0.14 /tmp
```


