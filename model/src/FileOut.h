#ifndef FILEOUT_CRX_H_
#define FILEOUT_CRX_H_

#include <fstream>
#include <string>

namespace crx {

class FileOut {

private:
    std::ofstream out;
    bool open;

public:

    std::ostream& operator<<(const std::string& val);
    std::ostream& operator<<(double val);
    std::ostream& operator<<(unsigned int val);
    std::ostream& operator<<(int val);

    FileOut(const std::string& filename);
    virtual ~FileOut();

    /**
     * Closes the recorder and writes any unwritten data to the file.
     */
    void close();

    void flush();

    bool isOpen() const {
        return open;
    }
};

} /* namespace mrsa */
#endif /* FILEOUTPUT_MRSA_H_ */
