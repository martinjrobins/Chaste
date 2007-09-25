#ifndef TESTARCHIVING_HPP_
#define TESTARCHIVING_HPP_

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <fstream>
#include <cxxtest/TestSuite.h>

#include "OutputFileHandler.hpp"

#include <boost/serialization/vector.hpp>
#include <boost/serialization/set.hpp>

#include <boost/serialization/export.hpp>

// see http://www.boost.org/libs/serialization/doc/index.html
class ParentClass;

class ChildClass    // this is an abstract of an AbstractCellCycleModel!
{
public:
    unsigned mTag;
    ParentClass *mpParent;
    ChildClass() : mTag(1)
    {
    }
    void SetParent(ParentClass *pParent)
    {
        mpParent = pParent;
    }
    
    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        // If Archive is an output archive, then & resolves to <<
        // If Archive is an input archive, then & resolves to >>
        //std::cout << "Child archiving\n" << std::flush;
        archive & mTag;
    }
};
//BOOST_CLASS_EXPORT(ChildClass)

class ParentClass   // this is an abstract of a MeinekeCryptCell.
{
public:
    unsigned mTag;
    ChildClass *mpChild;
    ParentClass(ChildClass *pChild) : mTag(0), mpChild(pChild)
    {
        mpChild->SetParent(this);
    }
    
    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        // If Archive is an output archive, then & resolves to <<
        // If Archive is an input archive, then & resolves to >>
        //std::cout << "Parent archiving\n" << std::flush;
        archive & mTag;
    }
};
BOOST_CLASS_EXPORT(ParentClass)

namespace boost
{
namespace serialization
{
/**
 * Allow us to not need a default constructor, by specifying how Boost should
 * instantiate a WntCellCycleModel instance.
 */
template<class Archive>
inline void save_construct_data(
    Archive & ar, const ParentClass * t, const unsigned int file_version)
{
    //std::cout << "Parent save construct\n" << std::flush;
    ar << t->mpChild;
}

/**
 * Allow us to not need a default constructor, by specifying how Boost should
 * instantiate a WntCellCycleModel instance.
 */
template<class Archive>
inline void load_construct_data(
    Archive & ar, ParentClass * t, const unsigned int file_version)
{
    // It doesn't actually matter what values we pass to our standard
    // constructor, provided they are valid parameter values, since the
    // state loaded later from the archive will overwrite their effect in
    // this case.
    // Invoke inplace constructor to initialize instance of my_class
    //std::cout << "Parent load construct\n" << std::flush;
    ChildClass* p_child;
    ar >> p_child;
    ::new(t)ParentClass(p_child);
}
}
} // namespace ...

class ClassOfSimpleVariables
{
private:
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        // If Archive is an output archive, then & resolves to <<
        // If Archive is an input archive, then & resolves to >>
        archive & mNumber;
        archive & mString;
        archive & mVectorOfDoubles; // include <boost/serialization/vector.hpp> for this
        archive & mVectorOfBools;
    }
    
    int mNumber;
    std::string mString;
    std::vector<double> mVectorOfDoubles;
    std::vector<bool> mVectorOfBools;
    
public:

    ClassOfSimpleVariables()
    {
        //Do nothing.  Used when loading into a pointer
    }
    ClassOfSimpleVariables(int initial,
                           std::string string,
                           std::vector<double> doubles,
                           std::vector<bool> bools)
            : mString(string),
            mVectorOfDoubles(doubles),
            mVectorOfBools(bools)
    {
        mNumber = initial;
    }
    
    int GetNumber() const
    {
        return mNumber;
    }
    
    std::string GetString()
    {
        return mString;
    }
    
    std::vector<double>& GetVectorOfDoubles()
    {
        return mVectorOfDoubles;
    }
    
    std::vector<bool>& GetVectorOfBools()
    {
        return mVectorOfBools;
    }
};

class TestArchiving : public CxxTest::TestSuite
{
public:
    void TestArchiveSimpleVars()
    {
        OutputFileHandler handler("archive",false);
        std::string archive_filename;
        archive_filename = handler.GetTestOutputDirectory() + "simple_vars.arch";
        
        // Create an ouput archive
        {
            std::ofstream ofs(archive_filename.c_str());
            boost::archive::text_oarchive output_arch(ofs);
            
            std::vector<double> doubles(3);
            doubles[0] = 1.1;
            doubles[1] = 1.2;
            doubles[2] = 1.3;
            
            std::vector<bool> bools(2);
            bools[0] = true;
            bools[1] = true;
            
            ClassOfSimpleVariables i(42,"hello",doubles,bools);
            
            // cast to const.
            output_arch << static_cast<const ClassOfSimpleVariables&>(i);
        }
        
        {
            // Create an input archive
            std::ifstream ifs(archive_filename.c_str(), std::ios::binary);
            boost::archive::text_iarchive input_arch(ifs);
            
            std::vector<double> bad_doubles(1);
            bad_doubles[0] = 10.3;
            
            std::vector<bool> bad_bools(1);
            bad_bools[0] = false;
            
            ClassOfSimpleVariables j(0,"bye",bad_doubles,bad_bools);
            
            // read the archive
            input_arch >> j ;
            
            // Check that the values
            TS_ASSERT_EQUALS(j.GetNumber(),42);
            TS_ASSERT_EQUALS(j.GetString(),"hello");
            TS_ASSERT_EQUALS(j.GetVectorOfDoubles().size(),3u);
            TS_ASSERT_EQUALS(j.GetVectorOfBools().size(),2u);
            
            TS_ASSERT_DELTA(j.GetVectorOfDoubles()[0],1.1,1e-12);
            TS_ASSERT_DELTA(j.GetVectorOfDoubles()[1],1.2,1e-12);
            TS_ASSERT_DELTA(j.GetVectorOfDoubles()[2],1.3,1e-12);
            
            TS_ASSERT_EQUALS(j.GetVectorOfBools()[0],true);
            TS_ASSERT_EQUALS(j.GetVectorOfBools()[1],true);
        }
    }
    
    void TestArchivingLinkedChildAndParent() throw (Exception)
    {
        OutputFileHandler handler("archive",false);
        std::string archive_filename;
        archive_filename = handler.GetTestOutputDirectory() + "linked_classes.arch";
        
        // Save
        {
            // Create an ouput archive
            std::ofstream ofs(archive_filename.c_str());
            boost::archive::text_oarchive output_arch(ofs);
            
            ChildClass* p_child = new ChildClass;
            ParentClass* p_parent = new ParentClass(p_child);
            
            p_child->mTag = 11;
            p_parent->mTag = 10;
            
            ParentClass* const  p_parent_for_archiving = p_parent;
            //ChildClass* const p_child_for_archiving = p_child;
            
            //output_arch << p_child_for_archiving;
            output_arch << p_parent_for_archiving;
            
            delete p_child;
            delete p_parent;
        }

        // Load
        {
            // Create an input archive
            std::ifstream ifs(archive_filename.c_str(), std::ios::binary);
            boost::archive::text_iarchive input_arch(ifs);
            
            //ChildClass* p_child;
            ParentClass* p_parent;
            
            input_arch >> p_parent;
            
            TS_ASSERT_EQUALS(p_parent->mTag, 10u);
            TS_ASSERT_EQUALS(p_parent->mpChild->mTag, 11u);
            TS_ASSERT_EQUALS(p_parent->mpChild->mpParent, p_parent);
            
            delete p_parent->mpChild;
            delete p_parent;
        }
    }
    
    
    void TestArchivingSetOfPointers() throw (Exception)
    {
        std::vector<double> doubles;
        std::vector<bool> bools;
        
        OutputFileHandler handler("archive",false);
        std::string archive_filename;
        archive_filename = handler.GetTestOutputDirectory() + "pointer_set.arch";
        
        // Save
        {
            // Create aClassOfSimpleVariablesn ouput archive
            std::ofstream ofs(archive_filename.c_str());
            boost::archive::text_oarchive output_arch(ofs);
            
            std::set<ClassOfSimpleVariables*> a_set;
            ClassOfSimpleVariables* p_one = new ClassOfSimpleVariables(42,"hello", doubles,bools);
            a_set.insert(p_one);
            ClassOfSimpleVariables* p_two = new ClassOfSimpleVariables(256,"goodbye", doubles,bools);
        
            a_set.insert(p_two);
            
            
            //output_arch << p_child_for_archiving;
            output_arch << static_cast<const std::set<ClassOfSimpleVariables*>&>(a_set);            
        }
        //Load
        {
            std::set<ClassOfSimpleVariables*> a_set;
            // Create an input archive
            std::ifstream ifs(archive_filename.c_str(), std::ios::binary);
            boost::archive::text_iarchive input_arch(ifs);
            
            
            TS_ASSERT_EQUALS(a_set.size(), 0u);
            input_arch >> a_set;
            TS_ASSERT_EQUALS(a_set.size(), 2u);
            for (std::set<ClassOfSimpleVariables*>::iterator it = a_set.begin();
                it!=a_set.end(); ++it)
            {
                   ClassOfSimpleVariables* p_class = *(it);
                   if (p_class->GetNumber()==42)
                   {
                        TS_ASSERT_EQUALS(p_class->GetNumber(), 42);
                        TS_ASSERT_EQUALS(p_class->GetString(),"hello");
                   }
                   else
                   {
                        TS_ASSERT_EQUALS(p_class->GetNumber(), 256);
                        TS_ASSERT_EQUALS(p_class->GetString(),"goodbye");
                   }
            }
        }
    }
};


#endif /*TESTARCHIVING_HPP_*/
