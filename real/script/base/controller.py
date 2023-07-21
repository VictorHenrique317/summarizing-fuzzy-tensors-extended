import os
from numpy import True_
from base.configs import Configs
from base.numpy_translator import NumpyTranslator
from models.attribute import Attribute
from base.crisp_translator import CrispTranslator
from base.school_dataset import SchoolDataset
from utils.commands import Commands
from post_analysis.grapher import Grapher
from base.file_system import FileSystem
from multiprocessing import Process
from base.retweets_dataset import RetweetsDataset
from base.retweets_2d_dataset import Retweets2DDataset
from models.experiment import Experiment


class Controller():
    def __init__(self, delete_iterations=None, delete_post_analysis=None, calculate_metrics=None) -> None:
        self.delete_iterations = delete_iterations
        self.delete_post_analysis = delete_post_analysis
        self.calculate_metrics = calculate_metrics

        self.dataset = None
        self.__configs_folder = "configs"
        self.dataset_folder = "../datasets"
        self.base_dataset_path = None
        self.algorithms = []
        self.__sorting_blacklist = ['nclusterbox', 'tribiclusterbox', 'nclusterboxnoperformanceimp']
        # self.__sorting_blacklist = []
        self.__numpy_translator = NumpyTranslator(self)
        #self.__crisp_translator = CrispTranslator(self)

        self.__current_iteration_number = None
        self.current_configuration_name = None
        self.current_iteration_folder = None
        self.current_iteration = None
        self.current_dataset = self.dataset
        self.current_dataset_path = None
        self.current_experiment = None

        self.__calculate_rss_evolution = False
        self.__calculate_quality = False

        

    def addAlgorithm(self, algorithm):
        if algorithm not in self.algorithms:
            self.algorithms.append(algorithm)

    def __decodeExperiment(self, experiment_path):
        print("Inverse decoding experiment...")
        with open(experiment_path, "r") as file:
            lines = file.readlines()

        with open(experiment_path, "w") as file:
            for (i, line) in enumerate(lines):
                inverse_decoded_line = self.current_dataset.decodePattern(line)

                if i < len(lines) - 1:
                    inverse_decoded_line += "\n"
    
                file.write(inverse_decoded_line)
    
    def __run(self):
        FileSystem.createIterationFolder()
        dimension = len(self.dataset.getDimension())
        if dimension == 3:
            self.__numpy_translator.run(self.dataset)
        self.current_iteration_folder = f"../iteration/{self.current_configuration_name}"

        u = 0.0
        print("="*120 + f"")
        self.current_experiment = f"u{u}"
        FileSystem.createExperimentFolder(self.current_experiment, self.current_configuration_name)

        for algorithm in self.algorithms:
            if algorithm.hasTimedOut(u):
                continue
            
            print(f"Running {algorithm.name}...")
            timedout = algorithm.run(u, Configs.getParameter("timeout"), boolean_tensor=True)
            if timedout:
                algorithm.timedOut(u)
                print("Deleting files...")
                FileSystem.deleteExperiment(self.current_configuration_name, u, algorithm.name)
            else:
                if algorithm.name not in self.__sorting_blacklist:
                    experiment = Experiment(algorithm.experiment_path, u, dimension)
                    experiment.sortPatterns(self.dataset)

                self.__decodeExperiment(algorithm.experiment_path)

            print("-"*120)

    def __resetAlgorithms(self):
        for algorithm in self.algorithms:
            algorithm.resetTimeOutInfo()

    def initiateSession(self):
        delete_iterations = self.delete_iterations
        if delete_iterations is None:
            delete_iterations = str(input("Delete previous iterations? Y/N: ")).strip().lower()

        if delete_iterations == "y":
            FileSystem.deleteIterationFolder()

        FileSystem.deletePostAnalysisFolder()

        print("Building datasets from raw data...")
        os.system("/app/datasets/retweets/preprocess.sh")
        os.system("/app/datasets/primaryschool/preprocess.sh")

        for config_file in Commands.listFolder(self.__configs_folder):
            if config_file.strip().split(".")[-1] == "off":
                print(f"Skipping {config_file}")
                continue

            print("#"*60 + f" CONFIGURATION = {config_file}")
            Configs.readConfigFile(f"{self.__configs_folder}/{config_file}")
            self.current_configuration_name = Configs.getParameter("configuration_name")
            
            if self.current_configuration_name == "retweets":
                self.dataset = RetweetsDataset()
                
            elif self.current_configuration_name == "retweets-2d":
                self.dataset = Retweets2DDataset()
                
            elif self.current_configuration_name == "school":
                self.dataset = SchoolDataset()

            else:
                raise ValueError(f"Configuration name {self.current_configuration_name} not supported")
            
            self.current_dataset = self.dataset
            self.base_dataset_path = self.current_dataset.path()
            self.current_dataset_path = self.current_dataset.path()

            Configs.setDimension(len(self.dataset.getDimension()))

            self.__resetAlgorithms()
            self.__run()
                    
    def __analyseConfiguration(self, configuration_name, save):
        FileSystem.createPostAnalysisFolder(configuration_name)
        FileSystem.createRssEvolutionFolders(configuration_name)

        post_analysis_folder = f"../post_analysis/{configuration_name}" 
        grapher = Grapher(configuration_name, self.dataset, calculate_rss_evolution=self.__calculate_rss_evolution,
                          calculate_quality=self.__calculate_quality)

        print("Plotting RSS Evolution graph")
        grapher.setAttribute(Attribute.RSS_EVOLUTION)
        grapher.setYLimits(0, 30_000)
        grapher.drawGraphs(post_analysis_folder, save)

    def initiatePostAnalysis(self, save=True):
        print("#"*120)
        FileSystem.deletePostAnalysisFolder()

        calculate_rss_evolution = self.calculate_metrics
        if calculate_rss_evolution is None:
            calculate_rss_evolution = str(input("Calculate RSS evolution? Y/N: ")).strip().lower()
            
        if calculate_rss_evolution == "y":
            self.__calculate_rss_evolution = True

        for config_file in Commands.listFolder(self.__configs_folder):
            if config_file.strip().split(".")[-1] == "off":
                print(f"Skipping {config_file}")
                continue
            
            Configs.readConfigFile(f"{self.__configs_folder}/{config_file}")
            self.current_configuration_name = Configs.getParameter("configuration_name")

            if self.current_configuration_name == "retweets":
                self.dataset = RetweetsDataset()

            elif self.current_configuration_name == "retweets-2d":
                self.dataset = Retweets2DDataset()

            elif self.current_configuration_name == "school":
                self.dataset = SchoolDataset()

            else:
                raise ValueError(f"Configuration name {self.current_configuration_name} not supported")
            
            self.current_dataset = self.dataset
            self.base_dataset_path = self.current_dataset.path()
            self.current_dataset_path = self.current_dataset.path()

            Configs.setDimension(len(self.dataset.getDimension()))
            
            print(f"Initiating post analysis for {self.current_configuration_name}...")
            self.__analyseConfiguration(self.current_configuration_name, save=save)

    def getParameter(self, parameter):
        return Configs.getParameter(parameter)
