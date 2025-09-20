# AudioSystemBridge

**AudioSystemBridge** is a lightweight C++ service that acts as a gateway between low-level audio systems (JACK, ALSA, LV2 via lilv) and higher-level application logic. It exposes a gRPC API for remote control and integration, enabling real-time audio and MIDI routing, plugin hosting, and transport synchronization.

## 🎯 Purpose

This service provides a unified interface to:
- Connect and manage JACK audio/MIDI ports
- Interact with ALSA devices
- Host and control LV2 plugins using lilv
- Expose control endpoints via gRPC for remote orchestration

## 🚀 Features

- JACK client registration and port management
- ALSA device querying and control
- LV2 plugin discovery and instantiation
- gRPC API for remote access and automation
- Designed for headless operation (e.g. Docker with dummy JACK backend)

## 🧱 Architecture

