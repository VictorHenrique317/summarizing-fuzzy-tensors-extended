NAME="victorhenrique5800/summarizing_fuzzy_tensors_extended_real"

if [ -d "iteration" ]; then
  # The directory exists, do nothing
  :
else
    mkdir iteration
fi

if [ -d "post_analysis" ]; then
  # The directory exists, do nothing
  :
else
    mkdir post_analysis
fi

docker build -t $NAME .
docker tag $NAME $NAME:latest