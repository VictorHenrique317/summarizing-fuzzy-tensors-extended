FROM ubuntu:latest
FROM python:latest
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
	apt-get install -y make && \
	apt-get install -y libboost-all-dev && \
	apt-get install -y g++ && \
	apt-get install -y r-base && \
	apt-get install -y time
		
RUN R -e "install.packages('reticulate')"
RUN pip install numpy
RUN pip install psutil
RUN pip install scipy
RUN pip install matplotlib
RUN pip install scikit-learn
RUN pip install pandas

COPY . /app
WORKDIR /app/algorithms/nclusterbox
RUN make

WORKDIR /app/algorithms/tribiclusterbox
RUN make

WORKDIR /app/algorithms/nclusterbox_no_performance_imp
RUN make

WORKDIR /app/algorithms/nclusterbox_no_subfiber_maximization
RUN make

WORKDIR /app/algorithms/gennsets
RUN make

WORKDIR /app/algorithms/num-noise
RUN make

WORKDIR /app/script
