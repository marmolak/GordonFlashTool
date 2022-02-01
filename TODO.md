TODO list
=========

- **DONE** (md5 is used for now) - sha/md5 checksum of fdd images stored in metadata
- store date of upload of fdd images to metadata
- initialization format (format means: initialize FAT fs - looks like empty fdd) ? - maybe prepare blank fdd image?
    - possible usage:
      - `gordon -f -d /dev/disk` -> format all possible slots
      - `gordon -f -d /dev/disk -s 123` -> format only selected slot (`-w` support?)
