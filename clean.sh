#!/bin/bash

# ألوان للإخراج
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}🧹 بدء تنظيف المشروع...${NC}"

# حذف مجلد البناء
if [ -d "build" ]; then
    echo -e "${GREEN}✓ حذف مجلد build${NC}"
    rm -rf build
fi

# حذف صورة الفلاش
if [ -f "flash_image.bin" ]; then
    echo -e "${GREEN}✓ حذف flash_image.bin${NC}"
    rm -f flash_image.bin
fi

# حذف ملفات مؤقتة
echo -e "${GREEN}✓ حذف الملفات المؤقتة${NC}"
rm -f sdkconfig.old
rm -rf components/*/build

# حذف ذاكرة التخزين المؤقت لـ CMake
rm -rf build_*/

echo -e "${GREEN}✅ التنظيف اكتمل!${NC}"
echo -e "${YELLOW}📌 استخدم ./rebuild.sh لإعادة البناء${NC}"
