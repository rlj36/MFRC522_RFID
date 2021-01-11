# Baseline firmware for the [Warp](https://github.com/physical-computation/Warp-hardware) family of hardware platforms
This is the firmware for the [Warp hardware](https://github.com/physical-computation/Warp-hardware) and its publicly available and unpublished derivatives. This firmware also runs on the Freescale/NXP FRDM KL03 evaluation board which we use for teaching at the University of Cambridge. When running on platforms other than Warp, only the sensors available in the corresponding hardware platform are accessible.

**Important Code Changes:**
warp-kl03-boot.c
Lines 1380-1384: RFID is initialised, and variables to save the tag's unique identifiers
Lines 2558 - 2640: Case '#' on the warp-firmware to add option #1) scan for RFID tag and save UID #2) check currently presented tag to see if it matches the saved UID

MFRC522.c & MFRC522.h
Completely new code files, both incredibly important to operation of the MFRC522 sensor

CMakelists.txt, build.sh
Both add the MFRC522.* implementation

devINA219.* is unchanged for measuring current consumption values (including configuration scaler from coursework 4)

