#!/bin/bash

# ألوان للإخراج
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}🔄 التأكد من وجود ملفات البناء...${NC}"

if [ ! -f "build/master_hive.elf" ]; then
    echo -e "${YELLOW}⚠️ ملف ELF غير موجود. جاري البناء أولاً...${NC}"
    ./rebuild.sh
fi

echo -e "${GREEN}✅ الملفات جاهزة.${NC}"
echo -e "${YELLOW}📌 افتح diagram.json في VS Code ثم اضغط F1 → Wokwi: Start Simulation${NC}"
echo ""
echo -e "${GREEN}أو استخدم الرابط المباشر:${NC}"
echo "https://wokwi.com/projects/new?template=esp32-s3-devkitc-1"
