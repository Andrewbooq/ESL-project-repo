## Intro
Open project of Embedded system laboratory that is used for studying embedded.

## Hardware
The project is based on nRF52840 SoC. Board PCA10059 (USB Dongle).

### nRF52840 SoC

The nRF52840 SoC is the most advanced member of the nRF52 Series. It meets the challenges of sophisticated applications that need protocol concurrency and a rich and varied set of peripherals and features.  It offers generous memory availability for both Flash and RAM, which are prerequisites for such demanding applications.

The nRF52840 is fully multiprotocol capable with full protocol concurrency. It has protocol support for Bluetooth LE, Bluetooth mesh, Thread, Zigbee, 802.15.4, ANT and 2.4 GHz proprietary stacks.

The nRF52840 is built around the 32-bit ARM® Cortex™-M4 CPU with floating point unit running at 64 MHz. It has NFC-A Tag for use in simplified pairing and payment solutions. The ARM TrustZone® CryptoCell cryptographic unit is included on-chip and brings an extensive range of cryptographic options that execute highly efficiently independent of the CPU. It has numerous digital peripherals and interfaces such as high speed SPI and QSPI for interfacing to external flash and displays, PDM and I2S for digital microphones and audio, and a full speed USB device for data transfer and power supply for battery recharging. 

Exceptionally low energy consumption is achieved using a sophisticated on-chip adaptive power management system.

[datasheet nRF52840 PCA10059 USB Dongle](https://github.com/user-attachments/files/17661446/datasheet.nRF52840.PCA10059.USB.Dongle.pdf)

### PCA10059 USB Dongle

[pca10059_schematic_and_pcb](https://github.com/user-attachments/files/17661461/pca10059_schematic_and_pcb.pdf)

## The common rules for the current repo

### 1 Tasks and projects
- All work shold be under tasks in [ESL-project](https://github.com/users/Andrewbooq/projects/2).

### 2 Branches and tags
- A branch for a new workshop should have prefix feature/ and includes Workshop number (feature/WS4_pwm_and_double_click).
- A branch for a bug fix should have prefix bug/ (bug/WS4_cover_debouncing_all_cases).
- After brunch is done (all commits are pushed) it's necessary to Merge the branch to master over Pull Request.
- After the branch is merged to master it's necessary to create a tag with name that is the same to the branch but without any prefix (WS4_pwm_and_double_click).
- 
### 3 Comments
- Comments to Pull Requests should be multiline. First line inludes the workshop number and description (Workshop3: Use GPIO), next line - empty, next line refers to a task number in the project (Related to #17):
  ```
  Workshop3: Use GPIO
  
  Related to #17
  ```


