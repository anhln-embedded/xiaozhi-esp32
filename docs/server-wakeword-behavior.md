# Server-Side Wakeword Behavior Configuration

## Overview

This document describes how to configure the **server-side LLM** to handle wake word events from xiaozhi ESP32 devices correctly.

## Problem Statement

When a xiaozhi device wakes up (via wake word detection or B0 button press), it sends a `hello` message to the server. The default LLM behavior might respond with generic greetings like "Xin chào", "Hello", or "wake up world", which wastes time and provides no value to the user.

## Expected Behavior

When the server receives a `hello` message from the device (indicating wake-up), the LLM should:

1. **Immediately call the `get_tech_news` tool** (or `get_next_news` if a queue exists)
2. **Read 1-2 news items** briefly and clearly via TTS
3. **End with a prompt**: "Bạn muốn nghe thêm tin nào không?" (Do you want to hear more news?)
4. **NEVER greet** with "Xin chào", "Chào bạn", "Hello", "wake up world", or similar pleasantries

## Implementation

### System Prompt Configuration

The server-side LLM system prompt should include these instructions:

```markdown
## WAKEWORD EVENT HANDLING (CRITICAL)

When you receive a hello message from the device (user woke up the device via wake word or button):

1. **DO NOT greet the user**
   - NEVER say: "Xin chào", "Chào bạn", "Hello", "Hi", "wake up world"
   - NO introductions or pleasantries

2. **Immediately provide value**
   - Call get_tech_news("công nghệ", 5) to fetch latest tech news
   - Or call get_next_news() if you have a news queue from previous session

3. **Deliver news concisely**
   - Read 1-2 news headlines/summaries
   - Keep each item brief (10-15 seconds maximum)
   - Use natural, conversational tone

4. **Prompt for continuation**
   - After reading news, ask: "Bạn muốn nghe thêm tin nào không?"
   - Wait for user response

5. **Handle follow-up**
   - If user says "tiếp theo", "kế tiếp", "next": call get_next_news()
   - If user asks a different question: switch to normal conversation mode
   - If no response after 5 seconds: return to idle
```

## Expected Flow

### Scenario 1: First Wake-up

```
[User says wake word or presses B0]
↓
ESP32: Sends {"type": "hello", ...} via WebSocket
↓
Server: Receives hello message
↓
LLM: Calls get_tech_news("công nghệ", 5)
↓
Server: Fetches RSS feeds
↓
LLM: "Tin mới nhất: Apple ra mắt iPhone 16 với chip A18 mới. 
      Tesla công bố doanh thu quý 3 tăng 20%. 
      Bạn muốn nghe thêm tin nào không?"
↓
TTS: Converts to speech and sends to device
```

### Scenario 2: User Asks for More

```
User: "Tiếp theo"
↓
LLM: Calls get_next_news()
↓
LLM: "Google phát triển AI mới có khả năng viết code. 
      Microsoft mua lại startup AI với giá 10 tỷ USD.
      Bạn muốn nghe thêm tin nào không?"
```

### Scenario 3: User Switches Topic

```
User: "Thời tiết hôm nay thế nào?"
↓
LLM: Switches to normal conversation mode, handles weather query
```

## Required Tools

The server must provide these tools to the LLM:

### 1. `get_tech_news`

Fetches latest technology news from RSS feeds.

**Parameters:**
- `category` (string): News category (e.g., "công nghệ", "khoa học")
- `count` (integer): Number of news items to fetch (default: 5)

**Returns:**
```json
{
  "news": [
    {
      "title": "Apple ra mắt iPhone 16",
      "summary": "Apple công bố iPhone 16 với chip A18 mới...",
      "source": "VnExpress",
      "published_at": "2026-07-13T10:30:00Z"
    },
    ...
  ]
}
```

### 2. `get_next_news`

Returns the next news item from the current queue.

**Parameters:** None

**Returns:**
```json
{
  "title": "Google phát triển AI mới",
  "summary": "Google công bố mô hình AI có khả năng...",
  "source": "Tuổi Trẻ",
  "published_at": "2026-07-13T09:15:00Z"
}
```

## WebSocket Protocol Integration

According to [websocket.md](./websocket.md), the device sends this hello message:

```json
{
  "type": "hello",
  "version": 1,
  "features": {
    "mcp": true,
    "aec": true
  },
  "transport": "websocket",
  "audio_params": {
    "format": "opus",
    "sample_rate": 16000,
    "channels": 1,
    "frame_duration": 60
  }
}
```

The server should:
1. Detect that this is an **initial connection** (new session, wake-up event)
2. Trigger the wakeword behavior in the LLM
3. NOT wait for user speech input before providing news

## Anti-patterns (What NOT to Do)

❌ **Wrong - Generic greeting:**
```
LLM: "Xin chào! Tôi có thể giúp gì cho bạn?"
→ Wastes time, provides no value
```

❌ **Wrong - Waiting for user input:**
```
LLM: [Waits for user to speak]
→ User doesn't know what to say, awkward silence
```

❌ **Wrong - Too verbose:**
```
LLM: "Chào buổi sáng! Hôm nay là ngày 13 tháng 7 năm 2026. 
      Tôi là trợ lý AI của bạn. Bạn muốn nghe tin tức không?"
→ Too long, wastes user's time
```

✅ **Correct - Immediate value:**
```
LLM: "Tin mới nhất: Apple ra mắt iPhone 16 với chip A18. 
      Tesla doanh thu Q3 tăng 20%. 
      Bạn muốn nghe thêm tin nào không?"
→ Provides value immediately, efficient
```

## Configuration Checklist

When setting up your xiaozhi server, ensure:

- [ ] System prompt includes wakeword handling instructions
- [ ] `get_tech_news` tool is implemented and connected
- [ ] `get_next_news` tool is implemented and connected
- [ ] LLM can detect hello messages and trigger wakeword behavior
- [ ] TTS is configured to speak Vietnamese naturally
- [ ] News queue is maintained across follow-up requests
- [ ] Timeout logic returns device to idle after no response

## Testing

Test the wakeword behavior by:

1. **Wake the device** (say wake word or press B0)
2. **Verify LLM response**:
   - ✓ No greeting ("Xin chào", etc.)
   - ✓ News is delivered immediately
   - ✓ 1-2 news items only
   - ✓ Ends with "Bạn muốn nghe thêm tin nào không?"
3. **Say "tiếp theo"**:
   - ✓ Next news item is delivered
   - ✓ Prompt repeats
4. **Say a different question**:
   - ✓ LLM switches to normal conversation mode

## Notes

- This behavior is **server-side only** - no changes needed in ESP32 firmware
- The ESP32 firmware in this repository handles wake word detection and sends the hello message
- The server implementation is outside the scope of this repository
- Adjust news sources, categories, and prompts based on your target audience

## Related Documentation

- [WebSocket Protocol](./websocket.md) - Device-server communication
- [MCP Protocol](./mcp-protocol.md) - Tool calling mechanism
- [MCP Usage](./mcp-usage.md) - How to implement tools

---

**Last Updated:** 2026-07-13  
**Status:** Production Ready  
**Applies to:** Server-side LLM configuration for xiaozhi devices
