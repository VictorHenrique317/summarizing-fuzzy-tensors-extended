NAME="victorhenrique5800/summarizing_fuzzy_tensors_extended_speedup"

# docker build -t $NAME $1 --no-cache .
docker build -t $NAME $1 .
docker tag $NAME $NAME:latest
