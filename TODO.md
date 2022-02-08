TODO list
=========

- **DONE** (md5 is used for now) - sha/md5 checksum of fdd images stored in metadata
- store date of upload of fdd images to metadata
- Format
    - **DONE** initialization format (format means: initialize FAT fs - looks like empty fdd) ? - maybe prepare blank fdd image?
        - possible usage:
        - (NOT GOOD IDEA) `gordon -f -d /dev/disk` -> format all possible slots
        - **DONE** `gordon -f -d /dev/disk -s 123` -> format only selected slot 
    - **DONE** Clear metadata after format? (NO) or `-w` support?
- Better parsing and checking FAT metadata
    - No "magic" jump instruction detection and try to read volume label from direntry - as a fallback, vendor ID should be still used.
- Provide some nice boot code for fun and profit!