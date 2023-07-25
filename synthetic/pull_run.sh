#!/bin/bash
VOLUME_NAME="summarizing_fuzzy_tensors_extended_synth"
NAME="victorhenrique5800/summarizing_fuzzy_tensors_extended_synth"
CANCER_NAME="victorhenrique5800/summarizing_fuzzy_tensors_extended_cancer"

# Unmounting and clearing volumes from the main image
CONTAINERS=$(docker ps -a --filter volume=$VOLUME_NAME -q)

for CONTAINER_ID in $CONTAINERS; do
  echo "Unmounting volume $VOLUME_NAME from container $CONTAINER_ID"
  docker rm -v $CONTAINER_ID
done

echo "Volume $VOLUME_NAME unmounted from all containers!"
docker volume rm $VOLUME_NAME
echo "Volume $VOLUME_NAME removed!"

docker pull $NAME:latest
docker pull $CANCER_NAME:latest

docker volume create $VOLUME_NAME
echo -e "\n"
docker run -it -v /var/run/docker.sock:/var/run/docker.sock -v $VOLUME_NAME:/app $NAME:latest python3 main.py

# Copying volume files to disk
CONTAINER_ID=$(docker ps -a --filter ancestor=$NAME -q)
rm -rf iterations
rm -rf post_analysis
docker cp $CONTAINER_ID:/app/iterations/. iterations
docker cp $CONTAINER_ID:/app/post_analysis/. post_analysis