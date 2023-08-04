FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive
# USER root

RUN apt-get update && \
	apt-get install -y make && \
	apt-get install -y libboost-all-dev && \
	apt-get install -y g++ && \
	apt-get install -y r-base && \
	apt-get install -y time && \
	apt-get install -y gawk && \
	apt-get install -y python3 && \
	apt-get install -y python3-pip
		
RUN R -e "install.packages('reticulate')" && \
	pip install numpy && \
	pip install psutil && \
	pip install scipy && \
	pip install matplotlib && \
	pip install scikit-learn && \
	pip install pandas && \
	pip install docker

COPY algorithms /app/algorithms

RUN cd /app/algorithms/nclusterbox && make clean && make && \
	cd /app/algorithms/tribiclusterbox && make clean && make && \
	cd /app/algorithms/nclusterbox_no_performance_imp && make clean && make

COPY script /app/script
COPY iteration /app/iteration
COPY datasets /app/datasets
COPY post_analysis /app/post_analysis

WORKDIR /app/script