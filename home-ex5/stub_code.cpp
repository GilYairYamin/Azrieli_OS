#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <assert.h>
#include <string.h>
#include <cmath>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 512
#define DIRECT_BLOCKS 3

// Function to convert decimal to binary char
char decToBinary(int n) {
    return static_cast<char>(n);
}

// #define SYS_CALL
// ============================================================================
class fsInode {
    int fileSize;
    int block_in_use;
    int data_blocks;

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
        data_blocks = 0;
        block_size = _block_size;
        directBlock1 = -1;
        directBlock2 = -1;
        directBlock3 = -1;
        singleInDirect = -1;
        doubleInDirect = -1;
    }

    int getFileSize() {
        return fileSize;
    }

    void addFileSize(int add) {
        fileSize += add;
    }

    int getDirectBlock(int index) {
        switch (index) {
        case 0:
            return directBlock1;
        case 1:
            return directBlock2;
        case 2:
            return directBlock3;
        }
        return -1;
    }

    void setDirectBlock(int index, int block) {
        switch (index) {
        case 0:
            directBlock1 = block;
            return;
        case 1:
            directBlock2 = block;
            return;
        case 2:
            directBlock3 = block;
            return;
        }
    }

    int getSingleIndirect() {
        return singleInDirect;
    }

    void setSingleIndirect(int block) {
        singleInDirect = block;
    }

    int getDoubleIndirect() {
        return doubleInDirect;
    }

    void setDoubleIndirect(int block) {
        doubleInDirect = block;
    }

    int getDataBlocks() {
        return data_blocks;
    }

    void addDataBlocks(int add) {
        data_blocks += add;
        addTotalBlocks(add);
    }

    int getTotalBlocks() {
        return block_in_use;
    }

    void addTotalBlocks(int add) {
        block_in_use += add;
    }
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

    void setFileName(string name) {
        file.first = name;
    }

    fsInode* getInode() {
        return file.second;
    }

    void setInode(fsInode* fsi) {
        file.second = fsi;
    }

    int getFileSize() {
        if (file.second == nullptr)
            return 0;
        return file.second->getFileSize();
    }

    bool isInUse() {
        return (inUse);
    }

    void setInUse(bool _inUse) {
        inUse = _inUse;
    }
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk {
    FILE* sim_disk_fd;

    bool is_formated;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the 
    //             first block is occupied. 
    int BitVectorSize;
    int* BitVector;

    int block_size;

    // Unix directories are lists of association structures, 
    // each of which contains one filename and one inode number.
    map<string, fsInode*>  MainDir;

    // OpenFileDescriptors --  when you open a file, 
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor. 
    vector< FileDescriptor > OpenFileDescriptors;


public:
// ------------------------------------------------------------------------
    fsDisk() {
        BitVector = nullptr;
        is_formated = false;

        sim_disk_fd = fopen(DISK_SIM_FILE, "r+");
        if (!sim_disk_fd)
            sim_disk_fd = fopen(DISK_SIM_FILE, "w+");

        assert(sim_disk_fd);
        clearDisk();
    }

    ~fsDisk() {
        clearDynamicData();
        fclose(sim_disk_fd);
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
        clearDynamicData();

        block_size = blockSize;
        BitVectorSize = DISK_SIZE / block_size;

        BitVector = new int[BitVectorSize];
        is_formated = true;

        clearDisk();
    }

    void clearDynamicData() {
        if (BitVector != nullptr)
            delete[] BitVector;

        for (auto it = MainDir.begin(); it != MainDir.end(); it++)
            delete it->second;

        MainDir.clear();
        OpenFileDescriptors.clear();
    }

    void clearDisk() {
        int ret_val;
        for (int i = 0; i < DISK_SIZE; i++) {
            ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fwrite("\0", 1, 1, sim_disk_fd);
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);
    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName) {
        if (!is_formated) {
            cerr << "ERR - disk not formatted" << endl;
            return -1;
        }

        if (MainDir.count(fileName) > 0) {
            cerr << "ERR - file name already exists" << endl;
            return -1;
        }

        MainDir[fileName] = new fsInode(block_size);
        return OpenFile(fileName);
    }

    // ------------------------------------------------------------------------
    int OpenFile(string fileName) {
        if (!isDiskFormatted())
            return -1;

        if (MainDir.count(fileName) == 0) {
            cerr << "ERR - file doesn't exist" << endl;
            return -1;
        }

        if (findFdByName(fileName) >= 0) {
            cerr << "ERR - file already open" << endl;
            return -1;
        }

        FileDescriptor fd(fileName, MainDir[fileName]);
        int newFdNum = getNextOpenFileDescriptor();
        if (newFdNum == OpenFileDescriptors.size())
            OpenFileDescriptors.push_back(fd);
        else
            OpenFileDescriptors[newFdNum] = fd;

        return newFdNum;
    }

    // ------------------------------------------------------------------------
    string CloseFile(int fd) {
        if (!isDiskFormatted())
            return "-1";

        string fileName = findFileNameByFd(fd);
        if (fileName == "-1") {
            cerr << "ERR - file descriptor doesn't exis" << endl;
            return "-1";
        }

        OpenFileDescriptors[fd].setInUse(false);
        return fileName;
    }

    // ------------------------------------------------------------------------
    int WriteToFile(int fd, char* buf, int len) {
        if (!isDiskFormatted())
            return -1;

        if (!doesFdExist(fd))
            return -1;

        fsInode* fsi = OpenFileDescriptors[fd].getInode();
        if (maxFileSize() <= fsi->getFileSize()) {
            cerr << "ERR - file is already full" << endl;
            return -1;
        }

        if (!addDataBlocksToFile(fsi, len)) {
            return -1;
        }

        if (!writeDataToFile(fsi, buf, len)) {
            return -1;
        }
        return len;
    }

    // ------------------------------------------------------------------------
    int DelFile(string fileName) {
        if (!isDiskFormatted())
            return -1;

        if (MainDir.count(fileName) == 0) {
            cerr << "ERR - file does not exist" << endl;
            return -1;
        }

        if (findFdByName(fileName) >= 0) {
            cerr << "ERR - file is open" << endl;
            return -1;
        }

        queue<int> blocks = gatherAllAllocatedBlocks(MainDir[fileName]);
        delete MainDir[fileName];
        MainDir.erase(fileName);
        freeBlocks(blocks);

        int fd = findFdByNameIgnoreUse(fileName);
        if (fd >= 0) {
            OpenFileDescriptors[fd].setInUse(false);
            OpenFileDescriptors[fd].setFileName("");
            OpenFileDescriptors[fd].setInode(nullptr);
        }
        return 1;
    }

    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char* buf, int len) {
        if (!isDiskFormatted())
            return -1;

        if (!doesFdExist(fd))
            return -1;

        fsInode* fsi = OpenFileDescriptors[fd].getInode();
        int res = readDataFromFile(fsi, buf, len);
        if (res < 0) {
            return -1;
        }
        return res;
    }

    // ------------------------------------------------------------------------
    int getFileSize(int fd) {
        if (!isDiskFormatted())
            return -1;

        string fileName = findFileNameByFd(fd);
        if (fileName == "-1") {
            cerr << "ERR - file descriptor doesn't exis" << endl;
            return -1;
        }

        return OpenFileDescriptors[fd].getFileSize();
    }

    // ------------------------------------------------------------------------
    int CopyFile(string srcFileName, string destFileName) {
        if (!isDiskFormatted())
            return -1;

        if (MainDir.count(srcFileName) == 0) {
            cerr << "ERR - no source file" << endl;
            return -1;
        }

        if (MainDir.count(destFileName) > 0) {
            cerr << "ERR - destination file already exists" << endl;
            return -1;
        }

        fsInode* srcFsi = MainDir[srcFileName];
        fsInode* destFsi = new fsInode(block_size);

        int neededBlocks = srcFsi->getTotalBlocks();

        if (!addDataBlocksToFile(destFsi, srcFsi->getFileSize())) {
            cerr << "ERR - not enough space" << endl;
            delete destFsi;
            return -1;
        }

        char buf[DISK_SIZE + 1];
        readDataFromFile(srcFsi, buf, srcFsi->getFileSize());
        writeDataToFile(destFsi, buf, srcFsi->getFileSize());
        MainDir[destFileName] = destFsi;
        return 1;
    }

    // ------------------------------------------------------------------------
    int RenameFile(string oldFileName, string newFileName) {
        if (!isDiskFormatted())
            return -1;

        if (MainDir.count(oldFileName) == 0) {
            cerr << "ERR - file doesn't exist" << endl;
            return -1;
        }

        if (MainDir.count(newFileName) > 0) {
            cerr << "ERR - new file name already in use" << endl;
            return -1;
        }

        if (findFdByName(oldFileName) >= 0) {
            cerr << "ERR - cannot change name of open file" << endl;
            return -1;
        }

        fsInode* fsi = MainDir[oldFileName];
        MainDir.erase(oldFileName);
        MainDir[newFileName] = fsi;
        return 1;
    }

private:
    queue<int> gatherAllAllocatedBlocks(fsInode* fsi) {
        queue<int> blocks;
        for (int i = 0; i < fsi->getDataBlocks(); i++) {
            gatherBlocksByIndex(fsi, i, blocks);
        }
        return blocks;
    }

    void gatherBlocksByIndex(fsInode* fsi, int index, queue<int>& blocks) {
        int tempBlock;
        int firstSingleIndirectIndex = DIRECT_BLOCKS;
        int singleIndirectIndex = index - firstSingleIndirectIndex;

        int firstDoubleIndirectIndex = DIRECT_BLOCKS + block_size;
        int doubleIndirectIndex = index - firstDoubleIndirectIndex;

        if (index < firstSingleIndirectIndex) {
            blocks.push(fsi->getDirectBlock(index));
            return;
        }

        if (index < firstDoubleIndirectIndex) {
            tempBlock = fsi->getSingleIndirect();
            if (index == firstDoubleIndirectIndex)
                blocks.push(fsi->getSingleIndirect());
            blocks.push(readDataByBlockAndOffset(tempBlock, singleIndirectIndex));
            return;
        }

        tempBlock = fsi->getDoubleIndirect();
        if (index == firstDoubleIndirectIndex)
            blocks.push(tempBlock);

        tempBlock = readDataByBlockAndOffset(tempBlock, doubleIndirectIndex / block_size);
        if (doubleIndirectIndex % block_size == 0)
            blocks.push(tempBlock);

        blocks.push(readDataByBlockAndOffset(tempBlock, doubleIndirectIndex % block_size));
    }

    int readDataByBlockAndOffset(int block, int offset) {
        return readDataByIndex(block * block_size + offset);
    }

    int readDataFromFile(fsInode* fsi, char* buf, int len) {
        int currentBlockIndex = 0;
        int currentBlock = getDataBlockByIndex(fsi, currentBlockIndex);
        int tempData;

        int i = 0;
        for (i = 0; i < min(len, fsi->getFileSize()); i++) {
            if (currentBlockIndex != i / block_size) {
                currentBlockIndex = i / block_size;
                currentBlock = getDataBlockByIndex(fsi, currentBlockIndex);
            }
            tempData = readDataByBlockAndOffset(currentBlock, i % block_size);
            buf[i] = (char)tempData;
        }
        buf[i] = '\0';

        return min(len, fsi->getFileSize());
    }

    int readDataByIndex(int index) {
        unsigned char cData;
        int ret_val = fseek(sim_disk_fd, index, SEEK_SET);
        ret_val = fread(&cData, 1, 1, sim_disk_fd);
        assert(ret_val == 1);
        return cData;
    }

    bool writeDataToFile(fsInode* fsi, char* buf, int len) {
        int startIndex = fsi->getFileSize();

        int charIndex = startIndex;
        int currentBlockIndex = startIndex / block_size;
        int currentBlock = getDataBlockByIndex(fsi, currentBlockIndex);

        for (int i = 0; i < len; i++) {
            if (i + startIndex >= maxFileSize())
                return true;
            charIndex = startIndex + i;
            if (currentBlockIndex != charIndex / block_size) {
                currentBlockIndex = charIndex / block_size;
                currentBlock = getDataBlockByIndex(fsi, currentBlockIndex);
            }
            writeDataByBlockAndOffset(currentBlock, charIndex % block_size, buf[i]);
            fsi->addFileSize(1);
        }
        return true;
    }

    int getDataBlockByIndex(fsInode* fsi, int index) {
        int tempBlock;
        int firstSingleIndirectIndex = DIRECT_BLOCKS;
        int singleIndirectIndex = index - firstSingleIndirectIndex;

        int firstDoubleIndirectIndex = DIRECT_BLOCKS + block_size;
        int doubleIndirectIndex = index - firstDoubleIndirectIndex;

        if (index < firstSingleIndirectIndex) {
            return fsi->getDirectBlock(index);
        }

        if (index < firstDoubleIndirectIndex) {
            tempBlock = fsi->getSingleIndirect();
            return readDataByBlockAndOffset(tempBlock, singleIndirectIndex);
        }

        tempBlock = fsi->getDoubleIndirect();
        tempBlock = readDataByBlockAndOffset(tempBlock, doubleIndirectIndex / block_size);
        return readDataByBlockAndOffset(tempBlock, doubleIndirectIndex % block_size);
    }

    void writeDataByBlockAndOffset(int block, int offset, int data) {
        writeDataByIndex(block * block_size + offset, data);
    }

    void writeDataByIndex(int index, int data) {
        unsigned char cData = data;
        int ret_val = fseek(sim_disk_fd, index, SEEK_SET);
        ret_val = fwrite(&cData, 1, 1, sim_disk_fd);
        assert(ret_val == 1);
    }

    int getNextOpenFileDescriptor() {
        for (int i = 0; i < OpenFileDescriptors.size(); i++) {
            if (!OpenFileDescriptors[i].isInUse())
                return i;
        }
        return OpenFileDescriptors.size();
    }

    int findFdByName(string fileName) {
        FileDescriptor* fd;
        for (int i = 0; i < OpenFileDescriptors.size(); i++) {
            fd = &(OpenFileDescriptors[i]);
            if (fd->isInUse() == true && fd->getFileName() == fileName)
                return i;
        }
        return -1;
    }

    int findFdByNameIgnoreUse(string fileName) {
        FileDescriptor* fd;
        for (int i = 0; i < OpenFileDescriptors.size(); i++) {
            fd = &(OpenFileDescriptors[i]);
            if (fd->getFileName() == fileName)
                return i;
        }
        return -1;
    }

    string findFileNameByFd(int fd) {
        if (!doesFdExist(fd)) {
            return "-1";
        }
        return OpenFileDescriptors[fd].getFileName();
    }

    bool doesFdExist(int fd) {
        if (fd < 0 || fd >= OpenFileDescriptors.size() || !OpenFileDescriptors[fd].isInUse()) {
            cerr << "ERR - no such file descriptor" << endl;
            return false;
        }
        return true;
    }

    bool isDiskFormatted() {
        if (!is_formated) {
            cerr << "ERR - disk not formatted" << endl;
            return false;
        }
        return true;
    }

    bool addDataBlocksToFile(fsInode* fsi, int inputLen) {
        int oldFileSize = fsi->getFileSize();
        int newFileSize = min(maxFileSize(), oldFileSize + inputLen);

        int allocateBlockAmount = neededBlocksFor(newFileSize) - neededBlocksFor(oldFileSize);
        queue<int> blocks = allocateBlocks(allocateBlockAmount);
        if (blocks.size() <= 0)
            return false;

        int startBlockIndex = fsi->getDataBlocks();
        int dataBlocksAddAmount = dataBlocksForFileSize(newFileSize) - dataBlocksForFileSize(oldFileSize);

        for (int i = 0; i < dataBlocksAddAmount; i++) {
            addDataBlockToFileByIndex(fsi, i + startBlockIndex, blocks);
        }
        return true;
    }

    void addDataBlockToFileByIndex(fsInode* fsi, int index, queue<int>& blocks) {
        int tempBlock = 0;
        int firstSingleIndirectIndex = DIRECT_BLOCKS;
        int singleIndirectIndex = index - firstSingleIndirectIndex;

        int firstDoubleIndirectIndex = DIRECT_BLOCKS + block_size;
        int doubleIndirectIndex = index - firstDoubleIndirectIndex;

        if (index < firstSingleIndirectIndex) {
            fsi->setDirectBlock(index, blocks.front());
            fsi->addDataBlocks(1);
            blocks.pop();
            return;
        }

        if (index == firstSingleIndirectIndex) {
            fsi->setSingleIndirect(blocks.front());
            fsi->addTotalBlocks(1);
            blocks.pop();
        }

        if (index < firstDoubleIndirectIndex) {
            tempBlock = fsi->getSingleIndirect();
            writeDataByBlockAndOffset(tempBlock, singleIndirectIndex, blocks.front());
            fsi->addDataBlocks(1);
            blocks.pop();
            return;
        }

        if (index == firstDoubleIndirectIndex) {
            fsi->setDoubleIndirect(blocks.front());
            fsi->addTotalBlocks(1);
            blocks.pop();
        }

        tempBlock = fsi->getDoubleIndirect();
        if (doubleIndirectIndex % block_size == 0) {
            writeDataByBlockAndOffset(tempBlock, doubleIndirectIndex / block_size, blocks.front());
            fsi->addTotalBlocks(1);
            blocks.pop();
        }

        tempBlock = readDataByBlockAndOffset(tempBlock, doubleIndirectIndex / block_size);
        writeDataByBlockAndOffset(tempBlock, doubleIndirectIndex % block_size, blocks.front());
        fsi->addDataBlocks(1);
        blocks.pop();
    }

    int dataBlocksForFileSize(int fileSize) {
        int dataBlocks = fileSize / block_size;
        dataBlocks += (fileSize % block_size > 0) ? 1 : 0;
        return dataBlocks;
    }

    int neededBlocksFor(int fileSize) {
        int dataBlocksNeeded = dataBlocksForFileSize(fileSize);
        int indirectBlocksNeeded = 0;

        if (dataBlocksNeeded <= DIRECT_BLOCKS)
            goto RETURN;

        indirectBlocksNeeded++;
        if (dataBlocksNeeded <= DIRECT_BLOCKS + block_size)
            goto RETURN;

        indirectBlocksNeeded++;
        for (int i = DIRECT_BLOCKS + block_size; i < dataBlocksNeeded; i++) {
            indirectBlocksNeeded++;
        }

    RETURN:
        return dataBlocksNeeded + indirectBlocksNeeded;
    }

    int maxFileSize() {
        return maxDataBlockAmount() * block_size;
    }

    int maxDataBlockAmount() {
        return DIRECT_BLOCKS + block_size * (1 + block_size);
    }

    queue<int> allocateBlocks(int amount) {
        queue<int> blocks;
        for (int i = 0; i < BitVectorSize; i++) {
            if (BitVector[i] == 0) {
                blocks.push(i);
                BitVector[i] = 1;
                if (blocks.size() >= amount)
                    return blocks;
            }
        }

        cerr << "ERR - could not allocate enough blocks" << endl;
        freeBlocks(blocks);
        return blocks;
    }

    void freeBlocks(queue<int> blocks) {
        int temp = 0;
        while (blocks.size() > 0) {
            temp = blocks.front();
            blocks.pop();
            BitVector[temp] = 0;
        }
    }
};

int main() {
    int blockSize;
    int direct_entries;
    string fileName;
    string fileName2;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE + 1];
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