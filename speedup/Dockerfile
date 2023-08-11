FROM victorhenrique5800/summarizing_fuzzy_tensors_extended_base

COPY nclusterbox /app/nclusterbox

RUN cd /app/nclusterbox && make clean && make install

COPY datasets /app/datasets
COPY scripts /app/scripts
COPY main.sh /app/main.sh

WORKDIR /app