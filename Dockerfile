FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y make libboost-all-dev g++ r-base time gawk python3 python3-pip && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN R -e "install.packages('reticulate')" && \
    pip install numpy psutil scipy matplotlib scikit-learn pandas docker