import base64
import re

png_path = r'C:\Users\hnaco\Downloads\logoptit.png'
html_path = r'f:\xiaozhi-esp32\managed_components\78__esp-wifi-connect\assets\wifi_configuration.html'

with open(png_path, 'rb') as f:
    b64_data = base64.b64encode(f.read()).decode('utf-8')

img_tag = f'<img class="logo-svg" src="data:image/png;base64,{b64_data}" alt="PTIT Logo">'

with open(html_path, 'r', encoding='utf-8') as f:
    html_content = f.read()

# Replace the SVG block
svg_pattern = re.compile(r'<svg class="logo-svg".*?</svg>', re.DOTALL)
html_content = svg_pattern.sub(img_tag, html_content)

with open(html_path, 'w', encoding='utf-8') as f:
    f.write(html_content)

print("Logo replaced successfully.")
