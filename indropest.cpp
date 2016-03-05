#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <getopt.h>
#include <ctime>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "Estimation/Estimator.h"
#include "Estimation/ResultPrinter.h"
#include "Estimation/IndropResults.h"
#include "Tools/Logs.h"
#include "Tools/RefGenesContainer.h"

using namespace std;
using namespace Estimation;

struct Params
{
	bool cant_parse;
	bool verbose;
	bool text_output;
	bool merge_tags;
	bool not_filtered;
	string output_name;
	string config_file_name;
	string log_prefix;
	string reads_params_file;

	Params() : cant_parse(false), verbose(false), text_output(false), merge_tags(false), not_filtered(false)
			, output_name(""), config_file_name(""), log_prefix(""), reads_params_file("")
	{}
};

static void usage()
{
	cerr << "\tindropest: estimate molecular counts per cell" << endl;
	cerr << "SYNOPSIS\n";
	cerr <<
	"\tindropest [-t, --text-output] [-m|--merge-cell-tags] [-v|--verbose] [-n | --not-filtered] "
	"[-l, --log-prefix logs_name] [-r, --reads-params filename] -c config.xml file1.bam [file2.bam ...]" << endl;
	cerr << "OPTIONS:\n";
	cerr << "\t-c, --config filename: xml file with estimation parameters" << endl;
	cerr << "\t-l, --log-prefix : logs prefix" << endl;
	cerr << "\t-m, --merge-cell-tags : merge linked cell tags" << endl;
	cerr << "\t-n, --not-filtered : print data for all cells" << endl;
	cerr << "\t-o, --output-file filename : output file name" << endl;
	cerr << "\t-r, --reads-params filename: file with serialized params from tags search step" << endl;
	cerr << "\t-t, --text-output : write out text matrix" << endl;
}

static Params parse_cmd_params(int argc, char **argv)
{
	Params params;

	int option_index = 0;
	int c;
	static struct option long_options[] = {
			{"config",     		required_argument, 0, 'c'},
			{"log-prefix",		required_argument, 0, 'l'},
			{"merge-cell-tags", no_argument,       0, 'm'},
			{"not-filtered",	no_argument, 	   0, 'n'},
			{"output-file",     required_argument, 0, 'o'},
			{"reads-params",     required_argument, 0, 'o'},
			{"text-output",     no_argument,       0, 't'},
			{"verbose",         no_argument,       0, 'v'},
			{0, 0,                                 0, 0}
	};
	while ((c = getopt_long(argc, argv, "vtmno:c:l:r:", long_options, &option_index)) != -1)
	{
		switch (c)
		{
			case 'v' :
				params.verbose = true;
				break;
			case 'm' :
				params.merge_tags = true;
				break;
			case 't' :
				params.text_output = true;
				break;
			case 'n' :
				params.not_filtered = true;
				break;
			case 'o' :
				params.output_name = string(optarg);
				break;
			case 'c' :
				params.config_file_name = string(optarg);
				break;
			case 'r' :
				params.reads_params_file = string(optarg);
				break;
			case 'l' :
				params.log_prefix = string(optarg);
				break;
			default:
				cerr << "indropest: unknown arguments passed" << endl;
				params.cant_parse = true;
				return params;
		}
	}

	if (optind > argc - 1)
	{
		cerr << "indropset: at least one bam file must be supplied" << endl;
		params.cant_parse = true;
		return params;
	}

	if (params.config_file_name == "")
	{
		cerr << "indropset: config file must be supplied" << endl;
		params.cant_parse = true;
		return params;
	}

	if (params.output_name == "")
	{
		if (params.text_output)
		{
			params.output_name = "cell.counts.txt";
		}
		else
		{
			params.output_name = "cell.counts.bin";
		}
	}

	return params;
}

int main(int argc, char **argv)
{
	Params params = parse_cmd_params(argc, argv);

	if (params.cant_parse)
	{
		usage();
		return 1;
	}

	if (params.log_prefix.length() != 0)
	{
		params.log_prefix += "_";
	}
	Tools::init_log(params.verbose, false, params.log_prefix + "est_main.log", params.log_prefix + "est_debug.log");

	vector<string> files;
	while (optind < argc)
	{
		files.push_back(string(argv[optind++]));
	}

	boost::property_tree::ptree pt;
	read_xml(params.config_file_name, pt);

	time_t ctt = time(0);
	try
	{
		L_TRACE << "Run: " << asctime(localtime(&ctt));
		Estimator estimator(pt.get_child("config.Estimation"));
		IndropResult results = estimator.get_results(files, params.merge_tags, params.not_filtered, params.reads_params_file);
		
		ctt = time(0);
		L_TRACE << "Done: " << asctime(localtime(&ctt));

		if (params.text_output)
		{
			ResultPrinter::print_text_table(params.output_name, results.cm);
			params.output_name += ".bin";
		}
#ifdef R_LIBS
		ResultPrinter::print_rds(params.output_name, results);
#else
		ResultPrinter::print_binary(params.output_name, results);
#endif
	}
	catch (std::runtime_error err)
	{
		L_ERR << err.what();
		return 1;
	}
	ctt = time(0);
	L_TRACE << "All done: " << asctime(localtime(&ctt));
}
