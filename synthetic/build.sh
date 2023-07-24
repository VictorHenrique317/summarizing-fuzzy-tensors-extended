NAME="victorhenrique5800/summarizing_fuzzy_tensors_extended_synth"

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

docker build -t $NAME $1 .
docker tag $NAME $NAME:latest
