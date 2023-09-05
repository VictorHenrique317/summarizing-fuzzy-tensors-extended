import re
import numpy as np
from base.configs import Configs
from base.file_system import FileSystem
from utils.commands import Commands

class CrispTranslator:
    def __init__(self, controller) -> None:
        self.__controller = controller

    def __generateInitialPatterns(self, observations, fuzzy_path):
        dimension = len(Configs.getParameter("dataset_size"))

        base_folder = self.__controller.current_iteration_folder
        base_folder += f"/tensors/initial_patterns"
        Commands.createFolders([base_folder])

        if dimension == 2:
            extraction_fields = 2
            sort_field = 3
            
        elif dimension == 3:
            extraction_fields = 3
            sort_field = 4
        else:
            raise Exception("Dimension not supported")
        
        print("Generating initial patterns...")
        command = f"""LC_ALL=C sort -k {sort_field}gr {fuzzy_path} | head -1000 | cut -d ' ' -f -{extraction_fields} > {base_folder}/init_patterns_co{observations}"""
        print(command)
        Commands.execute(command)


    def run(self, observations):
        fuzzy_path = self.__controller.current_iteration_folder
        fuzzy_path += f"/tensors/numnoise/dataset-co{observations}.fuzzy_tensor"
        
        crisp_tensor_path = self.__controller.current_iteration_folder
        crisp_tensor_path +=  f"/tensors/crisp/dataset-co{observations}.crisp_tensor"

        self.__generateInitialPatterns(observations, fuzzy_path)

        with open(fuzzy_path) as fuzzy_file:
            with open(crisp_tensor_path, 'w') as crisp_file:
                for line in fuzzy_file:
                    
                    line = [float(character) for character in line.split(" ")]
                    density = line[-1]
                    density = 1 if density > 0.5 else 0
                    if density == 0:
                        continue

                    line = [f"{int(value)}" for value in line]
                    line.pop()
                    # line[-1] = f"{density}"

                    new_line = " ".join(line)
                    new_line += "\n"
                    crisp_file.write(new_line)

        print("Translated fuzzy tensor to crisp tensor")
        print("-"*120)