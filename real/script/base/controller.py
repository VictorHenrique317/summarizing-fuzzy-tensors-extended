import os
from numpy import True_
from base.configs import Configs
from base.numpy_translator import NumpyTranslator
from models.attribute import Attribute
from base.crisp_translator import CrispTranslator
from base.school_dataset import SchoolDataset
from base.mat_translator import MatTranslator
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
        self.__sorting_blacklist = ['nclusterbox', 'tribiclusterbox', 'nclusterboxnoperformanceimp', 'nclusterboxcrisp', 'nclusterboxsac']
        # self.__sorting_blacklist = []
        self.__numpy_translator = NumpyTranslator(self)
        self.__mat_translator = MatTranslator(self)
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
        self.__datasets_built = False

        

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
        
        if dimension == 2:
            self.__mat_translator.run(self.dataset)

        self.current_iteration_folder = f"../iteration/{self.current_configuration_name}"

        u = 0.0
        print("="*120 + f"")
        self.current_experiment = f"u{u}"
        FileSystem.createExperimentFolder(self.current_experiment, self.current_configuration_name)

        for algorithm in self.algorithms:
            if algorithm.hasTimedOut(u):
                continue
            
            print(f"Running {algorithm.name}...")

            boolean_tensor = False
            if Configs.getParameter("configuration_name") == "school":
                boolean_tensor = True

            timedout = algorithm.run(u, Configs.getParameter("timeout"), boolean_tensor=boolean_tensor)
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

    def __buildDatasets(self):
        if self.__datasets_built:
            return
        
        self.__datasets_built = True
        print("Building datasets from raw data...")
        os.system("/app/datasets/retweets/preprocess.sh")
        os.system("/app/datasets/primaryschool/preprocess.sh")

    def initiateSession(self):
        delete_iterations = self.delete_iterations
        if delete_iterations is None:
            delete_iterations = str(input("Delete previous iterations? Y/N: ")).strip().lower()

        if delete_iterations == "y":
            FileSystem.deleteIterationFolder()

        FileSystem.deletePostAnalysisFolder()

        self.__buildDatasets()

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

            if Configs.getParameter("configuration_name") == "school":
                self.__initiateRandomStudy()
                    
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

        print("Plotting pattern nb graph")
        grapher.setAttribute(Attribute.PATTERN_NUMBER)
        grapher.setYLimits(0.6, 2_000_000)
        grapher.drawGraphs(post_analysis_folder, save)

        print("Plotting run time graph")
        grapher.setAttribute(Attribute.RUN_TIME)
        grapher.setYLimits(1e-3, 3000)
        grapher.drawGraphs(post_analysis_folder, save)

    def initiatePostAnalysis(self, save=True):
        print("#"*120)
        FileSystem.deletePostAnalysisFolder()

        calculate_rss_evolution = self.calculate_metrics
        if calculate_rss_evolution is None:
            calculate_rss_evolution = str(input("Calculate RSS evolution? Y/N: ")).strip().lower()
            
        if calculate_rss_evolution == "y":
            self.__calculate_rss_evolution = True

        self.__buildDatasets()

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

            if Configs.getParameter("configuration_name") == "school":
                self.__initiateRandomStudyAnalysis()

    def getParameter(self, parameter):
        return Configs.getParameter(parameter)
    
    def __initiateRandomStudy(self):
        print("\n ============================ Initiating random analysis on school dataset...")
        current_iteration_folder = f"../iteration/{self.current_configuration_name}/random_study"
        Commands.execute(f"rm -rf {current_iteration_folder}")
        Commands.execute(f"mkdir {current_iteration_folder}")

        nb_iterations = 30
        for iteration in range(0, nb_iterations+1):
            dimension = len(self.dataset.getDimension())

            u = 0.0
            print("="*120 + f"")
            self.current_experiment = f"u{u}"
            
            initial_patterns_nb = 1000
            if iteration == 0:
                iteration = "ground_patterns"
                initial_patterns_nb = None

            self.current_iteration_folder = f"{current_iteration_folder}/{iteration}"
            Commands.execute(f"mkdir {self.current_iteration_folder}")

            for algorithm in self.algorithms:
                if algorithm.hasTimedOut(u):
                    continue

                if algorithm.name != "nclusterbox":
                    continue
                
                print(f"Running {algorithm.name}...")

                if Configs.getParameter("configuration_name") != "school":
                    raise ValueError("Random study only supported for school dataset")
                
                dataset_path = self.current_dataset.path()
                
                experiment_path = f"{self.current_iteration_folder}/experiments/"
                Commands.execute(f"mkdir {experiment_path}")
                experiment_path += "nclusterbox.experiment"
                
                log_path = f"{self.current_iteration_folder}/logs/"
                Commands.execute(f"mkdir {log_path}")
                log_path += "nclusterbox.log"

                algorithm.run(u, Configs.getParameter("timeout"), boolean_tensor=True,
                                        custom_experiment_path=experiment_path, custom_log_path=log_path,
                                        initial_patterns_nb=initial_patterns_nb)
               
                if algorithm.name not in self.__sorting_blacklist:
                    experiment = Experiment(algorithm.experiment_path, u, dimension)
                    experiment.sortPatterns(self.dataset)

                self.__decodeExperiment(algorithm.experiment_path)

                print("-"*120)

    def __initiateRandomStudyAnalysis(self):
        print("\n ============================ Initiating random analysis on school dataset...")
        results_folder = f"../post_analysis/random_study"
        Commands.execute(f"rm -rf {results_folder}")
        Commands.execute(f"mkdir {results_folder}")
        
        dimension = len(self.dataset.getDimension())
        jaccard_sums = dict()
        ground_experiment = None
        ground_patterns = None
        nb_iterations = 0

        base_folder = f"../iteration/{self.current_configuration_name}/random_study"

        ground_experiment_path = f"{base_folder}/ground_patterns/experiments/nclusterbox.experiment"
        ground_experiment = Experiment(ground_experiment_path, 0.0, dimension)
        ground_patterns = list(ground_experiment.getPatterns())

        for iteration in Commands.listFolder(base_folder):
            if iteration == "ground_patterns":
                continue

            print(f"Initiating analysis for {iteration}...")
            nb_iterations += 1
            experiment_path = f"{base_folder}/{iteration}/experiments/nclusterbox.experiment"
            log_path = f"{base_folder}/{iteration}/logs/nclusterbox.log"
            
            experiment = Experiment(experiment_path, 0.0, dimension, custom_log_path=log_path)

            i =-1
            for ground_pattern in ground_patterns:
                i += 1

                highest_jaccard = 0
                for other_pattern in experiment.getPatterns():

                    jaccard = ground_pattern.jaccardIndex(other_pattern)
                    if jaccard > highest_jaccard:
                        highest_jaccard = jaccard

                jaccard_sums[i] = jaccard_sums.get(i, 0) + highest_jaccard
        
        jaccard_means = {i: jaccard_sum/nb_iterations for i, jaccard_sum in jaccard_sums.items()}

        with open(f"{results_folder}/full_mean_jaccards.txt", "w") as result_file:
            for (i, jaccard_mean) in jaccard_means.items():
                ground_pattern = ground_patterns[i]
                ground_pattern = ground_pattern.get()
                ground_pattern = [",".join(pattern_tuple) for pattern_tuple in ground_pattern]
                ground_pattern = " ".join(ground_pattern)

                result_file.write(f"{ground_pattern}: {jaccard_mean}\n")

        with open(f"{results_folder}/mean_jaccards.txt", "w") as result_file:
            for (i, jaccard_mean) in jaccard_means.items():
                result_file.write(f"{jaccard_mean}\n")