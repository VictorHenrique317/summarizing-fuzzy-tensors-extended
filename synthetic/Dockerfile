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

RUN cd ../tribiclusterbox
RUN make

RUN cd ../nclusterbox_no_performance_imp
RUN make

RUN cd ../gennsets
RUN make

RUN cd ../num-noise
RUN make

WORKDIR /app/script
