// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/BuildInfo.hpp"
#include "Common/CFactory.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshTransformer.hpp"

using namespace boost;
using namespace boost::program_options;
using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

int main(int argc, char * argv[])
{
  CFinfo << "COOLFLUID K3 Mesh Transformer\n" << CFflush;
  
  Core::instance().initiate(argc, argv);
  ExceptionManager::instance().ExceptionDumps = true;
	ExceptionManager::instance().ExceptionAborts = true;
	try
  {
		
		// map file extensions to readers and writers
		
		typedef std::pair<std::string,std::vector<CMeshReader::Ptr> > extensions_to_readers_pair_t;
		typedef std::pair<std::string,std::vector<CMeshWriter::Ptr> > extensions_to_writers_pair_t;
		std::map<std::string,std::vector<CMeshReader::Ptr> > extensions_to_readers;
		std::map<std::string,std::vector<CMeshWriter::Ptr> > extensions_to_writers;
		std::vector<CMeshReader::Ptr> readers;
		std::vector<CMeshWriter::Ptr> writers;
		typedef std::pair<std::string,std::string> transformers_description_t;
		std::map<std::string,std::string> transformers_description;
		std::map<std::string,CMeshTransformer::Ptr> name_to_transformers;

			

		CFactory::Ptr meshreader_fac = Core::instance().factories()->get_factory<CMeshReader>();

		boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *meshreader_fac ) )
		{
			CMeshReader::Ptr reader = bdr.build("reader")->as_type<CMeshReader>();
			readers.push_back(reader);
			boost_foreach(const std::string& extension, reader->get_extensions())
				extensions_to_readers[extension].push_back(reader);
		}
		
		CFactory::Ptr meshwriter_fac = Core::instance().factories()->get_factory<CMeshWriter>();

		boost_foreach(CBuilder& bdw, find_components_recursively<CBuilder>( *meshwriter_fac ) )
		{
			CMeshWriter::Ptr writer = bdw.build("writer")->as_type<CMeshWriter>();
			writers.push_back(writer);
			boost_foreach(const std::string& extension, writer->get_extensions())
				extensions_to_writers[extension].push_back(writer);
		}
		
		CFactory::Ptr meshtrans_fac = Core::instance().factories()->get_factory<CMeshTransformer>();

		boost_foreach(CBuilder& bdt, find_components_recursively<CBuilder>( *meshtrans_fac ))
		{
			CMeshTransformer::Ptr transformer = bdt.build("transformer")->as_type<CMeshTransformer>();
			transformers_description[bdt.builder_concrete_type_name()] = transformer->brief_description();
			name_to_transformers[bdt.builder_concrete_type_name()] = transformer;
		}
		
		options_description desc("General Options");
		desc.add_options()
		("help,h", value<std::string>()->implicit_value(std::string()) , "this help if no arg, or more detailed help of submodule")
		("input,i" , value<std::vector<std::string> >()->multitoken(), "input file(s)")
		("output,o", value<std::vector<std::string> >()->multitoken(), "output file(s)")
		("transform,t", value<std::vector<std::string> >()->multitoken(), "transformations")
		("dryrun,d", "dry run")
		("version,v", "show version")
		;
		variables_map vm;
		parsed_options parsed = parse_command_line(argc, argv, desc);
		store(parsed, vm);
		notify(vm);
		
		
		
		if (vm.count("help") || vm.size()==0)
		{
			std::string submodule = std::string();
			if (vm.size() != 0)
				submodule = vm["help"].as<std::string>();

			if (submodule == std::string())
			{
				// Default help
				CFinfo << CFendl << "Usage: " << argv[0] << " [options]" << CFendl << CFendl;
				CFinfo << desc << CFendl;
				std::vector< std::string > vk, vt;
				vk.push_back("Input formats:");        vt.push_back("");
				boost_foreach(const CMeshReader::Ptr& reader, readers)
				{
					vk.push_back("  " + reader->get_format());
					std::string extensions;
					boost_foreach(const std::string& ext, reader->get_extensions())
					extensions += ext + " ";
					vt.push_back(extensions);
				}
				vk.push_back("");                      vt.push_back("");
				vk.push_back("Output formats:");       vt.push_back("");
				boost_foreach(const CMeshWriter::Ptr& writer, writers)
				{
					vk.push_back("  " + writer->get_format());
					std::string extensions;
					boost_foreach(const std::string& ext, writer->get_extensions())
					extensions += ext + " ";
					vt.push_back(extensions);
				}     
				vk.push_back("");                      vt.push_back("");
				vk.push_back("Transformations:");      vt.push_back("(use --help 'transformation' for more info)");
				boost_foreach(transformers_description_t transformer, transformers_description)
				{
					vk.push_back("  " + transformer.first);     vt.push_back(transformer.second);
				}
				vk.push_back("");                      vt.push_back("");
				
				// output with keys strings adjusted to the same length
				unsigned l = 0;
				for (unsigned i=0; i<vk.size(); ++i)
					l = (l>vk[i].length()? l:vk[i].length());
				l+=2;
				for (unsigned i=0; i<vk.size(); ++i)
					vk[i].insert(vk[i].end(),l-vk[i].length(),' ');
				
				for (unsigned i=0; i<vk.size(); ++i)
					CFinfo << vk[i] << vt[i] << CFendl;
			}
			else
			{
				if (name_to_transformers.find(submodule) != name_to_transformers.end())
				{
					CFinfo << "\n" << submodule << ":" << CFendl;
					CMeshTransformer::Ptr transformer = name_to_transformers[submodule];
					CFinfo << transformer->help() << CFendl;
				}
				else
				{
					CFinfo << submodule << " does not exist" << CFendl;
				}

			}


			exit(0);
		}
		
		bool dryrun=false;
		if (vm.count("dryrun"))
		{
			CFinfo << "\nThis is what would happen without the dryrun option:" << CFendl << CFendl;
			dryrun=true;
		}
		
		if (vm.count("version"))
		{
			CFinfo << Core::instance().build_info()->version_header () << "\n";
		}
		
		// create mesh object
		CRoot::Ptr root = CRoot::create("root");
		CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
		
		
		typedef basic_option<char> Option;
		typedef std::basic_string<char> OptionValue;
		
		boost_foreach(Option option, parsed.options)
		{
			
			// parse myself in order, multiple instances of same option possible
			if (option.string_key=="input")
			{
				boost_foreach(OptionValue value, option.value)
				{
					filesystem::path inputfile (value);
					const std::string ext = inputfile.extension();
					CMeshReader::Ptr reader;
					if (!extensions_to_readers.count(ext))
					{
						Uint selection = 0;
						CFinfo << inputfile << " has ambiguous extension " << ext << CFendl;
						boost_foreach(const CMeshReader::Ptr selectreader , readers)
						CFinfo << "  [" << selection++ +1 << "]  " << selectreader->get_format() << CFendl;
						CFinfo << "Select the correct reader: " << CFflush;
						std::cin >> selection;
						reader = readers[--selection];
					}
					else
					{
						Uint selection = 0;
						if (extensions_to_readers[ext].size()>1)
						{
							CFinfo << inputfile << " with extension " << ext << " has multiple readers: " << CFendl;
							boost_foreach(const CMeshReader::Ptr selectreader , extensions_to_readers[ext])
							CFinfo << "  [" << selection++ +1 << "]  " << selectreader->get_format() << CFendl;
							CFinfo << "Select the correct reader: " << CFflush;
							std::cin >> selection;
							--selection;
						}
						reader = extensions_to_readers[ext][selection];
					}
					
					CFinfo << "\nReading " << inputfile << " with " << reader->get_format() << CFendl;
					
					if (!dryrun) reader->read_from_to(inputfile,mesh);
				}
			}
			else if (option.string_key=="transform")
			{
				const std::string transformer_name = option.value[0];
				std::string transformer_args;
				std::vector<std::string> parsed_transformer_args;
				for (Uint i=1; i<option.value.size(); ++i)
				{ 
					transformer_args += option.value[i];
					parsed_transformer_args.push_back(option.value[i]);
					if (i<option.value.size()-1)
						transformer_args += " ";
				}
				if (name_to_transformers.find(transformer_name) != name_to_transformers.end())
				{
					CMeshTransformer::Ptr transformer = name_to_transformers[transformer_name];
					CFinfo << "\nTransforming mesh with " << transformer_name << " [" << transformer_args << "]" << CFendl;
					if (!dryrun) transformer->transform(mesh, parsed_transformer_args);
				}
				else
				{
					CFinfo << transformer_name << " does not exist" << CFendl;
				}
			}
			else if (option.string_key=="output")
			{
				boost_foreach(OptionValue value, option.value)
				{
					filesystem::path outputfile (value);
					const std::string ext = outputfile.extension();
					
					CMeshWriter::Ptr writer;
					if (!extensions_to_writers.count(ext))
					{
						Uint selection = 0;
						CFinfo << outputfile << " has ambiguous extension " << ext << CFendl;
						boost_foreach(const CMeshWriter::Ptr selectwriter , writers)
						CFinfo << "  [" << selection++ +1 << "]  " << selectwriter->get_format() << CFendl;
						CFinfo << "Select the correct writer: " << CFflush;
						std::cin >> selection;
						writer = writers[--selection];
					}
					else
					{
						Uint selection = 0;
						if (extensions_to_writers[ext].size()>1)
						{
							CFinfo << outputfile << " with extension " << ext << " has multiple writers: " << CFendl;
							boost_foreach(const CMeshWriter::Ptr selectwriter , extensions_to_writers[ext])
							CFinfo << "  [" << selection++ +1 << "]  " << selectwriter->get_format() << CFendl;
							CFinfo << "Select the correct writer: " << CFflush;
							std::cin >> selection;
							--selection;
						}
						writer = extensions_to_writers[ext][selection];
					}
					
					CFinfo << "\nWriting " << outputfile << " with " << writer->get_format() << CFendl;
					
					if (!dryrun) writer->write_from_to(mesh,outputfile);
				}
			}
		}
  }
  catch ( std::exception& ex )
  {
    CFerror << "Unhandled exception: " << ex.what() << CFendl;
  }
  catch ( ... )
  {
    CFerror << "Detected unknown exception" << CFendl;
  }
	
  Core::instance().terminate();
	
  return 0;
}
