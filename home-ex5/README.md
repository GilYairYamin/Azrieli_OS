# Unix-like File System Simulator (C++)

A user-space implementation of an Inode-based file system. This project simulates a physical disk using a flat file and implements a complete Virtual File System (VFS) layer, handling block allocation, inode indexing, and persistent storage.

## 🚀 Key Features

- **Inode Architecture:** Implements a multi-level indexing structure:
  - **3 Direct Blocks:** For fast access to small files.
  - **Single Indirect:** For medium files.
  - **Double Indirect:** For large files, creating a tree-like block structure.
- **Disk Management:**
  - **BitVector Allocation:** Tracks free/busy disk blocks using an optimized integer array.
  - **Persistence:** Simulates a physical disk drive using `DISK_SIM_FILE.txt`, allowing data to persist between program runs.
- **File Operations:** Supports `Create`, `Open`, `Close`, `Read`, `Write`, `Delete` (with block reclamation), `Copy`, and `Rename`.
- **Directory Entry:** Manages a flat directory structure mapping filenames to Inode pointers.

## 🛠 Technical Architecture

### The Inode Structure

The core logic resides in the `fsInode` class, which abstracts the physical block layout.

- **Mixed Indexing:** To optimize for both small and large files, the system uses a hybrid approach. Small writes go directly to pointers 0-2. Large writes trigger the allocation of pointer blocks (Indirect), which store addresses of data blocks rather than data itself.

### Block Allocation

- **Formatting:** When `fsFormat` is called, the system initializes a **BitVector**.
- **Writing:** The `allocateBlocks` function scans the BitVector for contiguous zero-bits to satisfy write requests.
- **Reclamation:** When `DelFile` is called, the system traverses the Inode tree (Direct -> Indirect -> Double Indirect) to identify every block used by the file and flips their bits back to 0 in the BitVector.

## 💻 Usage

### Compilation

```bash
g++ -o fs_sim stub_code.cpp
```
