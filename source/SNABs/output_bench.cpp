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
#include <algorithm>  // Minimal and Maximal element
#include <numeric>    // std::accumulate
#include <string>
#include <vector>

#include <cypress/backend/power/netio4.hpp>  // Control of power via NetIO4 Bank
#include <cypress/cypress.hpp>               // Neural network frontend

#include "common/neuron_parameters.hpp"
#include "output_bench.hpp"
#include "util/read_json.hpp"
#include "util/spiking_utils.hpp"
#include "util/utilities.hpp"

namespace SNAB {
OutputFrequencySingleNeuron::OutputFrequencySingleNeuron(
    const std::string backend)
    : SNABBase(
          __func__, backend,
          {"Average frequency", "Standard deviation", "Maximum", "Minimum"},
          {"quality", "quality", "quality", "quality"},
          {"1/ms", "1/ms", "1/ms", "1/ms"}, {"neuron_type", "neuron_params"}),
      m_pop(m_netw, 0)
{
}

cypress::Network &OutputFrequencySingleNeuron::build_netw(
    cypress::Network &netw)
{
	std::string neuron_type_str = m_config_file["neuron_type"];
	SpikingUtils::detect_type(neuron_type_str);

	// discard out put
	std::ofstream out;
	// Get neuron neuron_parameters
	NeuronParameters neuron_params =
	    NeuronParameters(SpikingUtils::detect_type(neuron_type_str),
	                     m_config_file["neuron_params"], out);
	// Set up population
	m_pop =
	    SpikingUtils::add_population(neuron_type_str, netw, neuron_params, 1);
	return netw;
}

void OutputFrequencySingleNeuron::run_netw(cypress::Network &netw)
{
	// Debug logger, may be ignored in the future
	netw.logger().min_level(cypress::DEBUG, 0);

	// PowerManagementBackend to use netio4
	cypress::PowerManagementBackend pwbackend(
	    std::make_shared<cypress::NetIO4>(),
	    cypress::Network::make_backend(m_backend));
	netw.run(pwbackend, 100.0);
}

std::vector<cypress::Real> OutputFrequencySingleNeuron::evaluate()
{
	// Vector of frequencies
	std::vector<cypress::Real> frequencies;
	// Get spikes
	auto spikes = m_pop.signals().data(0);
	// Calculate frequencies
	if (spikes.size() != 0) {

		for (size_t i = 0; i < spikes.size() - 1; i++) {
			frequencies.push_back(cypress::Real(1.0) /
			                      (spikes[i + 1] - spikes[i]));
		}
	}
	else {
		frequencies.push_back(0.0);
	}

	// Calculate statistics
	cypress::Real max =
	    *std::max_element(frequencies.begin(), frequencies.end());
	cypress::Real min =
	    *std::min_element(frequencies.begin(), frequencies.end());
	cypress::Real avg =
	    std::accumulate(frequencies.begin(), frequencies.end(), 0.0) /
	    cypress::Real(frequencies.size());
	cypress::Real std_dev = 0.0;
	std::for_each(
	    frequencies.begin(), frequencies.end(),
	    [&](const cypress::Real val) { std_dev += (val - avg) * (val - avg); });
	std_dev = std::sqrt(std_dev / cypress::Real(frequencies.size() - 1));
	std::vector<cypress::Real> results = {avg, std_dev, max, min};
	return results;
}

OutputFrequencyMultipleNeurons::OutputFrequencyMultipleNeurons(
    const std::string backend)
    : SNABBase(__func__, backend,
                    {"Average frequency of neurons", "Standard deviation",
                     "Maximum av frequency", "Minimum av frequency"},
                    {"quality", "quality", "quality", "quality"},
                    {"1/ms", "1/ms", "1/ms", "1/ms"},
                    {"neuron_type", "neuron_params", "#neurons"}),
      m_pop(m_netw, 0)
{
}

cypress::Network &OutputFrequencyMultipleNeurons::build_netw(
    cypress::Network &netw)
{
	std::string neuron_type_str = m_config_file["neuron_type"];
	SpikingUtils::detect_type(neuron_type_str);

	// discard out put
	std::ofstream out;
	// Get neuron neuron_parameters
	NeuronParameters neuron_params =
	    NeuronParameters(SpikingUtils::detect_type(neuron_type_str),
	                     m_config_file["neuron_params"], out);

	m_num_neurons = m_config_file["#neurons"];

	// Set up population
	m_pop = SpikingUtils::add_population(neuron_type_str, netw, neuron_params,
	                                     m_num_neurons);
	return netw;
}

void OutputFrequencyMultipleNeurons::run_netw(cypress::Network &netw)
{
	// Debug logger, may be ignored in the future
	netw.logger().min_level(cypress::DEBUG, 0);

	// PowerManagementBackend to use netio4
	cypress::PowerManagementBackend pwbackend(
	    std::make_shared<cypress::NetIO4>(),
	    cypress::Network::make_backend(m_backend));
	netw.run(pwbackend, 100.0);
}

std::vector<cypress::Real> OutputFrequencyMultipleNeurons::evaluate()
{
	// Gather the average frequency of every neuron
	std::vector<cypress::Real> averages(m_num_neurons, -1);
	for (size_t i = 0; i < m_num_neurons; i++) {
		// Vector of frequencies
		std::vector<cypress::Real> frequencies;

		// Get spikes
		auto spikes = m_pop[i].signals().data(0);
		// Calculate frequencies
		if (spikes.size() != 0) {

			for (size_t i = 0; i < spikes.size() - 1; i++) {
				frequencies.push_back(cypress::Real(1.0) /
				                      (spikes[i + 1] - spikes[i]));
			}
		}
		else {
			frequencies.push_back(0.0);
		}

		// Calculate average
		averages[i] =
		    std::accumulate(frequencies.begin(), frequencies.end(), 0.0) /
		    cypress::Real(frequencies.size());
	}

	// Calculate statistics
	cypress::Real max = *std::max_element(averages.begin(), averages.end());
	cypress::Real min = *std::min_element(averages.begin(), averages.end());
	cypress::Real avg = std::accumulate(averages.begin(), averages.end(), 0.0) /
	                    cypress::Real(averages.size());
	cypress::Real std_dev = 0.0;
	std::for_each(
	    averages.begin(), averages.end(),
	    [&](const cypress::Real val) { std_dev += (val - avg) * (val - avg); });
	std_dev = std::sqrt(std_dev / cypress::Real(averages.size() - 1));

	std::vector<cypress::Real> results = {avg, std_dev, max, min};
	return results;
}
}