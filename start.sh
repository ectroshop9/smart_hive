#!/bin/bash
export IDF_PATH="$HOME/esp-idf"
export IDF_TOOLS_PATH="$HOME/.espressif"
export IDF_PYTHON_ENV_PATH="$HOME/.espressif/python_env/idf5.3_py3.12_env"

# تفعيل البيئة
source $IDF_PYTHON_ENV_PATH/bin/activate

# حقن المسارات (المترجم + الأدوات)
export PATH="$IDF_PYTHON_ENV_PATH/bin:$IDF_TOOLS_PATH/tools/xtensa-esp-elf/esp-13.2.0_20240530/xtensa-esp-elf/bin:$IDF_PATH/tools:$PATH"

# السطر السحري الذي يمنع خطأ "Import Module"
export PYTHONPATH="$IDF_PATH/tools:$IDF_PATH/tools/esp_idf_monitor:$PYTHONPATH"

# تجاوز الفحوصات
export IDF_SKIP_CHECK=1
df -h . | awk 'NR==2 {print "💾 المساحة المتاحة في القرص: " $4}'
echo "✅ البيئة جاهزة للعمل الآن!"