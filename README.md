# 4B25 Coursework 5 - Design of a Low Cost RFID Sensor
In this repository is the code for my implementation of the MFRC522 RFID sensor in the Warp-firmware environment. Attached as well is my report, labeled rlj36-coursework-5.pdf , as it was too large to upload to the Moodle (>5Mb). As well as this, here is a Google Drive link to a video I have recorded showing my implementation: https://drive.google.com/file/d/1fQccH0PM5kaXB0sDaY8To9bM-51M7FWv/view?usp=sharing

**User Guide for my Implementation**
Compile and download WARP.srec. Launch JLink.exe with jlink.command file and open JLinkRTTViewer in a separate window while the FRDM-KL03 evaluation board is connected to the PC by USB. Warp menu should boot, and then press '#' to enter the RFID Menu. Pressing '1' with an RFID tag present on the sensor will load the UID into current storage (will print "No card present" if no RFID tag is applied). Pressing '2' will verify whether the tag present has the same UID as the previously saved tag from '1'.

**Important Code Changes:**
warp-kl03-boot.c:
Added MFRC522.h
Lines 1380-1384: RFID is initialised, and variables to save the tag's unique identifiers
Lines 2558 - 2640: Case '#' on the warp-firmware to add option #1) scan for RFID tag and save UID #2) check currently presented tag to see if it matches the saved UID

MFRC522.c & MFRC522.h
Completely new code files, both incredibly important to operation of the MFRC522 sensor

CMakelists.txt, build.sh
Both add the MFRC522.* implementation

devINA219.* is unchanged for measuring current consumption values (including configuration scaler from coursework 4)

Can perform a git diff with the master branch if wanted.

Note: most of the MFRC522 library functions are modified for the Warp-firmware from existing Arduino or C libraries: 

https://github.com/ljos/MFRC522

https://github.com/miguelbalboa/rfid
