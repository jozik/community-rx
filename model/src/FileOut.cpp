#include <boost/filesystem.hpp>
#include <sstream>
#include <iostream>

#include "chi_sim/file_utils.h"

#include "FileOut.h"

namespace fs = boost::filesystem;

namespace crx {

FileOut::FileOut(const std::string& filename) :
        out(), open(true) {

    std::string fname = chi_sim::unique_file_name(filename);
    fs::path filepath(fname);
    if (!fs::exists(filepath.parent_path()))
        fs::create_directories(filepath.parent_path());

    out.open(fname.c_str());
}

void FileOut::close() {
    if (open) {
        out.flush();
        out.close();
        open = false;
    }
}

void FileOut::flush() {
    if (open) {
        out.flush();
    }
}

FileOut::~FileOut() {
    close();
}

std::ostream& FileOut::operator<<(const std::string& val) {
    if (open) {
        out << val;
    }
    return out;
}

std::ostream& FileOut::operator<<(unsigned int val) {
    if (open) {
        out << val;
    }
    return out;
}

std::ostream& FileOut::operator<<(int val) {
    if (open) {
        out << val;
    }
    return out;
}

std::ostream& FileOut::operator<<(double val) {
    if (open) {
        out << val;
    }
    return out;
}

} /* namespace mrsa */
