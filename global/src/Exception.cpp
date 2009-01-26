/*

Copyright (C) University of Oxford, 2005-2009

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


#include "Exception.hpp"
#include "LogFile.hpp"

Exception::Exception(std::string message,
                     std::string filename, const unsigned rLineNumber)
{
    std::stringstream line_number;
    line_number << rLineNumber;

    mMessage = std::string("\nChaste error: ") + filename + ":"  + line_number.str()  + ": " + message;

    ///// The following would write the error message to the log file, if one exists.
    ///// It's commented out because you end up with 100s of errors in the log from
    ///// element nodes being swapped around when the mesh is read in
    // no way of saying here whether this will be a fatal error, but write
    // it to the log file (if one exists) in case it is.
    // std::string log_file_message = "Exception occurred (although possibly handled), error message:\n" + message;
    // LOG(1, log_file_message);
}


std::string Exception::GetMessage() const
{
    return mMessage;
}
