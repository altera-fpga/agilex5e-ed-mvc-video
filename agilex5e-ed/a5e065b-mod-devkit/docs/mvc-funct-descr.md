# 4Kp60 Multi-Video Connectivity System Example Design for Agilex™ 5 Devices — Functional Description

---

## Contents

* [System Architecture](#system-architecture)
* [VVP-Based Video Pipeline IP Components](#vvp-based-video-pipeline-ip-components)
  * [HDMI RX/TX Subsystem](#hdmi-rxtx-subsystem)
  * [DP RX/TX Subsystem](#dp-rxtx-subsystem)
  * [SDI RX/TX Subsystem](#sdi-rxtx-subsystem)
  * [Deinterlacer IP](#deinterlacer-ip)
  * [Chroma Resampler IP](#chroma-resampler-ip)
  * [Color Space Converter IP](#color-space-converter-ip)
  * [Test Pattern Generator IP](#test-pattern-generator-ip)
  * [Video Scaler IP](#video-scaler-ip)
  * [Video Frame Buffer IP](#video-frame-buffer-ip)
  * [Video Switch IP](#video-switch-ip)
* [Processor Subsystem](#processor-subsystem)

---

## System Architecture

This section describes the architecture of the 4Kp60 Multi-Video Connectivity System Example Design for Agilex™ 5 Devices.

The design receives SDI video through an FPGA mezzanine card (FMC) daughter card, and receives HDMI and DP video through the onboard connectors on the modular development kit.

All three video interfaces support standards up to UHD and can deliver video resolutions up to 4Kp60 to the FPGA fabric. Each interface converts pixel data to AXI4-Stream, which provides connectivity to other IP cores in the [Altera Video and Vision Processing (VVP) Suite](https://www.altera.com/products/ip/po-3150/video-and-vision-processing-suite).

The VVP-based video pipeline processes the input video. The design then transmits video through the HDMI, DP, or SDI output interface according to the routing mode that you select in the software application.

The design comprises the following hardware and software components:

* **Hardware**—Three video datapaths, one for each input and output video interface. Each datapath includes a frame buffer and a video scaler that decouple frame rates and active video resolutions between the input and output interfaces. Input and output video switches route input video to the output interfaces that you select in the software application. The design also includes additional VVP Suite IP cores and an embedded processor subsystem.

  The following figure shows the main hardware components and subsystems.

<p align="center">
  <img src="./images/top-block-diagram.png" width="100%">
  <br><em>Multi-Video Connectivity Example Design—Top-Level Hardware Block Diagram</em>
</p>

* **Software**—A bare-metal application that runs on a Nios® V soft processor. The application provides runtime control and debug menus through a JTAG UART interface.

  The following figure shows the interaction between the Nios® V software and the hardware in the FPGA fabric.

<p align="center">
  <img src="./images/top-block-diagram-opt2.png" width="100%">
  <br><em>Multi-Video Connectivity Example Design—Top-Level System Block Diagram</em>
</p>

---

## VVP-Based Video Pipeline IP Components

This section describes the IP cores that implement the video pipeline.

### HDMI RX/TX Subsystem

The GTS HDMI IP implements the HDMI sink and source functions. HDMI uses Transition Minimized Differential Signaling (TMDS) encoding to transmit audio and video data between source and sink devices. The HDMI cable and connectors carry four differential pairs that form three TMDS color channels and one TMDS clock channel for HDMI 1.4 and HDMI 2.0. Each color channel transfers color data and auxiliary data.

The following figure shows the GTS HDMI IP block diagram for TMDS mode.

<p align="center">
  <img src="./images/hdmi_rxtx.png" width="80%">
  <br><em>GTS HDMI IP Block Diagram—TMDS Mode</em>
</p>

In addition to the TMDS channels, the HDMI interface carries the following sideband channels:

* **Display Data Channel (DDC)**—A Video Electronics Standards Association (VESA) channel that configures a single source and a single sink and exchanges status between those devices. The source uses the DDC to read the sink Enhanced Extended Display Identification Data (E-EDID) and to determine the sink configuration and capabilities.

* **Status and Control Data Channel (SCDC)**—A channel that exchanges status and control information between the source and the sink.

* **Consumer Electronics Control (CEC)**—An optional protocol that provides high-level control functions between audiovisual products.

* **HDMI Ethernet and Audio Return Channel (HEAC)**—An optional channel that provides Ethernet-compatible data networking between connected devices, and an audio return channel in the opposite direction from TMDS. HEAC also uses the Hot Plug Detect (HPD) line for link detection.

#### Related Information

* [GTS HDMI IP User Guide](https://docs.altera.com/r/docs/823533/26.1/gts-hdmi-ip-user-guide/gts-hdmi-ip-quick-reference)

### DP RX/TX Subsystem

VESA defines DisplayPort as an open digital communications interface. Typical applications include the following:

* Internal connections, such as interfaces within a PC or a monitor
* External display connections, including interfaces between a PC and a monitor or projector, between a PC and a TV, or between a device such as a DVD player and a TV display

The DisplayPort IP supports a scalable Main Link with 1, 2, or 4 lanes, and supports lane data rates of 1.62 Gbps, 2.7 Gbps, 5.4 Gbps, 8.1 Gbps, 10.0 Gbps, 13.5 Gbps, and 20.0 Gbps.

The following figure shows the DisplayPort IP block diagram.

<p align="center">
  <img src="./images/dp_rxtx.png" width="80%">
  <br><em>DisplayPort IP Block Diagram</em>
</p>

The Main Link transports video and audio streams with embedded clocking, which decouples the pixel and audio clocks from the transmission clock. The channel coding depends on the DisplayPort version and lane rate:

| DisplayPort version | Lane data rate | Channel coding |
|---------------------|----------------|----------------|
| DisplayPort 1.4 | Up to 8.1 Gbps | Scrambled ANSI 8b/10b |
| DisplayPort 2.1 | Up to 8.1 Gbps | Scrambled ANSI 8b/10b |
| DisplayPort 2.1 | 10.0 Gbps and above | 128b/132b |

The transmission includes redundancy for error detection. For secondary data, such as audio, the IP uses Reed-Solomon coding for error detection.

The DisplayPort AUX channel is an AC-coupled, terminated differential pair. The AUX channel uses Manchester II coding and provides a data rate of 1 Mbps. Each transaction completes in less than 500 μs, with a maximum burst data size of 16 bytes.

#### Related Information

* [DisplayPort IP User Guide](https://docs.altera.com/r/docs/683273/25.3/displayport-ip-user-guide/displayport-ip-quick-reference)

### SDI RX/TX Subsystem

The SDI receive (RX) and transmit (TX) subsystem is based on the GTS SDI II IP core and supports HD-SDI, 3G-SDI, 6G-SDI, and 12G-SDI.

The GTS SDI II IP includes the following components:

* Transceiver blocks, including PHY Management and Direct PHY IP
* Protocol block
* CV2AXI receiver bridge
* AXI2CV transmitter bridge

The following figure shows the GTS SDI II IP block diagram.

<p align="center">
  <img src="./images/sdi_rx.png" width="80%">
  <br><em>GTS SDI II IP Block Diagram</em>
</p>

The transceiver blocks implement serial transport. The Direct PHY IP converts high-speed serial data to and from formatted parallel data. PHY Management performs low-level configuration, tuning, and real-time monitoring, including clock and rate detection for uncompressed video.

The protocol block performs SDI-specific processing in the parallel data domain.

The CV2AXI bridge converts clocked video to AXI4-Stream (full variant). The AXI2CV bridge performs the reverse conversion. These bridges enable the SDI IP to use the Altera FPGA Streaming Video protocol. The Altera FPGA Streaming Video protocol is an AXI4-Stream-based protocol with extensions for metapackets and active video. The protocol provides connectivity to VVP Suite IP cores and to other AXI4-Stream-compliant video IP.

#### Related Information

* [GTS SDI II IP User Guide](https://docs.altera.com/r/docs/823539/25.3/gts-sdi-ii-ip-user-guide/gts-sdi-ii-ip-quick-reference)

### Deinterlacer IP

The video processing pipeline requires progressive video. The design therefore instantiates a Deinterlacer IP core at the front end of each of the three video interfaces to convert interlaced video to progressive format.

The IP accepts a stream of interlaced video fields and outputs progressive frames. The IP generates the progressive frames using a bob, weave, or motion-adaptive algorithm. Progressive video frames pass through the IP unchanged.

#### Related Information

* [Deinterlacer IP](https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/deinterlacer-ip)

### Chroma Resampler IP

The human visual system has higher spatial acuity for brightness than for color. Color spaces such as YCbCr separate luma (brightness) from chroma (color), which allows chroma to be sampled at a lower rate while preserving most perceived detail. Chroma subsampling reduces bandwidth for transport and storage, and can reduce FPGA resource utilization.

The Chroma Resampler (CRS) IP supports 4:4:4, 4:2:2, and 4:2:0 sampling on both streaming interfaces. You can enable the conversion paths required for any supported combination of input and output sampling formats. The sampling formats are defined as follows:

* **4:4:4**—Y, Cb, and Cr have the same number of samples. Each Y sample has one Cb sample and one Cr sample.
* **4:2:2**—Cb and Cr are sampled at half the luma rate horizontally. Each pair of Y samples shares one Cb sample and one Cr sample.
* **4:2:0**—Cb and Cr are sampled at half the luma rate horizontally and vertically. Each 2×2 group of four Y samples shares one Cb sample and one Cr sample.

This design performs video processing in the 4:4:4 chroma sampling format. The design therefore instantiates CRS IP cores in the following locations:

* At the input of the video datapaths, to convert the supported YCbCr input formats to 4:4:4 for downstream processing.
* Before the SDI TX interface, to convert the processed video to the 4:2:2 chroma sampling format that the SDI TX IP requires.

The HDMI TX and DP TX interfaces transmit RGB, which does not require chroma resampling.

#### Related Information

* [Chroma Resampler IP](https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/chroma-resampler-ip)

### Color Space Converter IP

Color space conversion is required when video moves between systems that use different color representations. For example, displaying television content on a computer monitor can require Y′CbCr to R′G′B′ conversion. Sending computer graphics into an SDI path can require the reverse conversion.

A color space defines how colors are represented in a three-dimensional coordinate system. R′G′B′ is commonly used for computer displays. Y′CbCr is commonly used for digital television and video transport.

This design performs video processing in R′G′B′. The design therefore instantiates Color Space Converter (CSC) IP cores in the following locations:

* At the input of the video datapaths, to convert Y′CbCr to R′G′B′ before video processing. When the input video is already R′G′B′, the software application configures the CSC IP in bypass mode.
* Before the SDI TX interface, to convert R′G′B′ to the Y′CbCr color space that the SDI TX IP requires.

You can update the CSC conversion coefficients at run time through an Avalon® memory-mapped interface.

#### Related Information

* [Color Space Converter IP](https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/color-space-converter-ip)

### Test Pattern Generator IP

The Test Pattern Generator (TPG) IP produces an Altera Streaming Video-compliant output stream. Each field contains one of the supported fixed test images. You can adjust the resolution, color space, chroma subsampling format, and pattern selection at run time.

In this design, the TPG IP is a video input source. The design configures the TPG IP to provide solid-color patterns and color-bar patterns. The TPG IP provides a video source for the routing use cases when no external video source is connected.

#### Related Information

* [Test Pattern Generator IP](https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/test-pattern-generator-ip)

### Video Scaler IP

The Scaler IP resizes the fields in an Altera Streaming Video-compliant input stream to produce output fields of a different height, width, or both. The Scaler IP provides three resizing algorithms—nearest neighbor, bilinear, and polyphase—that trade FPGA resource cost against output image quality. Nearest neighbor uses the fewest FPGA resources and produces the lowest output image quality. Polyphase produces the highest output image quality and uses the most FPGA resources.

In this design, the Scaler IP uses the polyphase algorithm. The software application upscales, bypasses, or downscales the input video according to the requested output resolution.

#### Related Information

* [Scaler IP](https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/scaler-ip)

### Video Frame Buffer IP

The Video Frame Buffer IP buffers video frames, and can drop or repeat frames to implement double buffering or triple buffering. The IP stores frames in external memory and provides Avalon® memory-mapped interfaces for connection to an external memory interface (EMIF).

In this design, the three frame buffers are configured for triple buffering. Each frame buffer synchronizes the corresponding input and output interfaces, because the input and output video clocks are asynchronous. The frame buffers also perform frame-rate conversion when the input frame rate does not match the output interface frame rate.

#### Related Information

* [Video Frame Buffer IP](https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/video-frame-buffer-ip)

### Video Switch IP

The Switch IP connects video inputs to video outputs to provide crosspoint switching, multipoint switching, and video broadcast functions.

This design instantiates two Switch IP cores:

* **Front-end switch**—A 4×3 video switch that routes any of the four video sources (HDMI RX, DP RX, SDI RX, and TPG) to any of the three video datapath channels.
* **Back-end switch**—A 3×3 video switch that routes any of the three video datapath channels to any of the three video output interfaces (HDMI TX, DP TX, and SDI TX).

For both switches, when an input is not used, the software application configures that input in consume mode.

The software application selects the routing use case. The following table summarizes the supported use cases and the input source that drives each output interface.

| Use case | Routing mode | HDMI TX source | DP TX source | SDI TX source |
|:--------:|--------------|----------------|--------------|---------------|
| 1 | One-to-one | HDMI RX | DP RX | SDI RX |
| 2 | One-to-any | TPG | TPG | TPG |
| 3 | One-to-any | HDMI RX | HDMI RX | HDMI RX |
| 4 | One-to-any | DP RX | DP RX | DP RX |
| 5 | One-to-any | SDI RX | SDI RX | SDI RX |
| 6 | Any-to-any | SDI RX | HDMI RX | DP RX |
| 7 | Any-to-any | HDMI RX | SDI RX | DP RX |
| 8 | Any-to-any | SDI RX | DP RX | HDMI RX |
| 9 | Any-to-any | DP RX | HDMI RX | SDI RX |
| 10 | Any-to-any | DP RX | HDMI RX | TPG |

The routing modes are defined as follows:

* **One-to-one**—Each output interface receives video from the input of the same interface type.
* **One-to-any**—One input source drives all three output interfaces.
* **Any-to-any**—Each output interface receives video from an independently selected input source.

#### One-to-One Routing

<p align="center">
  <img src="./images/routing_uc_00.png" width="100%">
  <br><em>Use Case 1—One-to-One Routing</em>
</p>

#### One-to-Any Routing

<p align="center">
  <img src="./images/routing_uc_01.png" width="100%">
  <br><em>Use Case 2—TPG to All Output Interfaces</em>
</p>

<p align="center">
  <img src="./images/routing_uc_02.png" width="100%">
  <br><em>Use Case 3—HDMI RX to All Output Interfaces</em>
</p>

<p align="center">
  <img src="./images/routing_uc_03.png" width="100%">
  <br><em>Use Case 4—DP RX to All Output Interfaces</em>
</p>

<p align="center">
  <img src="./images/routing_uc_04.png" width="100%">
  <br><em>Use Case 5—SDI RX to All Output Interfaces</em>
</p>

#### Any-to-Any Routing

<p align="center">
  <img src="./images/routing_uc_05.png" width="100%">
  <br><em>Use Case 6—SDI RX to HDMI TX, HDMI RX to DP TX, and DP RX to SDI TX</em>
</p>

<p align="center">
  <img src="./images/routing_uc_06.png" width="100%">
  <br><em>Use Case 7—HDMI RX to HDMI TX, SDI RX to DP TX, and DP RX to SDI TX</em>
</p>

<p align="center">
  <img src="./images/routing_uc_07.png" width="100%">
  <br><em>Use Case 8—SDI RX to HDMI TX, DP RX to DP TX, and HDMI RX to SDI TX</em>
</p>

<p align="center">
  <img src="./images/routing_uc_08.png" width="100%">
  <br><em>Use Case 9—DP RX to HDMI TX, HDMI RX to DP TX, and SDI RX to SDI TX</em>
</p>

<p align="center">
  <img src="./images/routing_uc_09.png" width="100%">
  <br><em>Use Case 10—DP RX to HDMI TX, HDMI RX to DP TX, and TPG to SDI TX</em>
</p>

#### Related Information

* [Switch IP](https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/switch-ip)

---

## Processor Subsystem

The design includes an embedded processor subsystem that hosts the control plane. A bare-metal software application runs on a Nios® V soft processor and configures the video pipeline through the memory-mapped control and status registers (CSRs) of the VVP Suite IP cores.

The application exposes runtime control and debug menus over a JTAG UART interface. Through these menus you select the routing use case, select the TX video standard for each interface, enable or disable individual output interfaces, cycle the input TPG patterns, and read back clock and video dimension status.

For the list of menu commands, refer to [Running the Demonstrations](./doc-mvc.md#running-the-demonstrations) in the main user guide.

#### Related Information

* [Nios® V Processor](https://www.altera.com/products/ip/po-3098/nios-v-processors)

---

[Return to the Main User Guide](./doc-mvc.md#functional-description)



