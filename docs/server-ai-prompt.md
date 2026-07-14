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
    "tool": "get_tech_news",
    "args": {
      "category": "công nghệ",
      "count": 10
    }
  }
}
```

**Fields:**
- `session_id`: WebSocket session identifier (string)
- `type`: Message type, luôn là `"mcp"` cho tool calls
- `payload`: JSON object chứa tool name và arguments
  - `tool`: Tên tool cần gọi (string)
  - `args`: Arguments object cho tool (object)

---

## 2. Available Tools

### **Tool: `get_tech_news`**

**Purpose:** Fetch technology news and read them via TTS

**Arguments:**
- `category` (string, required): News category
  - `"công nghệ"` - General technology news
  - `"startup công nghệ"` - Tech startup news
  - `"nghiên cứu khoa học"` - Science research news
- `count` (integer, required): Number of news items to fetch (default: 10)

**Behavior:**
1. Fetch news from RSS feeds hoặc news API based on category
2. Summarize news items thành format dễ đọc
3. Generate natural Vietnamese speech text
4. Convert text to speech (TTS)
5. Send TTS audio qua WebSocket

**Example response text format:**
```
Tin công nghệ mới nhất hôm nay:

Tin thứ nhất: Apple ra mắt iPhone 16 với chip A18 mới, tăng hiệu năng 30% so với thế hệ trước.

Tin thứ hai: Tesla công bố doanh thu quý 3 tăng 20%, đạt 25 tỷ USD.

Tin thứ ba: Google phát triển AI mới có khả năng viết code tự động.

Bạn muốn nghe thêm tin nào không?
```

---

## 3. Response Format

### **Audio Response (REQUIRED):**

Server PHẢI trả về **binary Opus audio frames** qua WebSocket:

**Specifications:**
- **Format:** Binary (NOT JSON)
- **Codec:** Opus
- **Sample Rate:** 16000 Hz (16 kHz)
- **Channels:** 1 (Mono)
- **Frame Duration:** 60ms per frame
- **Bit Rate:** 16-32 kbps (recommended)

**Encoding Flow:**
```
Text → TTS Engine → Raw PCM (16-bit, 16kHz, mono) → Opus Encoder → Binary Frames → WebSocket
```

**WebSocket Send:**
```python
# Python example
for opus_frame in audio_frames:
    await websocket.send_bytes(opus_frame)  # Binary send, NOT text
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
        sample_rate_hertz=16000
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

# Option 3: Using Edge TTS (Free)
import edge_tts
import asyncio

async def text_to_speech_edge(text):
    communicate = edge_tts.Communicate(text, "vi-VN-HoaiMyNeural")
    
    audio_data = b""
    async for chunk in communicate.stream():
        if chunk["type"] == "audio":
            audio_data += chunk["data"]
    
    return audio_data
```

### **Step 5: Encode to Opus**

```python
import opuslib

def encode_pcm_to_opus(pcm_data, sample_rate=16000, frame_duration=60):
    """
    Encode PCM audio to Opus format
    
    Args:
        pcm_data: Raw PCM bytes (16-bit, mono)
        sample_rate: Sample rate in Hz (16000)
        frame_duration: Frame duration in ms (60)
    
    Returns:
        List of Opus-encoded frames
    """
    encoder = opuslib.Encoder(sample_rate, 1, opuslib.APPLICATION_VOIP)
    
    # Calculate frame size in samples
    frame_size = int(sample_rate * frame_duration / 1000)
    
    # PCM data is 16-bit (2 bytes per sample)
    frame_bytes = frame_size * 2
    
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

### **Step 6: Send Audio to ESP32**

```python
async def send_audio_to_device(websocket, opus_frames):
    """Send Opus audio frames to ESP32 via WebSocket"""
    
    for frame in opus_frames:
        # IMPORTANT: Send as BINARY, not TEXT
        await websocket.send_bytes(frame)
        
        # Optional: Small delay to avoid overwhelming device
        await asyncio.sleep(0.05)  # 50ms between frames
    
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
    
    if tool_name == "get_tech_news":
        await execute_get_tech_news(websocket, tool_args)
    else:
        print(f"Unknown tool: {tool_name}")

async def execute_get_tech_news(websocket, args):
    """Execute get_tech_news tool and send TTS response"""
    
    category = args["category"]
    count = args["count"]
    
    print(f"Fetching {count} news items for category: {category}")
    
    # Step 1: Fetch news
    news_items = await fetch_news(category, count)
    
    # Step 2: Format text for TTS
    text = format_news_text(news_items, category)
    
    print(f"Generated TTS text ({len(text)} chars)")
    
    # Step 3: Generate TTS audio
    audio_data = await generate_tts(text)
    
    # Step 4: Encode to Opus
    opus_frames = encode_to_opus(audio_data)
    
    print(f"Encoded {len(opus_frames)} Opus frames")
    
    # Step 5: Send to device
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

Trước khi implement `get_tech_news`, test với tool đơn giản:

```python
async def handle_echo(websocket, args):
    """Simple echo tool for testing"""
    
    text = args.get("text", "Hello from server")
    
    # Generate TTS
    audio = await generate_tts(text)
    opus_frames = encode_to_opus(audio)
    
    # Send to device
    await send_audio_frames(websocket, opus_frames)
```

**Test message từ ESP32:**
```json
{
  "type": "mcp",
  "payload": {
    "tool": "echo",
    "args": {
      "text": "Xin chào từ server"
    }
  }
}
```

### **Test 2: Log Everything**

```python
async def handle_mcp_message(websocket, message):
    print(f"Received MCP message: {message}")
    
    try:
        data = json.loads(message)
        print(f"Parsed data: {data}")
        
        if data.get("type") == "mcp":
            payload = data["payload"]
            print(f"Tool: {payload['tool']}")
            print(f"Args: {payload['args']}")
            
            # Execute tool
            result = await execute_tool(websocket, payload)
            print(f"Tool execution result: {result}")
            
    except Exception as e:
        print(f"Error handling MCP message: {e}")
        import traceback
        traceback.print_exc()
```

### **Test 3: Verify Audio Format**

```python
# Save audio to file for inspection
with open("test_audio.opus", "wb") as f:
    for frame in opus_frames:
        f.write(frame)

# Verify with ffmpeg
# ffmpeg -i test_audio.opus -ar 16000 -ac 1 test_audio.wav
```

---

## 7. Common Issues & Solutions

### **Issue 1: ESP32 không phát âm thanh**

**Symptoms:**
- WebSocket connected
- MCP message received
- No audio playback

**Solutions:**
1. ✅ Verify audio format: **16kHz, mono, Opus**
2. ✅ Check binary send: Use `send_bytes()` NOT `send_text()`
3. ✅ Verify Opus encoding: Frame size = 960 samples (60ms at 16kHz)
4. ✅ Add delay between frames: 50ms recommended

### **Issue 2: Audio bị cắt/méo**

**Solutions:**
1. Check sample rate conversion: TTS output → 16kHz PCM
2. Verify Opus encoder settings: APPLICATION_VOIP, bitrate 16-32kbps
3. Add proper padding for last frame

### **Issue 3: Tool không được gọi**

**Solutions:**
1. Log incoming WebSocket messages
2. Verify JSON parsing
3. Check tool name matching (case-sensitive)
4. Ensure `type: "mcp"` is present

---

## 8. Quick Start Checklist

- [ ] Parse incoming MCP messages from ESP32
- [ ] Implement `get_tech_news` tool
- [ ] Fetch news from RSS/API
- [ ] Generate Vietnamese TTS text
- [ ] Convert text to speech (Edge TTS recommended)
- [ ] Encode PCM to Opus (16kHz, mono, 60ms frames)
- [ ] Send binary Opus frames via WebSocket
- [ ] Test with ESP32 device
- [ ] Verify audio playback
- [ ] Add error handling and logging

---

## 9. Sample Server Code (Full)

See complete implementation in: `server-mcp-implementation.py`

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
