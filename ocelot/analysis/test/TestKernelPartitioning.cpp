/*!
	\file TestKernelPartitioning.cpp
	\author Andrew Kerr <arkerr@gatech.edu>
	\date 
	
*/

// Ocelot Includes

#include <ocelot/parser/interface/PTXParser.h>
#include <ocelot/analysis/interface/KernelPartitioningPass.h>

// Hydrazine Includes
#include <hydrazine/implementation/ArgumentParser.h>
#include <hydrazine/implementation/Exception.h>
#include <hydrazine/implementation/debug.h>
#include <hydrazine/interface/Test.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 0

////////////////////////////////////////////////////////////////////////////////////////////////////

class Test: public test::Test {
public:
	std::string input;
	std::string prefix;
	std::string heuristic;
	
	bool dot;
	int size;
	
	bool verify;
	bool verbose;
	
public:
	Test(): input("ocelot/executive/tests/TestDivergence.ptx"), prefix(""), verify(false), verbose(true) {
	
		name  = "TestKernelPartitioning";
		
		description = "A test for KernelPartitioningPass analysis and transformation.";
	}
	
	bool doTest() {
		return testPartitioning(input);
	}
	
private:
	
	bool testPartitioning(std::string &file) {
		status << " For File: " << file << std::endl;
		ir::Module module;
		bool success = false;
		dot = true;
		
		try {
		
			status << " loading file: " << file << std::endl;
		
			module.load(file);
			analysis::KernelPartitioningPass pass;
			
			int baseId = 0;
			
			for (ir::Module::KernelMap::const_iterator kernel_it = module.kernels().begin();
				kernel_it != module.kernels().end(); ++kernel_it, ++baseId) {
				
				if (verbose) {
					status << "Kernel: " << kernel_it->second->name << std::endl;
				}
				
				analysis::KernelPartitioningPass::PartitioningHeuristic partitioning =
					analysis::KernelPartitioningPass::fromString(heuristic);
				
				analysis::KernelPartitioningPass::KernelGraph *graph = pass.runOnFunction(
					*kernel_it->second, baseId, partitioning, size);
				
				analysis::KernelPartitioningPass::AnnotatedWriter writer;
				
				std::stringstream name;
				
				name << prefix << kernel_it->second->name << ".dot";
				std::ofstream result(name.str().c_str());
				graph->write(result, writer);
				
				if (dot) {
					std::cout << "dot -Tpdf -o " << prefix + kernel_it->second->name + ".pdf" << " " 
						<< prefix + kernel_it->second->name + ".dot" << std::endl;
				}
								
				delete graph;
			}
		}
		catch(parser::PTXParser::Exception& e) {
			if(e.error == parser::PTXParser::State::NotVersion2_1)
			{
				status << "  Skipping file with incompatible ptx version." 
					<< std::endl;
			}
			status << "Load module failed with exception: " << e.what() << std::endl;
			return false;
		}
		return success;
	}
	
};

////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *arg[]) {

	hydrazine::ArgumentParser parser(argc, arg);
	
	Test test;
	parser.description( test.testDescription() );

	parser.parse("-i", "--input", test.input, "ocelot/executive/tests/TestDivergence.ptx",
		"Input PTX source to partition.");

	parser.parse("-a", "--algorithm", test.heuristic, "maximum",
		"Selects partitioning heuristic to use");
	
	parser.parse("-s", "--size", test.size, 1000,
		"Specifies size in instrucitons of subkernel");

	parser.parse("-p", "--prefix", test.prefix, "",
		"Prefix for output files: <prefix><kernelname>.dot");
		
	parser.parse("-t", "--test", test.verify, false, "Tests and verifies partitioning.");
	
	parser.parse( "-v", "--verbose", test.verbose, false, 
		"Print out info after the test." );
		
	parser.parse("-d", "--dot", test.dot, true, "Invokes dot on resulting .dot files");
	
	parser.parse();
	
	test.test();
	
	return test.passed();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

