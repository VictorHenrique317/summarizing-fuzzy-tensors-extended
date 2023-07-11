#!/bin/bash

NAME="summarazing-fuzzy-tensors-extended"

if [ -d "iterations" ]; then
  # The directory exists, do nothing
  :
else
    mkdir iterations
fi

if [ -d "post_analysis" ]; then
  # The directory exists, do nothing
  :
else
    mkdir post_analysis
fi
docker build -t $NAME .

docker volume create $NAME
echo -e "\n"
docker run -it -v $NAME:/app $NAME python3 main.py

CONTAINER_ID=$(docker ps -a --filter ancestor=$NAME -q)
rm -rf iterations
rm -rf post_analysis
docker cp $CONTAINER_ID:/app/iterations/. iterations
docker cp $CONTAINER_ID:/app/post_analysis/. post_analysis

if [ -z "$NAME" ]; then
  echo "Usage: $0 NAME"
  exit 1
fi

CONTAINERS=$(docker ps -a --filter volume=$NAME -q)

for CONTAINER_ID in $CONTAINERS; do
  echo "Unmounting volume $NAME from container $CONTAINER_ID"
  docker rm -v $CONTAINER_ID
done

echo "Volume $NAME unmounted from all containers!"

docker volume rm $NAME