/*
 *  SNABSuite -- Spiking Neural Architecture Benchmark Suite
 *  Copyright (C) 2016  Christoph Jenzen
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glob.h>

#include <cypress/cypress.hpp>
#include "common/benchmark.hpp"

using namespace SNAB;

int main(int argc, const char *argv[])
{
	if (argc != 3 && argc != 2 && !cypress::NMPI::check_args(argc, argv)) {
		std::cout << "Usage: " << argv[0] << " <SIMULATOR> [NMPI]" << std::endl;
		return 1;
	}

	if (argc == 3 && std::string(argv[2]) == "NMPI" &&
	    !cypress::NMPI::check_args(argc, argv)) {
		glob_t glob_result;
		glob(std::string("../config/*").c_str(), GLOB_TILDE, NULL,
		     &glob_result);
		std::vector<std::string> files;

		for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
			files.push_back(std::string(glob_result.gl_pathv[i]));
		}
		globfree(&glob_result);
		cypress::NMPI(argv[1], argc, argv, files);
		return 0;
	}
	BenchmarkExec bench(argv[1]);
	return 0;
}