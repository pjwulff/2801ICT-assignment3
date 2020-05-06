#include <iostream>
#include <fstream>
#include <set>
#include <utility>
#include <iomanip>

int
main()
{
	std::fstream ifile;
	std::ofstream ofile;
	ifile.open("finalInput.txt");
	ofile.open("finalInput-filtered.txt");
	size_t numv, nume;
	std::set<std::pair<size_t, size_t>> edges;
	ifile >> numv;
	ifile >> nume;
	size_t actual_e = 0;
	for (size_t i = 0; i < nume; ++i) {
		size_t f, t;
		std::string w;
		ifile >> f;
		ifile >> t;
		ifile >> w;
		std::pair<size_t, size_t> pair(f, t);
		if (edges.count(pair)) {
			continue;
		}
		edges.insert(pair);
		++actual_e;
		ofile << f << " " << t << " " << w << std::endl;
	}
	std::cout << actual_e << std::endl;
	return 0;
}
