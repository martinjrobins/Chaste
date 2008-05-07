/*

Copyright (C) University of Oxford, 2008

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Chaste is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Chaste is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details. The offer of Chaste under the terms of the
License is subject to the License being interpreted in accordance with
English Law and subject to any action against the University of Oxford
being under the jurisdiction of the English Courts.

You should have received a copy of the GNU Lesser General Public License
along with Chaste. If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef SIMPLEDATAWRITER_HPP_
#define SIMPLEDATAWRITER_HPP_

#include <fstream>
#include "OutputFileHandler.hpp"
#include "Exception.hpp"

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>

/**
 *  A basic data writer that is easier to use than ColumnDataWriter but has less
 *  functionality. NOTE: this is not an efficient writer.
 * 
 *  This class does not writer header lines so is ideal for immediately reading
 *  with MATLAB or Gnuplot.
 */
class SimpleDataWriter
{

public:
    /**
     *  Write the provided data out to the given file in columns
     *  
     *  @directory The directory, relative to TEST_OUTPUT
     *  @fileName  The full file name (no format will be apended)
     *  @data      The data. data[0] will written as the first column, data[1] the
     *   second, and so on. An exception is thrown if they are not the same size
     *  @cleanDirectory Whether to clean the directory (defaults to true)
     */
    SimpleDataWriter(std::string directory, std::string fileName, std::vector<std::vector<double> > data, bool cleanDirectory=true)
    {
        if(data.size()==0)
        {
            EXCEPTION("Data vector is empty");
        }
        
        for(unsigned i=0; i<data.size(); i++)
        {
            if(data[i].size()!=data[0].size())
            {
                EXCEPTION("Data vector sizes are not all equal");
            }
        }

        OutputFileHandler output_file_handler(directory, cleanDirectory);
        out_stream p_file = output_file_handler.OpenOutputFile(fileName);
                
        for(unsigned j=0; j<data[0].size(); j++)
        {
            for(unsigned i=0; i<data.size(); i++)
            {
                {
                    (*p_file) << data[i][j] << "\t";
                }
            }
            (*p_file) << "\n";
        }
        
        p_file->close();
    }                    

   

    /**
     *  Write the provided data out to the given file in 2 columns
     *  
     *  @directory The directory, relative to TEST_OUTPUT
     *  @fileName  The full file name (no format will be apended)
     *  @t         The first column of data
     *  @x         The second column of data. An exception is thrown if the size 
     *             of x is not the same as the size of t. 
     *  @cleanDirectory Whether to clean the directory (defaults to true)
     */
    SimpleDataWriter(std::string directory, std::string fileName, std::vector<double> t, std::vector<double> x, bool cleanDirectory=true)
    {
        std::vector<std::vector<double> > data;
        data.push_back(t);
        data.push_back(x);
        SimpleDataWriter(directory, fileName, data, cleanDirectory);
    }                    

    /**
     *  Write the provided data out to the given file in one column
     *  
     *  @directory The directory, relative to TEST_OUTPUT
     *  @fileName  The full file name (no format will be apended)
     *  @data      A std::vec of data
     *  @cleanDirectory Whether to clean the directory (defaults to true)
     */
    SimpleDataWriter(std::string directory, std::string fileName, std::vector<double> data, bool cleanDirectory=true)
    {
        std::vector<std::vector<double> > data_;
        data_.push_back(data);
        SimpleDataWriter(directory, fileName, data_, cleanDirectory);
    }                    

};
#endif /*SIMPLEDATAWRITER_HPP_*/
