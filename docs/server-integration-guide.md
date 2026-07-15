# Server Integration Guide — News Aggregator Mode

## Overview

Từ phiên bản firmware này, thiết bị **ESP32 Xiaozhi** đã được chuyển đổi từ **chat bot** (voice conversation) sang **news aggregator** (button-to-speech).

### Kiến trúc cũ (Chat Bot):

```
Button → ToggleChatState → Listening State → Mic ON → ASR → Server LLM → TTS
Wake word → WakeWordDetected → Listening State → ...
```

### Kiến trúc mới (News Aggregator):

```
Button Click         → Send MCP "mcp_news_tech"     → Server → RSS → TTS → Audio
Button Double Click  → Send MCP "mcp_news_startup"  → Server → RSS → TTS → Audio
Button Triple Click  → Send MCP "mcp_news_science"  → Server → RSS → TTS → Audio
```

---

## 1. Device-to-Server Message Format

### **Button Gesture → MCP Tool Mapping**

| Gesture | Tool Name | Description |
|---------|-----------|-------------|
| **Click** | `mcp_news_tech` | Công nghệ |
| **Double Click** | `mcp_news_startup` | Startup / Khởi nghiệp |
| **Triple Click** | `mcp_news_science` | Khoa học |

### **WebSocket Message from Device:**

Khi người dùng nhấn button, thiết bị sẽ gửi một JSON message qua WebSocket:

```json
{
  "session_id": "208aa116-3f0a-41f3-ae72-9a3ab0722c3b",
  "type": "mcp",
  "payload": {
    "tool": "mcp_news_tech",
    "args": {
      "limit": 10
    }
  }
}
```

**Fields:**
| Field | Type | Description |
|-------|------|-------------|
| `session_id` | string | WebSocket session ID (server cấp lúc handshake) |
| `type` | string | Luôn là `"mcp"` |
| `payload.tool` | string | Tên tool: `mcp_news_tech`, `mcp_news_startup`, `mcp_news_science` |
| `payload.args.limit` | integer | Số lượng tin muốn lấy (mặc định 10) |

### **Example từ log thực tế:**

```
I (153729) StateMachine: State: idle -> connecting
I (153729) Application: MCP button [1/3] → công nghệ
I (153729) Application: Sending MCP tool: mcp_news_tech
  ↓
→ WebSocket sends: {"session_id":"...","type":"mcp",
    "payload":{"tool":"mcp_news_tech","args":{"limit":10}}}
```

---

## 2. Server Response Protocol (TTS)

Sau khi nhận MCP message, server **PHẢI** trả lời theo protocol sau:

### **Step 1: `tts:start`** (JSON text frame)

```json
{"type":"tts","state":"start"}
```

Server báo hiệu bắt đầu phát TTS. Device chuyển từ `Connecting` → `Speaking`.

### **Step 2: Binary Opus frames** (WebSocket binary frames)

```
[Opus frame 1] → send_bytes()
[Opus frame 2] → send_bytes()
[Opus frame 3] → send_bytes()
...
```

**Audio Specifications (BẮT BUỘC):**
| Parameter | Value |
|-----------|-------|
| Format | **Binary** (NOT text/JSON) |
| Codec | **Opus** |
| Sample Rate | **24000 Hz** (MUST match device output) |
| Channels | **1** (Mono) |
| Frame Duration | **60ms** (1440 samples at 24kHz) |
| Bit Rate | 24-32 kbps |
| Encoder Mode | `APPLICATION_VOIP` |

> ⚠️ **Important:** Sample rate phải là **24000Hz**. Nếu server gửi 16000Hz, device sẽ log warning: `Server sample rate 16000 does not match device output sample rate 24000` và chất lượng âm thanh giảm do resampling.

### **Step 3: `tts:stop`** (JSON text frame)

```json
{"type":"tts","state":"stop"}
```

Server báo hiệu kết thúc TTS. Device chuyển từ `Speaking` → `Idle`, sẵn sàng cho button press tiếp theo.

### **Optional: `tts:sentence_start`** (JSON text frame)

```json
{"type":"tts","state":"sentence_start","text":"Đây là tin thứ nhất..."}
```

Server có thể gửi message này để device hiển thị text subtitle trên màn hình.

---

## 3. Complete Server Implementation (Python)

```python
import json
import asyncio
import feedparser
import opuslib
import io
from pydub import AudioSegment
import edge_tts

# ==============================
# WebSocket Message Handler
# ==============================

async def handle_message(websocket, message):
    """Main WebSocket message handler"""
    data = json.loads(message)
    
    # --- MCP Tool Call (from device) ---
    if data.get("type") == "mcp":
        payload = data["payload"]
        tool_name = payload["tool"]
        args = payload.get("args", {})
        limit = args.get("limit", 10)
        
        if tool_name == "mcp_news_tech":
            await execute_news(websocket, "công nghệ", limit)
        elif tool_name == "mcp_news_startup":
            await execute_news(websocket, "khởi nghiệp", limit)
        elif tool_name == "mcp_news_science":
            await execute_news(websocket, "khoa học", limit)
        else:
            print(f"Unknown tool: {tool_name}")
    
    # --- TTS Message (from server AI) ---
    # Khi server AI muốn gửi TTS, dùng send_tts()

# ==============================
# News Tool Execution
# ==============================

async def execute_news(websocket, category, limit):
    """
    Execute news tool: fetch RSS → format → TTS → send audio
    Called when device sends MCP message (button press)
    """
    print(f"[News] Fetching {limit} items for: {category}")
    
    try:
        # 1. Fetch RSS
        items = await fetch_rss(category, limit)
        
        # 2. Format TTS text
        text = format_tts_text(items, category)
        
        # 3. Generate TTS audio
        print(f"[News] Generating TTS ({len(text)} chars)...")
        audio_mp3 = await generate_tts_edge(text)
        
        # 4. Convert MP3 → PCM 24000Hz → Opus frames
        pcm_data = decode_to_pcm(audio_mp3, target_rate=24000)
        opus_frames = encode_to_opus(pcm_data, sample_rate=24000)
        
        # 5. Send via TTS protocol
        print(f"[News] Sending {len(opus_frames)} Opus frames...")
        await send_tts(websocket, opus_frames)
        print(f"[News] ✅ Audio sent successfully")
        
    except Exception as e:
        print(f"[News] ❌ Error: {e}")

# ==============================
# RSS Fetching
# ==============================

def get_rss_url(category):
    """Map category to DanTri RSS feed URL"""
    urls = {
        "công nghệ":   "https://dantri.com.vn/rss/suc-manh-so.rss",
        "khởi nghiệp": "https://dantri.com.vn/rss/kinh-doanh.rss",
        "khoa học":    "https://dantri.com.vn/rss/khoa-hoc-cong-nghe.rss",
    }
    return urls.get(category, urls["công nghệ"])

async def fetch_rss(category, limit=10):
    """Fetch news items from RSS feed"""
    url = get_rss_url(category)
    feed = feedparser.parse(url)
    return feed.entries[:limit]

# ==============================
# TTS Text Formatting
# ==============================

def format_tts_text(items, category):
    """Format news items into natural Vietnamese TTS text"""
    text = f"Dưới đây là {len(items)} tin {category} mới nhất."
    for i, item in enumerate(items, 1):
        text += f"Tin thứ {i}: {item.title}."
    text += "Hết danh sách tin. Cảm ơn đã lắng nghe."
    return text

# ==============================
# TTS Audio Generation
# ==============================

async def generate_tts_edge(text):
    """Generate TTS using Edge TTS (free, Vietnamese voice)"""
    communicate = edge_tts.Communicate(
        text, 
        "vi-VN-HoaiMyNeural",
        rate="-10%"  # Slightly slower for clarity
    )
    audio = b""
    async for chunk in communicate.stream():
        if chunk["type"] == "audio":
            audio += chunk["data"]
    return audio

# ==============================
# Audio Processing
# ==============================

def decode_to_pcm(mp3_data, target_rate=24000):
    """
    Decode MP3 to PCM raw bytes
    
    Args:
        mp3_data: MP3 bytes from Edge TTS
        target_rate: Target sample rate (24000 for ESP32)
    
    Returns:
        Raw PCM bytes (16-bit signed, mono)
    """
    audio = AudioSegment.from_file(io.BytesIO(mp3_data))
    audio = audio.set_channels(1)           # Mono
    audio = audio.set_frame_rate(target_rate)  # 24000Hz
    audio = audio.set_sample_width(2)       # 16-bit
    return audio.raw_data

def encode_to_opus(pcm_data, sample_rate=24000, frame_duration=60):
    """
    Encode PCM to Opus frames
    
    Each frame = 60ms of audio at target sample rate
    
    Args:
        pcm_data: Raw PCM bytes (16-bit, mono)
        sample_rate: Sample rate (24000)
        frame_duration: Frame size in ms (60)
    
    Returns:
        List of Opus-encoded byte frames
    """
    encoder = opuslib.Encoder(sample_rate, 1, opuslib.APPLICATION_VOIP)
    
    # Frame size calculation
    frame_size = int(sample_rate * frame_duration / 1000)  # 1440 @ 24kHz
    frame_bytes = frame_size * 2  # 2880 bytes per frame
    
    frames = []
    for i in range(0, len(pcm_data), frame_bytes):
        frame = pcm_data[i:i + frame_bytes]
        
        # Pad last frame if needed
        if len(frame) < frame_bytes:
            frame += b'\x00' * (frame_bytes - len(frame))
        
        opus_frame = encoder.encode(frame, frame_size)
        frames.append(opus_frame)
    
    return frames

# ==============================
# TTS Protocol (Send Audio to ESP32)
# ==============================

async def send_tts(websocket, opus_frames, frame_delay=0.06):
    """
    Send TTS audio to ESP32 via TTS protocol
    
    Protocol:
        1. {"type":"tts","state":"start"}  (JSON text)
        2. [Opus frame 1] send_bytes()     (Binary)
        3. [Opus frame 2] send_bytes()     (Binary)
        4. ...
        5. {"type":"tts","state":"stop"}   (JSON text)
    """
    # Step 1: Start TTS
    await websocket.send_json({"type":"tts","state":"start"})
    await asyncio.sleep(0.2)  # Allow device to prepare
    
    # Step 2: Send audio frames
    for frame in opus_frames:
        await websocket.send_bytes(frame)
        await asyncio.sleep(frame_delay)  # 60ms between frames
    
    # Step 3: Stop TTS
    await websocket.send_json({"type":"tts","state":"stop"})

# ==============================
# Quick Test Echo Tool
# ==============================

async def handle_echo(websocket, text="Xin chào từ server"):
    """Simple echo tool to test TTS pipeline"""
    audio = await generate_tts_edge(text)
    pcm = decode_to_pcm(audio, target_rate=24000)
    opus = encode_to_opus(pcm, sample_rate=24000)
    await send_tts(websocket, opus)
```

---

## 4. Server Response State Machine

```
[Device Button Press]
  ↓
Device: Send MCP message  →  {"type":"mcp","payload":{"tool":"mcp_news_tech","args":{"limit":10}}}
  ↓
[Server receives MCP]
  ↓
Server: Execute tool (fetch RSS, format TTS)
  ↓
Server: Send {"type":"tts","state":"start"}  →  Device: Connecting → Speaking
  ↓
Server: Send [binary Opus frames]            →  Device: Play audio
  ↓
Server: Send {"type":"tts","state":"stop"}   →  Device: Speaking → Idle
  ↓
[Ready for next button press]
```

---

## 5. Common Mistakes ❌ vs ✅

### ❌ **Gửi text content thay vì audio**

```python
# ❌ WRONG: Gửi text news lên WebSocket
await websocket.send("Dưới đây là 10 tin công nghệ...")
```

**Kết quả:** Device chuyển sang Listening state (tưởng user nói), không phát âm thanh.
**Log:** `StateMachine: State: connecting -> listening`
**Log:** `Application: << Dưới đây là 10 tin... >>`

### ✅ **Đúng: Gửi TTS protocol với binary audio**

```python
# ✅ CORRECT: Gửi TTS protocol
opus_frames = text_to_opus_24000("Dưới đây là 10 tin...")
await send_tts(websocket, opus_frames)
```

### ❌ **Sample rate 16000Hz (không match device)**

```python
# ❌ WRONG: Server gửi audio 16000Hz
# Log: "Server sample rate 16000 does not match device output sample rate 24000"
```

### ✅ **Sample rate 24000Hz (match device output)**

```python
# ✅ CORRECT: Server gửi audio 24000Hz
audio = audio.set_frame_rate(24000)
opus_frames = encode_to_opus(pcm_data, sample_rate=24000)
```

### ❌ **Gửi mixed text + audio**

```python
# ❌ WRONG: Vừa gửi text content vừa gửi audio
await websocket.send_json({"type":"stt","text":"..."})
await websocket.send_bytes(opus_frame)
```

### ✅ **Chỉ gửi audio, không gửi text**

```python
# ✅ CORRECT: Chỉ gửi TTS protocol
await send_tts(websocket, opus_frames)
```

---

## 6. Dependencies (Python)

```bash
# Core
pip install websockets     # WebSocket server
pip install opuslib        # Opus encoding
pip install edge-tts       # Vietnamese TTS (free)
pip install feedparser     # RSS feed parsing
pip install pydub          # Audio conversion

# Optional
pip install google-cloud-texttospeech  # Google TTS
pip install azure-cognitiveservices-speech  # Azure TTS

# FFmpeg (required by pydub for MP3 decoding)
# Download: https://ffmpeg.org/download.html
# Add to PATH
```

---

## 7. Testing Checklist

### **Server-side test:**
- [ ] Server nhận được MCP message từ ESP32 (log: `"Received MCP: mcp_news_tech"`)
- [ ] Tool `mcp_news_*` được execute thành công
- [ ] RSS feed fetch được tin tức
- [ ] TTS text format đúng (Vietnamese, ngắn gọn)
- [ ] Edge TTS generate audio thành công
- [ ] MP3 → PCM 24000Hz conversion OK
- [ ] PCM → Opus encoding OK (1440 samples/frame)
- [ ] TTS protocol: `tts:start` → binary frames → `tts:stop`

### **Device-side verification (serial monitor):**
- [ ] `MCP button [1/3] → công nghệ` (khi nhấn click)
- [ ] `MCP button [2/3] → startup / khởi nghiệp` (khi double click)
- [ ] `MCP button [3/3] → khoa học` (khi triple click)
- [ ] `StateMachine: State: idle -> connecting` (bắt đầu kết nối)
- [ ] `StateMachine: State: connecting -> speaking` (khi nhận tts:start)
- [ ] Không có log `State: connecting -> listening` (lỗi server gửi text)
- [ ] Không có warning `Server sample rate 16000 does not match...` (lỗi sample rate)
- [ ] `StateMachine: State: speaking -> idle` (khi nhận tts:stop, sẵn sàng press tiếp)

---

## 8. Troubleshooting

### **No audio playback**

1. **Check server sends binary audio** — NOT text JSON
   - Log pattern: `State: connecting -> listening` → server gửi text thay vì audio
   - Fix: Use `send_tts()` function above

2. **Check sample rate** — MUST be 24000Hz
   - Log: `Server sample rate 16000 does not match...` 
   - Fix: Set audio sample rate to 24000 before Opus encoding

3. **Check Opus frame size** — MUST be 60ms
   - At 24000Hz: frame_size = 1440 samples, frame_bytes = 2880
   - At 16000Hz: frame_size = 960 samples, frame_bytes = 1920

### **Button not working**

- Check device log for `MCP button [N/3] → ...`
- If no log: button handler not triggered (hardware issue)
- If log appears: MCP message sent, check server receives it

### **Audio cuts off**

- Server sends `tts:stop` too early — wait for all Opus frames to send first
- Network latency — reduce frame_delay from 0.06 to 0.04
- Audio too long — limit to 10 items or truncate titles

---

## 9. Architecture Diagram (Complete Flow)

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32 Device                             │
│                                                             │
│  Button Click ──► HandleButtonClick()                       │
│  Button DblClk ──► HandleButtonDoubleClick()                │
│  Button TrpClk ──► HandleButtonTripleClick()                │
│                      │                                      │
│                      ▼                                      │
│               SendMCPTool(tool_name)                        │
│                      │                                      │
│               SetDeviceState(Connecting)                    │
│                      │                                      │
│               OpenAudioChannel() (WebSocket)                │
│                      │                                      │
│               SendMcpMessage(payload) ──────► WebSocket ──┐ │
│                                                             │
└─────────────────────────────────────────────────────────────┘
                                                               │
                                                               ▼
┌─────────────────────────────────────────────────────────────┐
│                      Server                                 │
│                                                             │
│  WebSocket ◄────── Receive MCP message                      │
│                      │                                      │
│               Parse payload.tool                             │
│                      │                                      │
│         ┌────────────┼────────────┐                          │
│         ▼            ▼            ▼                          │
│  mcp_news_tech  mcp_news_startup  mcp_news_science          │
│         │            │            │                          │
│         ▼            ▼            ▼                          │
│         └────────────┼────────────┘                          │
│                      ▼                                      │
│               fetch_rss(category, limit)                     │
│                      │                                      │
│               format_tts_text(items)                        │
│                      │                                      │
│               generate_tts_edge(text) → MP3                 │
│                      │                                      │
│               decode_to_pcm(mp3, 24000Hz)                   │
│                      │                                      │
│               encode_to_opus(pcm) → Opus frames             │
│                      │                                      │
│  WebSocket ──── send_tts(websocket, opus_frames)            │
│                  ├── {"type":"tts","state":"start"} (JSON)   │
│                  ├── [Opus frame 1] (binary)                │
│                  ├── [Opus frame 2] (binary)                │
│                  ├── ...                                     │
│                  └── {"type":"tts","state":"stop"} (JSON)   │
│                                                             │
└─────────────────────────────────────────────────────────────┘
                                                               │
                                                               ▼
┌─────────────────────────────────────────────────────────────┐
│                    ESP32 Device (Audio Playback)             │
│                                                             │
│  tts:start ──► SetDeviceState(Speaking)                     │
│                                                             │
│  Opus frame ──► audio_service_.PushPacketToDecodeQueue()    │
│                      │                                      │
│               Opus decoder → PCM 24000Hz                    │
│                      │                                      │
│               AudioOutputTask → I2S → ES8311 Codec          │
│                      │                                      │
│               Speaker 🔊                                     │
│                                                             │
│  tts:stop ──► SetDeviceState(Idle)                          │
│                                                             │
│  (Ready for next button press)                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 10. References

- Firmware source: [main/application.cc](../main/application.cc)
- Button handlers: [main/boards/lichuang-dev/lichuang_dev_board.cc](../main/boards/lichuang-dev/lichuang_dev_board.cc)
- WebSocket protocol: [docs/websocket.md](./websocket.md)
- AI prompt guide: [docs/server-ai-prompt.md](./server-ai-prompt.md)

---

**Last Updated:** 2026-07-14  
**Status:** Production Ready  
**Target:** Server-side implementation for ESP32 News Aggregator
