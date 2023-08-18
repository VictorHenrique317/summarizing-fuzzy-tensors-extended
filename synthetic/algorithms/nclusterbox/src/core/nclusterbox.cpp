// Copyright 2018-2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include <iostream>
#include <thread>
#include <boost/program_options.hpp>
#include "sysexits.h"

#include "../utilities/NoInputException.h"
#include "../utilities/UsageException.h"
#include "../utilities/DataFormatException.h"
#include "../utilities/NoOutputException.h"
#include "SelectionCriterion.h"
#include "DenseRoughTensor.h"
#include "PatternFileReader.h"
#include "ConcurrentPatternPool.h"
#include "ModifiedPattern.h"
#include "RankPatterns.h"

using namespace boost::program_options;

int main(int argc, char* argv[])
{
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  AbstractRoughTensor* roughTensor;
  float verboseStep = 0;
  long long maxNbOfInitialPatterns = 0;
  int maxSelectionSize = 0;
  SelectionCriterion selectionCriterion;
  bool isRSSPrinted;
  {
#ifdef DETAILED_TIME
    steady_clock::time_point startingPoint;
#endif
    vector<thread> threads;
    bool isModifyingOrGrowing;
    {
      string patternDimensionSeparator;
      string patternElementSeparator;
      {
	int nbOfJobs;
	bool isGrow;
	// Parsing the command line and the option file
	try
	  {
	    string tensorFileName;
	    string outputFileName;
	    options_description generic("Generic options");
	    generic.add_options()
	      ("help,h", "produce this help message")
	      ("hio", "produce help on Input/Output format")
	      ("version,V", "display version information and exit")
	      ("opt", value<string>(), "set the option file name (by default, [tensor-file].opt if present)");
	    options_description basicConfig("Basic configuration (on the command line or in the option file)");
	    basicConfig.add_options()
	      ("verbose,v", value<float>(), "verbose output every arg seconds")
	      ("boolean,b", "consider the tensor Boolean")
	      ("out,o", value<string>(&outputFileName)->default_value("-"), "set output file name")
	      ("max,m", value<long long>(), "set max nb of initial patterns (by default, unbounded)")
	      ("forget,f", "do not store the visited patterns")
#ifdef DEBUG_MODIFY
	      ("jobs,j", value<int>(&nbOfJobs)->default_value(1), "set nb of simultaneously modified patterns")
#else
	      ("jobs,j", value<int>(&nbOfJobs)->default_value(max(thread::hardware_concurrency(), static_cast<unsigned int>(1))), "set nb of simultaneously modified patterns")
#endif
	      ("density,d", value<float>(), "set threshold between 0 (dense storage of the input tensor) and 1 (default, minimization of memory usage)")
	      ("msc", value<string>()->default_value("bic"), "set max selection criterion (rss, aic or bic)")
	      ("mss", value<int>(), "set max selection size (by default, unbounded)")
	      ("ns", "neither select nor rank output patterns")
	      ("shift,s", value<float>(), "shift memberhip degrees by constant in argument (by default, density of input tensor)")
	      ("expectation,e", "shift every memberhip degree by the max density of the slices covering it")
	      ("patterns,p", value<string>(), "set initial patterns, instead of the default ones")
	      ("os", value<string>(), "only select and rank the provided patterns")
	      ("grow,g", "remove nothing from the initial patterns");
	    options_description io("Input/Output format (on the command line or in the option file)");
	    io.add_options()
	      ("tds", value<string>()->default_value(" "), "set any character separating dimensions in input tensor")
	      ("tes", value<string>()->default_value(","), "set any character separating elements in input tensor")
	      ("pds", value<string>()->default_value(" "), "set any character separating dimensions in input patterns")
	      ("pes", value<string>()->default_value(","), "set any character separating elements in input patterns")
	      ("ods", value<string>()->default_value(" "), "set string separating dimensions in output patterns")
	      ("oes", value<string>()->default_value(","), "set string separating elements in output patterns")
	      ("pl", "print densities in the shifted (rather than input) tensor in output")
	      ("ps", "print sizes in output")
	      ("sp", value<string>()->default_value(" : "), "set string prefixing sizes in output")
	      ("ss", value<string>()->default_value(" "), "set string separating sizes in output")
	      ("pa", "print areas in output")
	      ("ap", value<string>()->default_value(" : "), "set string prefixing area in output")
	      ("pr", "print residual sum of squares of truncated output")
	      ("rp", value<string>()->default_value(" : "), "set string prefixing residual sum of squares in output");
	    options_description hidden("Hidden options");
	    hidden.add_options()
	      ("file", value<string>(&tensorFileName)->default_value("-"), "set tensor file");
	    positional_options_description p;
	    p.add("file", -1);
	    options_description commandLineOptions;
	    commandLineOptions.add(generic).add(basicConfig).add(io).add(hidden);
	    variables_map vm;
	    store(command_line_parser(argc, argv).options(commandLineOptions).positional(p).run(), vm);
	    notify(vm);
	    if (vm.count("help"))
	      {
		cout << "Usage: nclusterbox [options] [tensor-file]\n" << generic << basicConfig << "\nReport bugs to: lcerf@dcc.ufmg.br\n";
		return EX_OK;
	      }
	    if (vm.count("hio"))
	      {
		cout << io;
		return EX_OK;
	      }
	    if (vm.count("version"))
	      {
		cout << "nclusterbox version 0.46\nCopyright (C) 2023 Loïc Cerf.\nLicense GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n";
		return EX_OK;
	      }
	    if (vm.count("opt"))
	      {
		options_description config;
		config.add(basicConfig).add(io).add(hidden);
		if (vm["opt"].as<string>() == "-")
		  {
		    store(parse_config_file(cin, config), vm);
		  }
		else
		  {
		    ifstream optionFile(vm["opt"].as<string>());
		    if (!optionFile)
		      {
			throw NoInputException(vm["opt"].as<string>().c_str());
		      }
		    store(parse_config_file(optionFile, config), vm);
		  }
		notify(vm);
	      }
	    else
	      {
		if (vm.count("file"))
		  {
		    ifstream optionFile((tensorFileName + ".opt").c_str());
		    options_description config;
		    config.add(basicConfig).add(io).add(hidden);
		    store(parse_config_file(optionFile, config), vm);
		    optionFile.close();
		    notify(vm);
		  }
	      }
	    if (vm.count("max"))
	      {
		if (vm["max"].as<long long>() < 1)
		  {
		    throw UsageException("max option should provide a positive integer!");
		  }
		maxNbOfInitialPatterns = vm["max"].as<long long>();
	      }
	    if (vm.count("mss"))
	      {
		if (vm["mss"].as<int>() < 1)
		  {
		    throw UsageException("mss option should provide a positive integer!");
		  }
		maxSelectionSize = vm["mss"].as<int>();
	      }
	    const string sc = vm["msc"].as<string>();
	    if (sc == string("rss").substr(0, sc.size()))
	      {
		selectionCriterion = rss;
	      }
	    else
	      {
		if (sc == string("aic").substr(0, sc.size()))
		  {
		    selectionCriterion = aic;
		  }
		else
		  {
		    if (sc == string("bic").substr(0, sc.size()))
		      {
			selectionCriterion = bic;
		      }
		    else
		      {
			throw UsageException("msc option should provide either \"rss\" or \"aic\" or \"bic\"!");
		      }
		  }
	      }
	    if (vm.count("verbose"))
	      {
		if (vm.count("ns") && outputFileName == "-")
		  {
		    throw UsageException("ns and verbose together forbid to output to the standard output!");
		  }
		verboseStep = vm["verbose"].as<float>();
		if (!verboseStep)
		  {
		    verboseStep = -1;
		  }
	      }
	    float density;
	    if (vm.count("density"))
	      {
		density = vm["density"].as<float>();
		if (density < 0 || density > 1)
		  {
		    throw UsageException("density option should provide a float in [0, 1]!");
		  }
	      }
	    else
	      {
		density = 1;
	      }
	    if (vm.count("os"))
	      {
		const vector<string> ignoredOptions {"forget", "ns", "patterns", "grow"};
		for (const string& ignoredOption : ignoredOptions)
		  {
		    if (vm.count(ignoredOption))
		      {
			cerr << "Warning: " << ignoredOption << " option has no effect here, because os option used\n";
		      }
		  }
		isModifyingOrGrowing = false;
		PatternFileReader::openFile(vm["os"].as<string>().c_str());
	      }
	    else
	      {
		if (vm["jobs"].as<int>() < 1)
		  {
		    throw UsageException("jobs option should provide a positive integer!");
		  }
		isModifyingOrGrowing = true;
		if (vm.count("patterns"))
		  {
		    PatternFileReader::openFile(vm["patterns"].as<string>().c_str());
		  }
		else
		  {
		    if (vm.count("os"))
		      {
			PatternFileReader::openFile(vm["os"].as<string>().c_str());
		      }
		    else
		      {
			ConcurrentPatternPool::setDefaultPatterns(maxNbOfInitialPatterns);
		      }
		  }
		isGrow = vm.count("grow");
	      }
	    if (vm.count("expectation"))
	      {
		if (vm.count("shift"))
		  {
		    throw UsageException("shift and expectation options are mutually exclusive!");
		  }
		if (vm.count("density") && density)
		  {
		    cerr << "Warning: density option has no effect here; the expectation option always triggers a completely dense storage of the tensor\n";
		  }
		roughTensor = new DenseRoughTensor(tensorFileName.c_str(), vm["tds"].as<string>().c_str(), vm["tes"].as<string>().c_str(), vm.count("boolean"), verboseStep);
	      }
	    else
	      {
		if (vm.count("shift"))
		  {
		    if (vm["shift"].as<float>() < 0 || vm["shift"].as<float>() >= 1)
		      {
			throw UsageException("shift option should provide a float in [0, 1[!");
		      }
		    roughTensor = AbstractRoughTensor::makeRoughTensor(tensorFileName.c_str(), vm["tds"].as<string>().c_str(), vm["tes"].as<string>().c_str(), density, vm["shift"].as<float>(), vm.count("boolean"), verboseStep);
		  }
		else
		  {
		    roughTensor = AbstractRoughTensor::makeRoughTensor(tensorFileName.c_str(), vm["tds"].as<string>().c_str(), vm["tes"].as<string>().c_str(), density, vm.count("boolean"), verboseStep);
		  }
	      }
	    ModifiedPattern::setContext(roughTensor, !vm.count("forget"));
	    if (verboseStep)
	      {
		cout << "\rShifting tensor: done.\n";
	      }
	    AbstractRoughTensor::setOutput(outputFileName.c_str(), vm["ods"].as<string>().c_str(), vm["oes"].as<string>().c_str(), "", "", vm["sp"].as<string>().c_str(), vm["ss"].as<string>().c_str(), vm["ap"].as<string>().c_str(), vm["rp"].as<string>().c_str(), vm.count("pl"), vm.count("ps"), vm.count("pa"), vm.count("ns"));
	    if (AbstractRoughTensor::isDirectOutput())
	      {
		roughTensor->setNoSelection();
	      }
	    if (verboseStep)
	      {
		if (isModifyingOrGrowing)
		  {
		    cout << "Getting and modifying initial patterns ..." << flush;
		  }
		else
		  {
		    cout << "Getting patterns ..." << flush;
		  }
	      }
	    isRSSPrinted = vm.count("pr");
#ifdef DETAILED_TIME
	    startingPoint = steady_clock::now();
#endif
	    patternElementSeparator = vm["pes"].as<string>();
	    patternDimensionSeparator = vm["pds"].as<string>();
	  }
	catch (unknown_option& e)
	  {
	    cerr << "Unknown option!\n";
	    return EX_USAGE;
	  }
	catch (UsageException& e)
	  {
	    cerr << e.what() << '\n';
	    return EX_USAGE;
	  }
	catch (NoInputException& e)
	  {
	    cerr << e.what() << '\n';
	    return EX_NOINPUT;
	  }
	catch (DataFormatException& e)
	  {
	    cerr << e.what() << '\n';
	    return EX_DATAERR;
	  }
	catch (NoOutputException& e)
	  {
	    cerr << e.what() << '\n';
	    return EX_CANTCREAT;
	  }
	if (isModifyingOrGrowing)
	  {
	    threads.reserve(nbOfJobs);
	    if (isGrow)
	      {
		do
		  {
		    threads.emplace_back(ModifiedPattern::grow);
		  }
		while (--nbOfJobs);
	      }
	    else
	      {
		do
		  {
		    threads.emplace_back(ModifiedPattern::modify);
		  }
		while (--nbOfJobs);
	      }
	  }
      }
      if (ConcurrentPatternPool::readFromFile())
	{
	  PatternFileReader::read(patternDimensionSeparator.c_str(), patternElementSeparator.c_str(), AbstractRoughTensor::getIds2Labels(), maxNbOfInitialPatterns);
	}
    }
    if (isModifyingOrGrowing)
      {
	if (verboseStep)
	  {
	    if (verboseStep > 0)
	      {
		thread(ConcurrentPatternPool::printProgressionOnSTDIN, verboseStep).detach();
	      }
	    else
	      {
		cout << "\rGetting initial patterns: done.           \nModifying patterns ... " << flush;
	      }
	    for (thread& t : threads)
	      {
		t.join();
	      }
	    if (AbstractRoughTensor::isDirectOutput())
	      {
		cout << "\rModifying patterns: " << ModifiedPattern::getNbOfOutputPatterns() << " patterns with locally maximal explanatory powers.\n";
	      }
	    else
	      {
		ModifiedPattern::insertCandidateVariables();
		cout << "\rModifying patterns: " << AbstractRoughTensor::candidateVariables.size() << " patterns with locally maximal explanatory powers.\n";
	      }
	  }
	else
	  {
	    for (thread& t : threads)
	      {
		t.join();
	      }
	    if (!AbstractRoughTensor::isDirectOutput())
	      {
		ModifiedPattern::insertCandidateVariables();
	      }
	  }
#ifdef DETAILED_TIME
#ifdef GNUPLOT
	cout << '\t' << duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#else
	cout << "Explanatory power maximization time: " << duration_cast<duration<double>>(steady_clock::now() - startingPoint).count() << "s\n";
#endif
#endif
      }
    else
      {
	ConcurrentPatternPool::moveTo(AbstractRoughTensor::candidateVariables);
	if (verboseStep)
	  {
	    cout << "\rGetting patterns: " << AbstractRoughTensor::candidateVariables.size() << " patterns.\n";
	  }
#ifdef DETAILED_TIME
#ifdef GNUPLOT
	cout << "\t0";
#else
	cout << "Explanatory power maximization time: 0s\n";
#endif
#endif
      }
  }
  if (AbstractRoughTensor::candidateVariables.empty())
    {
#if defined NUMERIC_PRECISION || defined NB_OF_PATTERNS || defined DETAILED_TIME
#ifdef GNUPLOT
#ifdef NB_OF_PATTERNS
      cout << "\t0\t0";
#endif
#ifdef NUMERIC_PRECISION
      cout << "\t0";
#endif
#ifdef DETAILED_TIME
      cout << "\t0\t0";
#endif
#else
#ifdef NB_OF_PATTERNS
      cout << "Nb of patterns candidates for selection: 0\n";
#endif
#ifdef NUMERIC_PRECISION
      cout << "Numeric precision: 0\n";
#endif
#ifdef DETAILED_TIME
      cout << "Tensor reduction time: 0s\n";
#endif
#ifdef NB_OF_PATTERNS
      cout << "Nb of selected patterns: 0\n";
#endif
#ifdef DETAILED_TIME
      cout << "Selecting time: 0s\n";
#endif
#endif
#endif
    }
  else
    {
      RankPatterns::rank(roughTensor, verboseStep, maxSelectionSize, selectionCriterion, isRSSPrinted);
    }
  delete roughTensor;
#ifdef TIME
  AbstractRoughTensor::printCurrentDuration();
#endif
#if defined GNUPLOT && (defined NUMERIC_PRECISION || defined NB_OF_PATTERNS || defined DETAILED_TIME || defined TIME)
  cout << '\n';
#endif
  return EX_OK;
}
