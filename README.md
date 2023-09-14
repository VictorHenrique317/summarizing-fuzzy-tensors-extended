# Introduction
This directory contains everything to rerun all the experiments described in the article "Summarizing Boolean and fuzzy tensors with
sub-tensors", published in Data Mining and Knowledge Discovery.

## Synthetic section
The synthetic component of this project (located under synthetic/) is designed to establish an experimental environment for testing and analyzing various box clustering algorithms. 

During each iteration, a tensor with randomly planted patterns is generated, and varying levels of noise are applied to this tensor. The specified set of algorithms is run on each noised tensor derived from the original “clean” tensor. This process is repeated in every iteration, with each iteration using a different clean tensor, thereby producing a unique set of noised tensors. All files generated during this phase are stored under `synthetic/iterations`.

Upon completion of the executions, various metrics are computed and stored in the `synthetic/post_analysis` directory. These metrics represent an average across all iterations.

## Real section
The real component of this project is designed to establish an experimental environment for testing and analyzing various box clustering algorithms. Unlike the synthetic part, this section does not involve synthetic tensors, noise addition, or multiple iterations.

All files generated during this phase are stored under `real/iterations`, while the metrics are saved under `real/post_analysis`.

## Speedup Section
This section is dedicated to reproducing the speedup curves, which illustrate the performance enhancements achieved through multithreading. The generated plots are saved in the `speedup/plots` directory.

## Usage Instructions
The two sections outlined in this document function independently. Any configurations or executions performed in one section will not affect the other. Docker is employed to simplify the replication of experiments, considering the extensive dependency list. 

To execute the experiments, follow these steps:

1. Grant execution permissions to the bash scripts with the following command:
```bash
chmod a+x pull_images.sh build.sh synthetic/build.sh synthetic/run.sh real/build.sh real/run.sh
```
2. Download the necessary images from Docker Hub by executing:
```bash
./pull_images.sh
```
3. To reproduce the synthetic experiments run:
```bash
cd synthetic; ./run.sh; cd ..
```
4. To reproduce the real experiments run:
```bash
cd real; ./run.sh; cd ..
```
5. To reproduce the speedup curve run:
```bash
cd speedup; ./run.sh; cd ..
```

# Customization

The default configuration is included in the container available on Docker Hub. However, it's possible to run configurations that differ from the default. The following sections will guide you on how to do this. Please note that after making any changes, you'll need to rebuild the container before executing the customized experiments. Detailed instructions on how to customize and rebuild each section are provided below.

## Synthetic section customization

The synthetic section allows you to specify:

- dataset_size: The size of the tensor.
- pattern_size: The size of randomly planted patterns, all dimensions have equal size.
- n_patterns: The number of randomly planted patterns.
- correct_obs: The levels of noise to be applied to the clean tensor. (Inverse cumulative beta distribution), the lower the noisier.
- nb_iterations: The number of iterations.

All of these parameters can be modified in the configuration files located under `synthetic/script/configs/`. The default settings are consistent with those used in the article. Given the time-intensive nature of conducting 30 iterations, it's advisable to reduce the number of iterations if you're aiming for a quick review of the results.

To choose the algorithms to be used, go to `synthetic/script/main.py` and comment or uncomment lines 14 to 19 as required. Experiments can be conducted by uncommenting line 22, while metric calculation and storage can be performed by uncommenting line 23. If you wish to disable either the 3D or 2D experiments, simply add the .off extension to the respective configuration file. For instance, to disable 3D experiments, rename `3d_configs.json` to `3d_configs.json.off`.

After customization you have to execute:
```bash
cd synthetic; ./build.sh; cd ..
```
To re-build de synthetic image.

## Real section customization

To select the algorithms to use, navigate to `synthetic/script/main.py` and comment or uncomment lines 14 to 19 as needed. Experiments can be conducted by uncommenting line 22, and metric calculation and storage can be performed by uncommenting line 23. If you want to disable all experiments on a specific tensor, you can do so by adding the .off extension to the corresponding configuration file. For example, if you want to disable experiments on the "retweets_2d" tensor, simply rename `retweets2d_configs.json` to `retweets2d_configs.json.off`.

After customization you have to execute:
```bash
cd real; ./build.sh; cd ..
```
To re-build de real image.