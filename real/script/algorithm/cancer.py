from math import ceil
from algorithm.algorithm import Algorithm
from base.controller import Controller
from base.file_system import FileSystem
from base.configs import Configs
from utils.commands import Commands
import os
import re
import numpy as np
import docker

class Cancer(Algorithm):
    def __init__(self, controller:Controller) -> None:
        super().__init__()
        self.name = "cancer"
        self.__controller = controller
        self.__controller.addAlgorithm(self)

    def __calculateRank(self, u):
        return ceil(8/((1-u)/0.1))

    def __translateCancerPatterns(self, experiment_path): #pattern = [{}, {}, {}]
        dimensions = Configs.getDimensions()
        cancer_patterns = [] # [[{},{}], [{},{}]]
        lambda_0 = self.__controller.current_dataset.getDensity()

        experiment_folder = re.sub("/cancer.experiment", "", experiment_path)
        temp_folder = f"{experiment_folder}/temp"

        plain_patterns = os.listdir(temp_folder)
        for plain_pattern in plain_patterns:
            tuples = [set() for dimension in range(dimensions)]
            pattern_path = f"{temp_folder}/{plain_pattern}"
            
            data = ""
            with open(pattern_path) as pattern_file:
                for line in pattern_file:
                    data += line
            data = data.strip('[]')

            numpy_pattern = np.array(np.matrix(data))
            for index, value in np.ndenumerate(numpy_pattern):
                # if value < 0.5: # dont add dimensions to tuple
                if value <= lambda_0: # dont add dimensions to tuple
                    continue
                for n in range(len(index)): # iterates over all indices of the index
                    nth_tuple = tuples[n]
                    nth_tuple.add(index[n])

            cancer_patterns.append(tuples)

        return cancer_patterns

    def __createCancerFile(self):
        cancer_patterns = self.__translateCancerPatterns(self.experiment_path)

        with open(self.experiment_path, "a") as cancer_file:
            for pattern in cancer_patterns: #pattern = [{}, {}, {}]
                line = ""
                for d_tuple in pattern:
                    if len(d_tuple) == 0:
                        continue

                    line += str(d_tuple).replace("{","").replace("}","").replace(" ","") # d_tuple = {}
                    line += " "
                line = line.strip()
                if line == "":
                    print("CANCER found no patterns")
                    continue

                line += f" {self.__controller.current_dataset.calculateTuplesDensity(pattern):.6f}"
                line += "\n"
                cancer_file.write(line)

    def run(self, u, timeout, boolean_tensor=False):
        if Configs.getDimensions() > 2:
            return True

        matlab_folder = ""
        # if self.__controller.ufmgMode():
            # matlab_folder = Configs.ufmgMatlabFolder()

        rank = 7

        configuration_name = self.__controller.current_configuration_name

        current_experiment = self.__controller.current_experiment
        temp_base_folder = f"../iteration/{configuration_name}"

        translated_tensor_path = f"{self.__controller.base_dataset_path}.mat"
        translated_tensor_path = re.sub("\.\./", "/app/", translated_tensor_path)

        self.experiment_path = f"../iteration/{configuration_name}/output/{current_experiment}/experiments/cancer.experiment"
        temp_folder = f"../iteration/{configuration_name}/output/{current_experiment}/experiments/temp"
        self.log_path = f"../iteration/{configuration_name}/output/{current_experiment}/logs/cancer.log"

        current_iteration_folder = f"{self.__controller.current_iteration_folder}"
        current_iteration_folder = re.sub("\.\./", "/app/", current_iteration_folder)

        dataset_path = self.__controller.current_dataset_path

        cancer_image = "victorhenrique5800/summarizing_fuzzy_tensors_extended_cancer"
        volume = "summarizing_fuzzy_tensors_extended_real"
        mount_path = "/app"
        volumes = {f"{volume}": {'bind': mount_path, 'mode': 'rw'}}

        command = f"docker run -v {volume}:{mount_path} {cancer_image} {rank} {translated_tensor_path} "
        command += f"{current_iteration_folder} {current_experiment}"
        print(command)

        client = docker.from_env()
        args = [f"{rank}", f"{translated_tensor_path}", f"{current_iteration_folder}", f"{current_experiment}"]
        args[0] = str(int(args[0]))

        container = client.containers.run(cancer_image, detach=False, volumes=volumes, command=args, user='root')
        self.__createCancerFile()

        FileSystem.delete(temp_folder)

        # Commands.execute(f"mv {self.experiment_path} {temp_experiment_path}")
        # Commands.execute(f"../algorithms/nclusterbox/nclusterbox --os {temp_experiment_path} {dataset_path} -o {self.experiment_path}")
        # Commands.execute(f"rm {temp_experiment_path}")

        return False

        