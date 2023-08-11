FROM victorhenrique5800/summarizing_fuzzy_tensors_extended_base

COPY algorithms /app/algorithms

RUN cd /app/algorithms/nclusterbox && make clean && make && \
	cd /app/algorithms/tribiclusterbox && make clean && make && \
	cd /app/algorithms/nclusterbox_no_performance_imp && make clean && make && \
	cd /app/algorithms/gennsets && make clean && make && \
	cd /app/algorithms/num-noise && make clean && make

COPY script /app/script
COPY iterations /app/iterations
COPY post_analysis /app/post_analysis

WORKDIR /app/script