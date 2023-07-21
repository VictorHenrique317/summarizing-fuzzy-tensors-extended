import numpy as np
import pandas as pd
from sklearn.compose import ColumnTransformer
from sklearn.preprocessing import LabelEncoder
from models.pattern import Pattern
import os

class Retweets2DDataset:
    def __init__(self):
        self.__encoders = [LabelEncoder(), LabelEncoder()]

        self.__path = "../datasets/retweets/2d/influences"
        self.__processed_path = self.__path + "_processed"
        
        self.__initial_patterns_path = "../datasets/retweets/2d/init_patterns"
        self.__initial_patterns_path_processed = self.__initial_patterns_path + "_processed"
        
        self.__preprocess()
        self.__matrix = self.__toMatrix()
        self.__tensor_density = 0.002739787075376616
        # self.__tensor_density = self.__calculateTensorDensity()
        # self.__empty_model_rss = self.__calculateEmptyModelRss()
        self.__empty_model_rss = 3931.5276375190374
        

    def path(self):
        return self.__processed_path
    
    def rawPath(self):
        return self.__path

    def getMatrix(self):
        return self.__matrix

    def getEmptyModelRss(self):
        return self.__empty_model_rss

    def getDimension(self):
        columns = dict()
        with open(self.__path) as database:
            for line in database:
                line = line.strip().split(" ")

                for column in range(len(line)):
                    value = line[column]
                    column_set = columns.get(column, set())
                    column_set.add(value)

                    columns[column] = column_set

        columns = {column: len(column_set) for column, column_set in columns.items()}
        return [size for size in columns.values()][:-1]

    def getDensity(self):
        return self.__tensor_density

    def __calculateTensorDensity(self):
        tensor_sum = 0
        tensor_cells = 0

        for dims, actual_value in np.ndenumerate(self.__matrix):
            tensor_sum += actual_value
            tensor_cells += 1

        tensor_density = tensor_sum / tensor_cells
        print(f"Tensor density is: {tensor_density}")
        return tensor_density

    def __toMatrix(self):
        dataset = pd.read_csv(self.__processed_path, sep=' ', header=None)
        dataset = dataset.iloc[:, :].values
        matrix = np.zeros(self.getDimension())

        nb_indices = dataset.shape[0]
        print("Loading dataset matrix into memory...")
        for line in range(nb_indices):
            index = [int(dimension) for dimension in dataset[:, :-1][line]]
            density = dataset[:, -1][line]
            replacer_string = f"matrix{index} = {density}"
            exec(replacer_string)

        print("Done!")
        return matrix

    def __calculateEmptyModelRss(self):
        print("Calculating empty model rss..")
        rss = 0
        for dims, actual_value in np.ndenumerate(self.__matrix):
            rss += (actual_value - self.__tensor_density) ** 2

        print("Done!")
        print(f"Empty model rss: {rss}")
        return rss

    def __preprocess(self):
        print("Pre-processing retweets 2D dataset...")

        dataset = pd.read_csv(self.__path, sep=' ', header=None)
        dataset = dataset.iloc[:, :].values
        dataset[:, 0] = self.__encoders[0].fit_transform(dataset[:, 0])
        dataset[:, 1] = self.__encoders[1].fit_transform(dataset[:, 1])
        dataset = pd.DataFrame(data=dataset)

        dataset.to_csv(self.__processed_path, header=False, sep=" ", index=False)

        self.__preprocessInitialPatterns()
        print("Dataset was pre-processed!")

    def __decodeColumn(self, column_index, values):
        return self.__encoders[column_index].inverse_transform(values)
    
    def encodePattern(self, pattern_str):
        pattern = Pattern(pattern_str, 2)
        pattern_dims_values = pattern.get()
        encoded_pattern = ""

        for column_index in range(2):
            dim_values = [value for value in pattern_dims_values[column_index]]
            dim_values = self.__encoders[column_index].transform(dim_values)
            dim_values = ",".join(str(value) for value in dim_values)
            encoded_pattern += f"{dim_values} "

        encoded_pattern += f"{pattern.getDensity()}"
        return encoded_pattern

    def decodePattern(self, pattern):
        pattern = Pattern(pattern, 2)
        pattern_dims_values = pattern.get()
        inverse_encoded_pattern = ""
        for column_index in range(2):
            dim_values = [int(value) for value in pattern_dims_values[column_index]]
            dim_values = self.__decodeColumn(column_index, dim_values)
            dim_values = ",".join(str(value) for value in dim_values)
            inverse_encoded_pattern += f"{dim_values} "

        inverse_encoded_pattern += f"{pattern.getDensity()}"
        return inverse_encoded_pattern
    
    def getInitialPatternsPath(self):
        return self.__initial_patterns_path_processed

    def __preprocessInitialPatterns(self):
        initial_patterns = pd.read_csv(self.__initial_patterns_path, sep=' ', header=None)
        initial_patterns = initial_patterns.iloc[:, :].values
        initial_patterns[:, 0] = self.__encoders[0].transform(initial_patterns[:, 0])
        initial_patterns[:, 1] = self.__encoders[1].transform(initial_patterns[:, 1])
        initial_patterns = pd.DataFrame(data=initial_patterns)

        initial_patterns.to_csv(self.__initial_patterns_path_processed, header=False, sep=" ", index=False)