#ifndef HDF5DATAWRITER_HPP_
#define HDF5DATAWRITER_HPP_

#include <hdf5.h>
#include <petscvec.h>
#include <cassert>
#include "Exception.hpp"
#include "AbstractDataWriter.hpp"
#include "DataWriterVariable.hpp"
#include "OutputFileHandler.hpp"

class HDF5DataWriter// : public AbstractDataWriter
{
protected:
    std::string mDirectory; /**< Directory output files will be stored in. */
    std::string mBaseName; /**< The base name for the output data files. */
    bool mCleanDirectory;   /**< Whether to wipe the output directory */
    bool mIsInDefineMode; /**< Is the DataWriter in define mode or not */
    bool mIsFixedDimensionSet; /**< Is the fixed dimension set */
    bool mIsUnlimitedDimensionSet; /**< Is the unlimited dimension set */
    long mUnlimitedDimensionPosition; /**< The position along the unlimited dimension that writing of variables will take place*/
    long mFixedDimensionSize; /**< The size of the fixed dimension */    

    std::string mFixedDimensionName; /**< The name of the fixed dimension */
    std::string mFixedDimensionUnits; /**< The units of the fixed dimension */

    std::vector<DataWriterVariable> mVariables; /**< The data variables */
    
    static const int FIXED_DIMENSION_VAR_ID = -1; /**< id of fixed dimension variable */
        
    void CheckVariableName(std::string name); /**< Check variable name is allowed, i.e. contains only alphanumeric & _, and isn't blank */
    void CheckUnitsName(std::string name); /**< Check units name is allowed, i.e. contains only alphanumeric & _ */

    hid_t mFileId;
    hid_t mDsetId;
    
    long mCurrentTimeStep;
    
    const static unsigned DATASET_DIMS=3;
    hsize_t mDatasetDims[DATASET_DIMS]; 
    
public:
    HDF5DataWriter(std::string directory, std::string baseName, bool cleanDirectory=true);
    virtual ~HDF5DataWriter();
    int DefineFixedDimension(std::string dimensionName, std::string dimensionUnits, long dimensionSize);
    int DefineUnlimitedDimension(std::string variableName, std::string variableUnits);
    void AdvanceAlongUnlimitedDimension();
    int DefineVariable(std::string variableName, std::string variableUnits);
    virtual void EndDefineMode();
    
    void PutVector(int variableID, Vec petscVector);
    void PutStripedVector(int firstVariableID, int secondVariableID, Vec petscVector);
    
    void Close();
};

#endif /*HDF5DATAWRITER_HPP_*/
