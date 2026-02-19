import sys
from pathlib import Path
import ctypes as ct


class ArchiveEntryDisk(ct.Structure):
    _pack_ = 1
    _fields_ = (
        ("fileName", ct.c_char * 12),
        ("fileOffset", ct.c_uint32)
    )


class ArchiveEntry:
    def __init__(self, fileNameNoSZ: bytearray, fileOffset):
        self.fileName = fileNameNoSZ.decode("utf-8")
        self.fileOffset = fileOffset

        # This is set up later
        self.fileLength = 0


def readFilesInDat(data):
    dataLength = len(data)
    result = []

    # Read the first entry in the archive
    # This will give us the total size of the archive directory
    firstOnDisk = ArchiveEntryDisk.from_buffer_copy(data[0:16])
    firstInMem = ArchiveEntry(firstOnDisk.fileName, firstOnDisk.fileOffset)

    # The number of files is equal to the offset of the first file
    # divided by the size of each archive entry (16 bytes)
    numberOfFiles = firstOnDisk.fileOffset // 16

    # Add first entry to list
    result.append(firstInMem)

    # Add the rest of the entries to the list
    for i in range(1, numberOfFiles):
        curEntryDisk = ArchiveEntryDisk.from_buffer_copy(
            data[i * 16:(i * 16) + 16]
        )

        curEntryMem = ArchiveEntry(
            curEntryDisk.fileName,
            curEntryDisk.fileOffset
        )

        result.append(curEntryMem)

    # Now, calculate the lengths of each file...
    # Note that each file is stored one after another in the .DAT file.
    # The size of each file can be found using the offset from the next file
    # in the archive except for the last file.

    for i in range(0, numberOfFiles - 1):
        cur = result[i]
        nxt = result[i + 1]

        result[i].fileLength = result[i + 1].fileOffset - result[i].fileOffset

    # Now, calculate the last file offset using the total length of the archive
    result[-1].fileLength = dataLength - result[-1].fileOffset

    return result


def extractFile(archiveFile: ArchiveEntry, data: bytearray, outputDir: str):
    outputPath = Path(f"{outputDir}/{archiveFile.fileName}")

    try:
        outputPath.write_bytes(data)
    except IOError as e:
        print(f"Failed to write output file {outputPath}: {e.strerror}")
        exit(1)

    print(f"Wrote {outputPath}")


def main():
    print("Sim City 2000 Music Extractor\n")

    if len(sys.argv) < 3:
        print(
            "Usage: sc2kme [Path to SC2000.DAT] [Path to put extracted files]"
        )
        exit(1)

    datFile = Path(sys.argv[1])
    outputDir = Path(sys.argv[2])
    data = bytearray()

    try:
        data = bytearray(datFile.read_bytes())
    except FileNotFoundError:
        print(f"File not found: {sys.argv[1]}")
        exit(1)

    if not outputDir.exists():
        outputDir.mkdir()

    archiveFiles = readFilesInDat(data)

    print(f"Found {len(archiveFiles)} files in archive")

    for i in archiveFiles:
        if i.fileName.endswith(".XMI"):
            extractFile(
                i,
                data[i.fileOffset:i.fileOffset + i.fileLength],
                outputDir.name
            )


if __name__ == "__main__":
    main()
