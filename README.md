# Atomic Super Layer Network (ASLN)

## Local Cybersecurity Tool for Professionals

ASLN is a multi-layer security tool that adds dynamic key rotation and location spoofing on top of your existing VPN.

## Features

- Dynamic key rotation every 0.5 seconds using NASA API
- 5 Space Agencies: NASA, SpaceX, ESA, CNSA, Roscosmos
- Location spoofing: The Moon, Jupiter, Saturn, Starlink
- EXIF metadata cleaner for images
- Terminal-based CLI
- No VPS required (uses your own VPN)

## Installation

git clone https://github.com/ZeraEnwyoo/asln.git
cd asln
make
sudo make install
```

## Usage

```bash
asln help
asln agencies
asln connect nasa
asln status
asln disconnect
asln clean image.jpg
asln batch ./photos/
```

## Requirements

- WireGuard (for VPN layer)
- GCC compiler
- libcurl, openssl, libjpeg, libpng

## Architecture

```
User → VPN (WireGuard) → ASLN Layer → Internet
                              ↓
                    Hacker sees: Jupiter
                    Real IP: Hidden
```

---

## ⚠️ DISCLAIMER & WARNING ⚠️

### Not for Commercial Use

**THIS SOFTWARE IS NOT FOR COMMERCIAL USE.**

You may NOT:
- Sell this software
- Use this software in commercial products
- Use this software for profit
- Sell access to this software
- Integrate this software into paid services

### API Usage Warning

**DO NOT SELL OR ABUSE THE FOLLOWING APIS:**

- NASA API (https://api.nasa.gov)
- SpaceX API (https://api.spacexdata.com)
- ESA (European Space Agency)
- CNSA (China National Space Administration)
- Roscosmos (Russian Space Agency)

These APIs are provided for free for educational and personal use. Abusing or selling access to these APIs violates their terms of service.

### Legal Consequences

Violators will be:
1. Reported to the respective space agencies
2. Blacklisted from using these APIs
3. Subject to legal action

### Educational Purpose Only

This tool is designed for:
- Cybersecurity education
- Personal privacy protection
- Security research
- Learning about VPN technology

**DO NOT USE FOR ILLEGAL ACTIVITIES.**

---

## Credits

### Space Agencies

| Agency | Country | Role |
|--------|---------|------|
| **NASA** | USA | Primary API for dynamic key generation |
| **SpaceX** | USA | Starlink and spacecraft location data |
| **ESA** | Europe | European space asset locations |
| **CNSA** | China | Special events only (Tiangong, Wenchang, Jiuquan) |
| **Roscosmos** | Russia | Special events only (Baikonur, Vostochny) |

### API Endpoints

- NASA: `https://api.nasa.gov/planetary/apod`
- SpaceX: `https://api.spacexdata.com/v4`
- ESA: Public RSS feeds

### Development

- **Atomic Super Layer Network Community**
- **Zera** - Project Founder & Lead Developer
- **Wyoo-Setup file system
- **ASLN Contributors**
---

## License

**ASLN License - Not for Commercial Use**

Copyright (c) 2025 Atomic Super Layer Network Community

You may use this software for:
- Personal cybersecurity
- Educational purposes
- Security research
- Open source non-commercial projects

You may NOT:
- Sell this software
- Use in commercial products
- Remove credit notices

Violators will be reported to NASA, SpaceX, ESA, CNSA, and Roscosmos.

For commercial licensing, contact the ASLN Community.

---

## Special Note to Agencies

To NASA, SpaceX, ESA, CNSA, and Roscosmos:

This tool uses your public APIs for **educational purposes only**. It is designed to teach cybersecurity professionals about:
- Dynamic key rotation
- VPN technology
- API integration
- Privacy protection

No API keys are being sold. No commercial use is occurring. Rate limits are respected through caching.

## API Rate Limits

| Agency | Free Tier | Cache Duration |
|--------|-----------|----------------|
| NASA | 1000 requests/hour | 1 hour |
| SpaceX | No limit | 5 minutes |
| ESA | Public RSS | 1 hour |
| CNSA | Limited | Special events only |
| Roscosmos | Limited | Special events only |

ASLN implements aggressive caching to respect these limits.

If you have concerns, please contact the ASLN Community.

---
Special Thanks To:

Space Agencies:
- NASA (National Aeronautics and Space Administration)
- SpaceX
- ESA (European Space Agency)
- CNSA (China National Space Administration)
- Roscosmos (Russian Space Agency)

**Powered by Atomic Community**

```
"Connect from Earth, Appear from the Stars"

