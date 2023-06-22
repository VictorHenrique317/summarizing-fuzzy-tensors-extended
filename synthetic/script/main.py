from base.controller import Controller
from algorithm.getf import Getf
from algorithm.cancer import Cancer
from algorithm.tribiclusterbox import TriBiclusterBox
from algorithm.nclusterbox import NclusterBox
from algorithm.nclusterbox_no_performance_imp import NclusterBoxNoPerformanceImp
from algorithm.nclusterbox_no_subfiber_maximization import NclusterBoxNoSubfiberMazimization

controller = Controller()

# ========================== ALGORITHMS USED ========================== #
# getf = Getf(controller)
# cancer = Cancer(controller)
# triclusterbox = TriBiclusterBox(controller)
# nclusterboxnoperformanceimp = NclusterBoxNoPerformanceImp(controller)
nclusterboxnosubfibermaximization = NclusterBoxNoSubfiberMazimization(controller)
nclusterbox = NclusterBox(controller)
# ========================== ALGORITHMS USED ========================== #

controller.initiateSession()
controller.initiatePostAnalysis()
