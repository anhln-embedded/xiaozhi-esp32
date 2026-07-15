# Server AI System Prompt - ESP32 Xiaozhi Integration

## Overview

Bạn là AI assistant chạy trên server để xử lý requests từ ESP32 Xiaozhi device qua WebSocket. Device gửi MCP (Model Context Protocol) messages và bạn cần parse, execute tools, và trả về TTS audio.

---

## 1. MCP Protocol Format

### **Incoming Message từ ESP32:**

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
- `session_id`: WebSocket session identifier (string)
- `type`: Message type, luôn là `"mcp"` cho tool calls
- `payload`: JSON object chứa tool name và arguments
  - `tool`: Tên tool cần gọi (string) - **3 tools available**
  - `args`: Arguments object cho tool (object)

---

## 2. Available Tools

### **Tool 1: `mcp_news_tech`**

**Purpose:** Fetch technology news and read them via TTS

**Arguments:**
- `limit` (integer, optional, default: 10): Number of news items to fetch

**Button Press:** Lần 1 (cycle)

### **Tool 2: `mcp_news_startup`**

**Purpose:** Fetch startup/khởi nghiệp news and read them via TTS

**Arguments:**
- `limit` (integer, optional, default: 10): Number of news items to fetch

**Button Press:** Lần 2 (cycle)

### **Tool 3: `mcp_news_science`**

**Purpose:** Fetch science/khoa học research news and read them via TTS

**Arguments:**
- `limit` (integer, optional, default: 10): Number of news items to fetch

**Button Press:** Lần 3 (cycle)

**Behavior (ALL tools):**
1. Fetch news from RSS feeds (DanTri) based on tool type
2. Summarize news items thành format dễ đọc (max 1-2 câu mỗi tin)
3. Format thành Vietnamese speech text NGẮN GỌN
4. **Convert text to speech (TTS) - BẮT BUỘC**
5. **Send TTS audio qua WebSocket (BINARY, not text)**
6. **DO NOT send news text content as JSON - chỉ gửi audio**

**Example TTS text (internal use only, không gửi lên WebSocket):**
```
Dưới đây là 10 tin công nghệ mới nhất.
Tin thứ 1: Apple ra mắt iPhone 16 với chip A18 mới, tăng hiệu năng 30%.
Tin thứ 2: Tesla công bố doanh thu quý 3 tăng 20%.
Hết danh sách tin. Cảm ơn đã lắng nghe.
```

---

## 3. Response Protocol (CRITICAL)

### **TTS Audio Flow (BẮT BUỘC):**

Khi tool executed xong, server PHẢI gửi audio theo protocol sau, KHÔNG gửi text content:

```
[MCP Tool executed]
  ↓
[Fetch news → Format TTS text]
  ↓
[Generate TTS audio (Edge TTS / Google TTS)]
  ↓
[Encode PCM to Opus (24000Hz, mono, 60ms frames)]
  ↓
──────────────────────────────────────────────
  ↓
[Gửi JSON: {"type":"tts","state":"start"}]
  ↓
[Gửi Binary: Opus frame 1] ← send_bytes()
  ↓
[Gửi Binary: Opus frame 2]
  ↓
[Gửi Binary: Opus frame ...]
  ↓
[Gửi JSON: {"type":"tts","state":"stop"}]
  ↓
[Device chuyển Idle]
```

### **TTS Protocol Steps:**

**Step 1:** Gửi `tts:start` (JSON text)
```json
{"type":"tts","state":"start"}
```

**Step 2:** Gửi Opus audio frames (BINARY, not text)
```python
for frame in opus_frames:
    await websocket.send_bytes(frame)
    await asyncio.sleep(0.06)  # 60ms delay
```

**Step 3:** Gửi `tts:stop` (JSON text)
```json
{"type":"tts","state":"stop"}
```

### **Audio Specifications:**
- **Format:** Binary Opus frames (NOT text/JSON)
- **Sample Rate:** **24000 Hz (24 kHz)** - MUST match device output
- **Channels:** 1 (Mono)
- **Frame Duration:** 60ms per frame (1440 samples at 24kHz)
- **Bit Rate:** 24-32 kbps (recommended)
- **Encoder:** Opus (APPLICATION_VOIP)

### **🚫 WRONG - ĐỪNG LÀM THẾ NÀY:**
```python
# ❌ SAI: Gửi text content lên WebSocket
await websocket.send_json({"type":"text","content":"Tin thứ 1: ..."})

# ❌ SAI: Gửi news text trực tiếp
await websocket.send("Dưới đây là 10 tin công nghệ...")

# ❌ SAI: Gửi JSON mà không có audio
await websocket.send_json({"type":"listen","state":"detect","text":"..."})
```

### **✅ ĐÚNG - PHẢI LÀM THẾ NÀY:**
```python
# ✅ ĐÚNG: Gửi TTS audio protocol
await websocket.send_json({"type":"tts","state":"start"})
for frame in opus_frames:
    await websocket.send_bytes(frame)  # Binary audio
await websocket.send_json({"type":"tts","state":"stop"})
```

**Encoding Flow:**
```
Text → TTS Engine → Raw PCM (16-bit, 24000Hz, mono) → Opus Encoder → Binary Frames → WebSocket
```

---

## 4. Tool Implementation Guide

### **Step 1: Parse MCP Message**

```python
import json

# Receive WebSocket message
message = await websocket.receive_text()
data = json.loads(message)

# Check message type
if data.get("type") == "mcp":
    payload = data["payload"]
    tool_name = payload["tool"]
    tool_args = payload["args"]
    
    # Route to tool handler
    if tool_name == "get_tech_news":
        await handle_get_tech_news(websocket, tool_args)
```

### **Step 2: Fetch News**

```python
import feedparser

async def handle_get_tech_news(websocket, args):
    category = args["category"]
    count = args["count"]
    
    # Map category to RSS feed URLs
    rss_feeds = {
        "công nghệ": [
            "https://vnexpress.net/rss/so-hoa.rss",
            "https://techcrunch.com/feed/"
        ],
        "startup công nghệ": [
            "https://techcrunch.com/category/startups/feed/"
        ],
        "nghiên cứu khoa học": [
            "https://www.sciencedaily.com/rss/top.xml"
        ]
    }
    
    # Fetch news
    news_items = []
    for feed_url in rss_feeds.get(category, []):
        feed = feedparser.parse(feed_url)
        news_items.extend(feed.entries[:count])
    
    # Limit to requested count
    news_items = news_items[:count]
    
    return news_items
```

### **Step 3: Generate TTS Text**

```python
def format_news_for_tts(news_items, category):
    """Format news into natural Vietnamese speech text"""
    
    text = f"Tin {category} mới nhất hôm nay:\n\n"
    
    for i, item in enumerate(news_items, 1):
        title = item.get("title", "")
        summary = item.get("summary", "")[:200]  # Limit summary length
        
        text += f"Tin thứ {i}: {title}. "
        if summary:
            text += f"{summary}. "
        text += "\n\n"
    
    text += "Bạn muốn nghe thêm tin nào không?"
    
    return text
```

### **Step 4: Text-to-Speech (TTS)**

```python
# Option 1: Using Google Cloud TTS
from google.cloud import texttospeech

def text_to_speech_google(text):
    client = texttospeech.TextToSpeechClient()
    
    synthesis_input = texttospeech.SynthesisInput(text=text)
    
    voice = texttospeech.VoiceSelectionParams(
        language_code="vi-VN",
        ssml_gender=texttospeech.SsmlVoiceGender.FEMALE
    )
    
    audio_config = texttospeech.AudioConfig(
        audio_encoding=texttospeech.AudioEncoding.LINEAR16,
        sample_rate_hertz=24000  # MUST match device output rate
    )
    
    response = client.synthesize_speech(
        input=synthesis_input,
        voice=voice,
        audio_config=audio_config
    )
    
    return response.audio_content  # Raw PCM audio

# Option 2: Using Azure TTS
import azure.cognitiveservices.speech as speechsdk

def text_to_speech_azure(text):
    speech_config = speechsdk.SpeechConfig(
        subscription="YOUR_KEY",
        region="YOUR_REGION"
    )
    speech_config.speech_synthesis_language = "vi-VN"
    speech_config.speech_synthesis_voice_name = "vi-VN-HoaiMyNeural"
    
    audio_config = speechsdk.audio.AudioOutputConfig(use_default_speaker=False)
    synthesizer = speechsdk.SpeechSynthesizer(
        speech_config=speech_config,
        audio_config=audio_config
    )
    
    result = synthesizer.speak_text_async(text).get()
    return result.audio_data

# Option 3: Using Edge TTS (Free) - RECOMMENDED
import edge_tts
import asyncio

async def text_to_speech_edge(text):
    """Generate TTS at 24000Hz to match ESP32 output"""
    communicate = edge_tts.Communicate(text, "vi-VN-HoaiMyNeural")
    
    # Edge TTS returns 16000Hz by default, need resample to 24000Hz
    # Or use rate parameter to request higher quality
    communicate = edge_tts.Communicate(
        text, 
        "vi-VN-HoaiMyNeural",
        rate="-10%"  # Slightly slower for better clarity
    )
    
    audio_data = b""
    async for chunk in communicate.stream():
        if chunk["type"] == "audio":
            audio_data += chunk["data"]
    
    # Edge TTS returns MP3 at ~24000Hz. Convert to PCM 24000Hz for Opus encoding
    return audio_data  # Will be decoded to PCM 24000Hz before Opus encoding
```

### **Step 5: Resample Audio (IMPORTANT)**

Edge TTS returns MP3 at ~16000-24000Hz. Need to decode and ensure 24000Hz PCM:

```python
import io
from pydub import AudioSegment

def resample_to_24000(audio_data):
    """Resample audio to 24000Hz PCM 16-bit mono for ESP32 compatibility"""
    
    # Load audio (could be MP3 from Edge TTS)
    audio = AudioSegment.from_file(io.BytesIO(audio_data))
    
    # Convert to mono, 24000Hz, 16-bit PCM
    audio = audio.set_channels(1)
    audio = audio.set_frame_rate(24000)
    audio = audio.set_sample_width(2)  # 16-bit
    
    return audio.raw_data  # Raw PCM bytes
```

### **Step 6: Encode to Opus (24000Hz)**

```python
import opuslib

def encode_pcm_to_opus(pcm_data, sample_rate=24000, frame_duration=60):
    """
    Encode PCM audio to Opus format
    
    Args:
        pcm_data: Raw PCM bytes (16-bit, mono)
        sample_rate: Sample rate in Hz (24000 - MUST match device)
        frame_duration: Frame duration in ms (60)
    
    Returns:
        List of Opus-encoded frames
    """
    encoder = opuslib.Encoder(sample_rate, 1, opuslib.APPLICATION_VOIP)
    
    # Frame size: 60ms at 24000Hz = 1440 samples
    frame_size = int(sample_rate * frame_duration / 1000)  # 1440
    frame_bytes = frame_size * 2  # 2880 bytes per frame
    
    opus_frames = []
    
    for i in range(0, len(pcm_data), frame_bytes):
        frame = pcm_data[i:i + frame_bytes]
        
        # Pad last frame if needed
        if len(frame) < frame_bytes:
            frame += b'\x00' * (frame_bytes - len(frame))
        
        # Encode frame
        opus_frame = encoder.encode(frame, frame_size)
        opus_frames.append(opus_frame)
    
    return opus_frames
```

### **Step 7: Send Audio to ESP32 (TTS Protocol)**

```python
async def send_audio_to_device(websocket, opus_frames):
    """Send Opus audio frames to ESP32 via TTS protocol"""
    
    # Step 1: Send tts:start (JSON)
    await websocket.send_json({"type":"tts","state":"start"})
    await asyncio.sleep(0.2)  # Allow device to prepare
    
    # Step 2: Send audio frames (BINARY)
    for frame in opus_frames:
        await websocket.send_bytes(frame)
        await asyncio.sleep(0.06)  # 60ms delay (matches frame duration)
    
    # Step 3: Send tts:stop (JSON)
    await websocket.send_json({"type":"tts","state":"stop"})
    
    print(f"Sent {len(opus_frames)} audio frames to device")
```

---

## 5. Complete Implementation Example

```python
import json
import asyncio
import feedparser
import opuslib
from edge_tts import Communicate

async def handle_mcp_message(websocket, message):
    """Main handler for MCP messages from ESP32"""
    
    data = json.loads(message)
    
    if data.get("type") != "mcp":
        return
    
    payload = data["payload"]
    tool_name = payload["tool"]
    tool_args = payload["args"]
    
    if tool_name == "mcp_news_tech":
        await execute_news_tool(websocket, "công nghệ", tool_args.get("limit", 10))
    elif tool_name == "mcp_news_startup":
        await execute_news_tool(websocket, "khởi nghiệp", tool_args.get("limit", 10))
    elif tool_name == "mcp_news_science":
        await execute_news_tool(websocket, "khoa học", tool_args.get("limit", 10))
    else:
        print(f"Unknown tool: {tool_name}")

async def fetch_news(category, count):
    """Fetch news from DanTri RSS"""
    feeds = {
        "công nghệ": "https://dantri.com.vn/rss/suc-manh-so.rss",
        "khởi nghiệp": "https://dantri.com.vn/rss/kinh-doanh.rss",
        "khoa học": "https://dantri.com.vn/rss/khoa-hoc-cong-nghe.rss"
    }
    feed = feedparser.parse(feeds.get(category, feeds["công nghệ"]))
    return feed.entries[:count]

def format_news_text(items, category):
    """Format news into short TTS text"""
    text = f"Dưới đây là {len(items)} tin {category} mới nhất."
    for i, item in enumerate(items, 1):
        text += f"Tin thứ {i}: {item.title}."
    text += "Hết danh sách tin. Cảm ơn đã lắng nghe."
    return text

async def generate_tts(text):
    """Generate TTS using Edge TTS"""
    communicate = edge_tts.Communicate(text, "vi-VN-HoaiMyNeural")
    audio = b""
    async for chunk in communicate.stream():
        if chunk["type"] == "audio":
            audio += chunk["data"]
    return audio

def resample_to_pcm(audio_data, target_rate=24000):
    """Decode MP3 to PCM 24000Hz"""
    import io
    from pydub import AudioSegment
    audio = AudioSegment.from_file(io.BytesIO(audio_data))
    audio = audio.set_channels(1).set_frame_rate(target_rate).set_sample_width(2)
    return audio.raw_data

def encode_to_opus(pcm_data, sample_rate=24000, frame_duration=60):
    """Encode PCM to 60ms Opus frames"""
    import opuslib
    encoder = opuslib.Encoder(sample_rate, 1, opuslib.APPLICATION_VOIP)
    frame_size = int(sample_rate * frame_duration / 1000)
    frame_bytes = frame_size * 2
    
    frames = []
    for i in range(0, len(pcm_data), frame_bytes):
        frame = pcm_data[i:i + frame_bytes]
        if len(frame) < frame_bytes:
            frame += b'\x00' * (frame_bytes - len(frame))
        frames.append(encoder.encode(frame, frame_size))
    return frames

async def send_tts(websocket, opus_frames):
    """Send TTS audio via protocol: start → binary frames → stop"""
    await websocket.send_json({"type":"tts","state":"start"})
    await asyncio.sleep(0.2)
    for frame in opus_frames:
        await websocket.send_bytes(frame)
        await asyncio.sleep(0.06)
    await websocket.send_json({"type":"tts","state":"stop"})

async def execute_news_tool(websocket, category, limit):
    """Execute news tool: fetch → TTS → send audio"""
    print(f"Fetching {limit} {category} news...")
    
    items = await fetch_news(category, limit)
    text = format_news_text(items, category)
    
    audio_data = await generate_tts(text)
    pcm_data = resample_to_pcm(audio_data)
    opus_frames = encode_to_opus(pcm_data)
    
    print(f"Sending {len(opus_frames)} frames...")
    await send_tts(websocket, opus_frames)
    print("Audio sent successfully")
    await send_audio_frames(websocket, opus_frames)
    
    print("Audio sent successfully")

async def fetch_news(category, count):
    """Fetch news from RSS feeds"""
    
    feeds = {
        "công nghệ": ["https://vnexpress.net/rss/so-hoa.rss"],
        "startup công nghệ": ["https://techcrunch.com/category/startups/feed/"],
        "nghiên cứu khoa học": ["https://www.sciencedaily.com/rss/top.xml"]
    }
    
    news = []
    for url in feeds.get(category, []):
        feed = feedparser.parse(url)
        news.extend(feed.entries[:count])
    
    return news[:count]

def format_news_text(items, category):
    """Format news for natural speech"""
    
    text = f"Tin {category} mới nhất:\n\n"
    
    for i, item in enumerate(items, 1):
        text += f"Tin thứ {i}: {item.title}.\n"
    
    text += "\nBạn muốn nghe thêm tin không?"
    
    return text

async def generate_tts(text):
    """Generate TTS using Edge TTS (free)"""
    
    communicate = Communicate(text, "vi-VN-HoaiMyNeural")
    
    audio = b""
    async for chunk in communicate.stream():
        if chunk["type"] == "audio":
            audio += chunk["data"]
    
    return audio

def encode_to_opus(audio_data):
    """Encode audio to Opus format"""
    
    encoder = opuslib.Encoder(16000, 1, opuslib.APPLICATION_VOIP)
    
    frame_size = 960  # 60ms at 16kHz
    frame_bytes = frame_size * 2  # 16-bit samples
    
    frames = []
    for i in range(0, len(audio_data), frame_bytes):
        frame = audio_data[i:i + frame_bytes]
        
        if len(frame) < frame_bytes:
            frame += b'\x00' * (frame_bytes - len(frame))
        
        opus_frame = encoder.encode(frame, frame_size)
        frames.append(opus_frame)
    
    return frames

async def send_audio_frames(websocket, frames):
    """Send Opus frames to ESP32"""
    
    for frame in frames:
        await websocket.send_bytes(frame)
        await asyncio.sleep(0.05)
```

---

## 6. Testing & Debugging

### **Test 1: Echo Tool (Simple Test)**

Trước khi implement tool thật, test với echo tool:

```python
async def handle_echo(websocket, args):
    """Simple echo tool for testing"""
    text = args.get("text", "Xin chào từ server")
    
    # Generate TTS
    audio = await generate_tts(text)
    pcm_data = resample_to_pcm(audio)
    opus_frames = encode_to_opus(pcm_data)
    
    # Send via TTS protocol (start → audio → stop)
    await send_tts(websocket, opus_frames)
```

**Test message từ ESP32 (nếu đổi tạm tool):**
```json
{
  "type": "mcp",
  "payload": {
    "tool": "mcp_news_tech",
    "args": {"limit": 10}
  }
}
```

### **Test 2: Log Everything**

```python
async def handle_mcp_message(websocket, message):
    print(f"=== MCP RECEIVED: {message[:200]} ===")
    
    try:
        data = json.loads(message)
        tool_name = data.get("payload", {}).get("tool", "?")
        print(f"TOOL: {tool_name}")
        
        # Execute tool
        result = await execute_tool(websocket, data["payload"])
        print(f"✓ Tool {tool_name} executed")
            
    except Exception as e:
        print(f"✗ ERROR: {e}")
```

### **Test 3: Verify Audio Output**

```python
# Save audio to verify locally
with open("test_output.opus", "wb") as f:
    for frame in opus_frames:
        f.write(frame)

# Verify with ffmpeg (24000Hz, mono)
# ffmpeg -i test_output.opus -ar 24000 -ac 1 test_output.wav
```

---

## 7. Common Issues & Solutions

### **Issue 1: ESP32 không phát âm thanh (most common)**

**Symptoms:**
- WebSocket connected ✅
- MCP message received ✅  
- Server responded ✅
- No audio playback ❌

**Solutions:**
1. ✅ **Check TTS Protocol:** Phải gửi `tts:start` → binary frames → `tts:stop`. KHÔNG gửi text content
2. ✅ **Check sample rate:** Audio MUST be **24000Hz** (not 16000, not 44100)
3. ✅ **Check binary send:** Use `send_bytes()` NOT `send_text()` cho audio frames
4. ✅ **Verify Opus settings:** 60ms frames, mono, APPLICATION_VOIP
5. ✅ **Check delay:** 60ms between frames matches frame duration

**Quick debug:**
```python
# Log what you're sending
print(f"Sending tts:start")
await websocket.send_json({"type":"tts","state":"start"})
print(f"Sending {len(opus_frames)} binary frames at 24000Hz")
await send_audio_frames(websocket, opus_frames)
print(f"Sending tts:stop")
await websocket.send_json({"type":"tts","state":"stop"})
```

### **Issue 2: Server gửi text thay vì audio**

**Symptom:** Log shows `Application: << tin tức text >>` thay vì state transition

**Root cause:** Server gửi news content dạng text JSON thay vì encode thành Opus audio

**Fix:**
```python
# ❌ WRONG: Gửi text lên WebSocket
await websocket.send_json({"type":"list","state":"detect","text":"Tin thứ 1: ..."})

# ✅ RIGHT: Gửi TTS protocol với binary audio
audio_data = await generate_tts(text)        # TTS → audio
pcm_data = resample_to_pcm(audio_data)       # Decode → PCM 24000Hz
opus_frames = encode_to_opus(pcm_data)       # PCM → Opus frames
await websocket.send_json({"type":"tts","state":"start"})
for frame in opus_frames: await websocket.send_bytes(frame)
await websocket.send_json({"type":"tts","state":"stop"})
```

### **Issue 3: Sample rate mismatch (16000 vs 24000)**

**Symptom:** `W (xxx) Server sample rate 16000 does not match device output sample rate 24000`

**Fix:** Generate TTS audio at **24000Hz**, not 16000Hz
```python
# Edge TTS → decode → resample to 24000Hz
audio = await generate_tts(text)
pcm = resample_to_pcm(audio, target_rate=24000)  # force 24000Hz!
opus = encode_to_opus(pcm, sample_rate=24000)
```

### **Issue 4: Tool không được gọi**

**Solutions:**
1. Log incoming WebSocket messages
2. Verify JSON parsing: `data.get("type") == "mcp"`
3. **Check tool name:** Must be **`mcp_news_tech`**, **`mcp_news_startup`**, or **`mcp_news_science`**
4. Ensure arguments match: `{"limit": 10}`

---

## 8. Quick Start Checklist

- [ ] Parse incoming MCP messages (`type == "mcp"`)
- [ ] Implement 3 tools: `mcp_news_tech`, `mcp_news_startup`, `mcp_news_science`
- [ ] Fetch news from DanTri RSS based on tool
- [ ] Format TTS text (ngắn gọn, không dài dòng)
- [ ] Generate TTS audio at **24000Hz** (Edge TTS recommended)
- [ ] Decode MP3 → resample to **PCM 24000Hz stereo**
- [ ] Encode PCM **Opus (24000Hz, mono, 60ms frames)**
- [ ] Send **TTS protocol**: `tts:start` (JSON) → binary frames → `tts:stop` (JSON)
- [ ] **DO NOT** send text content as WebSocket message
- [ ] Test with ESP32 device → verify audio playback
- [ ] Add error handling and logging

---

## 9. Sample Server Code (Full)

Xem implementation hoàn chỉnh ở section 5. File mẫu: `server-mcp-implementation.py`

---

## References

- MCP Protocol: [docs/mcp-protocol.md](./mcp-protocol.md)
- WebSocket Protocol: [docs/websocket.md](./websocket.md)
- ESP32 Firmware: [main/application.cc](../main/application.cc)
- Edge TTS: https://github.com/rany2/edge-tts
- Opus Codec: https://opus-codec.org/

---

**Last Updated:** 2026-07-14  
**Status:** Production Ready  
**Target:** Server-side AI implementation for ESP32 Xiaozhi
