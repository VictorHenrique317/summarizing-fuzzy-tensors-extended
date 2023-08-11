FROM alpine:latest

RUN apk update && \
	apk add make && \
	apk add boost-dev && \
	apk add g++ && \
	apk add R && \
	apk add R-dev && \
	apk add gawk && \
	apk add python3 && \
	apk add python3-dev && \
	apk add py3-pip

RUN R -e "install.packages('reticulate', repos='https://cloud.r-project.org')" && \
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