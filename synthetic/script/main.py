from base.controller import Controller
from algorithm.getf import Getf
from algorithm.cancer import Cancer
from algorithm.tribiclusterbox import TriBiclusterBox
from algorithm.nclusterbox import NclusterBox
from algorithm.nclusterbox_no_performance_imp import NclusterBoxNoPerformanceImp
from algorithm.nclusterbox_crisp import NclusterBoxCrisp

controller = Controller(delete_post_analysis="y", 
                        calculate_metrics="y", 
                        delete_iterations="n")

# ========================== ALGORITHMS USED ========================== #
# getf = Getf(controller)
# cancer = Cancer(controller)
# triclusterbox = TriBiclusterBox(controller)
# nclusterboxnoperformanceimp = NclusterBoxNoPerformanceImp(controller)
# nclusterbox = NclusterBox(controller)
nclusterboxcrisp = NclusterBoxCrisp(controller)
# ========================== ALGORITHMS USED ========================== #

controller.initiateSession()
controller.initiatePostAnalysis()
