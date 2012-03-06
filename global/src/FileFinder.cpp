/*

Copyright (c) 2005-2012, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "FileFinder.hpp"

#include <cassert>

#include "BoostFilesystem.hpp"
#include "ChasteBuildRoot.hpp"
#include "Exception.hpp"
#include "GetCurrentWorkingDirectory.hpp"
#include "OutputFileHandler.hpp"

bool FileFinder::msFaking = false;

RelativeTo::Value FileFinder::msFakeWhat = RelativeTo::Absolute;

std::string FileFinder::msFakePath = "";


FileFinder::FileFinder()
    : mAbsPath("UNSET!")
{
}

FileFinder::FileFinder(const std::string& rRelativePath, RelativeTo::Value relativeTo)
{
    SetPath(rRelativePath, relativeTo);
}

FileFinder::FileFinder(const std::string& rLeafName, const FileFinder& rParentOrSibling)
{
    SetPath(rLeafName, rParentOrSibling);
}

FileFinder::FileFinder(const fs::path& rPath)
{
    SetPath(fs::complete(rPath).string(), RelativeTo::Absolute);
}

void FileFinder::SetPath(const std::string& rRelativePath, RelativeTo::Value relativeTo)
{
    switch (relativeTo)
    {
        case RelativeTo::ChasteSourceRoot:
            mAbsPath = ChasteBuildRootDir() + rRelativePath;
            break;

        case RelativeTo::ChasteTestOutput:
            mAbsPath = OutputFileHandler::GetChasteTestOutputDirectory() + rRelativePath;
            break;

        case RelativeTo::CWD:
            mAbsPath = GetCurrentWorkingDirectory() + "/" + rRelativePath;
            break;

        case RelativeTo::Absolute:
            mAbsPath = rRelativePath;
            break;

        case RelativeTo::AbsoluteOrCwd:
            if (FileFinder::IsAbsolutePath(rRelativePath))
            {
                mAbsPath = rRelativePath;
            }
            else
            {
                mAbsPath = GetCurrentWorkingDirectory() + "/" + rRelativePath;
            }
            break;

        default:
            // Getting here is impossible
            NEVER_REACHED;
            break;
    }

    if (msFaking && msFakeWhat == relativeTo)
    {
        // Fake the resulting path
        mAbsPath = msFakePath + "/" + rRelativePath;
    }

    // Remove any trailing /
    std::string::iterator it = mAbsPath.end();
    while (it != mAbsPath.begin() && *(--it) == '/')
    {
        // Iterator was decremented in the while test
    }
    if (it != mAbsPath.end() && (++it) != mAbsPath.end())
    {
        mAbsPath.erase(it, mAbsPath.end());
    }
}

void FileFinder::SetPath(const std::string& rLeafName, const FileFinder& rParentOrSibling)
{
    if (!rParentOrSibling.Exists())
    {
        EXCEPTION("Reference path '" << rParentOrSibling.GetAbsolutePath() << "' does not exist.");
    }
    if (rParentOrSibling.IsDir())
    {
        SetPath(rParentOrSibling.GetAbsolutePath() + rLeafName, RelativeTo::Absolute);
    }
    else
    {
        SetPath(rParentOrSibling.GetParent().GetAbsolutePath() + rLeafName, RelativeTo::Absolute);
    }
}

bool FileFinder::Exists() const
{
    return fs::exists(mAbsPath);
}

bool FileFinder::IsFile() const
{
    return fs::is_regular(mAbsPath);
}

bool FileFinder::IsDir() const
{
    return fs::is_directory(mAbsPath);
}

std::string FileFinder::GetAbsolutePath() const
{
    if (IsDir())
    {
        return mAbsPath + '/';
    }
    return mAbsPath;
}

bool FileFinder::IsNewerThan(const FileFinder& rOtherEntity) const
{
    assert(Exists());
    assert(rOtherEntity.Exists());
    return fs::last_write_time(mAbsPath) > fs::last_write_time(rOtherEntity.mAbsPath);
}

std::string FileFinder::GetLeafName() const
{
    return fs::path(mAbsPath).leaf();
}

std::string FileFinder::GetLeafNameNoExtension() const
{
    return fs::basename(mAbsPath);
}

FileFinder FileFinder::GetParent() const
{
    fs::path our_path(mAbsPath);
    EXCEPT_IF_NOT(our_path.has_branch_path());
    return FileFinder(our_path.branch_path().string(),
                      RelativeTo::Absolute);
}


/**
 * Recursively remove the given path.
 * @param rPath
 */
void RemoveAll(const fs::path& rPath)
{
    // First recursively remove any children
    if (fs::is_directory(rPath))
    {
        fs::directory_iterator end_iter;
        for (fs::directory_iterator dir_iter(rPath); dir_iter != end_iter; ++dir_iter)
        {
            RemoveAll(dir_iter->path());
        }
    }
    // Now remove the item itself
    fs::remove(rPath);
}

void FileFinder::Remove(bool force) const
{
    // Test for bad paths
    const std::string test_output(OutputFileHandler::GetChasteTestOutputDirectory());
    if (GetAbsolutePath().substr(0, test_output.length()) != test_output)
    {
        EXCEPTION("Cannot remove location '" << mAbsPath
                  << "' as it is not located within the Chaste test output folder ("
                  << test_output << ").");
    }
    if (mAbsPath.find("..") != std::string::npos)
    {
        EXCEPTION("Cannot remove location '" << mAbsPath
                  << "' as it contains a dangerous path component.");
    }
    if (!force)
    {
        fs::path sig_file(mAbsPath);
        if (IsFile())
        {
            // We need to look for the signature file in the parent folder
            sig_file.remove_leaf();
        }
        sig_file /= OutputFileHandler::SIG_FILE_NAME;
        if (!fs::exists(sig_file))
        {
            EXCEPTION("Cannot remove location '" << mAbsPath << "' because the signature file '"
                      << OutputFileHandler::SIG_FILE_NAME << "' is not present.");
        }
    }
    // Do the removal
    RemoveAll(mAbsPath);
}


bool FileFinder::IsAbsolutePath(const std::string& rPath)
{
    return fs::path(rPath).is_complete();
}

void FileFinder::ReplaceSpacesWithUnderscores(std::string& rPath)
{
    for (std::string::iterator it = rPath.begin(); it != rPath.end(); ++it)
    {
        if (*it == ' ')
        {
            *it = '_';
        }
    }
}

void FileFinder::ReplaceUnderscoresWithSpaces(std::string& rPath)
{
    for (std::string::iterator it = rPath.begin(); it != rPath.end(); ++it)
    {
        if (*it == '_')
        {
            *it = ' ';
        }
    }
}

void FileFinder::FakePath(RelativeTo::Value fakeWhat, const std::string& rFakePath)
{
    msFakeWhat = fakeWhat;
    msFakePath = rFakePath;
    msFaking = true;
}

void FileFinder::StopFaking()
{
    msFaking = false;
}
