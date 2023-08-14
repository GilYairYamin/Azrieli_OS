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

    int getDirectBlockNumber(int i) {
        switch (i) {
        case 0:
            return this->directBlock1;
        case 1:
            return this->directBlock2;
        case 2:
            return this->directBlock3;
        }
        return -1;
    }

    void setDirectBlockNumber(int val, int i) {
        switch (i) {
        case 0:
            this->directBlock1 = val;
        case 1:
            this->directBlock2 = val;
        case 2:
            this->directBlock3 = val;
        }
    }

    int getSingleIndirectNumber() {
        return this->singleInDirect;
    }

    void setSingleIndirectNumber(int val) {
        this->singleInDirect = val;
    }

    int getDoubleIndirectNumber() {
        return this->doubleInDirect;
    }

    void setDoubleIndirectNumber(int val) {
        this->doubleInDirect = val;
    }

    int getFileSize() {
        return this->fileSize;
    }

    void setFileSize(int newSize) {
        this->fileSize = newSize;
    }

    int getBlocksInUse() {
        return this->block_in_use;
    }
};

// ============================================================================
class FileDescriptor {
    pair<string, fsInode*> file;
    bool inUse;

public:
    FileDescriptor(string FileName, fsInode* fsi) {
        this->file.first = FileName;
        this->file.second = fsi;
        this->inUse = (fsi != nullptr);
    }

    string getFileName() {
        return this->file.first;
    }

    fsInode* getInode() {
        return this->file.second;
    }

    int getFileSize() {
        if (this->file.second == nullptr)
            return 0;
        return this->file.second->getFileSize();
    }

    bool isInUse() {
        return (this->inUse);
    }

    void setInUse(bool _inUse) {
        this->inUse = _inUse;
    }

    bool operator==(const FileDescriptor& other) {
        if (this->file.first == other.file.first && this->file.second == other.file.second)
            return true;
        return false;
    }

    bool operator!=(const FileDescriptor& other) {
        return !this->operator==(other);
    }
};

FileDescriptor NULL_FD("", nullptr);

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"

// vec toString
ostream& operator<<(ostream& os, const vector<int>& vec) {
    os << "[";
    if (vec.size() == 0) {
        os << "]";
        return os;
    }
    for (auto it = vec.begin(); it != vec.end(); it++) {
        if (it != vec.begin()) {
            os << ",";
        }
        os << *it;
    }
    os << "]";
    return os;
}

// ============================================================================
class fsDisk {
    FILE* sim_disk_fd;

    bool is_formated;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the 
    //             first block is occupied. 
    int BitVectorSize;
    int* BitVector;

    int currentBlockSize;

    // Unix directories are lists of association structures, 
    // each of which contains one filename and one inode number.
    map<string, fsInode*>  MainDir;

    // OpenFileDescriptors --  when you open a file, 
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor. 
    vector< FileDescriptor > OpenFileDescriptors;

    queue<int> fdNullPointers;

    void deallocMemory(vector<int> arr) {
        for (int i = 0; i < arr.size(); i++) {
            BitVector[arr[i]] = 0;
        }
    }

    vector<int> allocMemory(int num) {
        vector<int> res;
        int count = 0;
        for (int i = 0; count < num && i < BitVectorSize; i++) {
            if (BitVector[i] == 0) {
                res.push_back(i);
                count++;
                BitVector[i] = 1;
            }
        }

        if (res.size() < num) {
            this->deallocMemory(res);
            res.clear();
        }

        return res;
    }

    vector<char> readDataByBlock(int blockNumber) {
        vector<char> res;
        if (blockNumber < 0 || blockNumber >= this->BitVectorSize) {
            return res;
        }

        assert(sim_disk_fd);
        char buffy[DISK_SIZE] = { 0 };
        int index = blockNumber * this->currentBlockSize;

        int ret_val = fseek(sim_disk_fd, index, SEEK_SET);
        ret_val = fread(buffy, 1, this->currentBlockSize, sim_disk_fd);
        assert(ret_val == 1);

        for (int i = 0; i < this->currentBlockSize; i++) {
            res.push_back(buffy[i]);
        }
        return res;
    }

    char readDataByBlockAndOffset(int blockNumber, int offset = 0) {
        if (offset < 0 || offset >= this->currentBlockSize) {
            return 0;
        }
        vector<char> blockData = this->readDataByBlock(blockNumber);
        if (blockData.size() <= offset) {
            return 0;
        }
        return blockData[offset];
    }

    char readDataByByteIndex(int index) {
        int blockNumber = index / this->currentBlockSize;
        int offset = index % this->currentBlockSize;
        return this->readDataByBlockAndOffset(blockNumber, offset);
    }

    void writeDataByBlock(vector<char> newBlock, int blockNumber) {
        if (blockNumber < 0 || blockNumber >= this->BitVectorSize) {
            return;
        }

        char buffy[DISK_SIZE] = { 0 };
        for (int i = 0; i < this->currentBlockSize && i < newBlock.size(); i++) {
            buffy[i] = newBlock[i];
        }

        assert(sim_disk_fd);
        int index = blockNumber * this->currentBlockSize;
        int ret_val = fseek(sim_disk_fd, index, SEEK_SET);
        ret_val = fwrite(buffy, 1, this->currentBlockSize, sim_disk_fd);
        assert(ret_val == 1);
        fflush(sim_disk_fd);
        return;
    }

    void writeDataByBlockAndOffset(char data, int blockNumber, int offset = 0) {
        if (blockNumber < 0 || blockNumber >= this->BitVectorSize || offset < 0 || offset >= this->currentBlockSize) {
            return;
        }
        vector<char> newData = this->readDataByBlock(blockNumber);
        newData[offset] = data;
        this->writeDataByBlock(newData, blockNumber);
    }

    void writeDataByByteIndex(char data, int index) {
        int blockNumber = index / this->currentBlockSize;
        int offset = index % this->currentBlockSize;
        this->writeDataByBlockAndOffset(data, blockNumber, offset);
    }

    bool isDiskFormatted() {
        if (!this->is_formated) {
            cout << "disk not formatted yet" << endl;
            return false;
        }
        return true;
    }

    int getMaxDataBlocksAllocatableForFile() {
        return 3 + this->currentBlockSize * (1 + this->currentBlockSize);
    }

    int getMaxFileSize() {
        return this->currentBlockSize * this->getMaxDataBlocksAllocatableForFile();
    }

    int getAmountOfBlocksNeededForFileSize(int fileSize) {
        if (fileSize > this->getMaxFileSize())
            return -1;
        int directAmount = 3;
        int maxIndirect = 2;

        int neededDataBlocks = fileSize / this->currentBlockSize;
        int res = neededDataBlocks;

        if (neededDataBlocks <= directAmount)
            return res;

        neededDataBlocks -= directAmount;
        res += 1;

        if (neededDataBlocks <= this->currentBlockSize)
            return res;

        neededDataBlocks -= this->currentBlockSize;
        res += 1 + neededDataBlocks / this->currentBlockSize;

        return res;
    }

public:
// ------------------------------------------------------------------------
    fsDisk() {
        sim_disk_fd = fopen(DISK_SIM_FILE, "a+");
        this->fillBlanksDisk();
        this->is_formated = false;
    }

    void fillBlanksDisk() {
        assert(sim_disk_fd);
        char buffy[DISK_SIZE] = { '\0' };

        int ret_val = fseek(sim_disk_fd, 0, SEEK_SET);
        ret_val = fwrite(buffy, 1, DISK_SIZE, sim_disk_fd);
        assert(ret_val == DISK_SIZE);

        fflush(sim_disk_fd);
    }

    // ------------------------------------------------------------------------
    void listAll() {
        int i = 0;
        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it) {
            cout << "index: " << i << ": FileName: " << it->getFileName() << " , isInUse: "
                << it->isInUse() << " file Size: " << it->getFileSize() << endl;
            i++;
        }

        char bufy;
        cout << "Disk content: '";
        for (i = 0; i < DISK_SIZE; i++) {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fread(&bufy, 1, 1, sim_disk_fd);
            cout << bufy;
        }
        cout << "'" << endl;
    }

    // ------------------------------------------------------------------------
    void fsFormat(int blockSize = 4) {
        if (this->BitVector != nullptr) {
            delete[] this->BitVector;
        }
        this->currentBlockSize = blockSize;
        this->BitVectorSize = DISK_SIZE / blockSize;
        this->BitVector = new int[this->BitVectorSize];
        this->is_formated = true;
    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName) {
        if (!this->isDiskFormatted())
            return -1;


        if (this->MainDir.count(fileName) != 0) {
            cout << "file already exists" << endl;
            return -1;
        }

        fsInode* fsi = new fsInode(this->currentBlockSize);
        FileDescriptor fd(fileName, fsi);

        this->MainDir[fileName] = fsi;
        return this->OpenFile(fileName);
    }

    // ------------------------------------------------------------------------
    int OpenFile(string FileName) {
        if (!this->isDiskFormatted())
            return -1;

        if (this->MainDir.find(FileName) == this->MainDir.end()) {
            cout << "no file with such name" << endl;
            return -1;
        }

        for (int i = 0; i < this->OpenFileDescriptors.size(); i++) {
            if (this->OpenFileDescriptors[i] != NULL_FD && this->OpenFileDescriptors[i].getFileName() == FileName) {
                cout << "file already open" << endl;
                return i;
            }
        }

        FileDescriptor fd(FileName, this->MainDir[FileName]);
        int fdNum = this->OpenFileDescriptors.size();

        if (this->fdNullPointers.size() > 0) {
            int temp = this->fdNullPointers.front();
            fdNum = temp;
            this->fdNullPointers.pop();
            this->OpenFileDescriptors[fdNum] = fd;
        }
        else {
            this->OpenFileDescriptors.push_back(fd);
        }

        return fdNum;
    }

    // ------------------------------------------------------------------------
    string CloseFile(int fd) {
        if (!this->isDiskFormatted())
            return "-1";

        if (fd < 0 || fd >= this->OpenFileDescriptors.size() || this->OpenFileDescriptors[fd] == NULL_FD) {
            cout << "file isn't open" << endl;
            return "-1";
        }

        this->OpenFileDescriptors[fd] = NULL_FD;
        fdNullPointers.push(fd);

        return "aye";
    }

    // ------------------------------------------------------------------------
    int WriteToFile(int fd, char* buf, int len) {
        if (!this->isDiskFormatted())
            return -1;

        if (fd < 0 || fd >= this->OpenFileDescriptors.size() || this->OpenFileDescriptors[fd] == NULL_FD) {
            cout << "no such open file" << endl;
            return -1;
        }

        

        return 0;
    }

    // ------------------------------------------------------------------------
    int DelFile(string FileName) {
        if (!this->isDiskFormatted())
            return -1;
        return 0;
    }

    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char* buf, int len) {
        if (!this->isDiskFormatted())
            return -1;

        if (fd < 0 || fd >= this->OpenFileDescriptors.size() || this->OpenFileDescriptors[fd] == NULL_FD) {
            cout << "no such file descriptor" << endl;
            return -1;
        }
        fsInode* fsi = this->OpenFileDescriptors[fd].getInode();

        int currentFileSize = fsi->getFileSize();
        int maxBlockAmount = this->getMaxDataBlocksAllocatableForFile();

        if ((currentFileSize + len) / this->currentBlockSize > maxBlockAmount) {
            cout << "not enough space in file" << endl;
            return -1;
        }

        vector<char> buffy(buf, buf + len);

        return 0;
    }

    // ------------------------------------------------------------------------
    int getFileSize(int fd) {
        if (!this->isDiskFormatted())
            return -1;
        return 0;
    }

    // ------------------------------------------------------------------------
    int CopyFile(string srcFileName, string destFileName) {
        if (!this->isDiskFormatted())
            return -1;
        return 0;
    }

    // ------------------------------------------------------------------------
    int MoveFile(string srcFileName, string destFileName) {
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

    ~fsDisk() {
        fclose(sim_disk_fd);
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
            fs->fsFormat(blockSize);
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

