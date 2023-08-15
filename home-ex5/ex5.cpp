#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <cmath>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <queue>

using namespace std;

#define DISK_SIZE 512

// Function to convert decimal to binary char
char decToBinary(int n) {
    return static_cast<char>(n);
}

// #define SYS_CALL
// ============================================================================
class fsInode {
    int fileSize;
    int block_in_use;

    int directBlock1;
    int directBlock2;
    int directBlock3;

    int singleInDirect;
    int doubleInDirect;
    int block_size;


public:
    fsInode(int _block_size) {
        fileSize = 0;
        block_in_use = 0;
        block_size = _block_size;
        directBlock1 = -1;
        directBlock2 = -1;
        directBlock3 = -1;
        singleInDirect = -1;
        doubleInDirect = -1;

    }

    // YOUR CODE......

};

// ============================================================================
class FileDescriptor {
    pair<string, fsInode*> file;
    bool inUse;

public:
    FileDescriptor(string FileName, fsInode* fsi) {
        file.first = FileName;
        file.second = fsi;
        inUse = true;
    }

    string getFileName() {
        return file.first;
    }

    fsInode* getInode() {
        return file.second;
    }

    int getFileSize() {
        return 0;
    }

    bool isInUse() {
        return (inUse);
    }

    void setInUse(bool _inUse) {
        inUse = _inUse;
    }

    FileDescriptor& operator= (const FileDescriptor& fd) {
        this->file.first = fd.file.first;
        this->file.second = fd.file.second;
        this->inUse = fd.inUse;
    }

    bool operator==(const FileDescriptor& other) {
        return this->file.first == other.file.first && this->file.second == other.file.second && this->inUse == other.inUse;
    }

    bool operator!=(const FileDescriptor& other) {
        return !(*this == other);
    }
};

FileDescriptor NULL_FD("", nullptr);

ostream& operator<<(ostream& os, const vector<char>& dt) {
    for (auto it = dt.begin(); it != dt.end(); it++) {
        os << *it;
    }
    return os;
}

ostream& operator<<(ostream& os, vector<FileDescriptor>& dt) {
    int i = 0;
    for (auto it = dt.begin(); it != dt.end(); it++) {
        os << "index: " << i << ": FileName: " << it->getFileName() << " , isInUse: "
            << it->isInUse() << " file Size: " << it->getFileSize() << endl;
        i++;
    }
    return os;
}

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk {
    FILE* sim_disk_fd;
    bool is_formated;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the 
    //             first block is occupied. 
    int BitVectorSize;
    bool* BitVector;

    int block_size;

    // Unix directories are lists of association structures, 
    // each of which contains one filename and one inode number.
    map<string, fsInode*>  MainDir;

    // OpenFileDescriptors --  when you open a file, 
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor. 
    vector< FileDescriptor > OpenFileDescriptors;
    queue<int> existingFdsNotInUse;

public:
// ------------------------------------------------------------------------
    fsDisk() {
        sim_disk_fd = fopen(DISK_SIM_FILE, "r+");
        if (!sim_disk_fd) {
            sim_disk_fd = fopen(DISK_SIM_FILE, "w+");
        }
        assert(sim_disk_fd);

        char data[DISK_SIZE] = { '\0' };
        writeChunckToDisk(data, 0, DISK_SIZE);
    }

    ~fsDisk() {
        if (this->is_formated) {
            delete[] this->BitVector;
        }
        fclose(sim_disk_fd);
    }

    // ------------------------------------------------------------------------
    void listAll() {
        int i = 0;
        cout << this->OpenFileDescriptors;
        cout << "Disk content: '";
        cout << readVectorFromDisk(0, DISK_SIZE);
        cout << "'" << endl;
    }

    // ------------------------------------------------------------------------
    void fsFormat(int blockSize = 4, int direct_Enteris_ = 3) {
        direct_Enteris_ = 3;

        this->block_size = block_size;

        if (this->is_formated) {
            delete[] this->BitVector;
        }

        this->BitVectorSize = DISK_SIZE / blockSize;
        this->BitVector = new bool[this->BitVectorSize];

        this->is_formated = true;
    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName) {
        if (!this->isDiskFormatted())
            return -1;

        if (MainDir.find(fileName) != MainDir.end()) {
            cout << "file already exists" << endl;
            return -1;
        }

        fsInode* fsi = new fsInode(this->block_size);
        MainDir[fileName] = fsi;

        return OpenFile(fileName);
    }

    // ------------------------------------------------------------------------
    int OpenFile(string fileName) {
        if (!this->isDiskFormatted())
            return -1;

        if (MainDir.find(fileName) != MainDir.end()) {
            cout << "file already exists" << endl;
            return -1;
        }

        for (int i = 0; i < OpenFileDescriptors.size(); i++) {
            if (fileName == OpenFileDescriptors[i].getFileName()) {
                cout << "file already open" << endl;
                return i;
            }
        }

        FileDescriptor newFd(fileName, MainDir[fileName]);
        int newFdNum = 0;
        if (existingFdsNotInUse.size() > 0) {
            newFdNum = existingFdsNotInUse.front();
            existingFdsNotInUse.pop();
            OpenFileDescriptors[newFdNum] = newFd;
        }
        else {
            newFdNum = OpenFileDescriptors.size();
            OpenFileDescriptors[newFdNum] = newFd;
        }
        return newFdNum;
    }

    // ------------------------------------------------------------------------
    string CloseFile(int fd) {
        if (!this->isDiskFormatted() || !this->doesFileDescriptorExist(fd))
            return "-1";

        string fileName = OpenFileDescriptors[fd].getFileName();
        OpenFileDescriptors[fd] = NULL_FD;
        existingFdsNotInUse.push(fd);
        return fileName;
    }
    // ------------------------------------------------------------------------
    int WriteToFile(int fd, char* buf, int len) {
        if (!this->isDiskFormatted() || !this->doesFileDescriptorExist(fd))
            return -1;
        return 0;
    }
    // ------------------------------------------------------------------------
    int DelFile(string FileName) {
        if (!this->isDiskFormatted())
            return -1;

        if (findFileDescriptorByName(FileName) >= 0) {
            cout << "file is open and in use, therefore it cannot be deleted" << endl;
            return -1;
        }
    }
    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char* buf, int len) {
        if (!this->isDiskFormatted() || !this->doesFileDescriptorExist(fd))
            return -1;
        return 0;
    }

    // ------------------------------------------------------------------------
    int getFileSize(int fd) {
        if (!this->isDiskFormatted() || !this->doesFileDescriptorExist(fd))
            return -1;

        return OpenFileDescriptors[fd].getFileSize();
    }

    // ------------------------------------------------------------------------
    int CopyFile(string srcFileName, string destFileName) {
        if (!this->isDiskFormatted())
            return -1;
        return 0;
    }

    // ------------------------------------------------------------------------
    int RenameFile(string oldFileName, string newFileName) {
        if (!this->isDiskFormatted())
            return -1;

        return 0;
    }

private:
    bool isDiskFormatted() {
        if (this->is_formated) {
            return true;
        }
        cout << "Disk not formatted" << endl;
        return false;
    }

    int findFileDescriptorByName(string fileName) {
        for (int i = 0; i < OpenFileDescriptors.size(); i++) {
            if (OpenFileDescriptors[i].getFileName() == fileName) {
                return i;
            }
        }
        return -1;
    }

    bool doesFileDescriptorExist(int fd) {
        if (!doesFileDescriptorExistNoMessage(fd)) {
            cout << "no such file descriptor" << endl;
            return false;
        }
        return true;
    }

    bool doesFileDescriptorExistNoMessage(int fd) {
        return fd < 0 || fd >= OpenFileDescriptors.size() || OpenFileDescriptors[fd] == NULL_FD;
    }

    vector<char> readBlockFromDisk(int blockIndex) {
        int index = blockIndex * block_size;
        int len = block_size;
        return readVectorFromDisk(index, len);
    }

    vector<char> readVectorFromDisk(int index, int len) {
        vector<char> res;
        char data = '\0';
        for (int i = 0; i < len; i++) {
            data = readCharFromDisk(index + i);
            res.push_back(data);
        }
        return res;
    }

    char readCharFromDisk(int index) {
        char res = '\0';
        seekIndexOnDisk(index);
        int ret_val = fread(&res, 1, 1, sim_disk_fd);
        assert(ret_val == 1);
        return res;
    }

    void writeBlockToDisk(vector<char> data, int blockIndex) {
        int index = blockIndex * this->block_size;
        int len = this->block_size;
        writeVectorToDisk(data, index, len);
    }

    void writeVectorToDisk(vector<char> data, int index, int len) {
        for (int i = 0; i < len; i++) {
            writeToDiskAtIndexNoFlush(data[i], index + i);
        }
        fflush(sim_disk_fd);
    }

    void writeChunckToDisk(char* data, int index, int len) {
        for (int i = 0; i < len; i++) {
            writeToDiskAtIndexNoFlush(data[i], index + i);
        }
        fflush(sim_disk_fd);
    }

    void writeToDiskAtIndex(char data, int index) {
        writeToDiskAtIndexNoFlush(data, index);
        fflush(sim_disk_fd);
    }

    void writeToDiskAtIndexNoFlush(char data, int index) {
        seekIndexOnDisk(index);
        int ret_val = fwrite(&data, 1, 1, sim_disk_fd);
        assert(ret_val == 1);
    }

    void seekIndexOnDisk(int index) {
        int ret_val = fseek(sim_disk_fd, index, SEEK_SET);
        assert(ret_val == 0);
    }
};

int main() {
    int blockSize;
    int direct_entries;
    string fileName;
    string fileName2;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk* fs = new fsDisk();
    int cmd_;
    while (1) {
        cin >> cmd_;
        switch (cmd_) {
        case 0:   // exit
            delete fs;
            exit(0);
            break;

        case 1:  // list-file
            fs->listAll();
            break;

        case 2:    // format
            cin >> blockSize;
            cin >> direct_entries;
            fs->fsFormat(blockSize, direct_entries);
            break;

        case 3:    // creat-file
            cin >> fileName;
            _fd = fs->CreateFile(fileName);
            cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 4:  // open-file
            cin >> fileName;
            _fd = fs->OpenFile(fileName);
            cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 5:  // close-file
            cin >> _fd;
            fileName = fs->CloseFile(_fd);
            cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 6:   // write-file
            cin >> _fd;
            cin >> str_to_write;
            fs->WriteToFile(_fd, str_to_write, strlen(str_to_write));
            break;

        case 7:    // read-file
            cin >> _fd;
            cin >> size_to_read;
            fs->ReadFromFile(_fd, str_to_read, size_to_read);
            cout << "ReadFromFile: " << str_to_read << endl;
            break;

        case 8:   // delete file 
            cin >> fileName;
            _fd = fs->DelFile(fileName);
            cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 9:   // copy file
            cin >> fileName;
            cin >> fileName2;
            fs->CopyFile(fileName, fileName2);
            break;

        case 10:  // rename file
            cin >> fileName;
            cin >> fileName2;
            fs->RenameFile(fileName, fileName2);
            break;

        default:
            break;
        }
    }

}