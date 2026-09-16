# 4Kp60 Multi-Video Connectivity System Example Design for Agilex™ 5 Devices

The 4Kp60 Multi-Video Connectivity System Example Design for Agilex™ 5 Devices shows how a single FPGA device can unify HDMI, DisplayPort (DP), and serial digital interface (SDI) video using a common streaming protocol. The design provides video protocol conversion and processes video between these interfaces in real time.

## Description

The design receives SDI video through an FPGA mezzanine card (FMC) daughter card. Additionally, the design also receives HDMI and DP video through the onboard connectors on the modular development kit. All three video interfaces support standards up to UHD and can deliver video resolutions up to 4Kp60 to the FPGA fabric. Each interface converts pixel data to AXI4-Stream, which provides connectivity to other IP cores in the Altera Video and Vision Processing (VVP) Suite. The design comprises hardware and software components: The software is a bare-metal application that runs on a Nios® V soft processor. The application provides runtime control and debug menus through a JTAG UART interface. The hardware includes three video datapaths, one for each input and output video interface. Each path includes a frame buffer and a video scaler to decouple frame rates and active video resolutions between the input and output interfaces. Input and output video switches route input video to the output interfaces that you select in the software application. The hardware also includes an input TPG, additional VVP Suite IP cores for video preprocessing and an embedded processor subsystem.

## Project Details

- **Title**: 4Kp60 Multi-Video Connectivity System Example Design for Agilex™ 5 Devices
- **Source**: Github
- **Family**: Agilex 5
- **Quartus Version**: 26.1.1
- **Development Kit**: Agilex™ 5 FPGA and SoC E-Series 065B Modular Development Kit MK-A5E065BB32AEA
- **Device Part**: A5ED065BB32AE4S
- **Design Package**: agilex5e_mdk_4k_mvc_ed.zip
- **Category**: Video/Vision
- **URL**: https://github.com/altera-fpga/agilex5e-ed-mvc-video/tree/rel/26.1.1/agilex5e-ed/a5e065b-mod-devkit
- **download URL**: https://github.com/altera-fpga/agilex5e-ed-mvc-video/releases/download/rel-26.1.1/agilex5e_mdk_4k_mvc_ed.zip

---

## Documentation

- **Title**: User Guide
- **URL**: https://github.com/altera-fpga/agilex5e-ed-mvc-video/blob/rel/26.1.1/agilex5e-ed/a5e065b-mod-devkit/docs/doc-mvc.md

---

## Repository Overview

The repository contains the necessary files and collateral to create and build
the 4Kp60 Multi-Video Connectivity System Example Design for Agilex™ 5 Devices.

The product of this repository is generated using a software and
hardware flow, and it is listed in the following table:

| Product | Type | Description |
|----|----|----|
| `.sof` | SRAM Object File | FPGA bitstream to be loaded over JTAG |

---

## Quick Start

1. Download the example design from the repository listed in the following table.

| Component | Location | Branch / Tag |
|-----------|----------|--------------|
| Assets release tag | [rel-26.1.1](https://github.com/altera-fpga/agilex5e-ed-mvc-video/releases/tag/rel-26.1.1) | `rel-26.1.1` |

2. Extract the package.

3. Confirm that the FPGA design files are in `src/vds/`, `src/rtl/` and `src/ip/`.

4. Confirm that the software files are in `src/sw/`.

   The example design directory structure is as follows:

```text
<Example Design>/
├── top.qpf                  # Quartus project file
├── top.qsf                  # Quartus settings file
├── da_drc.dawf              # Design Assistant waiver file
├── README.md                # Repository readme
├── docs/                    # User guide for this example design
├── output_files/            # Precompiled SOF
├── script/                  # Scripts used to generate the example design
└── src/
    ├── ip/                  # .ip files
    ├── rtl/                 # RTL and SDC files
    ├── sw/                  # Bare-metal software application
    └── vds/                 # Tcl to generate the Visual Designer Studio (VDS) project
```

5. Launch the Quartus® Prime Pro Edition software.

6. Open `<Example Design>/top.qpf`.

7. On the **Processing** menu, click **Start Compilation**.

8. Wait until compilation completes.

After a successful compilation, the Quartus® Prime software generates the programming file in the `<Example Design>/output_files/` directory:

* `top.sof`

---

### Recompiling the Nios® V Software Application

To rebuild the Nios® V software application and merge it into the .sof, type the following commands:

```bash
cd <Example Design>/script/
quartus_sh -t post_swapp_mvc_gen.tcl
```

The script compiles the Nios® V application, merges the generated hexadecimal (HEX) ROM into the project, and generates an updated `top.sof` that contains the latest software.

---

## Documentation

[User Guide for the 4Kp60 Multi-Video Connectivity System Example Design](https://github.com/altera-fpga/agilex5e-ed-mvc-video/blob/rel/26.1.1/agilex5e-ed/a5e065b-mod-devkit/docs/doc-mvc.md)

---









