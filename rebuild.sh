#!/bin/bash

# ألوان للإخراج
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🚀 بدء عملية إعادة بناء مشروع MASTER_HIVE${NC}"

# 1. تنظيف سريع
echo -e "${YELLOW}📁 الخطوة 1: تنظيف الملفات القديمة...${NC}"
rm -rf build flash_image.bin sdkconfig.old

# 2. تفعيل البيئة
echo -e "${YELLOW}🔧 الخطوة 2: تفعيل بيئة ESP-IDF...${NC}"
source /opt/esp/idf/export.sh

# 3. بناء المشروع
echo -e "${YELLOW}🔨 الخطوة 3: بناء المشروع...${NC}"
idf.py build

# 4. التحقق من نجاح البناء
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ البناء اكتمل بنجاح!${NC}"
    
    # عرض حجم الملفات
    if [ -f "build/master_hive.bin" ]; then
        SIZE=$(ls -lh build/master_hive.bin | awk '{print $5}')
        echo -e "${GREEN}📦 حجم الملف: $SIZE${NC}"
    fi
    
    if [ -f "build/master_hive.elf" ]; then
        echo -e "${GREEN}✅ ملف ELF موجود: build/master_hive.elf${NC}"
    fi
    
    if [ -f "build/flasher_args.json" ]; then
        echo -e "${GREEN}✅ ملف flasher_args.json موجود${NC}"
    fi
    
    echo ""
    echo -e "${BLUE}🎯 الخطوات التالية:${NC}"
    echo -e "${YELLOW}1. افتح ملف diagram.json في VS Code${NC}"
    echo -e "${YELLOW}2. اضغط F1 → Wokwi: Start Simulation${NC}"
    echo -e "${YELLOW}3. أو استخدم idf.py flash للحرق على جهاز حقيقي${NC}"
else
    echo -e "${RED}❌ فشل البناء! تحقق من الأخطاء أعلاه.${NC}"
    exit 1
fi
