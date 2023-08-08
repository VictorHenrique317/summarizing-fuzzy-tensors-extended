FROM ubuntu

RUN apt-get update && \
    apt-get install -y python3 python3-pip && \
    apt-get install -y make && \
    apt-get install -y libboost-all-dev && \
    apt-get install -y g++

RUN pip3 install matplotlib && \
    pip install numpy && \
    pip install scipy

COPY nclusterbox /app/nclusterbox

RUN cd /app/nclusterbox && make clean && make install

COPY datasets /app/datasets
COPY scripts /app/scripts
COPY main.sh /app/main.sh

WORKDIR /app